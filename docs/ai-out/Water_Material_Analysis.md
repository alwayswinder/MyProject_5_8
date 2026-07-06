# Water_Material 详细分析报告

> 分析日期：2026-07-06
> 资源路径：`/Game/_MyTest/WaterTest/Water_Material`

---

## 基本信息

| 属性 | 值 |
|---|---|
| **路径** | `/Game/_MyTest/WaterTest/Water_Material` |
| **类型** | `Material` |
| **着色模型** | **Single Layer Water**（单层水） |
| **输出节点** | `MaterialExpressionSingleLayerWaterMaterialOutput` |
| **参数组** | `Water Shading` |
| **总表达式数** | **64** 个节点 |

---

## 输出连接（SingleLayerWaterMaterialOutput）

Single Layer Water 材质使用专用的输出节点，只有 **4 个引脚**：

| 引脚 | 连接来源 | 说明 |
|---|---|---|
| **ScatteringCoefficients** | `WaterCoefficientMask` (MF) | 散射系数（经蒙版处理后） |
| **AbsorptionCoefficients** | `WaterCoefficientMask` (MF) | 吸收系数（经蒙版处理后） |
| **PhaseG** | `Anisotropy` = **0.1** | 相位函数各向异性参数 |
| **ColorScaleBehindWater** | StaticSwitch `Caustics` | 焦散开关 → Caustics MF 或 1.0 |

---

## 图形结构（数据流）

整个材质图可以分为 **9 大功能模块**：

### 1. 材质属性初始化

```
WaterAttributes (MF_9) → GetMaterialAttributes → 提取 Refraction 等属性
WaterAttributes (MF_0) → Water_Underside (MF_7) → 底部计算
```

`GetMaterialAttributes` 从材质属性中读取 **Refraction**（折射）供焦散使用。

### 2. 波浪系统（核心）

```
WaterBodyIndex 选择:
  ├─ StaticSwitch_7 True  → ScalarParameter "WaterBodyIndex" (默认 0)
  └─ StaticSwitch_7 False → Custom_4: GetWaterWaveParamIndex(Parameters) (从 Tile 实例获取)

GetWaterTime (MF_12) → ComputeGerstnerWaves (MF_1)
                      输入: Water Body Index, WaterTime
                      输出: WaveAttributes → Reroute_8
```

- **Gerstner 波浪**是水面顶点位移和法线的核心计算
- 波浪属性分支到两条路径：
  - → `WaterTextureSurface`（水面纹理）
  - → `WaveVelocityMask`（波浪速度遮罩）

### 3. 水面纹理与法线

```
WaveAttributes → StaticSwitch_1 (Enable Waves, 默认=true)
  ├─ True:  → WaterTextureSurface (MF_2)
  │           输入: WaveAttributes, WaterTime (MF_17), CameraDepthFade 作为 BlendAlpha
  │           CameraDepthFade: FadeOffset=5000, FadeLength=10000
  └─ False: → StaticSwitch_4 → WaveVelocityMask (MF_18)
               StaticSwitch_5 → WaveVelocityMask / SetMaterialAttributes
```

- 远距离法线过渡：`Water Distant Normal Offset`(5000) + `Water Distant Normal Length`(10000)

### 4. 河流系统

```
SetWaveAttributes (MF_3) → Reroute_0 → Reroute_1 → Reroute_9

StaticSwitch_0 (Enable River, 默认=false):
  ├─ True:  WaterRiverFlowmaps (MF_19) ← MA + GetWaterTime (MF_16)
  └─ False: Reroute_9 (直接传递 SetWaveAttributes 结果)

输出 → Reroute_3 (主材质属性流)
```

### 5. 泡沫系统

```
Reroute_3 → BeachFoam (MF_8)

StaticSwitch_2 (Enable Ocean Foam #1, 默认=false):
  ├─ True:  BeachFoam → SampleFluidSimulation (MF_11) ← GetWaterTime (MF_15)
  └─ False: Reroute_6 → Reroute_2 → Reroute_3 (跳过泡沫，直接传递)
```

### 6. 散射/吸收计算

```
Scattering (VectorParam: 1,1,1,0.5):
  Multiply_0 = Scattering × Scattering (自身平方)

Foam Scattering (VectorParam: 1,1,1,5):
  Multiply_3 = FoamScattering × FoamScattering
  Multiply_2 = BeachFoam.FoamScattering × Multiply_3

StaticSwitch_3 (Enable Ocean Foam #2, 默认=false):
  ├─ True:  Multiply_2 (泡沫散射)
  └─ False: Constant_1 (0)

Add_2 = StaticSwitch_3 + Multiply_0
Divide_5 = Add_2 / 1000 (ConstB=1000) → Water_Underside (MF_10) Scattering

Absorption (VectorParam: 10,150,350,8):
  Divide_2 = 1 / Absorption.rgb (ConstA=1)
  Divide_1 = Divide_2 / Absorption.A (alpha=8) → Water_Underside (MF_10) Absorption
```

- **Absorption 默认值 `(10, 150, 350)`** 意味着：红色吸收少（穿透深），蓝色吸收多（穿透浅），这是**清澈水体**的典型设置
- 水体越深越呈现蓝色

### 7. 焦散系统

```
GetMaterialAttributes (Refraction) → WaterCaustics_GerstnerWaves (MF_4)

StaticSwitch_8 (Caustics, 默认=false):
  ├─ True:  焦散计算结果
  └─ False: Constant_2 (1.0 = 无效果)
```

### 8. 透明度蒙版与裁剪

