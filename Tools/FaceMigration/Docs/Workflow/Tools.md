# 工具与脚本选择

本页供 AI 执行时查入口。命令从目标项目根运行。统一入口为 `python Tools/FaceMigration/migrate.py`，任务配置在包内 `Jobs/`，运行输出在目标项目 `Saved/FaceMigration/Jobs/<任务>/Current/`。`migration.config.json` 仍是流程导航，角色求解仍以 Seven 为参考；文件可迁移不等于任意角色一键生成。

默认先读取用户指定的示例 `/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation`，保留其控制接口、图、形状、轨道、配色和参考图，再结合 Max 核验动作。新角色按差异重新绑定和定参；从零采样重建属于显式选择的实验路线，不再是默认入口。文档整理任务不连接编辑器执行构建。

## 工具连接

| 任务 | 入口 | 使用要求 |
|---|---|---|
| Max 对象、控制器及场景检查 | 3ds Max MCP 专用工具 | 先发现工具 schema，核对场景完整路径及采样帧；同名歧义必须处理 |
| Max 自定义采样 | Max MCP 执行 MaxScript，或 `Tools/FaceMigration/Scripts/max_run.ps1` | `-ScriptFile <脚本>`；多实例用 `-MaxPid`。采样脚本需保存底层值、异常恢复并读回 |
| UE 资产查询 | 内置 MCP `http://127.0.0.1:8000/mcp`，客户端 `Tools/FaceMigration/Scripts/ue-mcp.ps1` | `-Toolsets` 发现、`-Describe <完整工具集名>` 读 schema，再调用；没有 load_toolset 步骤 |
| UE RigVM / 层级操作 | `python Tools/FaceMigration/migrate.py run <脚本.py> --job <任务>` | PythonScriptPlugin 远程节点可用；先核对项目、网格和配置资产路径 |
| 离线参数拟合 | 普通 Python；部分脚本需 NumPy / SciPy | 先检查输入路径和数据版本；拟合本身不会同步 UE 图 |
| 文档完整性 | `python Tools/FaceMigration/migrate.py check` | 不连接编辑器、不改资产，校验导航、72 项映射、配方覆盖及数据保留；默认不生成报告文件 |

UE 远程节点不可用时，继续做离线资料检查并明确未进行实时检查；不为文档任务重启用户编辑器。发现多个 UE 节点时先明确执行目标，不能仅凭存在 UnrealEditor 进程判断接入正确项目。引擎路径从安装信息、项目关联、`UE_ENGINE_ROOT` 或 `--engine-root` 发现，不再绑定原机器绝对路径。

### 参考契约盘点（UE 5.8 形状 API）

读控制外观用下面这组接口（2026-11-10 在 5.8.4 / CL 58902052 实测）：

| 需要的内容 | 正确入口 | 常见误写 |
|---|---|---|
| 形状名 | `hierarchy.get_control_settings(key).shape_name` | `gizmo_name`（已废弃，仍可读但会告警） |
| 颜色 | `settings.shape_color`（RGBA） | `gizmo_color`（已废弃） |
| 形状尺度 | `hierarchy.get_global_control_shape_transform(key, True).scale3d` | `settings.shape_transform`（示例多为单位变换，尺度实际在全局形状变换里） |
| 局部形状偏移 | `hierarchy.get_global_control_offset_transform(key, True)` | — |
| 形状库 | `asset.get_editor_property('shape_libraries')` 的 `get_path_name()` | 没有 `get_shape_names()`；`ControlRigShapeDefinition` 也没有 `name` |
| 限位显示 | `settings.draw_limits`、`settings.shape_visible` | 不要与控制自身 `scale` 混用 |

`RigHierarchyController` 没有 `get_control_shape_transform`；形状读取在 `RigHierarchy` 上。探测失败时先怀疑接口名，不要先判“示例没有形状设置”。示例当前两个形状库：`/ControlRig/Controls/DefaultGizmoLibraryNormalized` 与项目内 `SL_PhonemeReference`。

## 按依赖查 Seven 脚本

下表是功能索引，**不是可整段重放的安装脚本**。旧构建器多为首次增量添加，目标已经存在时会拒绝；早期眉眼构建会重建节点并影响后续接线。新角色先生成独立参数和任务配置，适配共用脚本；已有完整 Rig 只改本次范围。

