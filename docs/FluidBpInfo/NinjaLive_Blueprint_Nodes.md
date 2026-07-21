# NinjaLive Blueprint 节点功能分析

> **蓝图路径：** `/Game/FluidNinjaLive/NinjaLive.NinjaLive`
> **父类：** Actor
> **图表面板：** EventGraph（245 节点）、UserConstructionScript（41 节点）

---

## 一、蓝图总览

此 Blueprint 是 NinjaLive 流体仿真的 Actor 层封装。它负责管理：

- 子组件（TraceMesh、InteractionVolume、ActivationVolume、EditorIcon、NinjaLiveComponent）的初始化和可见性
- Pawn 接近激活/停用仿真
- 用户输入和重叠交互的转发
- Actor 级参数到 NinjaLiveComponent 的覆盖同步
- 调试信息输出

---

## 二、事件（Events）

### 1. Event Tick（Event Tick）

Tick 事件驱动每帧的行为，包括：
- 获取 `DeltaSeconds` 并存储到变量
- 触发 `MousePressed` 状态检测
- 调用组件层面的交互逻辑

**关联关键节点：**
- `Get DeltaSeconds` → `Set DeltaSeconds`
- 分支判断 `DisableBlueprint` — 若禁用则跳过所有 Tick 逻辑
- 调用 NinjaLiveComponent 上的接口

### 2. Event BeginPlay（Event BeginPlay）

游戏启动时执行一次，完成所有初始化工作。执行流程如下：

```
BeginPlay
  → 判断 DisableBlueprint（若 true 则跳过）
  → SetInitialVisibility_2（初始化组件可见性）
  → 设置 InteractionVolume 引用
  → OverrideNinjaLiveComponentVariableValues（推送 Actor 参数到组件）
  → ClearNinjaLiveComponentInteractionLists（清空交互列表）
  → InteractionVolumeTransform（配置交互体积变换）
  → [分支] SimActivatedByPawnProximity
      → Set Pawn Proximity behaviour（配置接近检测）
  → InitialOverlapCheck（初始重叠检测）
  → BeginOverlapDetection（开始重叠检测）
  → EndOverlapDetection（结束重叠检测）
  → MouseAndTouch（鼠标/触摸输入初始化）
  → PRINT STATUS（打印初始化状态）
  → Bind Event to Owner RePlay Event（绑定重播事件）
  → [Switch] TraceMeshInactiveBehaviour（根据非活跃行为枚举切换）
      → 设置 TraceMesh 可见性、材质、碰撞响应等
```

### 3. RePlay（自定义事件）

用于重新启动仿真：
- 由 `OwnerRePlayEvent` 委托触发
- 重置各种仿真状态变量
- 内部使用 Sequence 节点协调多个重置步骤

---

## 三、碰撞图节点（Composite / Collapsed Graphs）

这些是 EventGraph 中折叠的子图，每个封装了独立的功能模块：

### 3.1 SetInitialVisibility_2

**功能：** 初始化所有子组件的编辑器可见性

**内部逻辑：**
1. 判断 `DisableBlueprint` → 若禁用则跳过
2. **TraceMesh：**
   - 读取 `ShowTraceMeshInEditor`
   - 调用 `SetVisibility` 控制编辑器中显示/隐藏
   - 读取 `TraceMeshSize` → `vector * float` 缩放 → `SetWorldScale3D`
3. **ActivationVolume：**
   - 读取 `ShowActivationVolumeInEditor`
   - 调用 `SetVisibility`
   - 读取 `ActivationVolumeSize` → `SetBoxExtent`
4. **InteractionVolume：**
   - 读取 `ShowInteractionVolumeInEditor`
   - 调用 `SetVisibility`
   - 读取 `InteractionVolumeSize` → `SetBoxExtent`
5. **EditorIcon（MaterialBillboard）：**
   - `UseTraceMeshAsInteractionVolume` 转整数 → 通过 Make Array 和 Get (a copy) 索引选择
   - `SetMaterial` 设置相应材质

### 3.2 OverrideNinjaLiveComponentVariableValues

**功能：** 将 Actor 上的"Live Component Overrides"类参数推送到 NinjaLiveComponent

