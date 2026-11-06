# Seven 面部控制器配方（不依赖 3ds Max）

> 本文记录**已经在 UE 里验证通过**的控制器。照表即可从零搭建，**不需要 Max 场景**。
> 新部位（还没有配方的）先走 [MaxControllerMigrationGuide.md](MaxControllerMigrationGuide.md)，
> 迁完把实测数据回填到本文。脚本是唯一真源，本文用于审阅、重建和交接。

---

## 0. 资产与骨架

| 项 | 值 |
|---|---|
| Rig 资产 | `/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders` |
| 预览网格 | `/Game/_MyTest/Mouth/Mesh/Seven` |
| 骨架 | 149 根骨（`import_bones_from_skeletal_mesh` 全量导入） |
| 本文件覆盖的骨 | `meibu_L/R`、`Bone007`~`Bone012` |

## 1. 全局约定（所有控制器通用）

- **坐标系**：`UE = (Max.x, −Max.y, Max.z)`；X 左右、Y 前后、Z 上下。
- **空间**：图里读写一律 `GlobalSpace`，`bInitial=True` 读初始值防漂移。
- **父级**：所有控制挂 `Root` 骨骼（世界变换为单位矩阵）。
- **控制自身 scale**：必须 `(1,1,1)`，尺寸只走 `set_control_shape_transform`。
- **形状**：统一 `Box_Thick`（默认形状库 `/ControlRig/Controls/DefaultGizmoLibraryNormalized`）。
- **命名**：`CTRL_BROW_<L|R>[_<IN|MID|OUT|BEND>]`；脚本节点前缀相互隔离。
- **颜色**：整体/点控橙色 `(1.0, 0.42, 0.04)`；弯曲青色 `(0.15, 0.85, 1.0)`。
- **限值**：整体 ±10、点控 ±10、弯曲 ±2。

## 2. 控制总表（28 个，全部已验证）

| 控制 | 显示名 | 类型 | 限值 | 映射 | 世界位置（UE） | 驱动骨骼 |
|---|---|---|---|---|---|---|
| `CTRL_BROW_L` | 左眉整体 | POSITION | ±10 | X/Z × 0.5 | `(4.0, 11.2, 161.2)` | `meibu_L` + `Bone007/008/009` |
| `CTRL_BROW_R` | 右眉整体 | POSITION | ±10 | X/Z × 0.5 | `(-4.0, 11.2, 161.2)` | `meibu_R` + `Bone010/011/012` |
| `CTRL_BROW_L_IN` | 左眉内点 | POSITION | ±10 | 1:1 | `(1.178, 9.763, 161.442)` | `Bone008` |
| `CTRL_BROW_L_MID` | 左眉中点 | POSITION | ±10 | 1:1 | `(4.071, 9.009, 161.795)` | `Bone009` |
| `CTRL_BROW_L_OUT` | 左眉外点 | POSITION | ±10 | 1:1 | `(6.748, 7.371, 162.146)` | `Bone007` |
| `CTRL_BROW_R_IN` | 右眉内点 | POSITION | ±10 | 1:1 | `(-1.177, 9.763, 161.442)` | `Bone010` |
| `CTRL_BROW_R_MID` | 右眉中点 | POSITION | ±10 | 1:1 | `(-4.070, 9.009, 161.795)` | `Bone011` |
| `CTRL_BROW_R_OUT` | 右眉外点 | POSITION | ±10 | 1:1 | `(-6.748, 7.371, 162.146)` | `Bone012` |
| `CTRL_BROW_L_BEND` | 左眉弯曲 | POSITION | ±2 | Z 分量分段 | `(3.17631, 11.5447, 162.256)` | `Bone008/009/007` |
| `CTRL_BROW_R_BEND` | 右眉弯曲 | POSITION | ±2 | Z 分量分段 | `(-2.2336, 11.5447, 162.256)` | `Bone010/011/012` |

> 括号里“内/中/外”按各自眉毛从眉心到眉尾排列：左眉 内=`Bone008`、中=`Bone009`、外=`Bone007`；右眉 内=`Bone010`、中=`Bone011`、外=`Bone012`。

### 2.1 眼周点控（16 个）

