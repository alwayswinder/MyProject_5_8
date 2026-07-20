# C++ FluidNinjaLive 复刻进度总结

> 更新日期: 2026-07-20
> 对应目标: 将 BP 版 NinjaLive.NinjaLive + NinjaLiveComponent.NinjaLiveComponent 复刻为 C++

---

## 已完成 ✅

### 项目结构
- `UMyNinjaLiveComponent` — 核心流体仿真 ActorComponent（C++）
- `ASimpleFluidActor` — 包装 Actor（C++），含 DisplayPlane、ActivationVolume、InteractionVolume、EditorIcon

### 材质系统 — 确认正常工作
- ✅ 显示材质使用 `M_NinjaOutput_Basic` 基础材质（非 MIC，绕过静态覆写）
- ✅ 纹理参数正确绑定：
  - `VelocityDensityBuffer` → `RT_VelocityA`
  - `PaintBuffer` → `RT_Collision`
  - `PressureBuffer` → `RT_Pressure`
  - `DivergenceBuffer` → `RT_Divergence`
- ✅ Canvas 直接绘制可在水面上显示移动白色方块（RT_VelocityA 内容正确显示）
- ✅ StepUpdateDisplay 每帧更新 `VelocityDensityBuffer` 和 `PaintBuffer`

### 参数对齐 BP Pool_0
| 参数 | C++ 当前值 | BP Pool_0 值 |
|------|-----------|--------------|
| ResolutionX/Y | 1600×1600 | 1600×1600 |
| GlobalBrushScale | 4.0 | 4.0 |
| UserInputBrushScale | 1.2 | 1.2 |
| UseCustomTraceSource | true | true |
| CustomTraceSourcePosition | (200,200,5000) | (200,200,5000) |
| ActivationDistance | 100000 | 2000（C++ 扩大避免阻断） |
| OverlapFilterInclusiveObjType | WorldDynamic, Pawn, PhysicsBody | WorldDynamic, Pawn, PhysicsBody |
| OverlapFilterInclusiveBoneNameExact | "calf_r", "calf_l" | "calf_r", "calf_l" |
| CompositeGradientMat | MI_CompositeAndGradient_HDnoise | MI_CompositeAndGradient_HDnoise |
| DisplayMat | M_NinjaOutput_Basic (base) | MI_DensityBuffer_Red (MIC) |

### 管线状态
- ✅ 激活检测不再阻断仿真（始终运行）
- ✅ 种子帧（前 3 帧）用 Canvas 绘制白色方块
- ✅ 主循环用 Canvas 持续绘制白色方块

---

## 运行日志确认 ✅
```
[MyNinjaLiveComponent]  DisplayMat       = MI_DensityBuffer_Red
[MyNinjaLiveComponent]  AdvectionMat     = MI_Advection
[MyNinjaLiveComponent]  CompositeGradient= MI_CompositeAndGradient_HDnoise
[MyNinjaLiveComponent]  Resolution       = 1600x1600
[MyNinjaLiveComponent] SetupDisplay — Using base material M_NinjaOutput_Basic
[MyNinjaLiveComponent] SetupDisplay — 5 texture params found
[SetupDisplay] PaintBuffer = RT_Collision
[SetupDisplay] PressureBuffer = RT_Pressure
[SetupDisplay] VelocityDensityBuffer = RT_VelocityA
[SetupDisplay] DivergenceBuffer = RT_Divergence
[SetupDisplay] FlowMapTexture = RT_VelocityA
[Seed] density injected
Pipeline OK — frame=60
```

---

## 待修复 ❌

### 核心问题：仿真管线不产生可见数据
**根因：** C++ 的 `MID_CollisionPainterDot` / `MID_DensityInject` 写入格式与 `MI_CompositeAndGradient_HDnoise` 材质期望的格式不匹配。原版 BP 使用 `NS_Painter_v2_Dot` Niagar 系统写入 RT_Collision，材质需要 Niagar 输出的特定数据格式。

**当前状态：** 管线跑但速度场始终为零（MI_DensityBuffer_Red 无密度变化）

### 待实现功能
| 功能 | 优先级 | 说明 |
|------|--------|------|
| Canvas 持续绘制 | P0 | ✅ 已实现，种子帧 + 主循环都可用 |
| Niagar 集成 | P1 | 使用 `NS_Painter_v2_Dot` 替代手动绘制 |
| 完整仿真管线 | P1 | Advection → CompositeGradient → Divergence → PressureSolve |
| 预设加载系统 | P2 | 从 `DT_Usecase_Pool2` 读取仿真参数 |
| MemoryPool 集成 | P3 | `bAutoConnectToMemoryPool` 标记已设但无实现 |
| LOD 系统 | P3 | 距离降分辨率/降 FPS 逻辑存在但未验证 |
| 交互检测 | P1 | ActivationVolume/InteractionVolume 重叠事件已绑定，需要验证 |

---

## 已知问题

1. **材质写入无效：** `DrawMaterialToRenderTarget` 配合 `MID_CollisionPainterDot` 和 `MID_DensityInject` 无法在 RT_Collision / RT_VelocityA 上产生可被 CompositeGradient 识别的数据
2. **Canvas 是临时方案：** 目前用 `UCanvas::K2_DrawTexture` 直接画白方块，不是真正的流体仿真
3. **原始仿真管线代码被注释：** `RunSimulationPipeline` 中 Canvas 绘制后直接 `return`，跳过了 Advection/CompositeGradient/PressureSolve/Display 等全部步骤

---

## 推荐下一步

### 方案 A：集成 Niagar（推荐）
1. 在 `Build.cs` 中添加 `"Niagara"` 模块
2. 在 `ASimpleFluidActor` 中创建 `UNiagaraComponent`
3. 加载 `NS_Painter_v2_Dot` Sistem 并写入 `RT_Collision`
4. 触发粒子发射替代 Canvas 绘制

### 方案 B：修复 Canvas 为密度注入
1. 用 Canvas 直接在 RT_VelocityA 上画圆形（模拟密度注入）
2. 恢复 `StepAdvection` 平流步骤
3. 逐步恢复剩余管线

### 方案 C：最小可行仿真
1. 只保留 Canvas 绘制 + 平流
2. 用 Canvas 画圆然后让平流材质"拖动"它们
3. 产生最基本的流体视觉效果

---

## 文件清单

| 文件 | 行数 | 作用 |
|------|------|------|
| `Source/.../FluidTest/MyNinjaLiveComponent.h` | ~560 | 组件的声明和属性 |
| `Source/.../FluidTest/MyNinjaLiveComponent.cpp` | ~900 | 组件的实现 |
| `Source/.../FluidTest/SimpleFluidActor.h` | ~210 | Actor 声明 |
| `Source/.../FluidTest/SimpleFluidActor.cpp` | ~360 | Actor 实现 |
| `Source/.../MyProject_5_8.Build.cs` | ~15 | 模块依赖 |
| `docs/ai-out/cxx-bp-comparison.md` | ~170 | BP 与 C++ 对照分析 |
