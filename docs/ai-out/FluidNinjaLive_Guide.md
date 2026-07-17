# FluidNinjaLive 学习使用指导文档

> **适用引擎**: Unreal Engine 5.4+ (当前项目 UE 5.8)  
> **插件类型**: 基于 Niagara 的 GPU 流体仿真系统  
> **作者/厂商**: Ninja Tools (Fab / UE Marketplace)  
> **文档版本**: v1.0 — 基于项目本地资产结构与官方资源整理

---

## 目录

1. [概述](#1-概述)
2. [核心架构](#2-核心架构)
3. [资产目录结构](#3-资产目录结构)
4. [核心资产详解](#4-核心资产详解)
5. [2D 流体仿真系统](#5-2d-流体仿真系统)
6. [输出材质系统](#6-输出材质系统)
7. [体积系统 (Volumetrics)](#7-体积系统-volumetrics)
8. [体积系统 v2 (Volumetrics_v2)](#8-体积系统-v2-volumetrics_v2)
9. [使用案例 (UseCases)](#9-使用案例-usecases)
10. [教程系统 (Tutorial)](#10-教程系统-tutorial)
11. [预设系统 (Presets)](#11-预设系统-presets)
12. [输入材质 (InputMaterials)](#12-输入材质-inputmaterials)
13. [常用工作流程](#13-常用工作流程)
14. [性能优化建议](#14-性能优化建议)
15. [常见问题与排错](#15-常见问题与排错)
16. [附录：完整资产清单](#16-附录完整资产清单)

---

## 1. 概述

### 1.1 什么是 FluidNinjaLive？

FluidNinjaLive 是 Unreal Engine 上一套**基于 GPU 的实时流体仿真系统**，完全基于 Niagara 粒子系统和材质系统构建。它能够在游戏运行时实时模拟 2D 流体（烟、火、水、岩浆等），并将仿真结果映射到 3D 表面或体积渲染中。

### 1.2 核心能力

| 能力 | 说明 |
|------|------|
| **2D 流体仿真** | 基于网格的 GPU 流体求解器（Navier-Stokes 方程），包含平流、扩散、压力求解、碰撞处理 |
| **实时交互** | 支持碰撞体、力场、粒子驱动等交互方式 |
| **材质输出** | 将仿真结果（密度、速度、压力等缓冲区）映射到任意材质 |
| **体积渲染** | 支持 Volume Smoke / Fog / Cloud，可结合流体仿真驱动 |
| **Niagara 集成** | 完全基于 Niagara 模块和系统，可自由扩展 |
| **预设系统** | 通过 DataTable 管理预设，快速切换不同效果 |

### 1.3 技术要点

- **完全使用 GPU 计算**：仿真计算通过材质绘制到 RenderTarget 完成
- **与 UE 渲染管线深度集成**：支持 Lumen、硬件光线追踪、Substrate 材质
- **Niagara 驱动**：碰撞、粒子捕捉、画笔等交互通过 Niagara 系统实现
- **无 C++ 依赖**：纯蓝图 + 材质 + Niagara，方便艺术家使用

---

## 2. 核心架构

FluidNinjaLive 采用**分层架构**，由以下几个层次组成：

### 2.1 控制层 (Controller)

- **NinjaLive_InterfaceController** (/Game/FluidNinjaLive/NinjaLive_InterfaceController)
  - 用户交互入口，处理输入指令
  - 管理仿真生命周期（启动/停止/重置）

### 2.2 仿真管理层

- **NinjaLive_PresetManager** (/Game/FluidNinjaLive/NinjaLive_PresetManager)
  - 预设加载、切换、保存
  - 与 DataTable 联动

- **NinjaLive_MemoryPoolManager** (/Game/FluidNinjaLive/NinjaLive_MemoryPoolManager)
  - 管理 RenderTarget 内存池
  - 优化 GPU 资源分配

### 2.3 工具层

- **NinjaLive_Utilities** (/Game/FluidNinjaLive/NinjaLive_Utilities)
  - 常用工具函数集合
  - 渲染目标管理、坐标转换等

### 2.4 组件层

- **NinjaLiveComponent** (/Game/FluidNinjaLive/NinjaLiveComponent)
  - 可附加到任意 Actor 的组件
  - 提供仿真控制的组件接口

- **NinjaLiveTraceMesh** (/Game/FluidNinjaLive/NinjaLiveTraceMesh)
  - 碰撞追踪网格组件
  - 处理物理碰撞到流体的映射

### 2.5 核心功能库

- **NinjaLiveFunctions** (/Game/FluidNinjaLive/Core/NinjaLiveFunctions)
  - 核心功能函数库（蓝图函数库）
- **NinjaLiveInterface** (/Game/FluidNinjaLive/Core/NinjaLiveInterface)
  - 接口定义
- **NinjaLiveGUI** (/Game/FluidNinjaLive/Core/NinjaLiveGUI)
  - UI 控件相关

### 2.6 数据流示意图

`
输入源 ──► 流体仿真引擎 ──► 输出缓冲区 ──► 材质渲染
  │              │                │
  ▼              ▼                ▼
碰撞/力场     GPU 计算       密度/速度/压力
粒子系统     (RenderTarget)   缓冲区纹理
`

---

## 3. 资产目录结构

Content/FluidNinjaLive/
├── NinjaLive.uasset                  # 主 Actor/管理器
├── NinjaLiveComponent.uasset         # 可附加组件
├── NinjaLiveTraceMesh.uasset         # 碰撞追踪网格组件
├── NinjaLive_InterfaceController.uasset # 接口控制器
├── NinjaLive_MemoryPoolManager.uasset   # 内存池管理器
├── NinjaLive_PresetManager.uasset       # 预设管理器
├── NinjaLive_Utilities.uasset           # 工具函数库
├── Help.uasset                          # 内置帮助
│
├── Core/                                # 核心资源
│   ├── Assets/                          # 核心资产（纹理、静态网格等）
│   ├── FluidSim/                        # 流体仿真材质系统
│   │   ├── Functions/                   # 仿真函数（MF_*）
│   │   ├── MI_Float/                    # 浮点精度仿真实例材质
│   │   └── MI_Float_UVflip/            # UV 翻转浮点实例材质
│   ├── GUI/                             # UI 贴图资源
│   ├── Materials/                       # 核心材质
│   ├── MediaPlayer/                     # 媒体播放器资源
│   └── Niagara/                         # Niagara 系统
│       ├── Emitters/                    # Niagara 发射器
│       ├── Modules/                     # Niagara 模块
│       ├── PreviewMaterials/           # 预览材质
│       ├── Scripts/                    # Niagara 脚本
│       ├── SPH/                        # SPH 相关资源
│       └── Systems/                    # Niagara 系统
│
├── InputMaterials/                     # 输入材质
│   └── Functions/                      # 输入材质函数
│
├── OutputMaterials/                    # 输出材质
│   ├── BaseMaterials/                  # 基础输出材质
│   ├── BaseMaterialsSpecial/           # 特殊基础材质
│   ├── Functions/                      # 输出材质函数
│   ├── Instance_Buffers/               # 缓冲区实例材质
│   ├── Instance_Examples/              # 示例实例材质
│   ├── Instance_Examples2/             # 示例实例材质 2
│   └── Instance_SimplePainter/         # 简单画笔模式实例材质
│
├── Presets/                            # 预设目录
│
├── Tutorial/                           # 教程资源
│   ├── Levels/                         # 教程关卡
│   ├── LevelsSpecial/                  # 特殊教程关卡
│   ├── Materials/                      # 教程材质
│   ├── Meshes/                         # 教程网格
│   ├── Misc/                           # 杂项
│   ├── Niagara/                        # 教程 Niagara 系统
│   ├── Sequencer/                      # 关卡序列
│   ├── Textures/                       # 教程纹理
│   └── UE_Mannequin_UsageExamples/     # 角色使用示例
│       ├── Animations/
│       ├── Character/
│       │   ├── Materials/
│       │   ├── Mesh/
│       │   └── Textures/
│       └── Materials/
│
├── UseCases/                           # 使用案例
│   ├── 002_BoatTrail/                  # 船只尾迹
│   ├── 004_WorldSpaceTiling/           # 世界空间平铺
│   ├── 005_VehicleTrail/               # 车辆尾迹
│   ├── 007_SmallWater/                 # 小水体
│   ├── 008_Lava/                       # 岩浆
│   ├── 009_GiantSmokePillars/          # 巨型烟柱
│   ├── 010_FireAndSmoke/               # 火焰与烟雾
│   ├── 011_DrivingParticles/           # 驱动粒子
│   ├── 012_NiagaraParticleCapture/     # Niagara 粒子捕捉
│   ├── 013_Foliage/                    # 植被交互
│   ├── 014_ImprovedWorldSpace/         # 改进的世界空间
│   ├── 016_Caustics/                   # 焦散效果
│   ├── 017_RiverAndLandscape/          # 河流与地形
│   └── 018_SnowAndSand/                # 雪与沙
│
├── Volumetrics/                        # 体积系统 v1
│   ├── AddMaterials/                   # 附加材质
│   ├── BaseMaterials/                  # 基础体积材质
│   │   ├── Functions/
│   │   └── Legacy/
│   ├── Instances_VolumeCloud/          # 体积云实例
│   ├── Instances_VolumeFog/            # 体积雾实例
│   ├── Instances_VolumeSmoke/          # 体积烟实例
│   │   ├── Demo/
│   │   └── Extended/
│   ├── Meshes/                         # 体积网格
│   ├── TempRenderTargets/              # 临时渲染目标
│   └── VolumeNoises/                   # 体积噪声
│
└── Volumetrics_v2/                     # 体积系统 v2
    ├── BaseMaterials/                  # v2 基础材质
    │   ├── Functions/
    │   └── MultiLayer/                 # 多层材质
    ├── BlueprintUtilities/             # 蓝图工具
    ├── Input_For_VolumeGeneric/        # 体积通用输入
    │   ├── CrossSectionGenerators/     # 截面生成器
    │   ├── CrossSectionTextures/       # 截面纹理
    │   ├── DepthMaps/                  # 深度图
    │   ├── HeightMapGenerators_RVT/    # 高度图生成器（RVT）
    │   └── HeightMapGenerators_Simulation/ # 高度图生成器（仿真）
    ├── Instances_VolumeGeneric/        # 体积通用实例
    │   ├── FluidSimDriven/             # 流体仿真驱动
    │   └── NonFluidSimDriven/          # 非流体仿真驱动
    ├── Misc/                           # 杂项
    │   ├── Materials_FakeShadow/       # 假阴影材质
    │   ├── Materials_Helper/           # 辅助材质
    │   ├── Materials_RVTsamplingExample/ # RVT 采样示例
    │   ├── Meshes/
    │   └── Textures/
    └── Niagara_VolumeGeneric/          # Niagara 体积通用系统

---

## 4. 核心资产详解

### 4.1 主控制资产

#### NinjaLive（主 Actor）
- **路径**: /Game/FluidNinjaLive/NinjaLive
- **功能**: 流体仿真的主控制器，管理仿真的全局状态
- **用法**: 拖放到关卡中，配置仿真参数

#### NinjaLiveComponent（组件）
- **路径**: /Game/FluidNinjaLive/NinjaLiveComponent
- **功能**: 可附加到任意 Actor 的组件，为 Actor 添加流体仿真能力
- **用法**: 在 Actor 的组件面板中添加

#### NinjaLiveTraceMesh（追踪网格）
- **路径**: /Game/FluidNinjaLive/NinjaLiveTraceMesh
- **功能**: 处理物理碰撞检测并映射到流体仿真网格
- **用法**: 作为组件附加到需要产生流体交互的 Actor 上

### 4.2 核心材质

#### M_FluidNinjaLiveBaseMat（基础材质）
- **路径**: /Game/FluidNinjaLive/Core/Materials/M_FluidNinjaLiveBaseMat
- **功能**: 流体仿真的基础主材质
- **实例**: 提供多种预设色调（Gray0-4, Brown, Sky1-5, Sky4dark 等）

#### M_FluidNinjaLiveLogo
- **路径**: /Game/FluidNinjaLive/Core/Materials/M_FluidNinjaLiveLogo
- **功能**: Logo 显示材质
- **实例**: Black, Blue, DarkBlue, Gray, Green, Purple, Red, Yellow

#### MI_FluidNinjaLive_TraceMesh_Inactive
- **路径**: /Game/FluidNinjaLive/Core/Materials/MI_FluidNinjaLive_TraceMesh_Inactive
- **功能**: 碰撞网格非活动状态材质

### 4.3 Niagara 系统

#### NS_Painter_v2_Dot / NS_Painter_v2_Line
- **路径**: /Game/FluidNinjaLive/Core/Niagara/Systems/
- **功能**: 画笔系统（点和线），用于向流体仿真输入密度/速度

#### NS_ParticleCaptureBasic
- **路径**: /Game/FluidNinjaLive/Core/Niagara/Systems/
- **功能**: 基础粒子捕捉系统

#### NE_ParticleCaptureBasic
- **路径**: /Game/FluidNinjaLive/Core/Niagara/Emitters/
- **功能**: 粒子捕捉发射器

### 4.4 核心纹理

| 资产名 | 用途 |
|--------|------|
| T_Black / T_White | 纯黑/白纹理 |
| T_VelocityNull | 零速度纹理（用于初始化） |
| T_Noise3, T_NoiseTemplate1, T_NoiseTemplate2 | 噪声纹理模板 |
| T_TextureTemplate1 | 纹理模板 |
| T_maskframe_256 / thick / thicker | 遮罩框架纹理 |
| T_maskframeH_256 / T_maskframeV_256 | 水平/垂直遮罩 |

---

## 5. 2D 流体仿真系统

### 5.1 仿真原理

FluidNinjaLive 的 2D 流体仿真基于**网格法（Eulerian 方法）**，在 GPU 上通过材质绘制到 RenderTarget 来实现。仿真核心组件位于 Core/FluidSim/ 目录。

### 5.2 仿真管线

仿真循环包含以下几个步骤：

`
1. 碰撞绘制 (CollisionPainter)
   ↓
2. 平流 (Advection)
   ↓
3. 外力合成 (Composite & Gradient)
   ↓
4. 散度计算 (Divergence)
   ↓
5. 压力求解 (Pressure Solver)
   ↓
6. 输出缓冲区更新
`

### 5.3 仿真材质详解

#### 主材质

| 材质 | 路径 | 功能 |
|------|------|------|
| M_Advection | Core/FluidSim/M_Advection | 平流计算：将密度/速度沿速度场移动 |
| M_CollisionPainter | Core/FluidSim/M_CollisionPainter | 碰撞绘制：将碰撞信息写入仿真网格 |
| M_CollisionPainterOffset | Core/FluidSim/M_CollisionPainterOffset | 碰撞偏移处理 |
| M_CompositeAndGradient | Core/FluidSim/M_CompositeAndGradient | 外力合成与梯度计算 |
| M_Divergence | Core/FluidSim/M_Divergence | 散度计算：确保流体不可压缩性 |
| M_Pressure | Core/FluidSim/M_Pressure | 压力求解器：迭代求解压力场 |

#### 材质函数

| 函数 | 路径 | 功能 |
|------|------|------|
| MF_FlipUVY | Core/FluidSim/Functions/MF_FlipUVY | 翻转 UV 的 Y 轴 |
| MF_GetAspect | Core/FluidSim/Functions/MF_GetAspect | 获取宽高比 |
| MF_SeparableMultiKernelDiffusion | Core/FluidSim/Functions/MF_SeparableMultiKernelDiffusion | 可分离多核扩散 |

#### 浮点精度版本 (MI_Float)

对于需要更高精度的场景，提供了浮点版本的仿真材质（以 MI_ 开头）：

- MI_Advection
- MI_CollisionPainter_Dot / MI_CollisionPainter_Line
- MI_CollisionPainterOffset
- MI_CompositeAndGradient / MI_CompositeAndGradient_HDnoise
- MI_Divergence
- MI_Pressure_Solver1
- MI_Pressure_Solver2_Step1 / MI_Pressure_Solver2_Step2

#### UV 翻转版本 (MI_Float_UVflip)

当流体需要翻转 UV 坐标时使用（如某些特殊表面映射）：

- MI_Advection_UVY
- MI_CollisionPainterOffset_UVY
- MI_CompositeAndGradient_UVY / MI_CompositeAndGradient_HDnoise_UVY
- MI_Divergence_UVY
- MI_Pressure_Solver1_UVY
- MI_Pressure_Solver2_Step1_RedCh / MI_Pressure_Solver2_Step2_GreenCh

### 5.4 仿真缓冲区

仿真引擎维护以下关键缓冲区（RenderTarget）：

| 缓冲区 | 描述 |
|--------|------|
| **密度缓冲区 (Density)** | 流体的密度/浓度（如烟的浓度、水的颜色） |
| **速度缓冲区 (Velocity)** | 流体的速度场（方向和大小） |
| **压力缓冲区 (Pressure)** | 压力场（用于不可压缩性约束） |
| **散度缓冲区 (Divergence)** | 中间计算结果 |
| **碰撞缓冲区 (Collision)** | 碰撞体信息 |

---

## 6. 输出材质系统

### 6.1 概述

输出材质位于 OutputMaterials/ 目录，负责将流体仿真的缓冲区数据渲染到 3D 表面上。这是流体仿真的**可视化层**。

### 6.2 基础输出材质

| 材质 | 功能 |
|------|------|
| M_NinjaOutput_Basic | 基础输出，直接映射密度/速度 |
| M_NinjaOutput_Basic_NegativeEmissiveEnabled | 支持负发光的版本 |
| M_NinjaOutput_Advanced | 高级输出，更多控制参数 |
| M_NinjaOutput_BufferVisualization | 缓冲区可视化（调试用） |
| M_NinjaOutput_SimplePainterMode | 简单画笔模式 |
| M_NinjaOutput_TranslucentReflective | 半透明反射材质 |
| M_NinjaOutput_WorldSpaceGeneric | 世界空间通用输出 |
| M_NinjaOutput_WorldSpaceGeneric_Multilayer | 多层世界空间输出 |

### 6.3 缓冲区实例材质 (Instance_Buffers)

这些材质实例直接显示各个仿真缓冲区的内容，用于调试和特殊效果：

- **密度缓冲区**: MI_DensityBuffer_Black, MI_DensityBuffer_Blue, MI_DensityBuffer_Red, MI_DensityBuffer_DetailMapped1~4, MI_DensityBuffer_Inverted_Normal 等
- **速度缓冲区**: MI_VelocityBuffer
- **压力缓冲区**: MI_PressureBuffer, MI_PressureBuffer_MeshDistort
- **散度缓冲区**: MI_DivergenceBuffer, MI_DivergenceBuffer_MeshDistort

### 6.4 效果实例材质 (Instance_Examples / Instance_Examples2)

#### 流体爆炸 (FluidBlast) 系列
- MI_FluidBlast_BloodOpaque — 血液效果
- MI_FluidBlast_LiquidOpaque1~4 — 液体效果（4 含 WPO）
- MI_FluidBlast_MudOpaque1~3 — 泥浆效果（带 FlowMap）
- MI_FluidBlast_RayMarching_Mono_v1 — 光线步进单色
- MI_FluidBlast_RayMarching_ColorGradient_v1 — 光线步进渐变色
- MI_FluidBlast_RayMarching_ColorPaintBuffer_v1~3 — 光线步进颜色画笔
- MI_FluidBlast_RayMarching_ColorToneMap_v1~2 — 光线步进色调映射
- MI_FluidBlast_Snow1, Snow1Opaque, Snow2 — 雪效果

#### 烟雾/雾气系列
- MI_SmokeChamber_v1_FlowMap1 — 烟雾室（带 FlowMap）
- MI_SmokeChamber_v1_RayMarching — 烟雾室光线步进
- MI_SmokeTest_v1~3 — 烟雾测试
- MI_MistValley_Greene — 雾谷

#### 水效果系列
- MI_Water_SingleLayer_Tile_Simulated — 单层瓦片模拟水
- MI_Water_SingleLayer_Vortex_Simulated — 单层涡旋模拟水
- MI_Water_SingleLayer_Tile_StaticTexture — 单层瓦片静态纹理水
- MI_Water_SingleLayer_Tile_None — 无效果水

#### 折射/扭曲系列
- MI_Swimmers_RefractionFromDivergence_v1~3 — 从散度计算的折射
- MI_Swimmers_RefractionFromPressure_v1 — 从压力计算的折射
- MI_TransparentRefraction_v1 — 透明折射

### 6.5 材质函数

| 函数 | 功能 |
|------|------|
| MF_BlurSampleOffsets | 模糊采样偏移 |
| MF_EdgeMask_HV_Separated | 水平和垂直分离的边缘遮罩 |
| MF_EdgeMask_Linear | 线性边缘遮罩 |
| MF_EdgeMask_Sinusoid | 正弦波边缘遮罩 |
| MF_FlowMapFunction | FlowMap 函数 |
| MF_NinjaOutput_WorldSpaceGeneric | 世界空间通用函数 |
| MF_NormalFromHeightmap / MF_NormalFromHeightmap_SineWaves | 从高度图生成法线 |
| MF_ParallaxOcclusionMappingSimple | 简单视差遮挡映射 |
| MF_PosBasedRndColors | 基于位置的随机颜色 |
| MF_Pow | 幂函数 |
| MF_ShadingFromHeight | 从高度着色 |
| MF_SharpenMono / MF_SharpenRGB | 锐化（单通道/三通道） |
| MF_VerticalDepth | 垂直深度 |

---

## 7. 体积系统 (Volumetrics)

### 7.1 概述

体积系统（v1）位于 Volumetrics/ 目录，提供基于体积渲染的烟雾、云雾效果。这些材质使用**光线步进（Ray Marching）**技术，在 3D 空间中渲染半透明体积。

### 7.2 基础体积材质

| 材质 | 功能 |
|------|------|
| M_VolumeSmoke | 体积烟雾基础材质 |
| M_VolumeFog | 体积雾基础材质 |
| M_VolumeCloud | 体积云基础材质 |
| M_Ocean | 海洋基础材质 |

### 7.3 基础材质函数

| 函数 | 功能 |
|------|------|
| MF_Volumetric_MeshAlignment | 体积网格对齐 |
| MF_Volume_UVW | 体积 UVW 坐标计算 |
| MF_SideRatio | 侧边比例 |
| MF_RayBox | 射线-盒子相交 |
| MF_RayBox_SetupForRayMarching | 光线步进初始化 |
| MF_FunctionLib | 函数库 |
| MF_FlowVolumeFunction | 体积 Flow 函数 |
| MF_AlignMeshToTheCamera | 相机对齐 |

### 7.4 体积烟雾实例 (Instances_VolumeSmoke)

包含大量预设的烟雾效果实例：

- **基础类型**: Unlit（无光照）, DirLit（方向光）, PointLit（点光）
- **风格变体**: Noisy（带噪声）, Exp（指数）, Bright（明亮）
- **Demo 系列**: Demo1Unlit, Demo2UnlitNoisy, Demo3Dir, Demo3Point, Demo4DirNoisy, Demo4PointNoisy
- **Extended 系列**: 扩展效果

### 7.5 体积云实例 (Instances_VolumeCloud)

| 实例 | 说明 |
|------|------|
| MI_VolumeCloud_Default | 默认云 |
| MI_VolumeCloud_DefaultLow | 低密度云 |
| MI_VolumeCloud_DefaultTiled | 平铺云 |
| MI_VolumeCloud_DefaultTiled2_Dark/Lite/Plastic/Thick | 平铺云变体 |
| MI_VolumeCloud_Fluffster | 蓬松云 |
| MI_VolumeCloud_Flat | 扁平云 |
| MI_VolumeCloud_Noiseless | 无噪声云 |
| MI_VolumeCloud_Red | 红色云 |
| MI_VolumeCloud_Storm | 暴风云 |
| MI_VolumeCloud_StratoSphereImpact | 平流层冲击云 |
| MI_VolumeCloud_StratoSphereCyclone | 平流层旋风云 |
| MI_VolumeCloud_Stylized | 风格化云 |
| MI_VolumeCloud_Thin | 薄云 |
| MI_VolumeCloud_AuroraBorealis | 极光效果 |

### 7.6 体积雾实例 (Instances_VolumeFog)

- MI_VolumeFog_Dryice — 干冰效果
- MI_VolumeFog_Homogen — 均匀雾
- MI_VolumeFog_Shrine — 圣殿光雾
- MI_VolumeFog_Trails — 雾迹
- MI_VolumeFog_Trails_LIVE17 — 雾迹（LIVE17 版本）

### 7.7 海洋材质

- M_Ocean — 海洋基础材质
- MI_OceanImpact — 海洋冲击效果
- MI_OceanPassive — 海洋静态效果

### 7.8 体积网格

- SM_CameraPlane — 相机对齐平面
- SM_InvertedBox — 倒置盒体（用于包裹场景）

### 7.9 体积噪声

- VolumeNoise1, VolumeNoise2 — 体积噪声纹理
- T_VolumeNoise1, T_VolumeNoise2, T_VolumeNoise2Inv — 2D 噪声贴图

---

## 8. 体积系统 v2 (Volumetrics_v2)

### 8.1 概述

体积系统 v2 是 FluidNinjaLive 的新一代体积渲染系统，位于 Volumetrics_v2/。相比 v1，v2 提供了更灵活的 **VolumeGeneric（通用体积）** 框架，支持更丰富的输入源和更复杂的多层效果。

### 8.2 关键改进

- **通用体积框架**: 统一处理烟雾、云雾、火焰等多种体积效果
- **流体仿真驱动**: 可以将 2D 流体仿真结果映射到体积渲染中
- **截面生成器**: 从 2D 纹理生成 3D 体积截面
- **多层材质**: 支持 Material Layer 系统
- **RVT 集成**: 支持运行时虚拟纹理采样
- **高度图生成**: 支持地形高度图驱动

### 8.3 演示关卡

| 关卡 | 路径 | 说明 |
|------|------|------|
| VolumeGeneric1_FeatureDemo_NoFluidSim | Volumetrics_v2/... | 功能演示（无流体仿真） |
| VolumeGeneric2_SmallScaleExamples_FluidSim | Volumetrics_v2/... | 小规模示例（带流体仿真） |
| VolumeGeneric3_MediumScaleExamples_Fluidsim | Volumetrics_v2/... | 中规模示例（带流体仿真） |
| VolumeGeneric4_MediumScaleExamples_NoFluidSim | Volumetrics_v2/... | 中规模示例（无流体仿真） |
| VolumeGeneric5_LargeScaleExamples_NoFluidSim | Volumetrics_v2/... | 大规模示例（无流体仿真） |
| VolumeGeneric6_LargeScaleExamples_FluidSim | Volumetrics_v2/... | 大规模示例（带流体仿真） |

### 8.4 基础材质

| 材质 | 功能 |
|------|------|
| M_VolumeGeneric_For_Clouds | 云的通用体积材质 |
| M_VolumeGeneric_For_Heterogeneous_And_Fog | 非均匀和雾的通用体积材质 |
| MF_VolumeGeneric_CoreFunction | 核心函数 |
| MF_UVW_ScaleOffset | UVW 缩放偏移 |
| MF_TextureSampler | 纹理采样器 |
| MF_EdgeMask_Linear_v2 | 线性边缘遮罩 v2 |
| MF_PolarCoordinates | 极坐标 |
| MF_FlowVolumeFunction_v2 | Flow 体积函数 v2 |
| MF_SamplePowMultAdd | 采样幂乘加 |

### 8.5 多层材质

- ML_VolumeGeneric_As_MaterialLayer — 作为材质层的通用体积
- MLB_LayerBlend_SimpleMax — 简单最大混合
- M_HostMaterial_For_MaterialLayers — 材质层宿主材质

### 8.6 输入系统 (Input_For_VolumeGeneric)

#### 截面生成器 (CrossSectionGenerators)
- M_CrossSectionBased3DVolume_VerySimpleExample — 简单截面体积示例
- M_CrossSectionGenerator_Mushroom — 蘑菇截面生成器
- MI_CrossSectionGenerator_Mushroom_SmallStatic / LargeStatic — 静态蘑菇截面
- MI_CrossSectionGenerator_Mushroom_LargeAnimated_v1 / v2 — 动态蘑菇截面

#### 截面纹理 (CrossSectionTextures)
大量 T_CrossSection* 纹理，用于定义体积的 3D 形状

#### 深度图 (DepthMaps)
深度图纹理

#### 高度图生成器
- HeightMapGenerators_RVT — 运行时虚拟纹理高度图
- HeightMapGenerators_Simulation — 仿真高度图

### 8.7 通用体积实例 (Instances_VolumeGeneric)

#### 流体仿真驱动 (FluidSimDriven)
将 2D 流体仿真的结果映射到 3D 体积中

#### 非流体仿真驱动 (NonFluidSimDriven)
使用静态或程序化噪声驱动的体积效果

### 8.8 Niagara 系统 (Niagara_VolumeGeneric)

- NE_VolumetricEmitter — 体积发射器
- NE_VolumetricEmitterMinimal — 最小化体积发射器
- NS_FVOL_with_RVTsampler — 全屏体积 + RVT 采样器
- NS_HVOL_with_RVTsampler — 半体积 + RVT 采样器

### 8.9 蓝图工具

- MakeActorComponentCameraFacing — 使组件面向相机
- WriteMaterialsToRenderTargets — 将材质写入渲染目标

---

## 9. 使用案例 (UseCases)

UseCases 目录包含了各种实际应用的完整示例，每个案例包含关卡、材质、Niagara 系统等完整资源。

### 9.1 案例列表

| 编号 | 案例名称 | 说明 |
|------|----------|------|
| 002 | BoatTrail | 船只在水面行驶产生的尾迹效果 |
| 004 | WorldSpaceTiling | 世界空间平铺的流体效果 |
| 005 | VehicleTrail | 车辆行驶的尾迹/尘土效果 |
| 007 | SmallWater | 小范围水体效果（水坑、水洼） |
| 008 | Lava | 岩浆流动效果 |
| 009 | GiantSmokePillars | 巨型烟柱效果 |
| 010 | FireAndSmoke | 火焰与烟雾组合效果 |
| 011 | DrivingParticles | 流体驱动粒子效果 |
| 012 | NiagaraParticleCapture | Niagara 粒子捕捉到流体仿真 |
| 013 | Foliage | 植被与流体交互效果 |
| 014 | ImprovedWorldSpace | 改进的世界空间流体映射 |
| 016 | Caustics | 焦散效果（水底光斑） |
| 017 | RiverAndLandscape | 河流与地形融合效果 |
| 018 | SnowAndSand | 雪地/沙地效果 |

### 9.2 案例详解

#### 002_BoatTrail — 船只尾迹
- **效果**: 船只在水面行驶时产生的水波尾迹
- **关键技术**: 碰撞检测映射到流体仿真，速度场驱动波纹
- **包含**: 车辆蓝图、流体仿真系统、水面材质

#### 004_WorldSpaceTiling — 世界空间平铺
- **效果**: 在世界空间中平铺流体效果，覆盖大面积
- **关键技术**: 世界空间 UV 计算、平铺处理

#### 005_VehicleTrail — 车辆尾迹
- **效果**: 车辆行驶产生的尘土/烟雾尾迹
- **关键技术**: Niagara 粒子 + 流体仿真 + 体积烟雾
- **包含**: 轿车蓝图 (Sedan_EngineDefault, Sedan_Chaos)、轮胎蓝图、烟雾材质

#### 007_SmallWater — 小水体
- **效果**: 小范围水体，如水坑、池塘
- **关键技术**: 局部空间流体仿真、水面反射/折射
- **包含**: M_FluidNinjaLiveBaseMat_NoSpecular, MI_FluidNinjaLiveBaseGray_NoSpecular

#### 008_Lava — 岩浆
- **效果**: 岩浆流动、发光效果
- **关键技术**: 发光材质、颜色渐变、速度场驱动

#### 009_GiantSmokePillars — 巨型烟柱
- **效果**: 从地面升起的巨大烟柱
- **关键技术**: 体积烟雾 + 流体仿真

#### 010_FireAndSmoke — 火焰与烟雾
- **效果**: 火焰燃烧产生的火焰和烟雾
- **关键技术**: 发光材质 + 体积烟雾
- **包含**: MI_FluidNinjaLiveBaseSky6（火焰色调变体）

#### 011_DrivingParticles — 驱动粒子
- **效果**: 用流体仿真驱动 Niagara 粒子的运动
- **关键技术**: 速度场采样、粒子跟随

#### 012_NiagaraParticleCapture — 粒子捕捉
- **效果**: 将 Niagara 粒子数据捕捉到流体仿真网格
- **关键技术**: 粒子到网格写入、RenderTarget

#### 013_Foliage — 植被交互
- **效果**: 流体与植被（草、树）的交互效果
- **关键技术**: 风场驱动、植被摆动

#### 014_ImprovedWorldSpace — 改进世界空间
- **效果**: 改进的世界空间流体映射技术
- **关键技术**: 世界空间 UV 改进算法

#### 016_Caustics — 焦散
- **效果**: 水面下方的焦散光斑效果
- **关键技术**: 流体仿真驱动焦散纹理

#### 017_RiverAndLandscape — 河流与地形
- **效果**: 河流在地形上流动，与环境融合
- **关键技术**: 世界空间 + 局部空间混合、地形跟随
- **子目录**:
  - MaterialsEnvironment — 环境材质
  - MaterialsPostProcess — 后处理材质
  - MaterialsWaterLocalSpace — 局部空间水材质
  - MaterialsWaterWorldSpace — 世界空间水材质
  - Niagara — Niagara 系统
  - Textures — 纹理资源
  - Meshes — 网格资源
  - Misc — 杂项

#### 018_SnowAndSand — 雪与沙
- **效果**: 雪地/沙地的动态效果，可交互
- **子目录**:
  - MaterialsInput — 输入材质
  - MaterialsIntermediate — 中间材质
  - MaterialsOutputGround — 地面输出材质
  - MaterialsOutputVolumetrics — 体积输出材质
  - Niagara — Niagara 系统
  - Meshes/ — 网格、材质函数、纹理
  - Misc — 杂项

### 9.3 如何使用案例

1. 在 Content Browser 中浏览到对应的 UseCase 目录
2. 打开其中的关卡文件（.umap）
3. 点击 Play 查看效果
4. 分析关卡中的 Actor 和组件配置
5. 复制需要的系统到自己的项目中

---

## 10. 教程系统 (Tutorial)

### 10.1 概述

教程系统位于 Tutorial/ 目录，提供了丰富的教学资源和示例，帮助用户快速上手。

### 10.2 教程内容

#### Niagara 系统

| 系统 | 路径 | 说明 |
|------|------|------|
| NS_2Dflow_BasicGridDemo | Tutorial/Niagara/NS_2Dflow_BasicGridDemo | 2D 流体基础网格演示 |
| NS_2Dflow_BasicGridDemo_LIVE17 | Tutorial/Niagara/NS_2Dflow_BasicGridDemo_LIVE17 | 2D 流体网格演示（LIVE17） |
| NS_2Dflow_FlowBalls | Tutorial/Niagara/NS_2Dflow_FlowBalls | 流动小球 |
| NS_2Dflow_Komplex | Tutorial/Niagara/NS_2Dflow_Komplex | 复杂 2D 流动 |
| NS_2Dflow_PersistentBrush | Tutorial/Niagara/NS_2Dflow_PersistentBrush | 持续画笔 |
| NS_3Dflow | Tutorial/Niagara/NS_3Dflow | 3D 流动 |
| NS_Blocks | Tutorial/Niagara/NS_Blocks | 方块效果 |
| NS_Demo_BasicGrid | Tutorial/Niagara/NS_Demo_BasicGrid | 基础网格演示 |
| NS_Demo_DebrisInWater | Tutorial/Niagara/NS_Demo_DebrisInWater | 水中碎片演示 |
| NS_Demo_FloatingFizz | Tutorial/Niagara/NS_Demo_FloatingFizz | 浮动气泡 |
| NS_Demo_WateryDrizzleDown | Tutorial/Niagara/NS_Demo_WateryDrizzleDown | 水滴下落 |

#### 教程材质

- 基础材质: M_CheckerBasic, M_MetallicBaseMat, M_SimpleGradient, M_SimpleTexture, M_transparent1
- 实例材质: MI_Checker* (多种颜色), MI_Shiny1/2, MI_SolidColor* (纯色)
- 体积材质: MI_VolumeSmoke_*, MI_Water_*, MI_DensityBuffer_* (教程版)
- 后处理: M_PostProcessVelocityBuffer

#### 教程网格

多种基础网格: 平面 (50x50~500x500)、球体 (50~200 分段)、圆柱体、圆环、弯曲平面等

#### 角色示例 (UE_Mannequin_UsageExamples)

- ThirdPersonCharacter_EngineDefault — 标准第三人称角色
- ThirdPersonCharacter_NinjaAsChildActorComponent — Ninja 作为子 Actor 组件
- ThirdPersonCharacter_NinjaAsComponent — Ninja 作为组件
- ThirdPersonCharacter_NinjaSpawnedAttached — Ninja 生成并附加
- 动画蓝图: ThirdPerson_AnimBP
- 角色网格: SK_Mannequin + 材质

#### 数据表

| 数据表 | 说明 |
|--------|------|
| DT_NinjaLive_ForCrossSection1 | 截面数据 |
| DT_NinjaLive_PaintDemo | 画笔演示 |
| DT_NinjaLive_PresetPlacementExample | 预设放置示例 |
| DT_NinjaLive_SemiPersistent2 / SemiPersistentDemo | 半持久数据 |

---

## 11. 预设系统 (Presets)

### 11.1 概述

FluidNinjaLive 通过预设系统快速切换不同的流体仿真效果。预设信息存储在 DataTable 中，由 NinjaLive_PresetManager 管理。

### 11.2 预设资产

- **NinjaPreset** (Core/NinjaPreset) — 预设数据结构
- **NinjaLive_PresetManager** — 预设管理器蓝图

### 11.3 预设包含的参数

典型的预设包含以下配置：

- **仿真参数**: 分辨率、迭代次数、粘性、扩散率
- **输入参数**: 密度注入速率、速度注入强度
- **输出参数**: 材质选择、颜色映射、透明度
- **交互参数**: 碰撞响应、力场强度

### 11.4 创建预设

1. 创建新的 DataTable 行（基于预设结构）
2. 配置仿真参数
3. 在关卡中通过 PresetManager 加载

---

## 12. 输入材质 (InputMaterials)

### 12.1 概述

输入材质负责将外部数据（如碰撞信息、粒子数据、用户绘制）转换为流体仿真引擎可以理解的格式。

### 12.2 输入材质函数

位于 InputMaterials/Functions/，包含用于处理输入数据的材质函数。

### 12.3 输入源类型

| 输入源 | 描述 |
|--------|------|
| **碰撞输入** | 将物理碰撞体位置/速度映射到仿真网格 |
| **Niagara 粒子输入** | 将粒子系统数据写入仿真缓冲区 |
| **用户绘制输入** | 通过画笔系统手动绘制密度/速度 |
| **纹理输入** | 使用静态纹理作为初始状态 |
| **媒体输入** | 通过 MediaPlayer 播放视频作为输入源 |

---

## 13. 常用工作流程

### 13.1 快速开始：在关卡中添加流体效果

`
1. 在 Content Browser 中找到 /Game/FluidNinjaLive/NinjaLive
2. 将其拖放到关卡中
3. 在细节面板中配置仿真参数：
   - 仿真分辨率（如 256x256）
   - 缓冲区格式
   - 仿真迭代次数
4. 选择输出材质（如 M_NinjaOutput_Basic）
5. 点击 Play 查看效果
`

### 13.2 为角色添加流体交互

`
方法 A：使用 NinjaLiveComponent
1. 打开角色蓝图
2. 添加 NinjaLiveComponent 组件
3. 配置碰撞响应
4. 在事件图表中调用控制函数

方法 B：使用 NinjaLiveTraceMesh
1. 在角色上添加 NinjaLiveTraceMesh 组件
2. 配置追踪参数
3. 与 NinjaLive 主 Actor 关联
`

### 13.3 创建自定义流体效果

`
1. 选择基础输出材质（如 M_NinjaOutput_Basic）
2. 创建材质实例
3. 调整参数：
   - 密度颜色映射
   - 速度场强度
   - 透明度/发光
   - 噪声细节
4. 应用到流体表面
5. 调整仿真参数优化效果
`

### 13.4 使用体积烟雾

`
1. 打开 Volumetrics/Instances_VolumeSmoke/
2. 选择一个体积烟雾材质实例
3. 应用到体积网格（如 SM_InvertedBox）
4. 可选：连接流体仿真驱动
5. 调整光照和密度参数
`

### 13.5 使用 Volumetrics_v2 通用体积

`
1. 打开演示关卡 VolumeGeneric1~6 查看效果
2. 复制需要的 Niagara 系统（NE_VolumetricEmitter）
3. 选择基础体积材质（M_VolumeGeneric_For_Clouds 或 M_VolumeGeneric_For_Heterogeneous_And_Fog）
4. 配置输入源：
   - 截面纹理（CrossSectionTextures）
   - 流体仿真缓冲区
   - 噪声纹理
5. 调整 UVW 变换和密度参数
`

### 13.6 Niagara 粒子捕捉流程

`
1. 添加 NS_ParticleCaptureBasic 系统到关卡
2. 配置粒子发射器参数
3. 将粒子数据写入 RenderTarget
4. 流体仿真读取粒子数据作为输入
5. 渲染到输出材质
`

---

## 14. 性能优化建议

### 14.1 仿真分辨率

- **低分辨率（128x128）**：移动端/低配，大范围效果
- **中等分辨率（256x256）**：默认推荐，平衡质量与性能
- **高分辨率（512x512）**：高质量，小范围精细效果
- **超高分辨率（1024x1024）**：仅用于特写/过场

### 14.2 迭代次数

- 压力求解器迭代次数直接影响仿真质量
- 推荐值：2-4 次（平衡），6-8 次（高质量）
- 每增加一次迭代，GPU 开销线性增加

### 14.3 活动缓冲区数量

- 尽量减少同时活动的仿真缓冲区
- 仅保留需要的缓冲区（密度、速度、压力）
- 使用 NinjaLive_MemoryPoolManager 管理内存

### 14.4 Niagara 粒子数量

- 粒子捕捉模式中，粒子数量影响性能
- 推荐值：500-2000 粒子（实时），2000-5000（高质量）
- 使用 LOD 和距离剔除

### 14.5 体积渲染优化

- 使用较低的分辨率渲染目标
- 减少光线步进步数
- 使用 Distance Fade 减少远处计算
- 使用半分辨率渲染

### 14.6 CPU vs GPU 开销

- 仿真计算完全在 GPU 上运行
- CPU 开销主要在 Niagara 粒子系统和碰撞检测
- 合理设置粒子更新频率

---

## 15. 常见问题与排错

### 15.1 流体不显示

| 可能原因 | 解决方案 |
|----------|----------|
| 材质未正确分配 | 检查输出材质实例是否正确设置 |
| RenderTarget 未创建 | 检查 NinjaLive_MemoryPoolManager 配置 |
| 仿真未启动 | 调用 InterfaceController 的启动函数 |
| 缓冲区格式不匹配 | 检查缓冲区格式（RGBA8 vs Float） |

### 15.2 流体行为异常

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 流体不流动 | 速度场为零 | 添加速度输入或力场 |
| 流体发散 | 压力求解迭代不足 | 增加压力求解器迭代次数 |
| 流体泄漏 | 边界处理不当 | 检查碰撞边界条件 |
| 性能卡顿 | 分辨率过高 | 降低仿真分辨率 |

### 15.3 材质编译错误

- 确保使用 SM6（Shader Model 6）编译模式
- 检查项目设置中的 DX12 支持
- 更新 GPU 驱动程序
- 清除材质编译缓存

### 15.4 Niagara 系统不工作

- 确认 Niagara 插件已启用
- 检查粒子发射器的 LOD 设置
- 确认 RenderTarget 权限设置正确

### 15.5 兼容性问题

- UE 5.4+ 推荐使用
- 需要 DX12 + SM6 支持
- Lumen + HWRT 可选，非必需
- 不支持移动端 GPU 计算（部分功能）

---

## 16. 附录：完整资产清单

### 16.1 主控制资产

| 资产 | 路径 |
|------|------|
| NinjaLive | /Game/FluidNinjaLive/NinjaLive |
| NinjaLiveComponent | /Game/FluidNinjaLive/NinjaLiveComponent |
| NinjaLiveTraceMesh | /Game/FluidNinjaLive/NinjaLiveTraceMesh |
| NinjaLive_InterfaceController | /Game/FluidNinjaLive/NinjaLive_InterfaceController |
| NinjaLive_MemoryPoolManager | /Game/FluidNinjaLive/NinjaLive_MemoryPoolManager |
| NinjaLive_PresetManager | /Game/FluidNinjaLive/NinjaLive_PresetManager |
| NinjaLive_Utilities | /Game/FluidNinjaLive/NinjaLive_Utilities |
| Help | /Game/FluidNinjaLive/Help |

### 16.2 核心枚举/结构

| 资产 | 路径 |
|------|------|
| InactiveBehaviour_Enum | Core/InactiveBehaviour_Enum |
| SingleObjectType_Enum | Core/SingleObjectType_Enum |
| SimPrecision_Enum | Core/SimPrecision_Enum |
| UserInput_Enum | Core/UserInput_Enum |
| QuantizerMode | Core/QuantizerMode |
| QuantizerAxisIgnore | Core/QuantizerAxisIgnore |
| RenderTargetListItem | Core/RenderTargetListItem |
| RenderTargetList | Core/RenderTargetList |
| NinjaPreset | Core/NinjaPreset |

### 16.3 核心 Niagara 模块/脚本

| 资产 | 类型 | 路径 |
|------|------|------|
| HeightFade | 模块 | Core/Niagara/Modules/ |
| Set2DVelocityColor | 模块 | Core/Niagara/Modules/ |
| VerticalPositionOffset | 模块 | Core/Niagara/Modules/ |
| WriteGridToRenderTargetBasic | 模块 | Core/Niagara/Modules/ |
| WriteParticlesToGridBasic | 模块 | Core/Niagara/Modules/ |
| TruncatePositionToVector | 脚本 | Core/Niagara/Scripts/ |

### 16.4 SPH 相关

| 资产 | 路径 |
|------|------|
| ENiagaraGrid2DResolution | Core/Niagara/SPH/ |
| Grid2D_CreateUnitToWorldTransform | Core/Niagara/SPH/ |
| Grid2D_SetResolution | Core/Niagara/SPH/ |
| ViewRayToEmitterPlane | Core/Niagara/SPH/ |
| ViewRayToEmitterPlane_Static | Core/Niagara/SPH/ |

### 16.5 输入材质

| 资产 | 路径 |
|------|------|
| InputMaterial 函数 | InputMaterials/Functions/ |

### 16.6 输出材质清单（简要）

| 类别 | 数量 |
|------|------|
| 基础材质 (M_) | 10+ |
| 材质函数 (MF_) | 15+ |
| 缓冲区实例 (MI_DensityBuffer_*) | 30+ |
| 流体爆炸实例 (MI_FluidBlast_*) | 20+ |
| 烟雾室实例 (MI_SmokeChamber_*) | 10+ |
| 颜色画笔实例 (MI_ColorBrush_*) | 10+ |

### 16.7 Volumetrics 清单

| 类别 | 数量 |
|------|------|
| 基础材质 (M_) | 5 |
| 材质函数 (MF_) | 10+ |
| 体积云实例 (MI_VolumeCloud_*) | 20+ |
| 体积雾实例 (MI_VolumeFog_*) | 10+ |
| 体积烟实例 (MI_VolumeSmoke_*) | 30+ |
| 体积噪声纹理 | 5 |
| 渲染目标 | 10+ |

### 16.8 Volumetrics_v2 清单

| 类别 | 数量 |
|------|------|
| 演示关卡 (.umap) | 6 |
| 基础材质 | 10+ |
| 截面纹理 (T_CrossSection*) | 20+ |
| 杂项纹理 | 50+ |
| Niagara 系统 | 4 |

---

## 参考资源

- **Unreal Engine Marketplace**: FluidNinjaLive 产品页面
- **官方文档**: 插件自带的 Help.uasset 包含内置帮助
- **演示关卡**: Volumetrics_v2 的 6 个演示关卡提供完整参考
- **教程内容**: Tutorial 目录提供丰富的学习资源
- **项目 AGENTS.md**: 项目根目录的 AGENTS.md 包含项目配置信息

---

> **文档生成日期**: 2026-07-16  
> **适用项目**: MyProject_5_8 (UE 5.8)  
> **文档维护**: 基于项目本地资产结构自动整理
