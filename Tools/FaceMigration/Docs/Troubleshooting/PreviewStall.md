# 正常帧预览停滞（Rig 时间不推进）

**现象**：预览输入已写入，层级里控制值和骨骼输出都会变，但 `ControlRig.get_absolute_time()` 停在某个值（观察到 `0.4 s`）；动画组件的 28 条 Morph 曲线全为 0，网格不动。

**判定**（三条同时成立才算停滞）：

1. 预览实例确实属于目标资产：`RigVMHost.get_vm().get_outer() == target`，且组件路径是本轮记录的组件，不是旧 PreviewWorld。
2. 输入已写入并读回一致，但 `live.get_absolute_time()` 在多次 Slate tick 后不变。
3. 同一资产用独立实例（`create_rig_vm_host()` + `execute('Forwards Solve')`）能得到正确输出。

第 3 条成立而第 2 条不成立时，问题在预览求值，不在求解公式；不要改图或增益去“修”。

## 排查顺序

1. 确认组件 `update_animation_in_editor=True`，并调用一次 `request_run_once_event('Forwards Solve')`。
2. 记录 `preview_component`、`preview_host`、`get_absolute_time()`、期望与实际曲线，写入 `preview_morph_check.json`（`pending=false` 后再判断）。
3. 运行 `python Tools/FaceMigration/migrate.py run refresh_runtime_face_preview.py --job <任务>`：保存输入 → 关闭目标资产编辑器 → 重开 → 恢复 73 个输入。历史记录显示重开后恢复。
4. 重开后再跑 `probe_runtime_face_morph_preview.py`，等 JSON 的 `pending=false` 且 `passed=true` 才继续 `verify_runtime_face_preview.py`。
5. 若两次 refresh 后仍停滞，停止重试，按“未通过”记录；不要连续重建预览世界制造更多旧实例。

## 2026-11-10 记录

目标 `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset1`（1926 节点，构建后立即保存重载）：

| 项目 | 值 |
|---|---|
| `preview_component` | `/Engine/Transient.World_9:` 与 `World_11:` 的 `AnimationEditorPreviewActor_0.ControlRigSkeletalMeshComponent_0` |
| `rig_absolute_time` | `0.4`（两次 refresh 后不变） |
| `half_error` | `0.50014`（期望 0.5，实际 0） |
| `restored_curve_error` | `0.0` |
| `restored_input_error` | `0.0` |
| 结论 | 未通过；`verify_runtime_face_preview.py` 被前置断言阻断 |

同一目标的独立实例求解正常：1175 组迁移对照 worst 全 0，上眼皮 Max 抽检 0.00344 cm / 0.00059°。因此本轮把“数值/结构通过”和“正常帧预览未通过”分开记录，`delivery_check.json` 不写整体通过。

## 相关环境事实

- 当前关卡 `L_SevenLipSync` 里没有 ControlRig actor，只有两个 SkeletalMeshActor，预览只能依赖资产编辑器的临时世界。
- 预览世界中可枚举到 74 个 `ControlRigShapeActor`，口型参考图 actor 存在且未隐藏；手柄是否可见可先用它做结构证据，但它不能代替正常帧曲线和网格验证。
- 编辑器关闭后旧 PreviewWorld 不会立刻回收；`ObjectIterator` 可能同时看到多个实例，必须按本轮记录的组件路径筛选。