```
SampleFluidSimulation → WaterOpacityMaskFromDepth (MF_13) →
  ├─ WaterCoefficientMask (MF_14) → SLW Output (Scattering/Absorption)
  └─ ClipDynamicBounds (MF_20) → Reroute_7

Reroute_7 → Reroute_4 → Reroute_5

StaticSwitch_6 (Enable Water VS Mapping, 默认=true):
  ├─ True:  WaterHeightMappingVS (MF_6) ← Reroute_7
  └─ False: Reroute_5 → SetMaterialAttributes
```

### 9. 顶点着色器输出

```
SetMaterialAttributes_0:
  ├─ MaterialAttributes: (未连接)
  ├─ Normal: Constant3Vector (0, 0, 1) — 纯蓝，即默认法线
  └─ WorldPositionOffset: Constant_0 (0)
```

---

## 参数汇总

### Scalar 参数

| 参数名 | 默认值 | 分组 | 用途 |
|---|---|---|---|
| **Anisotropy** | 0.1 | Water Shading | Phase G 各向异性（散射方向性） |
| **WaterBodyIndex** | 0 | None | 水体索引（用于多水体场景） |
| **Water Distant Normal Offset** | 5000 | None | 远距离法线过渡偏移 |
| **Water Distant Normal Length** | 10000 | None | 远距离法线过渡长度 |

### Vector 参数

| 参数名 | 默认值 (R,G,B,A) | 用途 |
|---|---|---|
| **Scattering** | (1, 1, 1, 0.5) | 散射系数 |
| **Absorption** | (10, 150, 350, 8) | 吸收系数（清澈水体） |
| **Foam Scattering** | (1, 1, 1, 5) | 泡沫散射系数 |

### Static Switch 参数（功能开关）

| 参数名 | 默认值 | 功能 |
|---|---|---|
| **Enable River** | ❌ false | 启用河流流动贴图 |
| **Enable Waves** | ✅ true | 启用 Gerstner 波浪 |
| **Enable Ocean Foam** (#1) | ❌ false | 启用海洋泡沫（BeachFoam → FluidSim） |
| **Enable Ocean Foam** (#2) | ❌ false | 泡沫散射叠加到散射计算 |
| **Enable Lake Transition** | ❌ false | 启用湖泊过渡 |
| **Enable Ocean Transition** | ❌ false | 启用海洋过渡 |
| **Enable Water VS Mapping** | ✅ true | 启用水体顶点着色高度映射 |
| **UseScalarWaterBodyIndex** | ❌ false | 使用标量索引 vs Tile 实例自动获取 |
| **Caustics** | ❌ false | 启用水下焦散效果 |

---

## 引用的 Material Function

| 函数 | 调用次数 | 用途 |
|---|---|---|
| `WaterAttributes` | 2 | 初始化默认水体材质属性 |
| `ComputeGerstnerWaves` | 1 | Gerstner 波浪位移计算 |
| `WaterTextureSurface` | 1 | 水面纹理和法线生成 |
| `SetWaveAttributes` | 1 | 将波浪属性写入材质属性集 |
| `Water_Underside` | 2 | 计算水面底部着色 |
| `CameraDepthFade` | 1 | 基于深度的远近混合 |
| `BeachFoam` | 1 | 海滩泡沫生成 |
| `SampleFluidSimulation` | 1 | 流体模拟采样（泡沫动画） |
| `WaterOpacityMaskFromDepth` | 1 | 基于深度的不透明度蒙版 |
| `WaterCoefficientMask` | 1 | 散射/吸收系数的水体类型蒙版 |
| `WaterCaustics_GerstnerWaves` | 1 | Gerstner 波浪焦散计算 |
| `WaveVelocityMask` | 1 | 波浪速度场蒙版 |
| `WaterRiverFlowmaps` | 1 | 河流流动贴图 |
| `WaterHeightMappingVS` | 1 | 顶点着色器高度映射 |
| `ClipDynamicBounds` | 1 | 动态边界裁剪 |
| `GetWaterTime` | 4 | 获取水体时间（用于动画） |

---

## 技术要点总结

1. **Single Layer Water** 是 UE5 的专用水体着色模型，使用物理基础的散射/吸收系数替代传统的 BaseColor/Metallic/Roughness
2. **散射 (Scattering)** 和 **吸收 (Absorption)** 是控制水体颜色的核心参数，遵循 Beer-Lambert 定律
3. **Absorption `(10, 150, 350)`** 表示红光吸收系数低（穿透深）、蓝光吸收系数高（穿透浅）—— 这是深水呈现蓝色的物理原理
4. **PhaseG = 0.1** 表示轻微前向散射（Henyey-Greenstein 相位函数）
5. 大量使用 **StaticSwitch** 实现功能模块化：默认只开启 Waves + VS Mapping，其他效果按需启用
6. 材质使用 UE 内置的 **Water Plugin** 函数库（`/Water/Materials/`），这是引擎官方水体渲染方案

---

## 数据流总览图

```
[WaterTime ×4] ──→ GerstnerWaves ──→ WaveAttributes ──→ WaterTextureSurface ──→ SetWaveAttributes
                                                        │                            │
                                                        │                      [Enable River?]
                                                        │                       ├─ True: WaterRiverFlowmaps
                                                        │                       └─ False: pass-through
                                                        │                            │
                                                  WaveVelocityMask              [Enable Ocean Foam?]
                                                                                 ├─ True: BeachFoam → FluidSim
                                                                                 └─ False: pass-through
                                                                                      │
                                                                              WaterOpacityMaskFromDepth
                                                                                   │            │
                                                                        WaterCoefficientMask   ClipDynamicBounds
                                                                              │                    │
                                                                        [SLW Output]     WaterHeightMappingVS
                                                                                              │
                                                                                      [VS Output]
```
