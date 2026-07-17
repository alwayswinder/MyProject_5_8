// SimpleFluidActor.cpp
// 使用 FluidNinjaLive 计算材质的最小 GPU 流体模拟实现

#include "SimpleFluidActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

ASimpleFluidActor::ASimpleFluidActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// 创建显示平面组件
	DisplayPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayPlane"));
	SetRootComponent(DisplayPlane);
	DisplayPlane->SetRelativeScale3D(FVector(5.0f, 5.0f, 1.0f));
}

// ---------------------------------------------------------------------------
// BeginPlay — 创建 RenderTarget 和动态材质实例
// ---------------------------------------------------------------------------
void ASimpleFluidActor::BeginPlay()
{
	Super::BeginPlay();

	// 检查材质引用是否完整
	if (!CollisionPainterDotMat || !AdvectionMat || !CompositeAndGradientMat ||
		!DivergenceMat || !PressureSolverMat || !DensityDisplayMat)
	{
		UE_LOG(LogTemp, Error, TEXT("[SimpleFluidActor] 缺少材质引用 —— 请在细节面板中全部指定。"));
		SetActorTickEnabled(false);
		return;
	}

	// ---- 创建所有 RenderTarget ----
	RT_DensityA   = CreateRT(TEXT("RT_DensityA"),   SimResolution);
	RT_DensityB   = CreateRT(TEXT("RT_DensityB"),   SimResolution);
	RT_VelocityA  = CreateRT(TEXT("RT_VelocityA"),  SimResolution);
	RT_VelocityB  = CreateRT(TEXT("RT_VelocityB"),  SimResolution);
	RT_Pressure   = CreateRT(TEXT("RT_Pressure"),   SimResolution);
	RT_Divergence = CreateRT(TEXT("RT_Divergence"), SimResolution);
	RT_Collision  = CreateRT(TEXT("RT_Collision"),  SimResolution);

	// 全部初始化为黑色
	ClearAllRTs();

	// ---- 创建动态材质实例（可写副本） ----
	MID_CollisionPainterDot   = UMaterialInstanceDynamic::Create(CollisionPainterDotMat,   this);
	MID_CollisionPainterOffset = CollisionPainterOffsetMat
		? UMaterialInstanceDynamic::Create(CollisionPainterOffsetMat, this)
		: nullptr;
	MID_Advection             = UMaterialInstanceDynamic::Create(AdvectionMat,             this);
	MID_CompositeAndGradient  = UMaterialInstanceDynamic::Create(CompositeAndGradientMat,  this);
	MID_Divergence            = UMaterialInstanceDynamic::Create(DivergenceMat,            this);
	MID_PressureSolver        = UMaterialInstanceDynamic::Create(PressureSolverMat,        this);
	MID_Display               = UMaterialInstanceDynamic::Create(DensityDisplayMat,        this);

	// 将动态显示材质应用到平面
	DisplayPlane->SetMaterial(0, MID_Display);

	UE_LOG(LogTemp, Log, TEXT("[SimpleFluidActor] 初始化完成 —— %dx%d, 平面尺寸 %.0f"), SimResolution, SimResolution, PlaneWorldSize);
}

// ---------------------------------------------------------------------------
// Tick — 完整的仿真循环
// ---------------------------------------------------------------------------
void ASimpleFluidActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime <= 0.0f) return;

	// ---- 1. 获取交互 Actor（玩家或自定义） ----
	AActor* Interactor = CustomInteractionActor
		? CustomInteractionActor
		: UGameplayStatics::GetPlayerPawn(this, 0);

	if (!Interactor) return;

	// 计算位置和速度
	FVector PlayerPos = Interactor->GetActorLocation();
	FVector PlayerVel = (PlayerPos - LastPlayerWorldPos) / DeltaTime;
	LastPlayerWorldPos = PlayerPos;

	// 转换到仿真 UV 空间
	FVector2D SimUV = WorldToSimUV(PlayerPos);
	FVector2D VeloEncoded = EncodeVelocity(PlayerVel);

	// ---- 2. 清空碰撞 RT ----
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Collision, FLinearColor::Black);

	// ---- 3. 碰撞绘制（玩家位置 → 碰撞 RT） ----
	StepCollisionPainter(SimUV, VeloEncoded, DeltaTime);

	// ---- 4. ★ 密度注入：将碰撞密度写入密度场 ----
	//     这是最关键的一步！否则密度 RT 永远是黑色。
	StepInjectDensity(SimUV, VeloEncoded);

	// ---- 5. 平流 —— 密度 ----
	StepAdvection(RT_DensityA, RT_DensityA, RT_DensityB, DeltaTime);
	SwapRT(RT_DensityA, RT_DensityB);

	// ---- 6. 平流 —— 速度（速度自平流） ----
	StepAdvection(RT_VelocityA, RT_VelocityA, RT_VelocityB, DeltaTime);
	SwapRT(RT_VelocityA, RT_VelocityB);

	// ---- 7. 外力合成（碰撞力注入速度场） ----
	StepCompositeGradient(DeltaTime);
	SwapRT(RT_VelocityA, RT_VelocityB);

	// ---- 8. 散度计算 ----
	StepDivergence();

	// ---- 9. 压力求解（迭代） ----
	StepPressureSolve();

	// ---- 10. 更新显示 ----
	UpdateDisplay();
}

