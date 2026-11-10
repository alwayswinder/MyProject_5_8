# Seven 面部迁移状态

本页统一维护当前目标、各组入口和已知边界。**2026-11-10 已按示例增量恢复控制设计**：以 `/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation` 为示例，在保留目标采样求解和头部跟随的基础上，恢复手柄、配色、操作轴/行程、24 条滑轨及口型图，并加入源输入矩阵转换。当前技术检查与显示设置对照通过，最终画面与拾取手感待用户验收。执行方法见[主流程](../../Docs/Workflow/Build.md)；此前从零版本的视觉拒绝记录仍保留，不能改写成已获用户确认。

2026-11-09 的完整图迁移、数值、保存重载和正常帧测试保留为[历史交付记录](Evidence/SevenRuntime/README.md)，不代表当前资产仍是当时版本。历史下眼皮和脸颊画面已获用户确认，也不能替代本次整体验收。

## 2026-11-10 流程验证（当成新角色）

用户要求把示例、当前 Max 角色和新建的 `NewControlRigRuntimeAsset1` 当作三个不同角色，验证流程是否通顺（该资产现已删除，见下）。执行结果：

| 阶段 | 结论 |
|---|---|
| 独立绑定探测 | 与示例等价：149 骨同名同序、同一 `Seven` 网格、87 Morph 全含、目标 0 控制 |
| 示例契约盘点 | 73 控制、2 个形状库（含 `SL_PhonemeReference`）、24 条滑轨 |
| 空目标构建 | 通过，1926 节点 / 2639 连接，导入约 6 分钟无响应 |
| 示例 → 目标 | 通过，1175 组 worst 全 0（保存前与重载后各一次） |
| 显示契约 | 通过，0 差异 |
| Max → 目标抽检 | 上眼皮 5 点，0.00344 cm / 0.00059° |
| 保存重载 | 通过，示例磁盘摘要未变 |
| 正常帧预览 | **未通过**：Rig 时间停在 0.4 s、28 条 Morph 全 0，两次 refresh 未恢复 |
| 旧控回归（12 部位验证器） | **不适用**：`previous_controls.json` 属已废弃采样求解谱系，10/12 误报失败 |
| 用户画面 | 待验收 |

问题清单与建议见该任务 `Current/Evidence/flow_findings.md`；预览停滞判定见[排障](../../Docs/Troubleshooting/PreviewStall.md)。其余 71 个控制没有重新拟合 Max，只做迁移等价对照。

## 2026-11-10 第二轮：按 TASK.md 协议完整重跑（当前）

环境被重置为新的空壳目标 `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset`（149 骨、0 控制、87 曲线、1 节点），上一轮的 `...Asset1` 已删除。本轮从 TASK.md 第 1、2 节重新生成配置 `Jobs/seven-taskflow-20261110.json`，走完读表 → 备份盘点 → 独立探测 → 空目标构建 → 三方验证 → 保存重载 → 正常帧预览。结论与上一轮一致：结构、数值、显示、重载通过；正常帧预览仍停滞（0.4 s，两次 refresh 无效）；12 个部位验证器 2/12（旧控回归谱系不符）。新增发现：任务单表格对齐格式会让解析正则失效、形状库 struct 只有 `shape_name` 可读、手动 `execute('Forwards Solve')` 能出 rig 曲线但 mesh 曲线仍 0、保存后目标不 dirty（上一轮记录的“必然重新 dirty”不成立）、Max 次级眼皮骨有 0.021 cm / 0.41° 的差异。报告在 `Saved/FaceMigration/Jobs/seven-taskflow-20261110/Current/Evidence/`，问题清单 `flow_findings.md`。

## 当前资产