| 控制 | 显示名 | 骨骼 | 世界位置（UE） | Max 来源 |
|---|---|---|---|---|
| `CTRL_EYE_R_CORNER_IN` | 右眼内眼角 | `Bone043` | `(-1.635, 8.018, 159.062)` | `zCircle083` |
| `CTRL_EYE_R_UP_IN` | 右眼上内 | `Bone027` | `(-2.335, 8.231, 159.183)` | `zCircle074` |
| `CTRL_EYE_R_UP_MID` | 右眼上中 | `Bone028` | `(-4.322, 8.263, 159.389)` | `zCircle075` |
| `CTRL_EYE_R_UP_OUT` | 右眼上外 | `Bone029` | `(-5.928, 7.268, 159.501)` | `zCircle076` |
| `CTRL_EYE_R_CORNER_OUT` | 右眼外眼角 | `Bone042` | `(-6.727, 6.340, 159.474)` | `zCircle082` |
| `CTRL_EYE_R_DOWN_OUT` | 右眼下外 | `Bone030` | `(-5.989, 6.863, 158.032)` | `zCircle077` |
| `CTRL_EYE_R_DOWN_MID` | 右眼下中 | `Bone031` | `(-4.278, 7.567, 157.347)` | `zCircle078` |
| `CTRL_EYE_R_DOWN_IN` | 右眼下内 | `Bone032` | `(-2.439, 7.895, 157.878)` | `zCircle079` |
| `CTRL_EYE_L_CORNER_IN` | 左眼内眼角 | `Bone044` | `(1.635, 8.018, 159.062)` | `zCircle084` |
| `CTRL_EYE_L_UP_IN` | 左眼上内 | `Bone036` | `(2.336, 8.231, 159.183)` | `zCircle068` |
| `CTRL_EYE_L_UP_MID` | 左眼上中 | `Bone037` | `(4.323, 8.263, 159.389)` | `zCircle069` |
| `CTRL_EYE_L_UP_OUT` | 左眼上外 | `Bone038` | `(5.928, 7.268, 159.501)` | `zCircle070` |
| `CTRL_EYE_L_CORNER_OUT` | 左眼外眼角 | `Bone045` | `(6.728, 6.341, 159.474)` | `zCircle085` |
| `CTRL_EYE_L_DOWN_OUT` | 左眼下外 | `Bone039` | `(5.990, 6.863, 158.032)` | `zCircle071` |
| `CTRL_EYE_L_DOWN_MID` | 左眼下中 | `Bone040` | `(4.278, 7.567, 157.347)` | `zCircle072` |
| `CTRL_EYE_L_DOWN_IN` | 左眼下内 | `Bone041` | `(2.439, 7.895, 157.877)` | `zCircle073` |

### 2.2 上眼皮闭合（2 个）

| 控制 | 显示名 | 骨骼 | 世界位置（UE） | Max 来源 |
|---|---|---|---|---|
| `CTRL_EYE_LID_R` | 右眼上眼皮 | `Bone027/028/029` | `(-4.14181, 9.81844, 160.624)` | `SYP_R001` |
| `CTRL_EYE_LID_L` | 左眼上眼皮 | `Bone036/037/038` | `(4.14179, 9.81847, 160.624)` | `SYP_L001` |

---

## 3. 配方 A：眉毛整体（左/右各一个）

**手感**：拖一个扁平条，整条眉毛平移；世界 X/Z 各 0.5 倍，Y 无效。

| 项 | 值 |
|---|---|
| 类型 / 限值 | `POSITION` / ±10 |
| 形状 | `Box_Thick`，shape scale `(0.42, 0.22, 0.05)`，橙色 |
| 父级 | `Root` |
| 骨骼 | L：`meibu_L` + `Bone007/008/009`；R：`meibu_R` + `Bone010/011/012` |
| 系数 | **0.5**（X/Z 分量，Y 分量不用） |

图（每根骨一条，串在 `Begin` 之后）：

```
GetControlVector(CTRL, LocalSpace)
  → VectorScale(Factor=0.5)
GetTransform(bone, GlobalSpace, bInitial=True)
  .Transform.Translation → VectorAdd(B=偏移)
  .Transform.Rotation / .Transform.Scale3D
  → MathTransformMake(Translation, Rotation, Scale)
  → SetTransform(Item=bone, GlobalSpace, bInitial=False, Value, Weight=1, bPropagateToChildren=False)
```