// ---------------------------------------------------------------------------
// 创建 RenderTarget
// ---------------------------------------------------------------------------
UTextureRenderTarget2D* ASimpleFluidActor::CreateRT(FName Name, int32 Size)
{
	UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(this, Name);
	RT->RenderTargetFormat = RTF_RGBA16f;
	RT->ClearColor = FLinearColor::Black;
	RT->InitAutoFormat(Size, Size);
	RT->UpdateResourceImmediate(true);
	return RT;
}

// ---------------------------------------------------------------------------
// 交换两个 RT 指针（双缓冲）
// ---------------------------------------------------------------------------
void ASimpleFluidActor::SwapRT(UTextureRenderTarget2D*& A, UTextureRenderTarget2D*& B)
{
	UTextureRenderTarget2D* Tmp = A;
	A = B;
	B = Tmp;
}

// ---------------------------------------------------------------------------
// 世界坐标 → 仿真 UV (0~1)
// 假设平面中心在 Actor 位置，平面大小为 PlaneWorldSize
// ---------------------------------------------------------------------------
FVector2D ASimpleFluidActor::WorldToSimUV(FVector WorldPos) const
{
	FVector Origin = GetActorLocation();
	float Half = PlaneWorldSize * 0.5f;
	return FVector2D(
		(WorldPos.X - (Origin.X - Half)) / PlaneWorldSize,
		(WorldPos.Y - (Origin.Y - Half)) / PlaneWorldSize
	);
}

// ---------------------------------------------------------------------------
// 将世界速度编码到纹理友好的 0~1 范围
// FluidNinjaLive 材质约定：0.5 = 零速度
// ---------------------------------------------------------------------------
FVector2D ASimpleFluidActor::EncodeVelocity(FVector WorldVelocity) const
{
	return FVector2D(
		FMath::Clamp(WorldVelocity.X / MaxVelocity * 0.5f + 0.5f, 0.0f, 1.0f),
		FMath::Clamp(WorldVelocity.Y / MaxVelocity * 0.5f + 0.5f, 0.0f, 1.0f)
	);
}

// ---------------------------------------------------------------------------
// 清空所有仿真 RT
// ---------------------------------------------------------------------------
void ASimpleFluidActor::ClearAllRTs()
{
	auto Clear = [this](UTextureRenderTarget2D* RT) {
		if (RT) UKismetRenderingLibrary::ClearRenderTarget2D(this, RT, FLinearColor::Black);
	};
	Clear(RT_DensityA);
	Clear(RT_DensityB);
	Clear(RT_VelocityA);
	Clear(RT_VelocityB);
	Clear(RT_Pressure);
	Clear(RT_Divergence);
	Clear(RT_Collision);
}

// ---------------------------------------------------------------------------
// 步骤：碰撞绘制
// 将玩家位置/速度通过 CollisionPainter 材质绘制到碰撞 RT
// ---------------------------------------------------------------------------
void ASimpleFluidActor::StepCollisionPainter(FVector2D PlayerUV, FVector2D PlayerVelocityEncoded, float DeltaTime)
{
	if (!MID_CollisionPainterDot || !RT_Collision) return;

	// 设置画笔参数
	MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Position"), FLinearColor(PlayerUV.X, PlayerUV.Y, 0.0f, 1.0f));
	MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Velocity"), FLinearColor(PlayerVelocityEncoded.X, PlayerVelocityEncoded.Y, 0.0f, 1.0f));
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushSize"), BrushSize);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushStrength"), BrushStrength);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushHardness"), 0.5f);

	// 绘制到碰撞 RT
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Collision, MID_CollisionPainterDot);
}