**覆盖的参数包括：**
- `OutputFilterMaterialIndex`
- `ShowLODdebugMessagesOnScreen`
- `GlobalBrushScale`
- `LOD1-ReduceIterations`
- `LOD2-ReduceSamplingFPS`
- `LOD3-ReduceCollisionAmount`
- `MinSamplingFPS`
- `LOD-FarBound`
- `LOD-NearBound`
- `DownscaleCollisionPainterResolution`
- `DownscalePressureResolution`
- `SingleTargetMode`

### 3.3 ClearNinjaLiveComponentInteractionLists

**功能：** 清空 NinjaLiveComponent 上的交互追踪列表，为新一帧或初始化做准备

**清理的列表：**
- `OverlappingComponents`
- `OverlappingActors`
- `SkeletalMesh-TempArray-Pairs`
- `OverlappingActorsInitial`
- `ContinuousInteractionSkeletalComponent`
- 等

### 3.4 InteractionVolumeTransform

**功能：** 设置交互体积（InteractionVolume）的变换和引用

**内部逻辑：**
1. `Get InteractionVolume` + `Get TraceMesh` → `Make Array`
2. `Get (a copy)` → 默认选中第一个
3. `Set InteractionVolumeTemplate` — 将数组元素存储为模板引用
4. `UseTraceMeshAsInteractionVolume` → `To Integer (Boolean)` — 用于后续索引选择

### 3.5 Set Pawn Proximity behaviour

**功能：** 实现 Pawn 接近检测循环，控制仿真激活/停用

**两个输入执行线：**

**输入 In1（初始化时配置）：**
1. 获取 `SimActivatedByPawnProximity` → Branch
2. 若启用：
   - 获取 `ActivationVolume`
   - 获取 `ActivatorProximityCheckFrequency`
   - 调用 `Delay`（按检测频率循环）
   - 获取 `OverlapBasedInteraction` → Branch
   - 若启用重叠检测：
     - 获取 `Activator` → `IsOverlappingActor`（判断激活器是否在激活体积内）
     - 设置 `PawnInsideActivationBounds`
     - 调用 `PRINT STATUS`
   - 循环回到 Delay

**输入 In2（外部触发时）：**
- 通过 Sequence 多路输出
- 设置 `PawnInsideActivationBounds` 的不同状态

### 3.6 PRINT STATUS

**功能：** 调试输出激活状态信息

**输入参数：** `In`（bool 值，PawnInsideActivationBounds）

**内部逻辑：**
1. 读取 `ActivationEventsDebugPrint` → 若关闭则跳过
2. 读取 `SaveDebugTextToLog` 控制输出目标
3. 读取 `DebugTextLifeTimeLength` 控制显示时长
4. 使用 `PrintString` 输出状态信息
5. 额外的字符串拼接：`Self-Reference` → `GetDisplayName` → `Append` → `PrintString`

### 3.7 InitialOverlapCheck

**功能：** 游戏开始时执行首次重叠检测，收集已经在交互体积内的物体

**内部逻辑：**
1. 获取 `ExcludeSpecificActorsFromOverlap`
2. `Append Array` — 将排除列表与当前检测合并
3. 遍历场景中的 Actor 检测是否重叠

### 3.8 BeginOverlapDetection

**功能：** 处理重叠开始的逻辑（当物体进入交互体积时）

- 由 `InitialOverlapCheck` 的 then 输出触发
- 内部执行向 NinjaLiveComponent 注册重叠物体的流程

### 3.9 EndOverlapDetection

**功能：** 处理重叠结束的逻辑（当物体离开交互体积时）

- 由 `BeginOverlapDetection` 的 then 输出触发
- 内部执行从 NinjaLiveComponent 移除重叠物体的流程

### 3.10 MouseAndTouch

**功能：** 处理鼠标和触摸输入

**内部逻辑：**
1. 获取 `UserInputBasedInteraction` 枚举
2. Switch 枚举值，根据不同的输入模式执行对应的处理逻辑
3. 获取 `NinjaLiveComponent` 引用
4. 设置 `UserInputBasedInteraction` 和 `OverlapBasedInteraction` 到 NinjaLiveComponent
5. 设置交互模式和参数

### 3.11 UnusedMethods

**功能：** 存放不再使用但保留的遗留方法

- 无输入/输出引脚（断开的子图）
- 用于保留旧逻辑供参考

---

## 四、UserConstructionScript（41 节点）

构建脚本在编辑器中每次 Actor 变更时执行，与 EventGraph 的 BeginPlay 逻辑类似但专门用于编辑器中的可视化更新。

**执行流程：**

