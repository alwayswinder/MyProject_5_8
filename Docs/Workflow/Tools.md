# 工具与脚本选择

本页供 AI 执行时查入口。命令从项目根目录运行。`migration.config.json` 是导航和约定配置，尚无通用构建器直接消费它；当前脚本是 Seven 的参考实现。

## 工具连接

| 任务 | 入口 | 使用要求 |
|---|---|---|
| Max 对象、控制器及场景检查 | 3ds Max MCP 专用工具 | 先发现工具 schema，核对场景完整路径及采样帧；同名歧义必须处理 |
| Max 自定义采样 | Max MCP 执行 MaxScript，或 `Scripts/max_run.ps1` | `-ScriptFile <脚本>`；多实例用 `-MaxPid`。采样脚本需保存底层值、异常恢复并读回 |
| UE 资产查询 | 内置 MCP `http://127.0.0.1:8000/mcp`，客户端 `Scripts/ue-mcp.ps1` | `-Toolsets` 发现、`-Describe <完整工具集名>` 读 schema，再调用；没有 load_toolset 步骤 |
| UE RigVM / 层级操作 | `python Scripts/ue_run.py <脚本.py>` | PythonScriptPlugin 远程节点可用；先核对项目、网格和脚本目标路径 |
| 离线参数拟合 | 普通 Python；部分脚本需 NumPy / SciPy | 先检查输入路径和数据版本；拟合本身不会同步 UE 图 |
| 文档完整性 | `python Scripts/validate_face_docs.py` | 不连接编辑器、不改资产，校验导航、72 项映射、配方覆盖及数据保留 |

UE 远程节点不可用时，继续做离线资料检查并明确未进行实时检查；不为文档任务重启用户编辑器。发现多个 UE 节点时先明确执行目标，不能仅凭存在 UnrealEditor 进程判断接入正确项目。`ue_run.py` 当前绑定本机 UE 5.8 Python 路径，换环境时先适配。

## 按依赖查 Seven 脚本

下表是功能索引，**不是可整段重放的安装脚本**。旧构建器多为首次增量添加，目标已经存在时会拒绝；早期眉眼构建会重建节点并影响后续接线。新角色先生成独立参数和构建入口；已有完整 Rig 只改本次范围。

| 功能 | Max 探测 / 拟合 | UE 构建 | 验证 / 保存重载 |
|---|---|---|---|
| 眉毛整体、点、弯曲 | `probe_animcraft_world.ms`、`probe_animcraft_rb*.ms`、`probe_animcraft_lb.ms`；见配方源数据 | `build_brow_two.py` → `build_brow_points.py` → `build_brow_bend.py` / `build_brow_bend_left.py` | 先修配方所列 API 和实例断言边界；`fix_brow_exec.py` 仅在已定位断链后使用 |
| 眼周点、上眼皮 | `probe_eye_r.ms` / `probe_eye_l.ms`、`probe_syp*.ms` | `build_eye_points.py`、`build_eye_lid_arc.py` | 早期基线采集：`Docs/Evidence/recheck_control_rig.py`，不代表完整 Rig 验收 |
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
| 口型参考图（可选） | 角色贴图、行序、Alpha 边界 | `build_phoneme_reference.py` | `finalize_phoneme_reference.py` / `verify_phoneme_reference_preview.py` |
| 八眼神 | `probe_eye_expression_sliders.ms`；已存参数 | `build_eye_expression_sliders.py` | `verify_eye_expression_sliders.py` / `reload_eye_expression_sliders.py` |
| 鼻侧 | `probe_nose_side_samples.ms` / `fit_nose_side_controls.py` | `build_nose_side_controls.py` | `verify_nose_side_controls.py` / `reload_nose_side_controls.py` |