| 功能 | Max 探测 / 拟合 | UE 构建 | 验证 / 保存重载 |
|---|---|---|---|
| 眉毛整体、点、弯曲 | `probe_animcraft_world.ms`、`probe_animcraft_rb*.ms`、`probe_animcraft_lb.ms`；见配方源数据 | `build_brow_two.py` → `build_brow_points.py` → `build_brow_bend.py` / `build_brow_bend_left.py` | 先修配方所列 API 和实例断言边界；`fix_brow_exec.py` 仅在已定位断链后使用 |
| 眼周点、上眼皮 | `probe_eye_r.ms` / `probe_eye_l.ms`、`probe_syp*.ms` | `build_eye_points.py`、`build_eye_lid_arc.py` | 早期基线采集：`Tools/FaceMigration/Characters/Seven/Evidence/recheck_control_rig.py`，不代表完整 Rig 验收 |
| 下眼皮 | `probe_xyp_samples.ms`；已存分解参数 | `build_lower_lids.py` | `verify_lower_lids.py` / `reload_lower_lid_target.py` |
| 六嘴唇 | `probe_lip_samples.ms` / `fit_lip_controls.py` | `build_lip_controls.py` | `verify_lip_controls.py` / `reload_lip_target.py` |
| 脸颊 | `probe_cheek_samples.ms` / `fit_cheek_controls.py` | `build_cheek_controls.py` | `verify_cheek_controls.py` / `reload_cheek_target.py` |
| 鼻子整体 | `probe_nose_samples.ms` / `fit_nose_controls.py` | `build_nose_controls.py` | `verify_nose_controls.py` / `reload_nose_target.py` |
| 三嘴控 | `probe_mouth_samples.ms` / `fit_mouth_controls.py` | `build_mouth_controls.py` | `verify_mouth_controls.py` / `reload_mouth_target.py` |
| 单眼目标 | `probe_eyeball_samples.ms` / `fit_eyeball_controls.py` | `build_eyeball_controls.py` | `verify_eyeball_controls.py` / `reload_eyeball_target.py` |
| 双眼总控 | `probe_gaze_master_samples.ms` / `fit_gaze_master.py` | `build_gaze_master.py` | `verify_gaze_master.py` / `reload_gaze_master.py` |
| 下巴 | `probe_chin_samples.ms` / `fit_chin_control.py` | `build_chin_control.py` | `verify_chin_control.py` / `reload_chin_control.py` |
| 整体表情 | `probe_expression_sliders.ms`；已存增益 | `build_expression_sliders.py` | `verify_expression_sliders.py` / `reload_expression_sliders.py` |
| 十二口型 | `probe_phoneme_sliders.ms`；已存参数 | `build_phoneme_sliders.py` | `verify_phoneme_sliders.py` / `reload_phoneme_sliders.py` |
| 口型参考图（示例已有则继承） | 角色贴图、行序、Alpha 边界 | `build_phoneme_reference.py` | `finalize_phoneme_reference.py` / `verify_phoneme_reference_preview.py` |
| 八眼神 | `probe_eye_expression_sliders.ms`；已存参数 | `build_eye_expression_sliders.py` | `verify_eye_expression_sliders.py` / `reload_eye_expression_sliders.py` |
| 鼻侧 | `probe_nose_side_samples.ms` / `fit_nose_side_controls.py` | `build_nose_side_controls.py` | `verify_nose_side_controls.py` / `reload_nose_side_controls.py` |