| 项目 | 路径 / 状态 |
|---|---|
| Max 源场景 | `D:/Download/Seven_Skin_New.max`，采样帧 0 |
| UE 网格 | `/Game/_MyTest/Mouth/Mesh/Seven`，历史核对 149 骨、87 个 Morph Target |
| 当前输出目标 | `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset`，类型 `ControlRigRuntimeAsset`；2026-11-10 第二轮由空壳构建，149 骨、72 动画控制 + 1 Visual Cue、28 曲线、1926 节点/2639 连接，已保存重载；正常帧预览未通过，画面待用户验收 |
| 历史输出目标 | `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset1`，2026-11-10 第一轮流程验证的对象；**该资产已不在磁盘**，相关报告只作历史记录 |
| 本轮示例 Rig | `/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation`；允许参考与复用，示例保持不变 |
| 完整名称与输入表 | [Seven.face-controls.json](face-controls.json)，72 项 |
| 2026-11-09 历史结构 | 149 骨、72 个动画控制 + 1 个参考图 Visual Cue、1926 节点、28 条 Morph 曲线；不作为当前盘点 |
| 当前保存结构 | 149 骨、72 个动画控制 + 1 个参考图 Visual Cue、9804 节点；层级保留网格的 87 条 Morph 曲线，其中面部接口驱动 28 条；24 条滑轨随头部运动 |
| 2026-11-10 流程测试记录 | 72 个动画控制、9166 节点、28 条 Morph；2846 组源姿势、保存重载及 79 组正常帧测试有数值通过记录，但未迁入完整示例控制设计及口型参考图 |
| 当前运行报告位置 | 目标项目 `Saved/FaceMigration/Jobs/seven-taskflow-20261110/Current/Evidence/`（含 `flow_findings.md`、`reference_inventory.json`、`adaptation_report.json`、`layout_validation.json`、`validation.json`、`delivery_check.json`）；任务配置 `Jobs/seven-taskflow-20261110.json`。上一轮 `seven-newchar-20261110` 保留为历史 |
| 当前 Max 对照 | 517 组本轮采样，含完整 UI 行程、中间值、全脸组合及脸颊阈值两侧；保存重载前后均通过，最大配准姿势误差约 0.0034 mm / 0.0012°，28 条曲线误差约 1.31e-6 |
| 当前显示对照 | 73 项形状、颜色、限位、行程、初始 offset 与 shape transform 无差异；两个形状库及 24 条滑轨的参考点/颜色/粗细一致，控制和滑轨适配为头部跟随 |
| 历史运行环境 | UE 5.8.4 / CL 58902052；旧配方的 5.8.3 是历史环境，执行时重新发现 |
| 2026-11-09 历史结构及功能依据 | [历史重载](Evidence/SevenRuntime/reload_report.json)、[1175 组迁移对照](Evidence/SevenRuntime/after_reload/transfer_validation.json)、[12 组源样本验证器](Evidence/SevenRuntime/after_reload/source_verifiers.json) |
| 历史名称覆盖依据 | 2026-11-09 迁移对照实时核对新资产全部 72 项；[旧名称核对说明](Evidence/ControllerCatalog/README.md)保留当时的 `live_checked=false`，不改写历史报告 |
| 2026-11-09 历史正常帧预览依据 | [75 组逐控与组合](Evidence/SevenRuntime/preview_controls_validation.json)、[28 条半程 Morph 及恢复](Evidence/SevenRuntime/preview_morph_check.json)、[73 个可见手柄绑定](Evidence/SevenRuntime/delivery_check.json) |

资产路径中的 `LidDiagnostic` 保留原名，该旧资产是本次完整迁移的基准。`/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders` 是早期 28 控制眉眼基线；其 330 节点数据只用于解释基础实现与上眼皮诊断，不能替代当前目标。后续增量修改以用户给出的资产为准。

## 功能清单

| 功能 | 动画控制数 | 实现与边界 |
|---|---:|---|
| 眉毛整体 / 三点 / 弯曲 | 10 | [基础配方](../../Docs/Recipes/SevenBrowsAndEyes.md)；整体 Y 掩码有数值复核，早期弯曲完整朝向的历史验证边界保留 |
| 眼周点 | 16 | [基础配方](../../Docs/Recipes/SevenBrowsAndEyes.md)；每骨平移，注意后续眼皮和眼球接线 |
| 上眼皮 | 2 | [诊断](../../Docs/Troubleshooting/UpperEyelid.md)；旋转轴已修正，保留约 0.340 mm / 0.394° 的既有近似 |
| 下眼皮 | 2 | [下眼皮](../../Docs/Recipes/XYP_LowerLids.md)；数值、重载及用户效果确认 |
| 六嘴唇 | 6 | [唇控](../../Docs/Recipes/LipControls.md)；中央与侧唇联动 |
| 左右脸颊 | 2 | [脸颊](../../Docs/Recipes/CheekControls.md)；用户确认变形，蓝轴和输入符号已补偿；参考残差单列 |
| 鼻子整体 | 1 | [鼻子](../../Docs/Recipes/NoseControl.md)；鼻尖三轴平移 |
| 左右嘴角 / 总嘴控 | 3 | [嘴部](../../Docs/Recipes/MouthControls.md)；正负分支、源 Euler 层合成 |
| 单眼目标 | 2 | [眼球](../../Docs/Recipes/EyeballControls.md)；目标朝向及眼皮 / 眼袋联动 |
| 双眼总控 | 1 | [总控](../../Docs/Recipes/GazeMasterControl.md)；两眼跟随；Initial 错误已修复 |
| 下巴 | 1 | [下巴](../../Docs/Recipes/ChinControl.md)；UI 行程为初版 1/3，增益补偿保持完整动作 |
| 慌张 / 无语 / 思考 / 尴尬 | 4 | [整体表情](../../Docs/Recipes/ExpressionSliders.md)；8 条头部 / 瞳孔 Morph |
| 十二口型 | 12 | [口型](../../Docs/Recipes/PhonemeSliders.md)；12 条 Morph，独立参考图不计动画控制 |
| 左右气 / 怒 / 忧 / 笑 | 8 | [眼神](../../Docs/Recipes/EyeExpressionSliders.md)；8 条 Morph，源左右编号和后缀不规则 |
| 左右鼻侧 | 2 | [鼻侧](../../Docs/Recipes/NoseSideControls.md)；第 3 层 Y 合成鼻尖扭转，保留整体平移 |
| 合计 | **72** | 完整接口见 [名称总表](../../Docs/Reference/ControllerNames.md) |