```
ConstructionScript
  → [分支] DisableBlueprint（若禁用则跳过）
  → 设置 InteractionVolumeTemplate
  → Vector * float 缩放 → SetWorldScale3D（TraceMeshSize）
  → [分支] ShowTraceMeshInEditor → SetVisibility（TraceMesh）
  → 设置 InteractionVolume BoxExtent
  → [分支] ShowInteractionVolumeInEditor → SetVisibility（InteractionVolume）
  → 设置 ActivationVolume BoxExtent
  → [分支] ShowActivationVolumeInEditor → SetVisibility（ActivationVolume）
  → [分支] UseTraceMeshAsInteractionVolume → Boolean → Integer
  → Make Array（材质选择）→ Get (a copy) → SetMaterial（EditorIcon）
```

---

## 五、变量列表（按分类）

### 5.1 Live Activation（激活控制）

| 变量名 | 类型 | 说明 |
|---|---|---|
| DisableBlueprint | bool | 完全禁用蓝图逻辑 |
| SimActivatedByPawnProximity | bool | Pawn 接近时自动激活仿真 |
| ShowActivationVolumeInEditor | bool | 编辑器中显示激活体积线框 |
| ActivationVolumeSize | Vector | 激活体积的缩放值 |
| ActivatorProximityCheckFrequency | double | 接近检测频率（秒） |
| ActivatorType | ECollisionChannel | 激活器碰撞通道类型 |
| Activator | Actor | 当前激活的 Actor 引用 |
| TraceMeshInactiveBehaviour | InactiveBehaviour_Enum | 非活跃时 TraceMesh 行为 |

### 5.2 Live Interaction（交互控制）

| 变量名 | 类型 | 说明 |
|---|---|---|
| ShowTraceMeshInEditor | bool | 编辑器中显示 TraceMesh |
| TraceMeshSize | Vector | TraceMesh 三维缩放 |
| UserInputBasedInteraction | UserInput_Enum | 用户输入交互模式 |
| OverlapBasedInteraction | bool | 基于重叠的交互 |
| ShowInteractionVolumeInEditor | bool | 编辑器中显示交互体积 |
| InteractionVolumeSize | Vector | 交互体积缩放 |
| UseTraceMeshAsInteractionVolume | bool | TraceMesh 兼任交互体积 |
| TrackActorPrimitiveComponentsWithTag | name | 追踪带此标签的基础组件 |
| TrackActorSkeletalMeshComponentsWithTag | name | 追踪带此标签的骨骼组件 |
| OverlapFilterInclusiveObjType | Array\<EObjectTypeQuery\> | 包含的物体类型过滤 |
| OverlapFilterInclusiveBoneNameExact | Array\<name\> | 精确骨骼名过滤 |
| OverlapFilterInclusiveBoneNamePartial | Array\<string\> | 骨骼名部分匹配过滤 |
| ExcludeSpecificActorsFromOverlap | Array\<Actor\> | 排除的特定 Actor |
| AutoExcludeLargeOverlappingObjects | bool | 自动排除大型重叠物体 |
| MultipleTouchLookup | Array\<bool\> | 多点触控查找表 |
| MousePressed | bool | 鼠标按下状态 |
| ForceTrackObjectsWithNocollisionFlag | bool | 强制追踪无碰撞标记物体 |
| ForceTrackBonesWithSimilarName | bool | 强制追踪相似名称骨骼 |

### 5.3 Live Activation（内部）

| 变量名 | 类型 | 说明 |
|---|---|---|
| PawnInsideActivationBounds | bool | Pawn 是否在激活范围内 |
| InitDone | bool | 初始化是否完成 |
| BeginPlaySupressed | bool | BeginPlay 是否被抑制 |

### 5.4 Live Debug（调试）

| 变量名 | 类型 | 说明 |
|---|---|---|
| ActivationEventsDebugPrint | bool | 打印激活事件调试信息 |
| SimContainerCapacityWarning | bool | 容器容量警告 |
| SaveDebugTextToLog | bool | 保存调试文本到日志 |
| DebugTextLifeTimeLength | double | 调试文本显示时长 |

### 5.5 Live Component Overrides（组件覆盖参数）