表中无目录前缀的文件均位于 `Tools/FaceMigration/Scripts/`。具体备份、输入恢复、拟合条件和误差见对应 [配方](../AGENTS.md#文件维护)及 [Seven 角色记录](../../Characters/Seven/README.md)。旧的 `probe_*`、`set_*`、`inspect_*`、`clear_rig.py` 不因出现在 Scripts 中就成为主流程步骤。

## 修复与布局必须按当前状态选择

下表是示例中的历史调整，不表示当前输出候选已经具备这些设置。先读取示例与目标差异；尤其不能对已缩短的下巴再次缩放，也不能把调试限位线当作正式滑轨。

| 已完成需求 | 对应脚本 | 重跑边界 |
|---|---|---|
| 脸颊蓝轴翻转并保留世界拖动效果 | `fix_cheek_handle_axes.py` | 已修复；拒绝重复应用，原轴向和输入补偿必须一起处理 |
| 总眼控改父级后初始目标回到眼前 | `fix_gaze_master_initials.py` | 建图已同步修正；先读 Initial，避免手工清零掩盖错误默认值 |
| 下巴 UI 行程缩为 1/3 | `resize_chin_travel.py` | 已缩放；拒绝重复缩放，动画旧值 / 增益需对应处理 |
| 表情在画面右侧、向右增加 | `update_expression_slider_layout.py` | 当前已应用；按角色和当前布局核对 |
| 眼神在画面左侧靠近角色 | `update_eye_expression_layout.py` | 当前已应用；同步轨道和手柄 |
| 参考图在滑块后方 | `update_phoneme_reference_depth.py` | 当前 Seven 已后移 6 cm；新角色重新定尺度 |
| Morph 层级曲线有值、预览暂未收到 | `refresh_expression_preview.py` / `refresh_phoneme_preview.py` / `refresh_eye_expression_preview.py` | 保留输入重开预览，正常动画帧后检查；眼神结束采样另用 `finish_eye_expression_preview.py` 检查恢复 |

## 示例派生与新式资产迁移入口

### 对已有采样求解增量恢复示例设计

`adapt_sampled_face_design.py` 保留现有求解及全部原执行连接，复制示例控制设置、形状库、口型图 Visual Cue 和 24 条滑轨，在原输入 Clamp 前接入显式矩阵转换。它要求当前任务的 `design_plan.json`、`solver_before.json`、`source_validation.json`，不能直接用于另一角色或未盘点的图。

`design_plan.json` 包含独立 `source` / `target`、`head_bone`、原模型 `source_axes`，以及逐控制的 `name`、`role_id`、`matrix`（源原始输入对 UI 的 3×3 矩阵）、`inverse`、`source_bounds`、UI 限位和测试姿势。矩阵须从源活动层、空间与示例接口核定；仅支持复用已经验证的采样求解，当前不自动推导任意角色。`solver_before.json` 是执行前盘点，不是另一目标的历史通过报告；须核对其节点、控制、源轴和目标路径。新角色应适配脚本中的结构断言。

执行前读取两套图并制作上述计划，在 Max 采集本轮覆盖新 UI 行程的独立样本，保存原值，采后恢复底层输入并读回。输入恢复、原始参考差异和固定参考配准另存报告。`seven-reference-adapt` 是 2026-11-10 上一轮的增量恢复任务，其目标 `TMp/NewControlRigRuntimeAsset` 已不在磁盘；当前流程验证任务为 `seven-newchar-20261110`，在空壳 `...Asset1` 上从示例构建，不使用旧 `seven-runtime` 的合并资产映射。`adapt_sampled_face_design.py` 只适用于“保留既有采样求解、再补示例设计”，对空目标不适用。

```powershell
python Tools/FaceMigration/migrate.py run adapt_sampled_face_design.py --job seven-reference-adapt
python Tools/FaceMigration/migrate.py run verify_sampled_face_design.py --job seven-reference-adapt
```

适配器拒绝重复添加 `DESIGN_` 节点。发生失败时先看备份、当前内存与报告，不能直接重跑已部分完成的转换。现有大图逐项添加节点可能长时间占用 UE 主线程；等待同一次调用结束，期间不再连接第二个 UE 修改或验证任务。按批次关闭自动重编译，完成后只编译一次；必要时用图文本批量导入替代大量交互式连线。

`verify_sampled_face_design.py` 分开写 `layout_validation.json` 及重载前/后数值报告，检查示例的 73 项显示接口、当前 Max 姿势、全部 149 骨、Morph、回零、重复求解及头部跟随。`Temp/verify_phase.txt` 为 `before_reload` 或 `after_reload`；正常帧网格输出仍需另验。当前报告只写本任务，不更新冻结证据。

`finalize_sampled_face_design.py` 要求本轮保存前与布局检查通过，保存、关闭目标编辑器并重载包，再运行保存后验证并重新打开目标。`verify_face_design_preview.py` 在真实目标预览实例按正常动画帧检查 72 控制代表姿势、组合与中性，共 76 组；同时核对实际网格骨骼、最终 Morph 类型/数值、时间推进及输入恢复。返回 scheduled 不是通过，须等 `preview_validation.json` 的 `pending=false` / `passed=true`。测试期间不并发修改同一预览输入，成功或失败都恢复临时编辑器偏好。

源通道盘点、网格 Morph 总数、动画输出曲线数分别记录。当前 Seven 网格有 87 个 Morph，但示例面部接口驱动 28 条；本轮对未绑定源通道另检查是否随输入变化，源 `L` / `R` 常量不计作新增动画控制。不能仅因名称在层级中出现就宣称已迁入动作，也不能把不属于当前接口的静态源权重当作动态曲线回归失败。

### 完整图迁移（仅空目标）

下表记录 Seven 既有完整图迁移实现，历史实测环境为 UE 5.8.4 / CL 58902052。源为用户指定的旧 Blueprint，脚本默认目标为 `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset`，使用独立任务配置适配实际输出路径。优先用 `python Tools/FaceMigration/migrate.py run <脚本.py> --job <新任务>`。这组脚本可作为复用实现入口，但已有内容的目标不能直接执行空目标构建器；应先比较并选择增量修复，或在用户指定的独立空目标上迁移。2026-11-10 已在空壳 `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset1` 上重跑成功：149 骨、72 动画控制 + 1 Visual Cue、28 曲线、1926 节点、2639 连接，`asset_overrides` 把默认目标键映射到 `...Asset1` 即可，脚本无需改动。构建期间 UE 主线程约 6 分钟无响应（`Responding=False`），属大图文本导入的已知行为，不要重复提交构建或并发连接第二个 UE 任务。

`build_runtime_face_controls.py` 已包含层级复制、`shape_libraries` 等属性复制及节点导入；仍须读取并检查绘制指令、轨道、参考图和材质依赖是否全部覆盖，不能把“完整图”理解为所有视觉依赖自然齐全。函数库、不同骨架和资产版本也需另行适配。

**路径配置注意：** 源和目标必须保持不同，整图构建器会断言拒绝两者相同。`Jobs/seven-runtime.json` 是旧部位脚本的合并路径适配配置，其中将示例路径映射到目标，不能直接用于这组整图构建与对照。另建任务，仅对实际输出路径及所需依赖作映射，保持示例只读。`--dry-run` 只检查配置和命令，不会验证 UE 目标是否为空。

| 脚本 | 范围与重跑边界 |
|---|---|
| `Tools/FaceMigration/Scripts/build_runtime_face_controls.py` | 完整层级与图迁移；仅接受空目标，备份后构建，已有内容拒绝覆盖 |
| `Tools/FaceMigration/Scripts/verify_runtime_face_controls.py` | 1175 组迁移对照及 12 个适配后的源样本验证器；报告隔离在 SevenRuntime，不覆盖历史参数或报告 |
| `Tools/FaceMigration/Scripts/finalize_runtime_face_controls.py` | 保存、关闭、包重载并复验；需先有保存前通过报告 |
| `Tools/FaceMigration/Scripts/refresh_runtime_face_preview.py` | 保存实例输入、重开目标编辑器并恢复；只在预览停滞时使用 |
| `Tools/FaceMigration/Scripts/probe_runtime_face_morph_preview.py` | 异步正常帧半程检查 28 条曲线及输入恢复；读取 JSON 的 pending / passed 后再判断结果 |
| `Tools/FaceMigration/Scripts/verify_runtime_face_preview.py` | 依赖本轮曲线检查通过的精确组件路径；75 组正常帧检查覆盖全部 72 控制、149 骨、28 曲线、组合及恢复 |

两项预览检查依次运行，不能同时修改预览输入；异步返回只说明已安排测试。表中的固定计数和旧报告属于历史验证范围，执行前检查是否适用于当前资产；当前报告必须写入新任务，不沿用[历史运行时资产报告](../../Characters/Seven/Evidence/SevenRuntime/README.md)的通过状态。外观和操作还需按[验证规范](Validation.md#11-示例max-和目标分开对照)另验。

`verify_runtime_face_controls.py` 同时跑两件事：1175 组示例→目标迁移对照（任何谱系都适用），以及把 12 个部位验证器重定向到目标。后者内部有“源动作段”和“旧控回归段”两类断言，前者读 `max_samples.json`，后者读 `previous_controls.json`。2026-11-10 实测：源动作段全部通过，旧控回归段有 10 个验证器失败，原因是该文件属于已废弃的采样求解资产（130 个 case 的 `bones` 为空，脚本回落到“149 骨全保持中性”的期望）。空目标新建或从示例派生时，旧控回归没有对象；`finalize_runtime_face_controls.py` 的前置断言因此会直接拒绝。此时应分段记录结论并手工完成保存重载，不要为了通过门禁去伪造同谱系基线，也不要把这类失败写成资产缺陷。

## 离线输入与交接

### 历史实验：不复用既有 Rig 的采样重建

只有用户明确要求“不参考已有实现”时才采用这条路线，该限制不能延续为其他任务的默认要求。2026-11-10 的独立重建虽通过数值检查，用户已指出控制外观与示例差异明显；其统一形状、行程和大量拟合节点不作为新流程模板。当前主路线允许参考实现，应优先使用上述派生/迁移或增量修复。

`build_sampled_face_controls.py` 使用本任务 `Current/Evidence/SourceModel/model.json`，只接受没有动画控制且仅有求解入口的 RuntimeAsset。模型应包含本次探测得到的有效约束父级、辅助变换链、独立拟合参数和输入映射；构建器不加载参考 Rig，不读取角色历史数值。它是采样模型的构建入口，不是新角色的一键采样器。

`verify_sampled_face_controls.py` 对照同任务 `SourceModel/source_validation.json` 的独立 Max 样本，检查 UE 实际实例的完整骨骼变换、曲线、回零、重复求解和头部移动旋转跟随。保存重载前后分别执行，结果写 `before_reload_validation.json` / `after_reload_validation.json`；正常帧预览仍需另验，不能用临时 host 的通过代替。

源中没有限位的轴，必须在本次报告中明确采样范围和 UE 行程。不能把有限采样范围的通过写成无限行程都已验证。

### 当前拟合输入

角色 `Evidence/<部位>/parameters.json`、`max_samples.json` 和 `previous_controls.json` 被构建 / 验证代码直接读取，必须保留。当前任务中生成的同名文件优先读取。部分 `fit_*.py` 仍需原始采样或管道输出文本，统一从目标项目 `Saved/FaceMigration/Jobs/<任务>/Current/Temp/` 读取；不能因交接目录中已有采样，就声称这些历史命令在干净环境直接可跑。

- Seven 的 149 骨采样名单可由 `python Tools/FaceMigration/Scripts/fit_cheek_controls.py --prepare` 从已留存嘴唇基准生成当前任务 Temp 下的 cheek_bone_names.txt；此旧命令使用默认 seven-local 任务，新角色通过对应任务配置生成自己的名单，不能沿用 Seven。
- 嘴角、眼球、总眼控、下巴拟合使用当前任务 Temp 下对应的 *_samples_raw.json。离线复现时，可核对格式后将该角色留存 max_samples.json 复制到拟合器指定位置，或适配读取路径；不要为了补中间文件覆盖源证据。
- 嘴唇、脸颊、鼻子拟合读 MCP 管道输出文本，需保留外层协议或适配为读取留存样本；不能把 JSON 直接改后缀当管道返回。
- 下眼皮和上眼皮没有统一的新角色拟合 CLI；按配方和诊断方法重建角色参数，随后完整采样验证。

完整迁移以 [主流程](Build.md) 的角色参数、源对照及重载结果为准。局部历史验证脚本可能固定控制数或假设后续部位尚不存在；验证当前完整 Rig 时先检查断言和比较范围，不能用早期快照要求覆盖当前合法功能。

早期 fix_brow_axis.py 的说明字符串含未转义 Windows 路径，本次改为原始字符串以恢复语法解析，未重新验收其修复逻辑。它不在上述有效构建入口中，不能把 Scripts 目录中的旧修复脚本都作为主流程。