各部位报告保留原批次含义。2026-11-09 额外对当时的新资产运行 1175 组迁移对照、12 个适配后的源样本验证器和 75 组正常帧预览；眉眼基础的迁移一致性不扩大成重新采集 Max 的全功能对照，所有动画、LOD 和最终视觉姿势也未完整验收。2026-11-10 的独立重建是另一批次，该批外观未通过，不能沿用前一批次的结构和显示结论。

2026-11-10 本轮适配保留全部 185 条原执行连接及 57 骨输出，再追加显示与输入适配。新发现并修复右脸颊横向为零时眼袋门控的约 2 mm 偏差；增加 22 组阈值两侧样本后复验通过。原始采样、备份、输入恢复、显示、数值、正常帧及持久化报告均在当前任务目录。原始 Max 中性与网格参考姿势仍有右脸颊约 0.16649 mm / 0.41026° 的差异，完整姿势对照使用固定每骨参考配准，源动作保持，不能将上述配准误差称为原始参考姿势完全一致。

最终正常帧验证覆盖 72 控制代表姿势、3 组全脸组合及中性，共 76 组；实际网格骨骼与 Rig 输出一致，28 条曲线均属于 Morph 类型，源姿势/最终曲线通过，输入与临时编辑器偏好已恢复。目标编辑器已打开；画面和拾取手感仍等待本轮用户验收。

## 已确定的布局

- 表情和口型在玩家正面视角的画面右侧，从左向右增加权重；眼神在画面左侧靠近角色，左右眼分两列，每列气 / 怒 / 忧 / 笑。
- 口型参考图逐行对齐，在滑块平面后方 6 cm：滑块 Y=18、图 Y=12；它是 `VIS_PHONEME_REFERENCE`，不参与求解和动画手柄计数。
- 鼻侧两个小手柄在鼻子整体下方；单眼目标以总控为父级，重置后应仍在眼前。

## 仍需保留的边界

1. 旧配方采用固定 Root / Rig 参考空间。本轮目标保留采样求解的头部跟随，并将手柄及滑轨随头部移动；本轮已测头部平移旋转，完整身体动画、LOD、非均匀缩放和最终用户操作仍须按用途验收。
2. 示例旧求解有约 0.393° 的上眼皮近似；本轮保留目标独立采样求解，当前 517 组的完整旋转误差约 0.0012°。这不改写旧证据，也不扩大成无限行程、全部动画姿势均通过。
3. 右脸颊中性参考差异约 `0.01665 cm / 0.41026°`。控制增量通过不能表述为绝对参考姿势完全一致；`Root_Camera` 静态基差异也不作为嘴控旋转误差。
4. UE 层级存在微小变换写入缓存容差；连续求解和独立实例公式验证要分别报告，见眼球配方。
5. 新增 Morph 后曾出现预览曲线缓存；应检查正常动画帧的最终 Morph 曲线及输入恢复，不能只看 Rig 层级值。
6. 现有 Seven 脚本有初次添加和早期断言限制；增量修改不能整套重放。新角色按 [主流程](../../Docs/Workflow/Build.md) 配置独立任务、重新绑定和定参，复用并适配共用脚本。

## 数据位置

[Evidence](Evidence/README.md) 保留参数、小报告和说明。2026-11-10 起，原始采样（`max_samples.json`）、旧控快照（`previous_controls.json`）、`before_outputs.json` 和 `SevenRuntime/graph_export.txt` 已移到目标项目 `Saved/FaceMigration/Characters/Seven/`（176.7 MB），CR58 早期批次移到 `Saved/FaceMigration/Imported/CR58/`；移动清单在 `preservation.manifest.json` 的 `moved_out`，`read_path` 会优先读项目侧。各部位文档的数量是对应批次，原 JSON 不改写成当前计数。`Saved/Codex/Backup/` 是本机备份，不随 Git 交接；所需证据已另存。

音频驱动口型、身体 IK/FK、七 Float 实验和 SevGame 版本兼容资料已移到项目 [ProjectNotes](../../ProjectNotes/README.md)，不属于本流程构建依赖。