节点前缀 `BROW_`：`BROW_CTRL_BROW_<L|R>_GET`、`_SCALE`、每骨 `BROW_BT_/_VADD_/_MAKE_/_SET_`。

自检（实例化后骨骼世界位移）：

| 输入 | 左眉 4 骨 | 右眉 4 骨 |
|---|---|---|
| `CTRL_BROW_L=(10,0,0)` | 全 `+5 X` | `0` |
| `CTRL_BROW_L=(0,0,10)` | 全 `+5 Z` | `0` |
| `CTRL_BROW_R=(10,0,0)` | `0` | 全 `+5 X` |
| `CTRL_BROW_R=(0,0,10)` | `0` | 全 `+5 Z` |

脚本：`Scripts/build_brow_two.py`（幂等）。

---

## 4. 配方 B：眉毛三点控（每侧三个）

**手感**：每根眉骨一个独立点控，拖动时该骨世界 1:1 跟随，点与点、左右互不影响。

| 项 | 值 |
|---|---|
| 类型 / 限值 | `POSITION` / ±10 |
| 形状 | `Box_Thick`，shape scale `(0.12, 0.10, 0.05)`，橙色 |
| 父级 | `Root` |
| 控制位置 | 对应骨骼的静止世界坐标（表见第 2 节） |
| 系数 | **1.0** |

图（接在整体控制的 `VADD` 之后，做向量相加）：

```
BROW_VADD_<骨>.Result ─┐
                       ├→ VectorAdd → BROW_MAKE_<骨>.Translation
BPT_<ctrl>_GET → SCALE ┘
```

节点前缀 `BPT_`：`BPT_<control>_GET`、`_SCALE`、`BPT_ADD_<bone>`。

自检：

| 输入 | 结果 |
|---|---|
| 任一点控 `(10,0,0)` / `(0,10,0)` / `(0,0,10)` | 只有对应骨 `10`，其余骨 `0` |
| 左侧点控 | 右侧骨全 `0`（反之亦然） |
| `CTRL_BROW_L=(10,0,0)` + `CTRL_BROW_L_IN=(0,0,10)` | `Bone008=(5,0,10)`，`Bone007/009=(5,0,0)` |

脚本：`Scripts/build_brow_points.py`（幂等）。

---

## 5. 配方 C：眉毛弯曲（左/右各一个）

**手感**：拖一个点，眉毛三根骨按正/负方向分段弯曲并绕世界 X 旋转；行程 ±2 cm。

| 项 | 值 |
|---|---|
| 类型 / 限值 | `POSITION` / ±2 |
| 取值 | 控制向量的 **Z 分量** `c` |
| 形状 | `Box_Thick`，shape scale `(0.15, 0.10, 0.05)`，青色 |
| 父级 | `Root` |
| 位置 | L `(3.17631, 11.5447, 162.256)`；R `(-2.2336, 11.5447, 162.256)` |
| 依赖 | 接在 `BPT_ADD_<骨>` 之后 |

位置映射：

```
c > 0 : offset = (c/2)   × D_pos
c < 0 : offset = (−c/2)  × D_neg
旋转    : NewRot = Initial × Delta，Delta 绕世界 X，角度 = −(dEuler) × c/2
```

> 没有固定外移项（zoff）。Max 拖动时 limiter 的 z 分量不参与驱动，早期加过会导致外缘外飘。

右眉数据（`D_pos` = y=+2 位移，`D_neg` = y=−2 位移，单位 cm / 度）：

| 骨 | `D_pos` | `D_neg` | `dEuler_pos` | `dEuler_neg` |
|---|---|---|---|---|
| `Bone010` | `(-0.01744, -0.19924, 0.3)` | `(0.08218, -0.20796, -0.5)` | `-9.1674` | `+11.4589` |
| `Bone011` | `(-0.18126, 0.08452, 0.8)` | `(0.29491, -0.31406, -0.62)` | `0` | `-3.4377` |
| `Bone012` | `(0, 0, 0.1)` | `(0, 0, -0.1)` | `+10.313` | `-4.5837` |

