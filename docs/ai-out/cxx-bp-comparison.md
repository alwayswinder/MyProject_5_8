# C++ 版 FluidNinjaLive 与蓝图原版对照分析

> 生成日期: 2026-07-20
> C++ 组件: `UMyNinjaLiveComponent` (Source/MyProject_5_8/FluidTest/)
> C++ Actor:  `ASimpleFluidActor` (Source/MyProject_5_8/FluidTest/)
> BP 组件:   `/Game/FluidNinjaLive/NinjaLiveComponent.NinjaLiveComponent`
> BP Actor:   `/Game/FluidNinjaLive/NinjaLive.NinjaLive`

---

## 1. Actor 结构对比

| 项目 | BP 原版 NinjaLive | C++ 版 ASimpleFluidActor | 状态 |
|---|---|---|---|
| Root | SceneComponent | SceneComponent | ✅ 一致 |
| 显示/Trace Mesh | `TraceMesh` (StaticMesh, `bVisible=false`) | `DisplayPlane` (StaticMesh, `bVisible=true`) | ⚠️ 设计不同 |
| 激活体积 | `ActivationVolume` (Box, 覆盖平面区域) | `ActivationVolume` (Box) | ✅ 一致 |
| 交互体积 | `InteractionVolume` (Box, 极薄 Z=5) | `InteractionVolume` (Box) | ✅ 一致 |
| 编辑器图标 | `EditorIcon` (MaterialBillboard) | 无 | ❌ 缺失 |
| 核心组件 | `NinjaLiveComponent` (BP) | `NinjaLiveComponent` (C++) | ✅ 一致 |

**关键差异**: 原版 TraceMesh 是**不可见**的（`bVisible=false`），仅用于碰撞检测和渲染目标输出。显示功能由 NinjaLiveComponent 内部管理。C++ 版的 DisplayPlane 既是碰撞面也是显示面。

---

## 2. 组件属性对比

### 2.1 仿真参数

| 属性 | BP 原版 | C++ 版 | 状态 |
|---|---|---|---|
| 分辨率 | `resolutionX/Y` 1600×1600 | `ResolutionX/Y` 512×512 (默认) | ⚠️ C++ 分辨率较低 |
| 仿真 FPS | `simFPS` (事件驱动/固定帧率) | `SimFPS` (0=每帧) | ✅ 一致 |
| LOD | 远近距离 LOD + 降低质量/FPS | 无 | ❌ 缺失 |
| 内存池 | `autoConnectToMemoryPool-IF-Found` | 无 | ❌ 缺失 |
| 预设表 | `defaultPreset` (DataTable) | 无 | ❌ 缺失 |

### 2.2 画笔参数

| 属性 | BP 原版 | C++ 版 | 状态 |
|---|---|---|---|
| 全局画笔缩放 | `globalBrushScale` = 4.0 | 无（只有 BrushSize=0.02） | ⚠️ 坐标系不同 |
| 用户输入缩放 | `userInputBrushScale` = 1.2 | 无 | ❌ |
| 跟随交互对象尺寸缩放 | `brushScaledByInteractingObjSize` | 无 | ❌ |
| 低速阻尼 | `dampenBrushBelowThisVelocity` = 0.01 | 无 | ❌ |

### 2.3 碰撞/追踪

| 属性 | BP 原版 | C++ 版 | 状态 |
|---|---|---|---|
| 追踪方式 | 自定义 Trace 通道名称 | `ObjectTypes` + `TraceChannel` | ⚠️ 不同机制 |
| 追踪距离 | `traceDistance` | `TraceDistance` = 5000 | ✅ |
| 自定义追踪源 | `useCustomTraceSource` + 偏移 | 相机位置 | ⚠️ 不同 |
| 碰撞遮罩 | `collisionMask` (Texture2D) | 无 | ❌ 缺失 |

### 2.4 材质引用

