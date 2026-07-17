# 小水池水交互物理模拟 — 复刻步骤指南

> **源关卡**: /Game/_MyTest/Island/Usecase_007_SmallWaterBodies_LIVE18  
> **插件**: FluidNinjaLive (NinjaLive)  
> **适用引擎**: UE 5.8  
> **文档版本**: v1.0 — 基于当前关卡实际配置整理

---

## 目录

1. [准备工作](#准备工作)
2. [步骤 1：设置基础环境（光照与大气）](#步骤-1设置基础环境光照与大气)
3. [步骤 2：创建地面与水池结构](#步骤-2创建地面与水池结构)
4. [步骤 3：放置流体仿真核心](#步骤-3放置流体仿真核心)
5. [步骤 4：配置碰撞与交互物体](#步骤-4配置碰撞与交互物体)
6. [步骤 5：设置灯光](#步骤-5设置灯光)
7. [步骤 6：后处理配置](#步骤-6后处理配置)
8. [步骤 7：角色配置](#步骤-7角色配置)
9. [步骤 8：最终检查与调整](#步骤-8最终检查与调整)
10. [关键原理总结](#关键原理总结)

---

## 准备工作

在开始之前，确保 Content Browser 中可以访问以下 FluidNinjaLive 资产目录：

| 路径 | 内容 |
|------|------|
| /Game/FluidNinjaLive/ | 主控制资产（NinjaLive Actor, Component, PresetManager 等） |
| /Game/FluidNinjaLive/Core/ | 核心资源（材质、Niagara 系统、纹理） |
| /Game/FluidNinjaLive/OutputMaterials/ | 输出材质（缓冲区渲染、效果材质） |
| /Game/FluidNinjaLive/UseCases/007_SmallWater/ | 小水体的专用材质（M_PoolTest1, MI_CheckerGreen18） |
| /Game/FluidNinjaLive/Tutorial/Meshes/ | 网格资源（平面、拱门等） |
| /Game/FluidNinjaLive/Tutorial/UE_Mannequin_UsageExamples/ | 第三人称角色蓝图 |

---

## 步骤 1：设置基础环境（光照与大气）

### 1.1 放置 DirectionalLight（定向光）

| 项目 | 内容 |
|------|------|
| **操作** | Modes 面板 > Lights > DirectionalLight，拖入关卡 |
| **目的** | 提供主光源方向，产生阴影，定义场景基调 |
| **调整** | 旋转角度模拟日光（如 Yaw=-45°, Pitch=-45°） |
| **作用原理** | 定向光的方向决定了水面高光反射的角度，影响水体视觉效果 |

### 1.2 放置 SkyLight（天光）

| 项目 | 内容 |
|------|------|
| **操作** | Modes > Lights > SkyLight，拖入关卡 |
| **目的** | 提供环境间接光照，照亮阴影区域 |
| **关键设置** | 勾选 Real Time Capture（实时捕获） |
| **作用原理** | 天光为水面提供环境反射，使水体不显得死黑 |

### 1.3 放置 Sky Atmosphere（天空大气）

| 项目 | 内容 |
|------|------|
| **操作** | Modes > Visual > Sky Atmosphere，拖入关卡 |
| **目的** | 模拟大气散射，产生真实的天空颜色和地平线渐变 |
| **作用原理** | 与 DirectionalLight 配合自动生效，影响水面反射的天空颜色 |

### 1.4 放置 ExponentialHeightFog（指数高度雾）

| 项目 | 内容 |
|------|------|
| **操作** | Modes > Visual > ExponentialHeightFog，拖入关卡 |
| **目的** | 添加距离雾效，增强场景纵深感 |
| **推荐值** | Fog Density = 0.02~0.05，Scattering Distribution = 0.2 |
| **作用原理** | 远处的水面/物体融合到背景中，增强真实感 |

---

## 步骤 2：创建地面与水池结构

### 2.1 放置地面（Floor）

| 项目 | 内容 |
|------|------|
| Actor 类别 | StaticMeshActor |
| 操作 | 从 Content Browser 拖入一个平面网格，或从 Modes > Geometry > Plane 创建 |
| 推荐网格 | /Game/FluidNinjaLive/Tutorial/Meshes/SM_plane_400x400 |
| 目的 | 作为场景的基础地面，角色行走的平台 |
| 材质 | 可赋予简单纯色或棋盘格材质作为临时地面 |
| 位置 | 将水池区域放在地面中央附近 |

### 2.2 放置 PoolBottom（水池底部）

| 项目 | 内容 |
|------|------|
| Actor 类别 | StaticMeshActor |
| 操作 | 拖入 /Engine/BasicShapes/Plane.Plane 到关卡 |
| 目的 | 定义水体的底部视觉边界，透过水面可以看到池底纹理 |
| 材质 | 赋予 /Game/FluidNinjaLive/UseCases/007_SmallWater/Misc/MI_CheckerGreen18 |
| 材质作用 | 绿色半透明棋盘格材质，模拟水池底部的瓷砖/砖纹效果 |
| 位置 | 放置在 NinjaLive Actor 下方，作为池底 |

### 2.3 放置 PoolBasePlane（水面承载平面）

| 项目 | 内容 |
|------|------|
| Actor 类别 | StaticMeshActor |
| 操作 | 拖入 /Engine/BasicShapes/Plane.Plane 到关卡 |
| 目的 | 核心视觉要素 -- 承载流体仿真输出的渲染平面，即实际显示水面的几何体 |
| 材质 | 赋予 /Game/FluidNinjaLive/UseCases/007_SmallWater/Misc/M_PoolTest1 |
| 材质作用 | 小水体的核心输出材质，接收流体仿真缓冲区数据，在平面上渲染出水波纹和涟漪效果 |
| 位置 | 放置在 PoolBottom 上方（Z 轴略高），形成水面与底部的空间层次 |

### 布局关系

```
                    +---------------------------+
                    |  PoolBasePlane (水面)     |  显示波纹
                    +---------------------------+
                    |  PoolBottom (池底)        |  透过水面可见
                    +---------------------------+
                    |  Floor (地面)             |  基础地面
                    +---------------------------+
```
---

## 步骤 3：放置流体仿真核心（最重要的一步）

### 3.1 放置 NinjaLive（主 Actor）

| 项目 | 内容 |
|------|------|
| 操作 | 从 Content Browser 拖入 /Game/FluidNinjaLive/NinjaLive 到关卡 |
| 目的 | 整个流体仿真的核心 -- 包含仿真引擎、碰撞检测、画笔系统等全部逻辑 |
| 蓝图类 | NinjaLive_C（基于 /Game/FluidNinjaLive/NinjaLive） |
| 重命名 | 建议改名为 "Pool" 便于识别 |

#### 3.1.1 组件结构

NinjaLive Actor 包含以下关键组件：

| 组件名 | 类型 | 作用 |
|--------|------|------|
| Root | SceneComponent | 场景根组件，定义 Actor 位置 |
| NinjaLiveComponent | NinjaLiveComponent | 仿真引擎核心 -- 管理 GPU 仿真计算、缓冲区、碰撞输入 |
| TraceMesh | NinjaLiveTraceMesh | 碰撞追踪网格 -- 检测角色/物体与流体的物理交互 |
| ActivationVolume | BoxComponent | 激活体积 -- 角色进入此范围时仿真自动启动/唤醒 |
| InteractionVolume | BoxComponent | 交互体积 -- 定义流体与物体发生物理交互的范围 |
| EditorIcon | BillboardComponent | 编辑器中显示的图标，运行时不可见 |

#### 3.1.2 关键配置参数

在细节面板中设置以下参数：

| 参数 | 当前关卡配置值 | 作用说明 |
|------|---------------|----------|
| resolutionX | 1600 | 仿真水平分辨率，越高细节越丰富但 GPU 消耗越大 |
| resolutionY | 1600 | 仿真垂直分辨率，与 X 一致表示正方形仿真域 |
| simPrecision | 16 bit | 仿真数值精度，16bit 适合大多数水体效果 |
| usePainter_V2_ToTrackObjects | True | 启用 Painter v2 系统追踪物体运动，是碰撞交互的核心开关 |
| preferredTraceChannelName | 项目自定义 | 定义检测碰撞的追踪通道，需与项目设置一致 |
| traceChannel (Fallback) | TraceTypeQuery3 (Visibility) | 备用追踪通道 |
| collisionChannel (Fallback) | ECC_GameTraceChannel1 (Projectile) | 备用碰撞通道，与本项目的 Projectile 通道一致 |
| autoConnectToMemoryPool | True | 自动连接 Memory Pool Manager 管理渲染目标内存 |
| lWCSupport | True | 大世界坐标支持，在 UE5 大世界中保持精度 |
| painter_V2_BrushVeloNoiseTexture | T_NoiseTemplate1 | 画笔速度噪声纹理，使笔触更自然 |

#### 3.1.3 体积范围配置

ActivationVolume（激活体积）：
- BoxExtent: (4000, 4000, 2500)
- 相对位置: (0, 0, 0)
- 作用：角色进入此范围时仿真自动激活，离开后暂停以节省 GPU 资源

InteractionVolume（交互体积）：
- BoxExtent: (1025, 1025, 5)
- 相对位置: (0, 0, 0)
- 作用：只有在此体积内的物体移动才会影响流体仿真，精确控制交互范围

### 3.2 放置 NinjaLive_Utilities（工具管理器）

| 项目 | 内容 |
|------|------|
| 操作 | 从 Content Browser 拖入 /Game/FluidNinjaLive/NinjaLive_Utilities |
| 目的 | 提供辅助功能：渲染目标管理、坐标转换、调试工具等 |
| 作用 | NinjaLive 系统运行所需的工具函数库，NinjaLiveComponent 会连接到此管理器获取内存池 |
| 组件 | Root (SceneComponent), EditorIcon (BillboardComponent) |

### 3.3 放置 NinjaLive_PresetManager（预设管理器）

| 项目 | 内容 |
|------|------|
| 操作 | 从 Content Browser 拖入 /Game/FluidNinjaLive/NinjaLive_PresetManager |
| 目的 | 管理流体仿真的预设配置，可以快速切换不同效果 |
| 组件 | Root, EditorIcon, WriteDataTableUtility（用于写入预设数据到 DataTable） |
| 作用 | 加载预设 DataTable 一键切换水的颜色、粘性、消散速度等参数 |

### 三者关系

```
NinjaLive_Utilities      提供内存池和工具函数
        |
        v
NinjaLive (Pool)         主仿真引擎
  +-- NinjaLiveComponent     仿真计算
  +-- TraceMesh              碰撞检测
  +-- ActivationVolume       激活范围
  +-- InteractionVolume      交互范围
        |
        v
NinjaLive_PresetManager  预设加载/切换
```
---

## 步骤 4：配置碰撞与交互物体

### 4.1 放置碰撞体（Cube / Cylinder / Sphere）

| 项目 | 内容 |
|------|------|
| 操作 | Modes > Geometry 拖入 Box / Cylinder / Sphere，或使用 StaticMesh 资产 |
| 目的 | 作为水池中的障碍物/装饰物，与流体产生物理交互 |
| 作用原理 | 当角色走过或物体移动时，TraceMesh 检测到碰撞，通过 Painter v2 系统在仿真网格上"绘制"速度和密度变化，产生水波荡漾的效果 |

推荐放置的碰撞物体类型：

| 物体类型 | 模拟效果 | 数量参考 |
|----------|----------|----------|
| 圆柱体 (Cylinder) | 模拟水中的石头/柱子，水绕流过产生尾迹 | 4-8 个 |
| 立方体 (Cube) | 模拟台阶/平台，水流冲击产生波纹 | 3-5 个 |
| 球体 (Sphere) | 模拟圆石/浮标，产生圆形扩散波纹 | 1-2 个 |

### 4.2 放置装饰性网格

| 项目 | 内容 |
|------|------|
| 操作 | 从 /Game/FluidNinjaLive/Tutorial/Meshes/ 拖入 SM_Arch1~5 等 |
| 目的 | 场景装饰，丰富视觉效果，不参与碰撞交互 |
| 作用 | 美化场景，增加视觉层次感 |

---

## 步骤 5：设置灯光

### 5.1 放置 PointLight（点光源，多个）

| 项目 | 内容 |
|------|------|
| 操作 | Modes > Lights > PointLight，在水池周围放置 3-4 个 |
| 目的 | 为水池区域提供局部照明，增强水体视觉效果 |
| 作用 | 照亮水面，使反射、折射效果更明显。水面材质的 Specular/Emissive 通道依赖光源产生高光 |

### 5.2 调整 LightSource（定向光）

| 项目 | 内容 |
|------|------|
| 操作 | 调整 DirectionalLight 的角度和强度 |
| 推荐值 | Intensity = 5~10 lux, Angle = 45 度俯角 |
| 作用 | 决定水面高光的方向和强度 |

---

## 步骤 6：后处理配置

### 6.1 放置 PostProcessVolume

| 项目 | 内容 |
|------|------|
| 操作 | Modes > Visual > PostProcessVolume，勾选 Infinite Extent (Unbound) |
| 目的 | 设置整体画面色调、对比度、曝光 |
| 推荐调整 | 适当降低曝光补偿使水体更通透，调整色调增加水下蓝调氛围 |

### 6.2 放置水下后处理（PostProcess_UnderWater）

| 项目 | 内容 |
|------|------|
| 操作 | 创建新的 PostProcessVolume，放置在水池区域 |
| 设置 | bUnbound = false（非无限范围）, Priority = 1 |
| 目的 | 当角色进入水下区域时切换视觉效果（模糊、偏蓝调） |
| 作用 | 此体积需要与 InteractionVolume 配合，角色进入水下时触发蓝色调/模糊效果，增强沉浸感 |
| 推荐效果 | 增加 Color Grading 的蓝色偏移，增加径向模糊 |

---

## 步骤 7：角色配置

### 7.1 放置第三人称角色

| 项目 | 内容 |
|------|------|
| 操作 | 从 /Game/FluidNinjaLive/Tutorial/UE_Mannequin_UsageExamples/ 拖入 ThirdPersonCharacter_EngineDefault |
| 目的 | 提供可操控角色，与水池产生交互 |
| 角色组件 | CollisionCylinder（碰撞胶囊体）, CharMoveComp（移动组件）, CameraBoom（相机弹簧臂）, FollowCamera（跟随相机）, CharacterMesh0（网格体） |
| 交互原理 | 角色走到水池中时，NinjaLiveTraceMesh 检测到角色碰撞体（CapsuleComponent），将其运动速度/位置信息传递给流体仿真，在水面上产生涟漪和波纹 |

### 7.2 交互工作流程

```
角色走入 InteractionVolume
        |
        v
ActivationVolume 检测到角色 -> 激活仿真引擎
        |
        v
TraceMesh 开始追踪角色碰撞体
        |
        v
Painter v2 系统将角色位置/速度写入仿真网格
        |
        v
GPU 求解器计算流体运动
        |
        v
M_PoolTest1 材质读取仿真缓冲区 -> 渲染水面波纹
```
---

## 步骤 8：最终检查与调整

### 8.1 关键连接检查清单

| 检查项 | 确认标准 | 若不满足会怎样 |
|--------|----------|----------------|
| NinjaLiveComponent 的 autoConnectToMemoryPool | = True | 仿真可能因内存分配失败而无法运行 |
| 关卡中有 NinjaLive_Utilities | 已放置 | Memory Pool 无法连接 |
| PoolBasePlane 的材质 | 已设置为 M_PoolTest1 | 水面没有波纹显示 |
| TraceMesh 的 StaticMesh | 已设置为 SM_plane_400x400 | 碰撞检测无网格参考 |
| NinjaLive 位置 | PoolBasePlane 在 InteractionVolume 范围内 | 水面不会产生交互 |
| InteractionVolume 大小 | BoxExtent 覆盖水面区域 | 角色走入水面但无交互 |
| preferredTraceChannelName | 与项目碰撞设置一致 | 碰撞检测失败，无法产生波纹 |
| 角色碰撞体 | 启用了 Physics/碰撞 | 角色穿过水面无交互 |

### 8.2 运行测试

1. 点击 Play 运行关卡
2. 控制角色走向水池区域
3. 观察角色进入 InteractionVolume 后水面是否产生涟漪
4. 测试不同移动速度对波纹大小的影响
5. 测试圆柱体/立方体等静态障碍物周围的绕流效果

### 8.3 效果调整参数

如果效果不佳，调整以下参数：

| 问题 | 调整参数 | 方向 |
|------|----------|------|
| 波纹衰减太快 | 减小密度/速度 Dissipation | 让波纹持续更久 |
| 波纹强度不够 | 增大 collisionStrength 或 painter brush velocity | 波纹更明显 |
| 仿真卡顿 | 降低 resolutionX/resolutionY | 从 1600 降到 1024 或 512 |
| 交互范围不对 | 调整 InteractionVolume 的 BoxExtent | 扩大或缩小交互区域 |
| 水面不透明 | 调整输出材质的 Opacity/Translucency | 让水更透明或更不透明 |

---

## 9. 关键原理总结

### 9.1 数据流

```
角色/物体移动
    |
    v
NinjaLiveTraceMesh（碰撞检测）
    |  使用 Painter V2 系统
    v
NinjaLiveComponent（GPU 仿真引擎）
    |  在 1600x1600 网格上求解 Navier-Stokes 方程
    |  包含：Advection -> Collision -> Divergence -> Pressure Solve
    v
仿真缓冲区（Density / Velocity / Pressure）
    |
    v
PoolBasePlane（M_PoolTest1 输出材质）
    |  读取缓冲区数据，计算波纹法线/位移/颜色
    v
最终视觉效果（水波荡漾、涟漪扩散、绕流）
```

### 9.2 核心组件职责

| 组件 | 类比 | 职责 |
|------|------|------|
| NinjaLive Actor | 大脑 | 统筹整个仿真系统的运行 |
| NinjaLiveComponent | CPU | 执行 GPU 仿真计算 |
| TraceMesh | 传感器 | 检测外部碰撞输入 |
| ActivationVolume | 电源开关 | 控制仿真何时运行以节省性能 |
| InteractionVolume | 检测区域 | 限定交互发生的空间范围 |
| PoolBasePlane | 显示器 | 将仿真结果显示为视觉效果 |
| NinjaLive_Utilities | 后勤部 | 提供内存管理和工具函数 |
| NinjaLive_PresetManager | 遥控器 | 快速切换不同预设效果 |

### 9.3 一句话总结

**碰撞 -> 仿真 -> 材质输出**，三个环节缺一不可。NinjaLive Actor 是大脑，PoolBasePlane 是显示器，角色/物体是输入设备。通过 Painter v2 系统将物理碰撞转化为流体运动的驱动力，最终在材质层面呈现为逼真的水波纹效果。

---

## 附录：源关卡 Actor 完整清单

以下是源关卡 `/Game/_MyTest/Island/Usecase_007_SmallWaterBodies_LIVE18` 中所有 Actor：

| 标签 | 类型 | 作用 |
|------|------|------|
| Pool | NinjaLive_C | 流体仿真主 Actor |
| NinjaLive_Utilities | NinjaLive_Utilities_C | 工具管理器 |
| NinjaLive_PresetManager_2 | NinjaLive_PresetManager_C | 预设管理器 |
| PoolBasePlane | StaticMeshActor (Plane + M_PoolTest1) | 水面显示平面 |
| PoolBottom | StaticMeshActor (Plane + MI_CheckerGreen18) | 水池底部 |
| Floor14 | StaticMeshActor | 地面 |
| Floor16 | StaticMeshActor | 辅助地面 |
| Sphere10 | StaticMeshActor | 碰撞球体 |
| Cylinder5~12 | StaticMeshActor | 圆柱碰撞体（8个） |
| Cube32~40 | StaticMeshActor | 立方体碰撞体 |
| SM_Arch1~5 | StaticMeshActor | 拱门装饰 |
| LightSource_1 | DirectionalLight | 主光源 |
| PointLight9~12 | PointLight | 点光源（4个） |
| SkyLight_1 | SkyLight | 天光 |
| SkyAtmosphere_1 | SkyAtmosphere | 天空大气 |
| ExponentialHeightFog_1 | ExponentialHeightFog | 指数雾 |
| PostProcessVolume_1 | PostProcessVolume | 后处理 |
| PostProcess_UnderWater_0 | PostProcessVolume | 水下后处理 |
| ThirdPersonCharacter_EngineDefault2 | Character | 第三人称角色 |

---

> **文档生成日期**: 2026-07-16  
> **适用项目**: MyProject_5_8 (UE 5.8)  
> **源关卡**: /Game/_MyTest/Island/Usecase_007_SmallWaterBodies_LIVE18  
> **插件版本**: FluidNinjaLive