// ---------------------------------------------------------------------------
// 步骤：密度注入
// 将碰撞画笔直接绘制到密度 RT 上，使玩家位置产生白色密度
// 这是确保密度场有内容可显示的关键步骤
// ---------------------------------------------------------------------------
void ASimpleFluidActor::StepInjectDensity(FVector2D PlayerUV, FVector2D PlayerVelocityEncoded)
{
	if (!MID_CollisionPainterDot || !RT_DensityA) return;

	// 复用碰撞画笔材质，直接画到密度 RT
	MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Position"), FLinearColor(PlayerUV.X, PlayerUV.Y, 0.0f, 1.0f));
	MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Velocity"), FLinearColor(PlayerVelocityEncoded.X, PlayerVelocityEncoded.Y, 0.0f, 1.0f));
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushSize"), BrushSize);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushStrength"), BrushStrength);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushHardness"), 0.5f);

	// 直接绘制到密度 RT（透明混合，叠加密度）
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_DensityA, MID_CollisionPainterDot);
}

// ---------------------------------------------------------------------------
// 步骤：平流（半拉格朗日）
//   沿着速度场反向采样，实现流体的"流动"
//
//   参数：
//     Src     – 要平流的场（密度或速度）
//     DstRead – 绑定到 "DensityBuffer" 参数的纹理
//     DstWrite– 写入的目标 RT
// ---------------------------------------------------------------------------
void ASimpleFluidActor::StepAdvection(UTextureRenderTarget2D* Src,
	UTextureRenderTarget2D*& DstRead,
	UTextureRenderTarget2D*& DstWrite,
	float DeltaTime)
{
	if (!MID_Advection || !Src || !DstWrite) return;

	MID_Advection->SetTextureParameterValue(TEXT("VelocityBuffer"), RT_VelocityA);
	MID_Advection->SetTextureParameterValue(TEXT("DensityBuffer"), Src);
	MID_Advection->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);
	MID_Advection->SetScalarParameterValue(TEXT("Dissipation"), Dissipation);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, DstWrite, MID_Advection);
}

// ---------------------------------------------------------------------------
// 步骤：外力合成与梯度
//   将碰撞 RT 中的速度注入到速度场中
// ---------------------------------------------------------------------------
void ASimpleFluidActor::StepCompositeGradient(float DeltaTime)
{
	if (!MID_CompositeAndGradient || !RT_VelocityA || !RT_Collision || !RT_VelocityB) return;

	MID_CompositeAndGradient->SetTextureParameterValue(TEXT("VelocityBuffer"), RT_VelocityA);
	MID_CompositeAndGradient->SetTextureParameterValue(TEXT("CollisionBuffer"), RT_Collision);
	MID_CompositeAndGradient->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_VelocityB, MID_CompositeAndGradient);
}

// ---------------------------------------------------------------------------
// 步骤：散度计算
//   计算速度场的散度，用于压力求解
// ---------------------------------------------------------------------------
void ASimpleFluidActor::StepDivergence()
{
	if (!MID_Divergence || !RT_VelocityA || !RT_Divergence) return;

	MID_Divergence->SetTextureParameterValue(TEXT("VelocityBuffer"), RT_VelocityA);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Divergence, MID_Divergence);
}

// ---------------------------------------------------------------------------
// 步骤：压力求解（迭代）
//   迭代求解压力泊松方程，使速度场满足不可压缩条件
// ---------------------------------------------------------------------------
void ASimpleFluidActor::StepPressureSolve()
{
	if (!MID_PressureSolver || !RT_Pressure || !RT_Divergence) return;

	// 每帧先清空压力缓冲区
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Pressure, FLinearColor::Black);

	// 迭代求解
	for (int32 i = 0; i < PressureIterations; i++)
	{
		MID_PressureSolver->SetTextureParameterValue(TEXT("PressureBuffer"), RT_Pressure);
		MID_PressureSolver->SetTextureParameterValue(TEXT("DivergenceBuffer"), RT_Divergence);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Pressure, MID_PressureSolver);
	}
}

// ---------------------------------------------------------------------------
// 更新显示 — 将密度 RT 绑定到平面材质
// ---------------------------------------------------------------------------
void ASimpleFluidActor::UpdateDisplay()
{
	if (MID_Display && RT_DensityA)
	{
		MID_Display->SetTextureParameterValue(TEXT("DensityBuffer"), RT_DensityA);
	}
}