| 属性 | BP 原版 | C++ 版 | 状态 |
|---|---|---|---|
| 核心仿真材质数组 | `coreSimMaterials` (4项) | 独立属性 (AdvectionMat 等) | ⚠️ 风格不同 |
| 输出材质数组 | `output_materials` (8项) | `DisplayMat` (单项) | ⚠️ 简化 |
| 输出材质选择 | `outputMaterialSelected` | 无 | ❌ |

---

## 3. 仿真管线对比

### 3.1 管线步骤

| 步骤 | BP 原版 | C++ 版 | 参数名修复 |
|---|---|---|---|
| 1. 碰撞绘制 | CollisionPainter | `StepCollisionPainter` | ✅ Position/Velocity 等 |
| 2. 密度注入 | 集成在 Painter 中 | `StepInjectDensity` | ✅ Position/Velocity/BrushSize 等 |
| 3. 平流 (密度) | M_Advection → `"Texture"` | `StepAdvection` → `"Texture"` | ✅ 已修复 |
| 4. 平流 (速度) | M_Advection → `"Texture"` | `StepAdvection` → `"Texture"` | ⚠️ 当前注释掉 |
| 5. 外力合成 | M_CompositeAndGradient | `StepCompositeGradient` | ✅ VeloInputTexture, VeloPainter |
| 6. 散度 | M_Divergence → `"Texture"` | `StepDivergence` → `"Texture"` | ✅ 已修复 |
| 7. 压力求解 | M_Pressure → `"Texture"` | `StepPressureSolve` → `"Texture"` | ✅ 已修复 |
| 8. 显示 | `output_materials` → `"VelocityDensityBuffer"` | `StepUpdateDisplay` → `"VelocityDensityBuffer"` | ✅ 已修复 |

### 3.2 纹理参数名映射（全部已修复）

| 材质 | BP 原版参数 | 修复前 C++ (错误) | 修复后 C++ (正确) |
|---|---|---|---|
| `M_CollisionPainter` | `Position`, `Velocity`, `EdgeMask` | ✅ 正确 | ✅ 正确 |
| `M_Advection` | `Texture` | ❌ `VelocityBuffer`, `DensityBuffer` | ✅ `Texture` |
| `M_CompositeAndGradient` | `VeloInputTexture`, `VeloPainter` | ❌ `VelocityBuffer`, `CollisionBuffer` | ✅ `VeloInputTexture`, `VeloPainter` |
| `M_Divergence` | `Texture` | ❌ `VelocityBuffer` | ✅ `Texture` |
| `M_Pressure` | `Texture` | ❌ `PressureBuffer`, `DivergenceBuffer` | ✅ `Texture` |
| `M_NinjaOutput_Basic` | `VelocityDensityBuffer` | ❌ `DensityBuffer` | ✅ `VelocityDensityBuffer` |

---

## 4. 材质资产引用关系

```
内容浏览器路径:
├── FluidNinjaLive/
│   ├── Core/
│   │   └── FluidSim/
│   │       ├── M_CollisionPainter.M_CollisionPainter            ← 碰撞绘制母材质
│   │       ├── M_Advection.M_Advection                          ← 平流母材质 (仅 "Texture" 参数!)
│   │       ├── M_CompositeAndGradient.M_CompositeAndGradient    ← 外力合成母材质
│   │       ├── M_Divergence.M_Divergence                        ← 散度母材质
│   │       ├── M_Pressure.M_Pressure                            ← 压力求解母材质
│   │       └── MI_Float/
│   │           ├── MI_CollisionPainter_Dot.MI_CollisionPainter_Dot      ← C++ 使用
│   │           ├── MI_Advection.MI_Advection                            ← C++ 使用
│   │           ├── MI_Divergence.MI_Divergence                          ← C++ 使用
│   │           ├── MI_Pressure_Solver1.MI_Pressure_Solver1              ← C++ 使用
│   │           └── MI_CompositeAndGradient_HDnoise.MI_CompositeAndGradient_HDnoise ← C++ 使用
│   └── OutputMaterials/
│       ├── BaseMaterials/
│       │   └── M_NinjaOutput_Basic.M_NinjaOutput_Basic          ← 显示母材质
│       └── Instance_Buffers/
│           └── MI_DensityBuffer_Red.MI_DensityBuffer_Red        ← C++ 使用 (红色密度可视化)
```