| 变量名 | 类型 | 说明 |
|---|---|---|
| OverrideComponentVariables | bool | 启用变量覆盖 |
| OutputFilterMaterialIndex | int | 输出过滤材质索引 |
| ShowLODdebugMessagesOnScreen | bool | 屏幕显示 LOD 调试信息 |
| GlobalBrushScale | double | 全局笔刷缩放 |
| LOD1-ReduceIterations | bool | LOD1 减少迭代 |
| LOD2-ReduceSamplingFPS | bool | LOD2 降低采样帧率 |
| LOD3-ReduceCollisionAmount | bool | LOD3 减少碰撞量 |
| MinSamplingFPS | int | 最低采样帧率 |
| LOD-FarBound | double | LOD 远边界 |
| LOD-NearBound | double | LOD 近边界 |
| DownscaleCollisionPainterResolution | int | 降低碰撞绘制分辨率 |
| DownscalePressureResolution | int | 降低压力分辨率 |
| SingleTargetMode | bool | 单目标模式 |

### 5.6 Live System（系统内部）

| 变量名 | 类型 | 说明 |
|---|---|---|
| RT_DensityPreview | TextureRenderTarget2D | 密度预览渲染目标 |
| InactiveGrayMaterial | MaterialInstance | 非活跃灰色材质 |
| TimeCounterForBrush | double | 笔刷计时器 |
| DeltaSeconds | double | 帧时间差 |
| Time | double | 累计时间 |
| TickRateCustom | double | 自定义 Tick 率 |
| NinjaLIVECollisionExclude | Array\<NinjaLive_C\> | 碰撞排除的 Actor 列表 |
| SKMeshTagged | bool | 骨骼网格已标记 |
| OwnerRePlayEvent | MCDelegate | 所有者重播事件委托 |

### 5.7 Live Components（组件引用）

| 变量名 | 类型 | 说明 |
|---|---|---|
| InteractionVolumeTemplate | PrimitiveComponent | 交互体积模板引用 |
| OverlappingSkeletalMesh | PrimitiveComponent | 当前重叠的骨骼网格 |

### 5.8 Live Interface Helpers（接口辅助）

| 变量名 | 类型 | 说明 |
|---|---|---|
| BrushStrengthTemp2 | double | 笔刷强度临时值 2 |
| InputFeedbackTemp1 | double | 输入反馈临时值 1 |
| DisableAndNotTickBlock | bool | 禁用且不 Tick 阻塞 |

---

## 六、EventGraph 执行流总图

```
Event BeginPlay
  │
  ├─ DisableBlueprint? ──True──→ (跳过所有)
  │
  ├─ SetInitialVisibility_2
  │   ├─ TraceMesh: 可见性 + 缩放
  │   ├─ ActivationVolume: 可见性 + BoxExtent
  │   └─ InteractionVolume: 可见性 + BoxExtent + EditorIcon 材质
  │
  ├─ InteractionVolumeTransform
  │   ├─ 构建组件数组
  │   └─ 设置 InteractionVolumeTemplate
  │
  ├─ OverrideNinjaLiveComponentVariableValues
  │   └─ 将 13 个覆盖参数推送到 NinjaLiveComponent
  │
  ├─ ClearNinjaLiveComponentInteractionLists
  │   └─ 清空所有交互追踪列表
  │
  ├─ SimActivatedByPawnProximity? ──True──→ Set Pawn Proximity behaviour
  │                                              └─ 循环检测 Delay → IsOverlappingActor
  │
  ├─ InitialOverlapCheck → BeginOverlapDetection → EndOverlapDetection
  │
  ├─ MouseAndTouch
  │   └─ 设置输入模式到 NinjaLiveComponent
  │
  ├─ PRINT STATUS
  │
  ├─ Bind RePlay Event
  │
  └─ [Switch] TraceMeshInactiveBehaviour
      ├─ NewEnumerator0: 隐藏 + 关闭碰撞
      ├─ NewEnumerator1: 设置非活跃材质
      ├─ NewEnumerator2: 设置 MI_Output + 非活跃材质
      └─ 其他枚举: 不同可见性/材质/碰撞组合

Event Tick
  │
  ├─ DisableBlueprint? ──True──→ (跳过)
  │
  ├─ 更新 DeltaSeconds
  └─ Tick 转发到 NinjaLiveComponent

RePlay
  │
  └─ Sequence: 重置仿真状态
      ├─ 重置激活状态
      ├─ 清除交互列表
      └─ 触发组件重播
```

---

*文档生成时间：2026-07-21 | 数据来源：MCP CDO + Blueprint Graph 查询*
