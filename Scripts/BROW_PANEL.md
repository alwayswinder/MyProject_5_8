# 眉毛表情控制器：3ds Max → UE Control Rig 对照

## 1. 3ds Max 侧（源）

场景：`E:\AI\3dMax\Save\Seven_Skin.max`（3ds Max 2026，进程 42344）

面板根：`Circle117`（Link 约束挂在头上），面板本身是一组 **可拾取的 3D 控件**，不是 UI 对话框：

| 行 | 轨道（Rectangle） | 手柄（Circle） | 标签（Text） | 参数 |
|---|---|---|---|---|
| 1 | `Rectangle002` | `Circle113` | `Text001` | 眉毛变弯 |
| 2 | `Rectangle003` | `Circle114` | `Text002` | 眉毛旋转 |
| 3 | `Rectangle004` | `Circle115` | `Text003` | 左眉毛隐藏 |
| 4 | `Rectangle005` | `Circle116` | `Text004` | 右眉毛隐藏 |

- 轨道尺寸：宽 8.04、高 1.33（cm），标签字号 0.64
- 每条轨道上挂一个 Circle 手柄，沿轨道 X 拖动即 0→1
- 对应的控制器点：`K_SX_MeiMao_Z003/004`（眉心）、`K_SX_MeiMao_L003/004`、`K_SX_MeiMao_R003/004`
- 场景里**没有** expression controller / wire parameter / 自定义属性 —— 这 4 个滑块的驱动逻辑尚未接到场景数据上（面板是纯 UI + 控制器点）

## 2. UE 侧（实现）

资产：`/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders`（`CR_Seven_MorphSliders`）

新增 4 个控件，挂在 `MorphBoard` 上，排在原 7 个 morph 滑块下方（z 121.35 / 115.35 / 109.35 / 103.35 cm，行距 6 cm），每个都带一条 `Box_Solid` 轨道：

| 控件 | Display Name | 范围 | 驱动 |
|---|---|---|---|
| `CTRL_BROW_BEND` | 眉毛变弯 | 0..1 | 曲线 `browInnerUp`（×1.0）+ `browDownLeft`（×0.55）+ `browDownRight`（×0.55） |
| `CTRL_BROW_ROT` | 眉毛旋转 | 0..1（0.5 = 中立） | `meibu_L` / `meibu_R` 骨骼绕自身 **Y** 轴 ±15° |
| `CTRL_BROW_HIDE_L` | 左眉毛隐藏 | 0..1 | `meibu_L` 缩放 (1-v) |
| `CTRL_BROW_HIDE_R` | 右眉毛隐藏 | 0..1 | `meibu_R` 缩放 (1-v) |

骨骼：`head`（父）、`meibu_L`、`meibu_R`（从 Seven 骨架导入，父挂 head）。

RigVM 图（全部串在 `Begin` 之后）：

- `BROW_BEND_Get` → `BROW_BEND_Set0/1/2`（`BROW_BEND_Mul1/2` 出 0.55 系数）
- `BROW_ROT_Get` → `BROW_ROT_Sub(-0.5)` → `BROW_ROT_Mul(30°)` → `BROW_ROT_Quat`（轴 Y）→ `BROW_ROT_SetL` → `BROW_ROT_SetR`
- `BROW_HIDE0_Get` → `BROW_HIDE0_Sub(1-v)` → `BROW_HIDE0_Vec` → `BROW_HIDE0_Set`；右侧同理

骨架 `Seven_Skeleton` 上已确认 `browInnerUp` / `browDownLeft` / `browDownRight` 三条曲线是 **morph target** 曲线，能直接驱动 BlendShape。

## 3. 自检结果（脚本跑出来的）

| 项 | 期望 | 实测 |
|---|---|---|
| 编译状态 | 非 error | `BS_UP_TO_DATE` |
| BEND=1 | browInnerUp=1.0，browDown*=0.55 | 1.0 / 0.55 / 0.55 ✔ |
| ROT=1 | 局部旋转 +15° | quat (0, 0.130526, 0, 0.991445) = 15.0° ✔ |
| HIDE_L=1 | meibu_L scale = 0 | (0,0,0) ✔ |
| HIDE_R=1 | meibu_R scale = 0 | (0,0,0) ✔ |

## 4. 可调项

改 `Scripts/build_brow_panel.py` 顶部常量后重跑（脚本幂等，会先清掉旧的 `BROW_*` 节点和控件）：

- `BEND_CURVES` —— 变弯用的 morph 与权重（当前 `browInnerUp` 1.0 + `browDownLeft/Right` 0.55）
- `ROT_DEGREES` —— 旋转最大角度（当前 15°，总幅度 ±15°）
- 旋转轴：脚本里 `BROW_ROT_Quat.Axis`（当前 Y；换 X/Z 可改旋转平面）
- `ROW_STEP` / 面板 X、Y —— 板子位置与行距

## 5. 复现命令

```powershell
# 在编辑器里执行（PythonScriptPlugin 远程执行通道）
python Scripts\ue_run.py Scripts\build_brow_panel.py
```

`Scripts/ue_run.py` 会用 `bRemoteExecution=True` 连到正在运行的编辑器并在主线程执行脚本。