---

## 5. 交互系统对比

| 特性 | BP 原版 | C++ 版 | 状态 |
|---|---|---|---|
| 交互检测 | BoxComponent 重叠事件 | 相机 LineTrace | ⚠️ 完全不同的机制 |
| 激活检测 | ActivationVolume 重叠 | CheckPawnProximity (距离) | ⚠️ 不同实现 |
| 过滤方式 | `OverlapFilterInclusiveObjType` + 骨骼名 | ObjectTypes 过滤 | ⚠️ 简化 |
| 多目标追踪 | PV2 追踪点系统 | `SimTargets` 数组 | ⚠️ 简化 |

**关键差异**: 原版通过 Actor 重叠 `InteractionVolume`（极薄 Box，Z=5）来检测交互，支持按骨骼名（如 `calf_l`/`calf_r`）过滤。C++ 版通过相机射线碰撞检测，玩家看向平面时在命中点绘制。

---

## 6. 当前 C++ 实现状态

### 已修复 ✅
- [x] 5 个仿真步骤的材质纹理参数名全部对齐
- [x] `DisplayMat` 和 `CompositeGradientMat` 改为可选材质
- [x] DisplayPlane 默认材质设置（编辑器视口可见）
- [x] `PlaneWorldSize` 自动从网格包围盒计算
- [x] 体积尺寸跟随 PlaneWorldSize 自动同步
- [x] 默认分辨率 512×512、画笔 0.02
- [x] **SetupDisplay 初始化 VelocityDensityBuffer** — 之前只在 StepUpdateDisplay (RunSimulationPipeline 内部) 设置此参数，而管线在没有玩家 Pawn 时提前返回，导致平面材质始终读取默认纹理而非密度 RT。修复后平面无论管线是否运行都能正确读取 RT_DensityA。

### 待修复 ❌
- [ ] **平流步骤**: M_Advection 只有一个 `"Texture"` 参数，期望 RG=Velocity, B=Density。需要先复合速度+密度到单一 RT
- [ ] **压力求解**: M_Pressure 只有一个 `"Texture"` 参数，散度数据需要复合进去
- [ ] **LOD 系统**: 缺少远近 LOD 控制
- [ ] **内存池**: 无 MemoryManager 集成
- [ ] **预设系统**: 无 DataTable 预设支持
- [ ] **交互方式**: 与蓝图原版不同，建议补充重叠式交互

### 当前 Bug 追踪

| 问题 | 状态 | 说明 |
|---|---|---|
| 显示全红 | 待重新编译 | 诊断填充被替换为中心点后需重新编译 |
| 平流破坏密度 | 已跳过 | 平流步骤暂时注释，待实现速度-密度复合 |
| 画笔太小 | 待测试 | 默认 BrushSize=0.02 (2% UV) |
| 参数名不匹配 | 已修复 | 全部 5 个材质参数名已修正 |

---

## 7. 下一步建议

1. **编译测试**: 重新编译，PIE 中观察中心亮点 + 密度轨迹
2. **实现速度-密度复合**: 添加 `RT_VelocityDensity` 中间 RT
3. **恢复平流步骤**: 复合后调用 M_Advection
4. **恢复压力求解**: 复合 Pressure + Divergence 后调用 M_Pressure
5. **删除诊断代码**: 确认管线正常后移除 BeginPlay 中的测试代码
6. **参数调优**: 画笔大小、消散率、分辨率等