左眉数据：

| 骨 | `D_pos` | `D_neg` | `dEuler_pos` | `dEuler_neg` |
|---|---|---|---|---|
| `Bone007` | `(0, 0, 0.1)` | `(0, 0, -0.1)` | `-10.3132` | `+4.5837` |
| `Bone008` | `(0.01744, -0.19924, 0.3)` | `(-0.08219, -0.20795, -0.5)` | `+9.1673` | `-11.4591` |
| `Bone009` | `(0.18126, 0.08452, 0.8)` | `(-0.2949, -0.31406, -0.62)` | `0` | `+3.4378` |

图（共享 6 个节点 + 每骨 12 个）：

```
GET(GetControlVector) → DOT(向量点乘 (0,0,1)) → c
c → ISPOS(Greater 0) / HALF(c*0.5) / NHALF(c*-0.5) / ABS / HALFABS

每骨：
  POSMUL(Value=D_pos, Factor=HALF)   NEGMUL(Value=D_neg, Factor=NHALF)
  → SELPOS(Condition=ISPOS)
  → ADD1(A=BPT_ADD_<骨>, B=SELPOS)
  → ADD2(A=ADD1, B=ZOFF)            -- ZOFF 当前恒 0，仅为兼容保留
  → MAKE.Translation

  PANG(FloatMul: HALF × -dEuler_pos)  NANG(FloatMul: NHALF × -dEuler_neg)
  → SELANG → RAD(× deg2rad) → QUAT(Axis=(1,0,0))
  → QROT(QuatMul A=BT_Get.Rotation, B=QUAT) → MAKE.Rotation
```

> `ZOFF` / `ADD2` / `ABS` / `HALFABS` 是早期「z=-1 固定外移」留下的，现在值恒 0。
> 从零搭建时可以直接省掉这四个节点，`ADD1` 直连 `MAKE.Translation`。

节点前缀：右 `BB_`、左 `BBL_`。

自检（端点）：

| 输入 | 左眉 | 右眉 |
|---|---|---|
| `BEND=(0,0,2)` | `Bone007 (0,0,0.1)` / `Bone008 (0.0174,-0.1992,0.3)` / `Bone009 (0.1813,0.0845,0.8)`，旋转 `-10.313°/+9.167°/0` | `Bone010 (-0.0174,-0.1992,0.3)` / `Bone011 (-0.1812,0.0845,0.8)` / `Bone012 (0,0,0.1)`，旋转 `-9.167°/0/+10.313°` |
| `BEND=(0,0,-2)` | `(0,0,-0.1)` / `(-0.0822,-0.2079,-0.5)` / `(-0.2949,-0.3141,-0.62)`，旋转 `+4.584°/-11.459°/+3.438°` | `(0.0822,-0.2079,-0.5)` / `(0.2949,-0.3141,-0.62)` / `(0,0,-0.1)`，旋转 `+11.459°/-3.438°/-4.584°` |

脚本：`Scripts/build_brow_bend.py`（右）、`Scripts/build_brow_bend_left.py`（左）。

### 4.5 配方 D：眼周点控（16 个）

**手感**：每根眼周骨一个独立点控，拖动时该骨世界 1:1 跟随，点与点、左右互不影响。

| 项 | 值 |
|---|---|
| 类型 / 限值 | `POSITION` / ±10 |
| 形状 | `Box_Thick`，shape scale `(0.12, 0.10, 0.05)`，绿色 `(0.20, 0.90, 0.50)` |
| 父级 | `Root` |
| 控制位置 | 对应骨骼的静止世界坐标（表见 2.1） |
| 系数 | **1.0** |

图：每根骨一条**独立链**（不接眉毛的 `BROW_`/`BPT_`，执行流从 `Begin` 分出）：

```
GetControlVector(EYE_CTRL, LocalSpace) → VectorScale(Factor=1.0)
GetTransform(bone, GlobalSpace, bInitial=True)
  .Transform.Translation → VectorAdd(B=偏移)
  .Transform.Rotation / .Transform.Scale3D
  → MathTransformMake → SetTransform(GlobalSpace, bInitial=False, bPropagateToChildren=False)
```

