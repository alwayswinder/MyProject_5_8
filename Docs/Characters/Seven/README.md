# Seven 面部迁移状态

本页统一维护当前目标、各组入口和已知边界。2026-11-09 已将同一 Seven 完整面部图迁入用户指定的新式资产，并实际完成数值、保存重载和正常帧预览测试，详见 [本次交付与问题记录](../../Evidence/SevenRuntime/README.md)。历史下眼皮和脸颊画面已获用户确认；新资产的最终画面仍待用户确认。

## 当前资产

| 项目 | 路径 / 状态 |
|---|---|
| Max 源场景 | `D:/Download/Seven_Skin_New.max`，采样帧 0 |
| UE 网格 | `/Game/_MyTest/Mouth/Mesh/Seven`，历史核对 149 骨、87 个 Morph Target |
| 本次交付 Rig | `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset`，类型 `ControlRigRuntimeAsset` |
| 完整旧版迁移基准 | `/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation`，未修改源磁盘包 |
| 完整名称与输入表 | [Seven.face-controls.json](../Seven.face-controls.json)，72 项 |
| 最近保存结构 | 149 骨、72 个动画控制 + 1 个参考图 Visual Cue、1926 节点、28 条 Morph 曲线 |
| 本次运行环境 | UE 5.8.4 / CL 58902052；旧配方的 5.8.3 是历史环境 |
| 结构及功能依据 | [本次重载](../../Evidence/SevenRuntime/reload_report.json)、[1175 组迁移对照](../../Evidence/SevenRuntime/after_reload/transfer_validation.json)、[12 组源样本验证器](../../Evidence/SevenRuntime/after_reload/source_verifiers.json) |
| 名称覆盖依据 | 本次迁移对照实时核对新资产全部 72 项；[旧名称核对说明](../../Evidence/ControllerCatalog/README.md)保留当时的 `live_checked=false`，不改写历史报告 |
| 正常帧预览依据 | [75 组逐控与组合](../../Evidence/SevenRuntime/preview_controls_validation.json)、[28 条半程 Morph 及恢复](../../Evidence/SevenRuntime/preview_morph_check.json)、[73 个可见手柄绑定](../../Evidence/SevenRuntime/delivery_check.json) |

资产路径中的 `LidDiagnostic` 保留原名，该旧资产是本次完整迁移的基准。`/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders` 是早期 28 控制眉眼基线；其 330 节点数据只用于解释基础实现与上眼皮诊断，不能替代当前目标。后续增量修改以用户给出的资产为准。

## 功能清单

| 功能 | 动画控制数 | 实现与边界 |
|---|---:|---|
| 眉毛整体 / 三点 / 弯曲 | 10 | [基础配方](../../Recipes/SevenBrowsAndEyes.md)；整体 Y 掩码有数值复核，早期弯曲完整朝向的历史验证边界保留 |
| 眼周点 | 16 | [基础配方](../../Recipes/SevenBrowsAndEyes.md)；每骨平移，注意后续眼皮和眼球接线 |
| 上眼皮 | 2 | [诊断](../../Troubleshooting/UpperEyelid.md)；旋转轴已修正，保留约 0.340 mm / 0.394° 的既有近似 |
| 下眼皮 | 2 | [下眼皮](../../Recipes/XYP_LowerLids.md)；数值、重载及用户效果确认 |
| 六嘴唇 | 6 | [唇控](../../Recipes/LipControls.md)；中央与侧唇联动 |
| 左右脸颊 | 2 | [脸颊](../../Recipes/CheekControls.md)；用户确认变形，蓝轴和输入符号已补偿；参考残差单列 |
| 鼻子整体 | 1 | [鼻子](../../Recipes/NoseControl.md)；鼻尖三轴平移 |
| 左右嘴角 / 总嘴控 | 3 | [嘴部](../../Recipes/MouthControls.md)；正负分支、源 Euler 层合成 |
| 单眼目标 | 2 | [眼球](../../Recipes/EyeballControls.md)；目标朝向及眼皮 / 眼袋联动 |
| 双眼总控 | 1 | [总控](../../Recipes/GazeMasterControl.md)；两眼跟随；Initial 错误已修复 |
| 下巴 | 1 | [下巴](../../Recipes/ChinControl.md)；UI 行程为初版 1/3，增益补偿保持完整动作 |
| 慌张 / 无语 / 思考 / 尴尬 | 4 | [整体表情](../../Recipes/ExpressionSliders.md)；8 条头部 / 瞳孔 Morph |
| 十二口型 | 12 | [口型](../../Recipes/PhonemeSliders.md)；12 条 Morph，独立参考图不计动画控制 |
| 左右气 / 怒 / 忧 / 笑 | 8 | [眼神](../../Recipes/EyeExpressionSliders.md)；8 条 Morph，源左右编号和后缀不规则 |
| 左右鼻侧 | 2 | [鼻侧](../../Recipes/NoseSideControls.md)；第 3 层 Y 合成鼻尖扭转，保留整体平移 |
| 合计 | **72** | 完整接口见 [名称总表](../../Reference/ControllerNames.md) |

各部位报告保留原批次含义。本次额外对新资产运行 1175 组迁移对照、12 个适配后的源样本验证器和 75 组正常帧预览；眉眼基础的迁移一致性不扩大成重新采集 Max 的全功能对照，所有动画、LOD 和最终视觉姿势也未完整验收。

## 已确定的布局

- 表情和口型在玩家正面视角的画面右侧，从左向右增加权重；眼神在画面左侧靠近角色，左右眼分两列，每列气 / 怒 / 忧 / 笑。
- 口型参考图逐行对齐，在滑块平面后方 6 cm：滑块 Y=18、图 Y=12；它是 `VIS_PHONEME_REFERENCE`，不参与求解和动画手柄计数。
- 鼻侧两个小手柄在鼻子整体下方；单眼目标以总控为父级，重置后应仍在眼前。

## 仍需保留的边界

1. 目前采用固定 Root / Rig 参考空间；头部或身体动画空间适配未完整验收，新角色按需要实现并验证。
2. 上眼皮圆弧和内眼角非线性并未完全还原；后续眼球、下巴的组合验证仍保留约 0.393° 的既有上眼皮残差。
3. 右脸颊中性参考差异约 `0.01665 cm / 0.41026°`。控制增量通过不能表述为绝对参考姿势完全一致；`Root_Camera` 静态基差异也不作为嘴控旋转误差。
4. UE 层级存在微小变换写入缓存容差；连续求解和独立实例公式验证要分别报告，见眼球配方。
5. 新增 Morph 后曾出现预览曲线缓存；应检查正常动画帧的最终 Morph 曲线及输入恢复，不能只看 Rig 层级值。
6. 现有 Seven 脚本有固定路径、初次添加和早期断言限制；增量修改不能整套重放。新角色按 [主流程](../../Workflow/Build.md) 生成其独立入口。

## 数据位置

[Evidence](../../Evidence/README.md) 保留源采样、参数、回归、重载及布局修正报告。各部位文档的数量是对应批次，原 JSON 不改写成当前计数。`Saved/Codex/Backup/` 是本机备份，不随 Git 交接；所需证据已另存。

音频驱动口型、身体 IK/FK、七 Float 实验和 SevGame 版本兼容资料已移到项目 [ProjectNotes](../../../ProjectNotes/README.md)，不属于本流程构建依赖。
