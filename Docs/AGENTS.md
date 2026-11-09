# AI 入口：3ds Max 面部控制迁移到 Unreal Control Rig

本目录只维护这条迁移流程。用户提供角色控制器名称表及 Max / UE 资产信息后，由 AI 完成源探测、角色适配、构建、验证和记录；用户无需阅读内部资料，最终画面由用户检查。不使用 computer-use。

## 从这里开始

1. 读取 [迁移配置](migration.config.json)，确认用户此次目标、角色输入和目标资产。
2. 执行唯一主流程 [Workflow/Build.md](Workflow/Build.md)。名称表填写用 [Markdown 模板](Templates/CharacterFaceControllers.md)或 [JSON 模板](Templates/CharacterFaceControllers.json)，二选一即可。
3. 按需阅读 [探测与实现方法](Workflow/MigrationGuide.md)、[工具和脚本索引](Workflow/Tools.md)、[验证规范](Workflow/Validation.md)。
4. 只读取本次涉及的部位配方与证据。Seven 的当前资产、计数和已知边界统一见 [角色记录](Characters/Seven/README.md)，完整名称见 [名称总表](Reference/ControllerNames.md)。

## 执行约定

- **以用户给出的目标资产为准。** Seven 本次交付是 `/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset`（新式 Runtime Asset）；完整旧版参考是 `/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation`，两者均有 72 个动画控制。早期 `/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders` 仅有 28 个眉眼控制，文件名中的 Diagnostic / MorphSliders 不表示功能范围。项目根旧概览不能覆盖此处角色记录。本次构建、预览及问题见 [运行时资产记录](Evidence/SevenRuntime/README.md)。
- 名称表是功能入口映射。新角色必须重新探测骨骼、Morph、活动控制层、限位、空间、枢轴和增益；不复制 Seven 的绝对坐标或数值。现有脚本大多固定 Seven，适配目标与依赖后再执行，当前没有读取表格即通用一键生成的程序。
- 修改前备份磁盘包和内存状态；采样保存并恢复帧、底层控制值和预览输入，异常时同样恢复并读回。保留已有控制接口、执行链和骨骼 / 曲线输出。
- 辅助虚拟体的变换与表达式属于求解逻辑；无需作为可见模型导出，但不能遗漏其效果。共享目标先按源顺序合成，避免覆盖已有控制。
- 数值验证、预览曲线、保存重载、用户画面反馈分开记录；不把编译成功、名称存在或历史报告写成新一次效果通过。上眼皮近似残差等已知边界不得在整理时删掉。

## 文件维护

| 目录 | 唯一用途 |
|---|---|
| `Workflow/` | 接表到交付的执行步骤、探测方法、工具和验证规范 |
| `Reference/` | 功能与 Max / UE 精确名称对应，不存另一份驱动参数 |
| `Templates/` | 新角色可填写输入，不预填新角色绑定名 |
| `Characters/` | 已填角色表、角色当前状态与限制 |
| `Recipes/` | 部位的实际实现、参数解释、脚本及验证范围 |
| `Troubleshooting/` | 仍有复用价值的故障原因与判定依据 |
| `Evidence/` | 源采样、拟合参数、旧控回归和报告；其中 JSON 会被脚本直接读取 |

顶层只保留本 AI 入口和 `migration.config.json`。文档链接相对当前文档；JSON 的文档 / 证据路径相对项目根。新增角色数据放独立目录，不覆盖 Seven。

`Evidence/` 中的旧计数是对应批次快照，不能批量改成当前计数；采样、参数和 `previous_controls.json` 是复现及回归输入，不因文件大或日期早而删除。变更路径时同步模板、角色表、脚本引用和链接，并运行 `python Scripts/validate_face_docs.py`。