节点前缀 `EYE_`：`EYE_<control>_GET`、`_SCALE`、每骨 `EYE_BT_/_VADD_/_MAKE_/_SET_`。

自检：

| 输入 | 结果 |
|---|---|
| 任一眼控 `(10,0,0)` / `(0,10,0)` / `(0,0,10)` | 只有对应骨移动 `10`，其余 15 根骨 `0` |
| 左右眼控制 | 互不影响（已逐点验证） |

脚本：`Scripts/build_eye_points.py`（幂等，重建 16 个控制 + `EYE_` 节点）。

> 眼周控制不依赖眉毛节点，可以单独跑；`Begin` 的执行流允许同时分出眉毛链和眼链。

### 4.6 配方 E：上眼皮闭合（左右各一个）

**手感**：拖控制沿世界 **−Z**（向下）方向，上眼皮三根骨绕固定世界轴旋转并轻微位移，拖到底闭合。Max 里反向无效果，UE 也用单侧限值锁死。X/Y 被锁在 0，手柄只能上下拖。

| 项 | 值 |
|---|---|
| 类型 / 限值 | `POSITION` / **X=Y=0，Z ∈ [-10, 0]**（单侧） |
| 取值 | 控制向量的 **Z 分量** `ctrlZ`（蓝色上下轴）；`d = -ctrlZ * 13.5`，向下拖到 -10 对应 Max 行程末端 |
| 形状 | `Box_Thick`，shape scale `(0.14, 0.10, 0.05)`，紫色 `(0.80, 0.45, 1.0)` |
| 父级 | `Root` |
| 位置 | R `(-4.14181, 9.81844, 160.624)`；L `(4.14179, 9.81847, 160.624)` |
| 依赖 | 接在 `EYE_VADD_<骨>` 之后（配方 D 必须先存在） |

位置/旋转数据（按 Max 世界 cm 归一，已转成 UE 坐标；轴取 Max 行程末端值）：

| 骨 | `dP / cm` | 旋转轴（UE） | `deg / cm` |
|---|---|---|---|
| `Bone027` | `(-0.0000952, -0.0021720, -0.0091253)` | `(0.8825, 0.0110, -0.4701)` | `+0.10967` |
| `Bone028` | `(0.0001785, -0.0040398, -0.0151811)` | `(0.0744, 0.0248, -0.9969)` | `+0.20796` |
| `Bone029` | `(-0.0006482, -0.0036973, -0.0098726)` | `(-0.4341, 0.0248, -0.9005)` | `+0.20796` |
| `Bone036` | `(0.0000951, -0.0021721, -0.0091251)` | `(-0.8825, -0.0110, -0.4701)` | `+0.10967` |
| `Bone037` | `(-0.0001785, -0.0040401, -0.0151811)` | `(-0.0744, -0.0248, -0.9969)` | `+0.20796` |
| `Bone038` | `(0.0006481, -0.0036974, -0.0098724)` | `(0.4341, -0.0248, -0.9005)` | `+0.20796` |

图（接在眼周链的 `VADD` 之后）：

```
位置：pos = P + R(axis, angle*s) * (t0 - P) * (1 + k1*s)   -- P = yanqiu_L/R（眼球中心）
      t0 = EYE_VADD，s = d/135
旋转：EYE_BT_<骨>.Rotation → LID_QROT(Delta * Initial) → MAKE.Rotation
```

> **位置必须是绕眼球中心的圆弧，不能沿 dP 直线插值。**
> Max 里眼皮骨挂在眼球中心的虚拟体（`Point004`/`yanqiu`）上，骨骼走圆弧；
> UE 早期版本用「初始位置 + dP*d」直线平移，端点一致但中途走弦，
> 与 Max 轨迹最大偏差 **1.28 mm**——眼皮与眼球间隙小时就会在中途穿模。
> 改成圆弧 + 径向缩放后，Bone037 的 8 个采样点最大误差 **0.037 mm**，端点误差 **0.002 mm**。
> 参数按骨各一套（axis / angle / k1），用 `Scripts/calc_lid_arc_params.py` 从 Max 端点数据算。

> `MathQuaternionMul` 的顺序必须是 `A=Delta, B=Initial`（世界轴左乘）。反过来 `Initial * Delta`
> 会把世界轴当局部轴，骨架闭合时眼皮骨会朝眼球里转——3ds Max 里不穿模、UE 里穿模就是这个原因。

