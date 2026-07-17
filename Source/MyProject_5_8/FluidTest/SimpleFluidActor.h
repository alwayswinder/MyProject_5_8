// SimpleFluidActor.h
// 使用 FluidNinjaLive GPU 计算材质的最小流体模拟 Actor
//
// 每帧仿真管线：
//   1. CollisionPainter  -- 将玩家位置绘制到碰撞 RT
//   2. 密度注入           -- 将碰撞密度写入密度场
//   3. Advection         -- 平流密度场和速度场
//   4. CompositeGradient -- 将碰撞力注入速度场
//   5. Divergence        -- 计算散度
//   6. PressureSolver    -- 迭代求解压力泊松方程
//   7. Display           -- 在平面上显示密度 RT

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleFluidActor.generated.h"

class UStaticMeshComponent;
class UTextureRenderTarget2D;
class UMaterialInstance;
class UMaterialInstanceDynamic;

UCLASS(Blueprintable)
class ASimpleFluidActor : public AActor
{
	GENERATED_BODY()

public:
	ASimpleFluidActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	// ---------------------------------------------------------
	// 组件
	// ---------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* DisplayPlane;

	// ---------------------------------------------------------
	// FluidNinjaLive 材质引用（在细节面板中指定）
	// ---------------------------------------------------------
	/** 碰撞画笔（点模式）- 将碰撞位置绘制到 RT */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Materials")
	UMaterialInstance* CollisionPainterDotMat;

	/** 碰撞偏移 - 将碰撞速度注入速度场 */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Materials")
	UMaterialInstance* CollisionPainterOffsetMat;

	/** 平流材质 - 半拉格朗日平流，移动密度/速度场 */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Materials")
	UMaterialInstance* AdvectionMat;

	/** 外力合成 - 将碰撞力叠加到速度场 */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Materials")
	UMaterialInstance* CompositeAndGradientMat;

	/** 散度计算材质 */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Materials")
	UMaterialInstance* DivergenceMat;

	/** 压力求解材质 - 迭代求解泊松方程 */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Materials")
	UMaterialInstance* PressureSolverMat;

	/** 显示材质 - 应用到 DisplayPlane 上，黑白显示密度缓冲区 */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Materials")
	UMaterialInstance* DensityDisplayMat;

	// ---------------------------------------------------------
	// 仿真参数
	// ---------------------------------------------------------
	/** 仿真分辨率（建议 64~1024） */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings", meta = (ClampMin = 64, ClampMax = 1024))
	int32 SimResolution = 256;

	/** 压力求解器迭代次数（越多越不可压缩，但越耗 GPU） */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings", meta = (ClampMin = 1, ClampMax = 16))
	int32 PressureIterations = 4;

	/** 碰撞画笔大小（UV 空间 0~1） */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings", meta = (ClampMin = 0.0f))
	float BrushSize = 0.05f;

	/** 碰撞画笔强度 */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings", meta = (ClampMin = 0.0f))
	float BrushStrength = 1.0f;

	/** 仿真平面的世界空间尺寸（用于 UV 映射） */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings")
	float PlaneWorldSize = 2000.0f;

	/** 最大速度（用于速度编码，值越大玩家移动产生的影响越小） */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings")
	float MaxVelocity = 500.0f;

	/** 密度消散率（每帧衰减，0.99 = 缓慢消散） */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings")
	float Dissipation = 0.99f;

	/** 自定义交互 Actor；为空时自动使用 GetPlayerPawn(0) */
	UPROPERTY(EditAnywhere, Category = "FluidSim|Settings")
	AActor* CustomInteractionActor = nullptr;

private:
	// ---------------------------------------------------------
	// 运行时 RenderTargets
	// ---------------------------------------------------------
	UPROPERTY()
	UTextureRenderTarget2D* RT_DensityA;      // 密度缓冲区 A
	UPROPERTY()
	UTextureRenderTarget2D* RT_DensityB;      // 密度缓冲区 B（双缓冲）
	UPROPERTY()
	UTextureRenderTarget2D* RT_VelocityA;     // 速度缓冲区 A
	UPROPERTY()
	UTextureRenderTarget2D* RT_VelocityB;     // 速度缓冲区 B（双缓冲）
	UPROPERTY()
	UTextureRenderTarget2D* RT_Pressure;      // 压力缓冲区
	UPROPERTY()
	UTextureRenderTarget2D* RT_Divergence;    // 散度缓冲区
	UPROPERTY()
	UTextureRenderTarget2D* RT_Collision;     // 碰撞输入缓冲区

	// ---------------------------------------------------------
	// 动态材质实例（可写的副本）
	// ---------------------------------------------------------
	UPROPERTY()
	UMaterialInstanceDynamic* MID_CollisionPainterDot;
	UPROPERTY()
	UMaterialInstanceDynamic* MID_CollisionPainterOffset;
	UPROPERTY()
	UMaterialInstanceDynamic* MID_Advection;
	UPROPERTY()
	UMaterialInstanceDynamic* MID_CompositeAndGradient;
	UPROPERTY()
	UMaterialInstanceDynamic* MID_Divergence;
	UPROPERTY()
	UMaterialInstanceDynamic* MID_PressureSolver;
	UPROPERTY()
	UMaterialInstanceDynamic* MID_Display;

	// ---------------------------------------------------------
	// 内部辅助函数
	// ---------------------------------------------------------
	/** 创建指定大小的 RenderTarget */
	UTextureRenderTarget2D* CreateRT(FName Name, int32 Size);
	/** 交换两个 RT 指针（双缓冲） */
	void SwapRT(UTextureRenderTarget2D*& A, UTextureRenderTarget2D*& B);
	/** 将世界坐标映射到仿真 UV 空间 (0~1) */
	FVector2D WorldToSimUV(FVector WorldPos) const;
	/** 将世界速度编码到纹理友好的 0~1 范围（0.5 = 零速度） */
	FVector2D EncodeVelocity(FVector WorldVelocity) const;
	/** 清空所有 RT */
	void ClearAllRTs();

	/** 步骤：碰撞绘制（将玩家位置绘制到碰撞 RT） */
	void StepCollisionPainter(FVector2D PlayerUV, FVector2D PlayerVelocityEncoded, float DeltaTime);
	/** 步骤：密度注入（将碰撞 RT 中的密度写入密度场） */
	void StepInjectDensity(FVector2D PlayerUV, FVector2D PlayerVelocityEncoded);
	/** 步骤：平流 */
	void StepAdvection(UTextureRenderTarget2D* Src, UTextureRenderTarget2D*& DstRead, UTextureRenderTarget2D*& DstWrite, float DeltaTime);
	/** 步骤：外力合成与梯度 */
	void StepCompositeGradient(float DeltaTime);
	/** 步骤：散度计算 */
	void StepDivergence();
	/** 步骤：压力求解（迭代） */
	void StepPressureSolve();
	/** 更新平面显示 */
	void UpdateDisplay();

	/** 记录上一帧玩家位置，用于计算速度 */
	FVector LastPlayerWorldPos = FVector::ZeroVector;
};
