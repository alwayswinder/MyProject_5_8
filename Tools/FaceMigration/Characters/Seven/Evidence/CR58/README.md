# CR58：新项目中按配方重建 Seven 面部 Control Rig

目标：`/Game/_MyTest/Mouth/Rig/NewControlRigRuntimeAsset`（`ControlRigRuntimeAsset`），
项目 `E:/Unreal/Projects/_TEST/MyProject_CR_5_8`，引擎 5.8.4 / CL 58902052。

## 输入与做法

- 用户新建空 Rig，已导入 Seven 骨架（149 骨）与网格 Morph 曲线（87 条）。
- 按 [主流程](../../../../Docs/Workflow/Build.md) 顺序，用部位配方脚本在目标上**重建**控制、层级和 RigVM 图；
  未搬运旧资产层级或导出节点文本。
- 当时的 cr_runtime_compat.py 把 `ControlRigBlueprint` 入口映射到 RuntimeAsset
  （`asset.hierarchy` / `get_editor_asset()` / `create_rig_vm_host()`）。
- 当时的 cr58_exec.py 按 RuntimeAsset 语义重放配方脚本（ASSET、save、`Begin` 节点名、
  单参数 `break_link`、验证脚本固定旧资产等改写）；原脚本保持可追溯。
- 目标曲线先清空，由配方脚本按需重建 28 条 Morph 输出曲线。

## 当前结构（保存并重载后一致）

| 项 | 数值 |
|---|---|
| 动画控制 | 72 / 72（缺失 0、多余 0） |
| 参考图 Visual Cue | 1（`VIS_PHONEME_REFERENCE`） |
| 骨骼 | 149 |
| 曲线 | 28 |
| 节点 | 1923 |
| 连接 | 2641 |
| 执行链 | 单一链头，109 条执行连接（与旧版同级） |

## 验证状态

| 检查 | 结果 |
|---|---|
| 结构、控制名覆盖 | 当时通过（probe_cr58_stats.py；历史任务专用入口） |
| 中性姿势 vs 历史快照 | 149 骨差异 0 |
| 唇组源样本对照（60 组） | 通过：位置 0.00025 cm、旋转 0.00081°、缩放 1.25e-6；非目标骨 0；回零 0 |
| 保存 / 重载 | 通过：控制、骨骼、曲线、节点、连接全部一致 |
| 全量 A/B 骨骼对照（1776 组） | 1750 组与旧交付资产完全一致；其余 26 组差异全部是 `CTRL_BROW_L/R` 的 Y 分量作用在 `meibu_*`/`Bone007`/`Bone010`，属旧资产 Y 轴未屏蔽的历史偏差；新 Rig 的 Y 无效果，与 Max 实测一致 |
| 曲线输出 A/B（732 组） | 通过：28 条 Morph 曲线逐值差异 0 |
| 正常帧预览（网格实际变形） | 部分：目标资产编辑器预览世界已建成，**73 个手柄实例**（72 动画控制 + 1 参考图）与 Seven 预览网格组件均存在；尚未实测拖动手柄后的网格顶点变形 |
| 手柄可见性 / 布局截图 | 未执行 |

早期一次 `verify_lip_controls.py` 报出的 2.15 cm 回归差异，经上述 A/B 证明来自**过期的
LipControls 历史快照**（该快照记录于眼皮实现变更前），不是新 Rig 与旧交付资产的行为差异；
新 Rig 与旧交付资产在眼皮输入下逐骨一致。


## 历史入口与后续执行

当时使用 build_cr58_job.py 和 cr58_job.json 逐步重放；clear_cr58_rig.py / clear_cr58_curves.py 用于干净重建，fix_cr58_exec_heads.py 修复悬空执行链。它们属于当时任务记录，不作为当前可迁移入口。归档重放副本及其绝对路径保留原字节用于追溯，禁止原样执行。

后续采用包内 [共用入口](../../../../README.md)、[资产适配配置](../../../../Jobs/seven-runtime.json)和 `Scripts/runtime_compat.py`，不再生成项目专用脚本或写回此历史目录。该配置只描述同角色的资产映射，目录整理没有重建本资产或重新验收历史结论。

## 已知边界

1. 旧交付资产的眉毛 Y 轴未屏蔽（历史偏差）；新 Rig 按 Max 实测把 Y 置零，因此两者在该轴
   相关输入上不同。这是有意保留的修正，不是回归。
2. 目标 Rig 原有 87 条网格 Morph 曲线，为让配方脚本建立自己的 28 条输出曲线，已清空；
   清空前备份在 `Saved/Codex/Backup/CR58_*`。
3. 本次未重新采集 Max；参数沿用同一角色的既有采样，符合"数值仅限 Seven"的前提。
4. 正常帧预览（预览网格实际变形、手柄可见性、布局截图）尚未执行；眼镜/最终画面需用户确认。
5. 对照用的旧交付资产副本曾临时放入 `Content/_MyTest/Mouth/TMp/`，编辑器仍持有引用，
   删除失败；确认无用后可在关闭该资产编辑器后删除。