节点前缀 `LID_`：每条链 `LID_<control>_GET/_DOT/_D`，每骨 `LID_POS_/_ADD_/_ANG_/_RAD_/_QUAT_/_QROT_`。

自检（`LID=(0,0,-10)`，即拖到底）：

| 骨 | dP | 旋转角 | 旋转轴（与 Max 对照） |
|---|---|---|---|
| `Bone027` | `(-0.0128, -0.2932, -1.2319)` | `14.805°` | `(0.883, 0.011, -0.470)` |
| `Bone028` | `(0.0242, -0.5454, -2.0494)` | `28.074°` | `(0.074, 0.025, -0.997)` |
| `Bone029` | `(-0.0875, -0.4991, -1.3329)` | `28.074°` | `(-0.434, 0.025, -0.901)` |
| `Bone036` | `(0.0128, -0.2932, -1.2319)` | `14.805°` | `(-0.883, -0.011, -0.470)` |
| `Bone037` | `(-0.0242, -0.5454, -2.0494)` | `28.074°` | `(-0.074, -0.025, -0.997)` |
| `Bone038` | `(0.0875, -0.4991, -1.3327)` | `28.074°` | `(0.434, -0.025, -0.901)` |

脚本：`Scripts/build_eye_lid_arc.py`（**当前版本**，圆弧 + 径向缩放；幂等，依赖配方 D 的 `EYE_VADD_*` / `EYE_BT_*`）。
`Scripts/build_eye_lid.py` 是旧的直线平移版本，只保留作对照，**不要再跑**（会把位置改回直线）。

> 限值单侧：X/Y 恒为 0、Z ∈ [-10, 0]。向上拖手柄不动（与 Max 的 limiter 一致），
> 不会出现反向翻转眼皮。闭合后拖回 0 即睁眼。
> 改限值用 `Scripts/set_lid_single_side.py`（只改设置、不重建图）；验证用 `Scripts/probe_lid_limits.py`。

---

## 6. 搭建顺序与叠加关系

图里的累加顺序（从初始值出发）：

```
GetTransform(bInitial=True).Translation
  → +整体偏移（配方 A）
  → +点控偏移（配方 B）
  → +弯曲偏移（配方 C）
  → MAKE.Translation → SET
```

脚本执行顺序（必须按这个顺序）：

```powershell
python Scripts\ue_run.py Scripts\clear_rig.py              # 仅在需要全量重导骨骼时
python Scripts\ue_run.py Scripts\build_brow_two.py         # 配方 A
python Scripts\ue_run.py Scripts\build_brow_points.py      # 配方 B（依赖 A 的 BROW_VADD_*）
python Scripts\ue_run.py Scripts\build_brow_bend.py        # 配方 C 右（依赖 B 的 BPT_ADD_*）
python Scripts\ue_run.py Scripts\build_brow_bend_left.py   # 配方 C 左（依赖 B 的 BPT_ADD_*）
python Scripts\ue_run.py Scripts\build_eye_points.py       # 配方 D（独立，不依赖眉毛节点）
python Scripts\ue_run.py Scripts\build_eye_lid_arc.py      # 配方 E（依赖 D 的 EYE_VADD_* / EYE_BT_*）
```

重跑规则：

- 只重跑 A → 会清掉 `BROW_` 节点，必须补跑 B、C。
- 只重跑 B → 会清掉 `BPT_` 节点，必须补跑 C（左右都要）。
- C 的脚本只清自己的 `BB_` / `BBL_` 前缀，互不影响。
- D 只清 `EYE_` 前缀；**重跑 D 后必须补跑 E**（E 依赖 `EYE_VADD_*` / `EYE_BT_*`）。
- E 只清 `LID_` 前缀，不影响眉毛和眼周点控。
- **E 的执行流坑**：`LID_*` 节点是用 `Execute` 引脚创建的，会插进执行流；删掉后
  ControlRig 可能把 `Begin` 直接接到 `EYE_SET_Bone043`，导致 `BROW_SET_meibu_L`
  失去 Execute 输入——表现就是「眼睛能拖、眉毛整条链不动」。
  `build_eye_lid.py` 已内置 `ensure_exec_chain()` 自动补回；老资产可跑
  `Scripts\fix_brow_exec.py` 手动修。修完执行流应为：
  `Begin → BROW_SET_meibu_L → … → BROW_SET_Bone012 → EYE_SET_Bone043 → …`。
  自检：`Scripts\probe_all_controls2.py`（眉毛整体 10 → 本侧三骨各 +5）。