表中无目录前缀的文件均位于 `Scripts/`。具体备份、输入恢复、拟合条件和误差见对应 [配方](../AGENTS.md#文件维护)及 [Seven 角色记录](../Characters/Seven/README.md)。旧的 `probe_*`、`set_*`、`inspect_*`、`clear_rig.py` 不因出现在 Scripts 中就成为主流程步骤。

## 修复与布局必须按当前状态选择

| 已完成需求 | 对应脚本 | 重跑边界 |
|---|---|---|
| 脸颊蓝轴翻转并保留世界拖动效果 | `fix_cheek_handle_axes.py` | 已修复；拒绝重复应用，原轴向和输入补偿必须一起处理 |
| 总眼控改父级后初始目标回到眼前 | `fix_gaze_master_initials.py` | 建图已同步修正；先读 Initial，避免手工清零掩盖错误默认值 |
| 下巴 UI 行程缩为 1/3 | `resize_chin_travel.py` | 已缩放；拒绝重复缩放，动画旧值 / 增益需对应处理 |
| 表情在画面右侧、向右增加 | `update_expression_slider_layout.py` | 当前已应用；按角色和当前布局核对 |
| 眼神在画面左侧靠近角色 | `update_eye_expression_layout.py` | 当前已应用；同步轨道和手柄 |
| 参考图在滑块后方 | `update_phoneme_reference_depth.py` | 当前 Seven 已后移 6 cm；新角色重新定尺度 |
| Morph 层级曲线有值、预览暂未收到 | `refresh_expression_preview.py` / `refresh_phoneme_preview.py` / `refresh_eye_expression_preview.py` | 保留输入重开预览，正常动画帧后检查；眼神结束采样另用 `finish_eye_expression_preview.py` 检查恢复 |

## Seven 本次新式资产入口

本次 UE 实测版本为 5.8.4 / CL 58902052。目标固定为 `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset`，旧 Blueprint 仍是参考。下列脚本均通过 `python Scripts/ue_run.py <脚本>` 执行：

| 脚本 | 范围与重跑边界 |
|---|---|
| `Scripts/build_runtime_face_controls.py` | 完整层级与图迁移；仅接受空目标，备份后构建，已有内容拒绝覆盖 |
| `Scripts/verify_runtime_face_controls.py` | 1175 组迁移对照及 12 个适配后的源样本验证器；报告隔离在 SevenRuntime，不覆盖历史参数或报告 |
| `Scripts/finalize_runtime_face_controls.py` | 保存、关闭、包重载并复验；需先有保存前通过报告 |
| `Scripts/refresh_runtime_face_preview.py` | 保存实例输入、重开目标编辑器并恢复；只在预览停滞时使用 |
| `Scripts/probe_runtime_face_morph_preview.py` | 异步正常帧半程检查 28 条曲线及输入恢复；读取 JSON 的 pending / passed 后再判断结果 |
| `Scripts/verify_runtime_face_preview.py` | 依赖本轮曲线检查通过的精确组件路径；75 组正常帧检查覆盖全部 72 控制、149 骨、28 曲线、组合及恢复 |

两项预览检查依次运行，不能同时修改预览输入；异步脚本返回只说明已安排测试，最终结果看 [运行时资产报告](../Evidence/SevenRuntime/README.md)。

## 离线输入与交接

`Evidence/<部位>/parameters.json`、`max_samples.json` 和 `previous_controls.json` 被构建 / 验证代码直接读取，必须保留。部分 `fit_*.py` 仍从 `Saved/Codex/*_raw.json` 或管道输出文本读输入；不能因交接目录中已有采样，就声称这些历史命令在干净环境直接可跑。

- Seven 的 149 骨采样名单可由 `python Scripts/fit_cheek_controls.py --prepare` 从已留存嘴唇基准生成 `Saved/Codex/cheek_bone_names.txt`；新角色生成自己的名单，不能沿用它。
- 嘴角、眼球、总眼控、下巴拟合使用对应 `Saved/Codex/*_samples_raw.json`。离线复现时，可核对格式后将该角色留存 `max_samples.json` 复制到拟合器指定位置，或适配读取路径；不要为了补中间文件覆盖源证据。
- 嘴唇、脸颊、鼻子拟合读 MCP 管道输出文本，需保留外层协议或适配为读取留存样本；不能把 JSON 直接改后缀当管道返回。
- 下眼皮和上眼皮没有统一的新角色拟合 CLI；按配方和诊断方法重建角色参数，随后完整采样验证。

完整迁移以 [主流程](Build.md) 的角色参数、源对照及重载结果为准。局部历史验证脚本可能固定控制数或假设后续部位尚不存在；验证当前完整 Rig 时先检查断言和比较范围，不能用早期快照要求覆盖当前合法功能。

早期 `Scripts/fix_brow_axis.py` 在整理前已存在字符串转义语法问题；它不在上述有效构建入口中，原脚本保持原样，审计报告单独记录该历史问题。不能把 Scripts 目录中的旧修复脚本都作为可执行主流程。