---

## 7. 从零重建（不需要 Max）

1. 备份资产：
   ```powershell
   Copy-Item Content\_MyTest\Mouth\Rig\CR_Seven_MorphSliders.uasset `
       Saved\Codex\Backup\CR_Seven_MorphSliders_<时间戳>.uasset
   ```
2. （可选）`clear_rig.py` 全量重导 149 根骨。
3. 按第 6 节顺序跑四个脚本。
4. 对照第 2~5 节的自检表核对 JSON 输出。
5. 截图确认控制位置与形状。
6. 保存后重开资产，确认 `BS_UP_TO_DATE`、控制仍在。

## 8. 验收清单

- [ ] 10 个眉毛控制全部存在，类型 `POSITION`，父级 `Root`。
- [ ] 16 个眼周控制全部存在，类型 `POSITION`，父级 `Root`。
- [ ] 2 个上眼皮控制存在：`(0,0,-10)` → 三骨 dP/旋转与 4.6 表一致；限值单侧（X/Y=0，Z∈[-10,0]），`+Z` 被夹回 0。
- [ ] 形状全部 `Box_Thick`，控制 scale 全 `(1,1,1)`。
- [ ] 整体：`±10` → 本侧 4 骨各 `±5`，另一侧 0。
- [ ] 眉点控：`±10` → 本骨 `±10`，其余 0。
- [ ] 眼控：`±10` → 对应骨 `±10`，其余 15 根眼骨 0。
- [ ] 弯曲：`±2` → 位移/旋转与第 5 节表一致。
- [ ] 叠加：整体 + 点控 + 弯曲同时给值 = 三者之和。
- [ ] 编译 `BS_UP_TO_DATE`，无重复节点（`BROW_` 36、`BPT_` 18、`BB_` 43、`BBL_` 43、`EYE_` 96、`LID_` 42）。
- [ ] 场景里没有残留的探测值（弯曲控制回 0）。

## 9. 扩展新部位的模板

新部位迁完后，按下面的格式追加一节：

```
### 配方 X：<部位名>（<左/右各几个>）

| 项 | 值 |
|---|---|
| 控制名 / 显示名 | ... |
| 类型 / 限值 / 映射 | ... |
| 世界位置 | ... |
| 形状 / 颜色 | ... |
| 驱动骨骼 | ... |
| 图（插入点、前缀） | ... |
| 自检 | ... |
| 脚本 | ... |
```

并同步更新：

- 第 2 节总表
- 第 6 节叠加顺序（若是新的层级）
- 第 8 节验收清单

## 10. 已知边界

- 弯曲控制只响应 **Z 轴**拖动，X/Y 无效（与 Max 一致）。
- 左右弯曲的斜率**互相不镜像**（`D_pos/D_neg` 符号不同），不能直接套用一侧数据。
- 点控与弯曲都挂在骨骼上，不驱动 `meibu_*`；只有整体控制带 `meibu_*`。
- 本文件覆盖眉毛、眼周与上眼皮；其余面部部位（嘴、脸颊、下巴等）仍在 Max 里，尚未建配方。
- 眼周只做了"点控"（每骨 1:1），没有整体位移/开合控制；眼睑开合由配方 E 的上眼皮控制负责。
- 上眼皮限值单侧：**X/Y 锁 0，Z ∈ [-10, 0]**，只能向下拖（Max 的 limiter 同样单向）；增益 13.5 倍——Max 的实际拖动行程约 135 cm，UE 里压缩到 10 cm。
- 旋转类控制必须用 `MathQuaternionMul(A=Delta, B=Initial)`，否则世界轴被当局部轴，眼皮会穿模。
- Max 里的 `KPoint*` 是表达式从动点，不参与 UE 驱动；UE 里直接驱动骨骼。
