# 3ds Max 控制器迁移到 UE Control Rig 指南

适用范围：把 3ds Max 里的面部控制器（`K_` 点、滑条、Animcraft 曲线手柄）在 UE 5.8 Control Rig 里复现成同样的手感。

本文按 `CR_Seven_MorphSliders` 眉毛控制的三次重做整理，所有数值都是实测，不是推算。**先读第 5 节的踩坑表，能省掉大半返工。** 时间紧就直接照第 8 节 SOP 和第 9 节模板抄。

> 已经迁完、验证通过的部位（眉毛 10 个控制）整理在
> [SevenFaceControllerRecipes.md](SevenFaceControllerRecipes.md)——那份文档不需要 Max 文件，
> 照表就能从零搭建。本文只负责「新部位怎么查 Max、怎么迁、怎么验证」。

---

## 0. 总览

| 步骤 | 产出 | 判定标准 |
|---|---|---|
| 1. 查 Max | 控制器名、类型、父级、三轴各拖 10 后谁动、动多少 | 记录成「位移 → 位移」的系数 |
| 2. 搭 UE | 骨骼链、控制类型/形状/位置、RigVM 图 | 数值自检通过，且编辑器里拖得动 |
| 3. 对轴向 | Space 选择 + 分量映射 | 单轴单拖，模型方向对 |

顺序不能反。轴向问题必须在**单轴干净状态**下验证：一次只变一个分量。

### 0.1 十分钟快速路径（按这个顺序，不要跳）

| # | 动作 | 工具 / 命令 | 验收 |
|---|---|---|---|
| 1 | 备份 rig 与 max | `Copy-Item ... Saved\Codex\Backup\..._<时间戳>.uasset` | 备份文件存在且大小 > 0 |
| 2 | Max 枚举目标对象 | `max_run.ps1` + 模板 A | 拿到 `classOf` / 父级 / 世界坐标 |
| 3 | 结构 dump | 模板 A（同脚本） | 分清 `position_list` 各层、表达式、约束 |
| 4 | 世界移动矩阵 | 模板 A（同脚本） | 每个对象 × 3 轴，记录命中的对象与位移 |
| 5 | 定主动控制 | 1.3.1 判定表 | 能说出「谁是控制、谁是从动」 |
| 6 | 左右配对 | 1.6 | 镜像对象成对，命名对不上也以坐标为准 |
| 7 | 换算验证 | 3.1 | 用 2–3 根同名骨骼确认 Max→UE 公式 |
| 8 | 读 UE 骨骼坐标 | `probe_ue_brow_bones.py` | 控制位置 = 目标骨骼静止世界坐标 |
| 9 | 建控制 + 图 | 模板 B（改数据表即可） | 编译 `BS_UP_TO_DATE` |
| 10 | 自检 | 模板 B 内置 | 单轴 1:1、无交叉、叠加正确 |
| 11 | 截图 | `CaptureEditorImage`（ue-mcp.ps1） | 手柄贴在目标部位、大小合适 |
| 12 | 保存 + 记文档 | `save_loaded_asset` | uasset 时间戳更新、指南补映射表 |

**时间参考**：本次六个点控从查 Max 到保存约 20 分钟；熟练后目标 10 分钟。

---

## 1. 第一步：查清 Max 侧

### 1.1 先分清是骨骼、morph 还是两者都有

| 现象 | 判定 |
|---|---|
| 拖控制器，网格顶点动，骨骼不动 | 纯 morph / BlendShape |
| 拖控制器，骨骼动，顶点跟着骨骼走 | 纯骨骼 |
| 两者一起变 | 两者都有（最常见） |

面板类控制器（`Rectangle002` + `Circle113` + `Text001`）通常只是 UI，真正驱动的是挂在旁边的 `K_` 点或 Animcraft 曲线手柄。

### 1.2 怎么在 Max 里跑脚本（这次的做法）

3ds Max 装了 `3dsmax-mcp` 插件（`C:\ProgramData\Autodesk\ApplicationPlugins\3dsmax-mcp`），它在 `\\.\pipe\3dsmax-mcp-pid-<pid>` 上开命名管道，协议是「一行 JSON 进，一行 JSON 出」。

仓库里的 `Scripts/max_run.ps1` 已经封装好：

```powershell
pwsh -NoProfile -File Scripts\max_run.ps1 -ScriptFile Scripts\probe_animcraft_world.ms
```

多开 Max 时用 `-MaxPid <pid>` 指定目标（默认取最近启动的 `3dsmax` 进程）；`Get-Process 3dsmax` 可看 PID 与当前场景窗口标题。

脚本里发的是：

```json
{"type":"maxscript","command":"( ...maxscript... )"}
```

返回 `{"success":true,"result":"...","meta":{...}}`。`safeMode=true` 时 `doscommand` / `createfile` / `python.execute` 会被拦，其余照常执行。

### 1.3 探测脚本要点

0. **先按前缀枚举，别直接猜对象名**：`for o in objects where (matchPattern o.name pattern:"zCircle*") collect o`，一次打印数量、类、父级、世界坐标。本次 49 个 `zCircle` 里只有 6 个是目标，目标靠「世界坐标 + 镜像成对」筛出来的。
1. 打印控制器自身：`classOf`、`parent`、`pos`、`rotation`、`scale`、`position.controller` 的 list 各项。
2. **用世界空间移动**再读结果，避免父级缩放干扰：
   ```maxscript
   move n [10,0,0]     -- 世界位移
   ```
   不要用 `n.pos += [10,0,0]`，那是父空间，遇到 `-0.94` 缩放会算错。
3. 移动前后遍历 `objects` 比较 `(o.transform).row4`（Biped 对象没有 `.pos`，会报错）。
4. 每次测完复位。**复位存 `transform` 而不是 `pos`**：`local saved = n.transform` … `n.transform = saved`，父级有旋转/缩放时只恢复 `pos` 会偏。
5. 单次测试的位移阈值用 `> 0.002`，只打印命中的对象名 + 位移，否则几百个对象刷屏。
6. 一次只测一个对象、一个轴；测完立刻复位再测下一个。

### 1.3.1 谁才是主动控制：三步判定

**不能按名字或父子关系猜。** 本次 `zCircle` 是 `KPoint` 的子对象，但主动的是 `zCircle`，父级 `KPoint` 反而被表达式锁死。

1. **看结构**：`position_list` 里有没有 `Position_Expression` / 约束 / wire。有表达式的点通常是从动。
2. **世界移动**：`move n [10,0,0]`，比较所有对象 `transform.row4` 的前后差。
3. **看是否复位**：移动后对象立刻回到原值 → 被表达式/约束锁死，是从动点。

| 移动后的现象 | 判定 | 本次例子 |
|---|---|---|
| 自身动 + 某根骨骼 **1:1** 动 | 主动控制，骨骼直接跟随 | `zCircle051` → `Bone010` |
| 自身动 + 多根骨/多个对象动 | 主控，继续拆每个对象的系数 | `Animcraft_*_Brow_Z_Ctrl` |
| 只有自身动 | 纯显示/形状（或只驱动顶点 morph） | 纯 UI 面板 |
| 自身不动（被弹回） | 从动点，被表达式锁定 | `KPoint051`（`MeiMao_SX` 表达式） |

> 表达式控制器**读不到变量绑定**（见 1.4），所以「它绑在谁身上」这个问题只能靠移动响应反推，不要试图枚举。

### 1.4 表达式控制器

`position_list` 的第 2 项常是 `Position_Expression`。取公式：

```maxscript
local expr = (getNodeByName "meibu_R")[#Transform][#Position][2].controller
expr.getExpression()
```

但**变量绑定读不到**：`getNumVariables` / `getVariableName` / `getVariableTarget` 在这版全报错。别猜绑定，直接移动控制器看骨骼世界位移，用实测反推。

### 1.5 记录表（填完再进 UE）

| 项 | 示例（Animcraft 右眉整体） | 示例（本次点控 `zCircle051`） |
|---|---|---|
| Max 对象名 / 类型 | `Animcraft_Right_Brow_Z_Ctrl`（NURBSCurveShape） | `zCircle051`（SplineShape） |
| 父级 | `Point471 → DAnimcraft_Master_GRP → … → BIPRoot` | `KPoint051`（被动，表达式锁定） |
| 坐标（Max 世界） | — | `(-1.178, -9.763, 161.442)` |
| 镜像对象 | `Animcraft_Left_Brow_Z_Ctrl` | `zCircle049` |
| 世界 X +10 | 整条右眉 +5 | `Bone010` +10 |
| 世界 Y +10 | 无效果 | `Bone010` +10 |
| 世界 Z +10 | 整条右眉 +5 | `Bone010` +10 |
| 驱动对象（精确名） | `meibu_R` + `Bone010/011/012` | `Bone010` 一根 |
| 系数 | `0.5` | `1.0` |
| 交叉影响 | 左眉完全不动 | 其余骨 = 0 |
| 是否带动 `meibu_*` | 是 | 否 |
| morph 参与 | 无 | 无 |
| 与其它控制的叠加 | 与点控同时驱动同一批骨 | 与整体眉控制同时驱动同一根骨 |

**系数 = 骨骼位移 ÷ Max 位移**，逐轴分别算；三轴不一致就说明有父级缩放或局部空间问题，先查再进 UE。

### 1.6 左右镜像配对

- 判据是**坐标**：X 取反、Y/Z 相同（本骨架 Y 是前后）。命名不一定对称——本次 `zCircle051` 的镜像是 `049`，不是 `052`。
- 配对后左、右各测一遍三轴。用 `GlobalSpace` 时两侧应当完全一致；若符号相反，说明有 `.pos` 或局部轴参与，回头查空间。
- 数量对不上时（比如 6 个点只找到 5 个镜像），先打印所有同前缀对象的世界坐标，按位置聚类，不要按名字猜。

---

## 2. 第二步：UE 侧结构

### 2.1 清空并全量重导骨骼

改资产前先备份：

```powershell
Copy-Item Content\_MyTest\Mouth\Rig\CR_Seven_MorphSliders.uasset Saved\Codex\Backup\..._<时间戳>.uasset
```

`Scripts/clear_rig.py` 做的事，顺序别乱：

1. 删掉除 `Begin` 外的所有 RigVM 节点（`ctrl.remove_node(n, False)`）
2. 删控制、null（控制先删，避免父级删除报错）
3. `import_bones_from_skeletal_mesh(mesh, 'None', True, True, False, False, False)`

   参数含义：`replace_existing_bones=True, remove_obsolete_bones=True`。Seven 骨架一次导入 **149 根**。

> 返回值只列「新加」的骨骼，已存在的不在列表里，别拿它当总数。用 `get_all_keys()` 统计。

### 2.2 控制类型

| Max 侧手感 | UE Control 类型 |
|---|---|
| 拖一个球/点，三轴自由移动 | `POSITION` |
| 只在一个平面里拖（滑条、2D 手柄） | `VECTOR2D` |
| 只是一个 0~1 的滑块 | `FLOAT` |

**注意**：已被 Sequencer 打过关键帧的控制**不要改类型**。类型一改，轨迹通道失配，值传不进 rig，表现就是「怎么改都没反应」。要改就先删轨迹。

### 2.3 控制形状与摆放（这次返工最多的地方）

| 目标 | 做法 |
|---|---|
| 手柄贴在目标部位 | `pos` 用**该部位骨骼的世界坐标**，不要凭感觉放 |
| 形状像一枚扁平条 | `Box_Thick`（粗线框盒），`shape_name='Box_Thick'` |
| 尺寸 | 见下方「手柄大小只能走 shape transform」 |
| 颜色 | `settings.shape_color = LinearColor(1.0, 0.42, 0.04, 1.0)`（橙） |

**5.8 的形状名有两套，别混：**

- 形状资产：`Engine\Plugins\Animation\ControlRig\Content\Controls\ControlRig_*.uasset`，例如 `Box_1mm` / `Box_3mm` / `Box_solid`。
- 默认形状库（实际生效的这套）：`/ControlRig/Controls/DefaultGizmoLibraryNormalized`，名字是 `Box_Thin` / `Box_Thick` / `Box_Solid`。
  `Box_Thick` 的 proxy 就是 `ControlRig_Box_3mm`（粗线框），`Box_Thin` 是 `ControlRig_Box_1mm`。
- 本项目所有眉毛控制统一用 **`Box_Thick`**。名字大小写必须与库里一致（写 `Box_thick` 不报错，但显示会回退）。

确认 shape_name 是否存在：

```python
lib = unreal.load_asset('/ControlRig/Controls/DefaultGizmoLibraryNormalized')
[s.get_editor_property('shape_name') for s in lib.get_editor_property('shapes')]
```

**手柄大小只能走 shape transform，绝不能改控制本身的缩放。**

控制是 `POSITION` 时，控制 transform 的 scale 会参与 gizmo 拖动换算。一旦设成非均匀缩放（比如用 `(0.42, 0.22, 0.05)` 把 `Box_1mm` 压成扁条），拖 1 cm 世界距离，控制值按 `1/0.05 = 20` 被放大，眉毛就飞出去 20 倍。而且三个轴比例不同，缩放方向的手感还会各不相同。

正确写法：

```python
# 控制 transform：只定位置，缩放必须是 1
off = h.get_global_control_offset_transform(k)
off.translation = unreal.Vector(*pos)
off.rotation = yaw
off.scale3d = unreal.Vector(1.0, 1.0, 1.0)
h.set_control_offset_transform(k, off, True, True)
h.set_control_offset_transform(k, off, False, True)

# 手柄外观：shape transform，只影响显示
shape_tf = unreal.Transform()
shape_tf.scale3d = unreal.Vector(0.42, 0.22, 0.05)
h.set_control_shape_transform(k, shape_tf, True)   # initial
h.set_control_shape_transform(k, shape_tf, False)  # current
```

两个坑：

- `RigControlSettings.shape_transform` 是**只读**属性，直接赋值会报 `read-only and cannot be set`，只能用 `set_control_shape_transform`。
- `initial` 和 `current` **都要设**。只设 current 时，编辑器里显示的仍是初始值（默认 1,1,1），看上去手柄依旧是巨大的一块。

**尺寸坑**：`Box`（无后缀）默认是 1 米级的盒子，`scale 1.9` 会直接糊满半个屏幕；`Box_Thin` / `Box_Thick` 才是 10 cm 级，且两者基准尺寸相同（Thick 只是线更粗，约 3 mm），所以从 `Box_1mm` 换 `Box_Thick` 时 **shape scale 不用改**。摆之前先想清楚基准尺寸。

**先量 Max 原手柄**：`numKnots` + `getKnotPoint` 算包围盒，本次六个 zCircle 直径约 0.8 cm；UE 里 `Box_Thick` 的 shape scale 按「目标直径 ÷ 10」估（0.12 ≈ 1.2 cm），再截图微调。点与点间距不足 3 cm 时，手柄别超过 1.5 cm，否则会互相压住。

**位置坑**：不要假设「Y 就是左右」。这具骨架 `meibu_L` 在 `(4.46, 7.49, 161.39)`、`meibu_R` 在 `(-4.46, 7.49, 161.39)` —— **X 才是左右，Y 是前后，Z 是上下**。按坐标轴直觉放会把两个手柄都甩到脑后。

### 2.4 控制挂哪儿

父级用 `Root` 骨骼（世界变换是单位矩阵），再用 `set_control_offset_transform` 把 `initial/current` 都设成目标世界变换：

```python
off = h.get_global_control_offset_transform(k)
off.translation = unreal.Vector(*pos)
off.rotation = yaw_quat
off.scale3d = unreal.Vector(1.0, 1.0, 1.0)   # 必须 1，原因见 2.3
h.set_control_offset_transform(k, off, True, True)   # initial
h.set_control_offset_transform(k, off, False, True)  # current
```

---

## 3. 第三步：驱动图与轴向

### 3.1 两个引擎的坐标系

本骨架（Seven）实测：

| | 3ds Max | UE（本骨架） |
|---|---|---|
| 上 | Z | Z |
| 前 | −Y | +Y |
| 左 | +X | +X |
| 手性 | 右手 | 左手 |

**换算：`UE = (Max.x, −Max.y, Max.z)`**，两边单位都是 cm。

验证方法：挑 2–3 根同名骨骼比世界坐标。本次实测：

- `Bone008`：Max `(1.178, −9.763, 161.442)` → UE `(1.178, 9.763, 161.442)`
- `meibu_L`：Max `(4.458, −7.490, 161.395)` → UE `(4.458, 7.490, 161.395)`

> 文档里常见的 FBX 公式是 `UE = (−Max.y, −Max.x, Max.z)`（对应 UE 前 +X、右 +Y 的朝向）。
> **本骨架不是那个朝向**（前 +Y），套错公式会让所有控制绕头转 90°。
> 永远先用骨骼坐标确认公式，再摆控制——这一步 30 秒，能省半小时。

### 3.2 GlobalSpace 还是 LocalSpace

**结论：这套骨架用 `GlobalSpace` + `SetTransform`。**

实测（`set_global_transform` 加 5 cm）：

| 空间 | `meibu_L` +X/+Y/+Z | `meibu_R` +X/+Y/+Z |
|---|---|---|
| Global | `(5,0,0)` / `(0,5,0)` / `(0,0,5)` | 同左，完全一致 |
| Local | `(-5,0,0)` / `(0,0,-5)` / `(0,5,0)` | `(5,0,0)` / `(0,0,-5)` / `(0,5,0)` |

`LocalSpace` 下左右两侧**符号相反**（`meibu_L` 带 `(-1,-1,-1)` 缩放），写「局部 Y = 世界 Z」只能对一侧成立，另一侧会反向。上一版就是这么写坏左眉的。

`GlobalSpace` 每根骨都直线位移、左右一致，所以图里读写都用 `GlobalSpace`，配合 `bInitial=True` 读初始值避免逐帧累积漂移。

### 3.3 节点与引脚

```
GetControlVector(CTRL, LocalSpace)          -- 控制值本身（挂在 Root 下，等价于世界）
   → VectorScale(Factor = 系数)
GetTransform(bone, GlobalSpace, bInitial=True)
   .Transform.Translation → VectorAdd(B = 偏移)
   .Transform.Rotation / .Transform.Scale3D
   → MathTransformMake(Translation, Rotation, Scale)
   → SetTransform(Item=bone, Space=GlobalSpace, bInitial=False, Value, Weight=1, bPropagateToChildren=False)
```

引脚坑：

| 坑 | 现象 | 正确写法 |
|---|---|---|
| `GetTransform.Transform` 的缩放子引脚 | `Cannot find pin '...Transform.Scale'` | 叫 **`Transform.Scale3D`** |
| `SetTransform` 的输入引脚 | 找 `Transform` 找不到 | 叫 **`Value`** |
| `SetTransform` 没有 AffectChildren | — | 用 `bPropagateToChildren` |
| `RigHierarchy` 没有 `set_translation` | `AttributeError` | 用 `set_global_transform` / `set_local_transform` |

### 3.4 系数

Max 里拖 10 cm → 骨骼 5 cm，UE 控制设 `±10`、系数 `0.5`。

**但系数不是固定的**：本次眉毛点控是 1:1（10 → 10），整体眉控制是 0.5。每个控制器单独测，别照抄上一个。
公式：`系数 = 骨骼世界位移 / Max 世界位移`；三轴分别核一遍。

### 3.5 与已有控制叠加（同一根骨有多个控制源）

同一根骨已经被一个控制驱动时，**不要**在后面再串一条 SetTransform——第二条要么读到已被改过的值，要么两条都读 `bInitial` 互相覆盖。
正确做法是在 `MAKE` 之前插一个 `VectorAdd`，把两路偏移加在一起：

```
GetTransform(bone, Global, bInitial=True).Translation
  → VADD1(A=初始, B=控制1偏移)      -- 已有
  → VADD2(A=VADD1, B=控制2偏移)     -- 新插
  → MAKE.Translation
  → SET
```

```python
# 先断开 MAKE 原来的 Translation 连接，再接回
ctrl.break_link('BROW_MAKE_%s.Translation' % bone)
ctrl.add_link(add2 + '.Result', 'BROW_MAKE_%s.Translation' % bone, False)
```

- `RigVMController.break_link(pin_path)` 存在；`add_link` **不会**自动替换旧连接。
- 多条链共用 `bInitial=True` 的 GetTransform 是安全的，只要都读同一个初始值再相加。
- 幂等脚本要用**独立前缀**（本次用 `BPT_`），不要复用会被其它脚本整体清掉的 `BROW_`。

带旋转的叠加（新控制还影响骨骼朝向时）：

```
BT_Get(bone, Global, bInitial=True).Transform.Rotation
  → QuatMul(A=初始, B=Delta)   -- 世界 X 轴增量
  → MAKE.Rotation
Delta = QuatFromAxisAndAngle(Axis=(1,0,0), Angle=-dEuler * deg2rad)
```

- 用 `break_link('BROW_MAKE_<骨>.Rotation')` 断开原连接；
- `A/B` 顺序与角度符号见 4.6——这两处错了会出现「角度对、方向乱」。

---

## 4. 实战：眉毛控制（整体 + 六个点控）

### 4.1 Max 侧

`Animcraft_Right_Brow_Z_Ctrl` / `Animcraft_Left_Brow_Z_Ctrl`，各驱动本侧 `meibu_*` + 3 根眉骨，世界 X/Z 各 0.5 倍，Y 无效，左右互不影响。

子对象 `Animcraft_*_Brow_Ctrl` 位置被表达式锁死，拖动只有 0.1–0.8 cm 形状差，**不是可交互主控**，不要照它建控制。

### 4.2 UE 侧

| 项 | 值 |
|---|---|
| Rig | `/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders` |
| 控制 | `CTRL_BROW_L` / `CTRL_BROW_R`，`POSITION`，±10 |
| 显示名 | 左眉整体 / 右眉整体 |
| 形状 | `Box_Thick`，橙色；尺寸走 shape transform `(0.42, 0.22, 0.05)`，控制自身缩放 `(1,1,1)` |
| 位置 | `(4.0, 11.2, 161.2)` / `(-4.0, 11.2, 161.2)`，父 `Root` |
| 骨骼 | L：`meibu_L` + `Bone007/008/009`；R：`meibu_R` + `Bone010/011/012` |
| 系数 | `0.5` |

### 4.3 自检数据（实例化后骨骼世界位移，cm）

| 输入 | 左眉 4 骨 | 右眉 4 骨 |
|---|---|---|
| `CTRL_BROW_L = (10,0,0)` | 全 +5 X | 0 |
| `CTRL_BROW_L = (0,0,10)` | 全 +5 Z | 0 |
| `CTRL_BROW_R = (10,0,0)` | 0 | 全 +5 X |
| `CTRL_BROW_R = (0,0,10)` | 0 | 全 +5 Z |

### 4.4 复现

```powershell
python Scripts\ue_run.py Scripts\clear_rig.py       # 清空 + 全量重导骨骼
python Scripts\ue_run.py Scripts\build_brow_two.py  # 两个控制 + 图 + 自检（幂等）
```

### 4.5 六个点控（zCircle049/050/048、zCircle051/052/053）

Max 侧：每侧眉毛三个可拾取样条点（`SplineShape`），拖动时**只驱动本侧一根眉骨**，
世界三轴都是 **1:1**，左右、点与点之间都无交叉，也不带动 `meibu_*`。
它们的父级 `KPoint049…053` 的 position 被 `MeiMao_SX` 表达式锁定，**K 点不是主动控制，zCircle 才是**。

| Max | UE 控制 | 显示名 | 骨骼 | 控制世界位置（UE） |
|---|---|---|---|---|
| `zCircle049` | `CTRL_BROW_L_IN` | 左眉内点 | `Bone008` | `(1.178, 9.763, 161.442)` |
| `zCircle050` | `CTRL_BROW_L_MID` | 左眉中点 | `Bone009` | `(4.071, 9.009, 161.795)` |
| `zCircle048` | `CTRL_BROW_L_OUT` | 左眉外点 | `Bone007` | `(6.748, 7.371, 162.146)` |
| `zCircle051` | `CTRL_BROW_R_IN` | 右眉内点 | `Bone010` | `(-1.177, 9.763, 161.442)` |
| `zCircle052` | `CTRL_BROW_R_MID` | 右眉中点 | `Bone011` | `(-4.070, 9.009, 161.795)` |
| `zCircle053` | `CTRL_BROW_R_OUT` | 右眉外点 | `Bone012` | `(-6.748, 7.371, 162.146)` |

- 类型 `POSITION`，限值 ±10，**系数 1.0**（不是整体眉的 0.5）
- 形状 `Box_Thick`，shape transform `(0.12, 0.10, 0.05)`（Max 侧样条直径约 0.8 cm）
- 父级 `Root`；控制位置 = 对应骨骼的静止世界坐标（Max zCircle 与骨骼重合）
- 坐标换算实测为 **`UE = (Max.x, −Max.y, Max.z)`**（本骨架不使用 `(−y, −x, z)`）
- 图上与整体控制相加：`BROW_VADD_<骨>` → `BPT_ADD_<骨>` → `BROW_MAKE_<骨>` → `SET_<骨>`

自检（实例化后骨骼世界位移，cm）：

| 输入 | 骨骼位移 |
|---|---|
| 任一点控 `(10,0,0)` | 只有对应骨 `(10,0,0)`，其余 0 |
| 任一点控 `(0,10,0)` / `(0,0,10)` | 只有对应骨 `(0,10,0)` / `(0,0,10)` |
| `CTRL_BROW_L=(10,0,0)` + `CTRL_BROW_L_IN=(0,0,10)` | `Bone008=(5,0,10)`，`Bone007/009=(5,0,0)` |

复现：`python Scripts\ue_run.py Scripts\build_brow_points.py`（幂等）
 
> 点控依赖 `BROW_VADD_*` / `BROW_MAKE_*` 节点。若重跑 `clear_rig.py` 或 `build_brow_two.py`
> 把整体眉图重建了，需要再跑一次 `build_brow_points.py` 把相加节点接回去。

### 4.6 右眉弯曲（Animcraft_Right_Brow_Ctrl）

**这个不是单骨骼控制。** `Animcraft_Right_Brow_Ctrl`（NURBSCurveShape，父级 `Animcraft_Right_Brow_Z_Ctrl`）
的 `position_list[2]` 是逐轴 `float_limit`：x 锁在 0、y ∈ [-2,2]、拖动时 z 被夹到 -1。
位置值 → `KPoint051/052/053` 的 `Position_Expression`（变量 `MeiMao_SX`）→ 三根眉骨
`Bone010/011/012` 的位置 + 绕世界 X 的旋转；`meibu_R` 不动。

所以拖动是**分段线性**，不是单一系数：
- y>0 与 y<0 两条斜率不同（K 点里的 `if` 分支）；
- limiter 夹住后，拖 10 和拖 20 结果相同（拖不出更多）；
- 拖动后 `p2` 的 z 分量虽然显示 `-1`，但它**不进入骨骼驱动**——实际位移与 `z=0` 完全相同。
  别把"z=-1 的外移"写进图里（早期版本加过，表现就是弯曲时眉毛边缘外飘）。

实测基准（`position=(0,0,0)`）：Bone010 `(-1.17773,-9.76263,161.442)`、
Bone011 `(-4.07024,-9.00898,161.795)`、Bone012 `(-6.74794,-7.37118,162.146)`，欧拉 X 都是 90°。

| 相对基准 | Bone010 | Bone011 | Bone012 |
|---|---|---|---|
| `y=+2, z=0` dP | `(-0.01744,-0.19924,0.3)` | `(-0.18126,0.08452,0.8)` | `(0,0,0.1)` |
| `y=-2, z=0` dP | `(0.08218,-0.20796,-0.5)` | `(0.29491,-0.31406,-0.62)` | `(0,0,-0.1)` |
| 旋转 `y=+2`（度） | `-9.1674` | `0` | `+10.313` |
| 旋转 `y=-2`（度） | `+11.4589` | `-3.4377` | `-4.5837` |

UE 实现：

| 项 | 值 |
|---|---|
| 控制 | `CTRL_BROW_R_BEND`，`POSITION`，±2，取控制值 **Z 分量** `c` |
| 位置 | `(-2.2336, 11.5447, 162.256)`，父 `Root` |
| 形状 | `Box_Thick`，青色 `(0.15, 0.85, 1.0)`，shape scale `(0.15, 0.10, 0.05)` |
| 位置映射 | `c>0`: `(c/2)*D_pos`；`c<0`: `(-c/2)*D_neg` |
| 旋转 | 绕世界 X 轴；`NewRot = Initial * Delta`，角度 = `-(dEuler)` |
| 图 | `BPT_ADD_<骨>` → `ADD1(+分支偏移)` → `MAKE.Translation`；`MAKE.Rotation` 改接 `QROT` |
| 分支节点 | `MathFloatGreater` + `MathVectorSelectBool` / `MathFloatSelectBool` |

自检（实例化后骨骼世界位移 / 绕 X 旋转，cm / 度）：

| 输入 | Bone010 | Bone011 | Bone012 |
|---|---|---|---|
| `BEND=(0,0,2)` | `dP=(-0.0174,-0.1992,0.3)` / `-9.167°` | `(-0.1812,0.0845,0.8)` / `0` | `(0,0,0.1)` / `+10.313°` |
| `BEND=(0,0,-2)` | `(0.0822,-0.2079,-0.5)` / `+11.459°` | `(0.2949,-0.3141,-0.62)` / `-3.438°` | `(0,0,-0.1)` / `-4.584°` |
| `CTRL_BROW_R=(10,0,0)` + `BEND=(0,0,2)` | `(4.9826,-0.1992,0.3)` | `(4.8188,0.0845,0.8)` | `(5,0,0.1)` |

三个关键坑：

- **实际拖动值才是准的**：手动 `controller[2].value = [0,2,-1]` 与拖动后读到的 `(0,2,-1)` 结果不同
  （limiter 的 z 不参与驱动）。一律以 `move` 后的骨骼世界位移为准。
- **四元数合成顺序**：`MathQuaternionMul` 要 `A=初始旋转, B=增量`（`Initial * Delta`），
  并且增量角度取 `-dEuler`——UE 的 Rotator.Roll 与 Max 欧拉 X 方向相反。
  用 `Delta * Initial` 会变成绕骨骼局部轴，结果 pitch/yaw/roll 全部串味。
- **SelectBool 引脚**名是 `Condition` / `IfTrue` / `IfFalse`，不是 `bPickTrue` / `A` / `B`（写错直接 `Cannot find pin`）。

复现：`python Scripts\ue_run.py Scripts\build_brow_bend.py`（幂等）

### 4.7 左眉弯曲（Animcraft_Left_Brow_Ctrl）

与 4.6 镜像：`Animcraft_Left_Brow_Ctrl`（NURBSCurveShape，父级 `Animcraft_Left_Brow_Z_Ctrl`，
世界位置 `(3.17631,-11.5447,162.256)`，scale `+0.9426`），同样是 float_limit + K 点表达式，
驱动 `Bone008/009/007`（内/中/外）的位置 + 绕世界 X 旋转，`meibu_L` 不动。

> 左眉实际拖动时 `p2=(0,±2,0)`，z 保持 0（父级正缩放），所以**没有右眉那种 z=-1 的干扰**。
> 弯曲时眉毛外缘不会外飘——7.1 版本的图里误加过 zoff，表现就是外缘偏移。

基准（`position=(0,0,0)`）：Bone007 `(6.74793,-7.37117,162.146)`、
Bone008 `(1.17773,-9.76263,161.442)`、Bone009 `(4.07027,-9.00897,161.795)`，欧拉 X 都是 90°。

| 相对基准 | Bone007 | Bone008 | Bone009 |
|---|---|---|---|
| `y=+2, z=0` dP | `(0,0,0.1)` | `(0.01744,-0.19924,0.3)` | `(0.18126,0.08452,0.8)` |
| `y=-2, z=0` dP | `(0,0,-0.1)` | `(-0.08219,-0.20795,-0.5)` | `(-0.2949,-0.31406,-0.62)` |
| 旋转 `y=+2`（度） | `-10.3132` | `+9.1673` | `0` |
| 旋转 `y=-2`（度） | `+4.5837` | `-11.4591` | `+3.4378` |

UE：`CTRL_BROW_L_BEND`，`POSITION`，±2，位置 `(3.17631, 11.5447, 162.256)`，
`Box_Thick` 青色，形状与逻辑和 4.6 完全一致（正负分支 + `Initial * Delta`，无 zoff）。

自检（与 Max 对照一致）：

| 输入 | Bone007 | Bone008 | Bone009 |
|---|---|---|---|
| `BEND=(0,0,2)` | `dP=(0,0,0.1)` / `-10.313°` | `(0.0174,-0.1992,0.3)` / `+9.167°` | `(0.1813,0.0845,0.8)` / `0` |
| `BEND=(0,0,-2)` | `(0,0,-0.1)` / `+4.584°` | `(-0.0822,-0.2079,-0.5)` / `-11.459°` | `(-0.2949,-0.3141,-0.62)` / `+3.438°` |

复现：`python Scripts\ue_run.py Scripts\build_brow_bend_left.py`（幂等）

---

## 5. 踩坑速查表

| 坑 | 现象 | 解决 |
|---|---|---|
| 用控制 transform 的 scale 调手柄大小 | 拖一点手柄，眉毛飞很远（灵敏度被 `1/scale` 放大），各轴还不一致 | 控制 scale 保持 1，尺寸改走 `set_control_shape_transform` |
| `settings.shape_transform` 赋值 | `read-only and cannot be set` | 用 `set_control_shape_transform(key, tf, initial)` |
| 只设 current 的 shape transform | 手柄仍是巨大一块 | `initial` 和 `current` 都设 |
| 视口显示 `Previewing Reference Pose` | 拖控制完全没反应 | 这是编辑器状态，不是资产问题，切到 `Previewing Rig` |
| 用 `LocalSpace` + 局部 Y | 左眉一上、右眉一下 | 这套骨架改用 `GlobalSpace` |
| 按「Y = 左右」摆控制 | 手柄跑到脑后 / 头上 | 先读骨骼世界坐标，X 才是左右 |
| `Box` 直接摆 | 手柄变成糊满屏幕的巨大盒子 | 用 `Box_Thick`，或把 shape scale 缩到 0.1 量级 |
| `GetTransform.Transform.Scale` | 报 `Cannot find pin` | 写 `Scale3D` |
| `SetTransform` 连“Transform” | 找不到引脚 | 输入引脚叫 `Value` |
| `RigHierarchy.set_translation` | `AttributeError` | 用 `set_global_transform` / `set_local_transform` |
| 在脚本里 `inst.execute('Forwards Solve')` 于 debugged rig | 编辑器预览骨骼被推到世界某处 | 用 `create_control_rig()` 建临时实例自检 |
| 只看返回值不看画面 | 数值对、方向错 | 数值自检 + 编辑器截图双确认 |
| 只测一侧 | 另一侧符号反了没发现 | 左右各测一遍 |
| 拿 `import_bones` 返回值当骨骼总数 | 数量对不上 | 用 `get_all_keys()` 统计 |
| 按父子关系判断谁是控制器 | 子对象 `zCircle` 才是主动，父级 `KPoint` 被表达式锁死 | 一律用世界移动 + 全对象 diff 判定（1.3.1） |
| MaxScript 里把函数命名成 `snapshot` | `Cannot assign to read-only variable: snapshot` | 换名（本次用 `snapAll`） |
| 脚本里拼 `getSourceFileName()` | `Unable to convert: undefined to type: String` | 该 API 在 MCP 环境不可用，删掉或 `try` 包住 |
| 移动后只恢复 `pos` | 父级带旋转/缩放时会偏 | 保存/恢复整个 `n.transform` |
| 系数照抄上一个控制器 | 点控写 0.5，拖 10 只动 5 | 每个控制器实测（3.4） |
| 手动设 `controller[2].value` 当作拖动 | 端点对、边缘偏移 | 一律以 `move` 后的骨骼位移为准（4.6） |
| 左右直接复用同侧 zoff 数据 | 左眉弯曲时外缘外飘 | 左右分别实测；左眉实际拖动 z 不参与（4.7） |
| 两条 SetTransform 串行做叠加 | 后一条覆盖前一条，或都读 bInitial 互相打架 | MAKE 前插 VectorAdd（3.5） |
| 幂等脚本复用公共前缀 | 重跑整体脚本后点控节点悬空或消失 | 独立前缀 + 在文档写清重跑顺序 |
| 截图时没选中控制 | 手柄与骨骼线混在一起，看不清位置 | 先 `hc.set_selection([...])` 再 `CaptureEditorImage` |
| Max 里拖 5/10/20 结果完全一样 | 以为数据测错 | 是 `float_limit`/约束 clamp，拖不出更多；改用 `controller[2].value` 扫值 |
| 只恢复 `n.transform` 就以为复原了 | 复合控制器的值仍停在极限，后续基准全错 | 显式把 `controller[i].value` 设回原值 |
| 旋转叠加后 pitch/yaw/roll 串味 | 四元数顺序或符号错 | `MathQuaternionMul` 用 `A=初始, B=增量`；角度取 `-dEuler`（4.6） |
| `MathVectorSelectBool` 引脚写 `bPickTrue` | `Cannot find pin` | 引脚是 `Condition` / `IfTrue` / `IfFalse` |
| shape_name 写成 `Box_thick` | 不报错，但显示回退成默认形状 | 用库里的准确大小写 `Box_Thick`（2.3） |
| `CaptureEditorImage` 报 `Failed to capture any editor windows` | 编辑器不在前台/不在 rig 编辑器页 | 先 `OpenEditorForAsset` 打开 rig，再 `AppActivate` 编辑器进程后截图 |

---

## 6. 验证流程

1. **数值**：`rig.create_control_rig()` → `request_init()` → 设控制值 → `execute('Forwards Solve')` → 读 `get_global_transform(bone).translation`，对比静止状态。每个控制 × 3 轴 × 左右都要跑。
2. **交叉**：给 A 控制值时，B/C 骨骼必须是 0。只看「目标动了」不够，还要看「别的没动」。
3. **叠加**：同一根骨有两个控制源时，两路同时给值，结果必须是两路位移之和（例：整体 10 → +5，点控 Z 10 → +10，最终 `(5,0,10)`）。
4. **画面**：`hc.set_selection(keys)` 选中控制，再 `CaptureEditorImage`，确认手柄位置、形状、大小、颜色。数值对但画面错，通常是坐标换算或父级空间问题。
5. **持久化**：保存后重开资产，控制与节点都在、编译不报错、数值不变。

---

## 7. 脚本索引

| 文件 | 用途 |
|---|---|
| `Scripts/max_run.ps1` | 向运行中的 3ds Max 发 MaxScript（命名管道） |
| `Scripts/probe_animcraft_brow*.ms`、`probe_animcraft_world.ms` | Max 侧结构 / 世界位移探测 |
| `Scripts/inspect_cr7_state.py` | 打印 rig 当前控制、骨骼、节点 |
| `Scripts/clear_rig.py` | 清空 rig + 全量重导骨骼 |
| `Scripts/build_brow_two.py` | 建左右眉整体控制 + 图 + 自检（幂等） |
| `Scripts/probe_template_a.ms` | 模板 A 的可运行版（枚举 + 结构 + 移动矩阵） |
| `Scripts/build_brow_points.py` | 建左右眉各三个点控 + 图 + 自检（幂等） |
| `Scripts/build_brow_bend.py` | 建右眉弯曲控制（多骨骼 + 旋转 + 分支）+ 自检（幂等） |
| `Scripts/build_brow_bend_left.py` | 建左眉弯曲控制（右侧镜像）+ 自检（幂等） |
| `Scripts/build_eye_points.py` | 建左右眼周 16 个点控 + 自检（幂等） |
| `Scripts/probe_eye_r.ms` / `probe_eye_l.ms` | Max 侧眼周 zCircle 结构与驱动响应 |
| `Scripts/probe_animcraft_rb*.ms` | Max 侧 Animcraft_Right_Brow_Ctrl 结构 / 值扫描 |
| `Scripts/probe_animcraft_lb.ms` | Max 侧 Animcraft_Left_Brow_Ctrl 镜像数据 |
| `Scripts/probe_quat_order.py` | 验证 UE 四元数合成顺序与轴（旋转类控制必查） |
| `Scripts/probe_all_shapelibs.py` | 枚举所有 shape library 与 shape_name（查 Box_Thick 这类名字） |
| `Scripts/probe_zcircle*.ms` | Max 侧 zCircle 结构 / 拖动响应探测 |
| `Scripts/probe_ue_brow_bones.py` | 打印 UE 眉毛骨骼世界坐标 / 父级（定控制位置用） |
| `Scripts/set_brow_points_preview.py` | 给 debugged rig 喂值，配合截图看画面 |
| `Scripts/ue-mcp.ps1` | 调编辑器 MCP：`CaptureEditorImage` 截图等 |
| `Scripts/probe_axes.py` | 打印骨骼世界变换与局部轴 |
| `Scripts/probe_translation_space.py` | 实测 Global / Local 位移差 |
| `Scripts/ue_run.py` | 在运行中的编辑器里执行 Python |
| `Saved/Codex/Backup/` | 改动前的 `.uasset` 备份 |

> 改资产前先 `Copy-Item` 备份 `.uasset`，再把脚本包在 `unreal.ScopedEditorTransaction` 里，方便撤销。

---

## 8. 批量迁移 SOP（下次照抄）

### 8.1 开始前 30 秒检查

| 检查 | 命令 | 通过标准 |
|---|---|---|
| Max 开着正确场景 | `Get-Process 3dsmax` + 模板 A 打印 `maxFileName` | 文件名对得上 |
| UE 编辑器在跑 | `Get-Process UnrealEditor` | 有进程 |
| Python 远程执行通 | `python Scripts\ue_run.py Scripts\inspect_cr7_state.py` | 末尾 `UE_REMOTE_OK` |
| MCP 截图可用 | `ue-mcp.ps1 -Toolset EditorToolset.EditorAppToolset -Tool GetOpenAssets` | 返回 rig 资产路径 |
| 资产已备份 | `Copy-Item ... Saved\Codex\Backup\..._<时间戳>.uasset` | 备份文件存在 |

### 8.2 Max 侧（约 5 分钟）

1. **枚举**（模板 A 的 A1 段）：拿到对象名、`classOf`、父级、世界坐标；同名前缀全列出来。
2. **结构**（模板 A 的 A2 段，同一脚本顺带输出）：`position_list` 每一层、`Position_Expression` 公式、有没有 wire/约束。
3. **移动矩阵**（模板 A 的 A3 段）：每个候选对象 × 三轴，各 `move` 10 cm，记录命中对象与位移；只用 `transform.row4` 比较，阈值 0.002。
4. **判定**（1.3.1）：区分主动控制 / 从动点 / 纯形状。
5. **配对**（1.6）：按坐标找镜像，左右各复测一遍。
6. **填表**（1.5）：把系数写成 `骨骼位移 ÷ Max 位移`。

### 8.3 UE 侧（约 5 分钟）

1. 读目标骨骼的静止世界坐标（`probe_ue_brow_bones.py`，改 `names` 列表）。
2. 复制模板 B（`build_brow_points.py`），只改顶部 `POINTS` 表与控制前缀。
3. 跑脚本，看返回的 JSON 自检：
   - 每个控制单轴 `10` → 对应骨 `10`，其余 `0`；
   - 左右互不影响；
   - 叠加项 = 两路系数之和。
4. 截图：先 `hc.set_selection(keys)`，再 `CaptureEditorImage`；确认手柄位置、大小、颜色。

   ```powershell
   $raw = pwsh -NoProfile -File Scripts\ue-mcp.ps1 -Toolset EditorToolset.EditorAppToolset `
       -Tool CaptureEditorImage -Arguments '{}' -Raw
   $json = $raw | ConvertFrom-Json
   [IO.File]::WriteAllBytes("Saved\Codex\shot.png", [Convert]::FromBase64String($json.returnValue.data))
   ```

5. `save_loaded_asset` 保存，记下资产时间戳。

### 8.4 收尾（2 分钟）

- 在本文第 4 节追加映射表（Max 名 → UE 控制 → 骨骼 → 坐标 → 系数）。
- 写清重跑顺序（谁依赖谁的节点）。
- 把新探测/构建脚本加进第 7 节索引。

---

## 9. 脚本模板

### 9.1 模板 A：Max 侧三段探测

把 `TARGETS` 换成要查的对象名（可以用前缀批量生成），一次跑完 A1–A3：

```maxscript
(
    local TARGETS = #("zCircle051", "zCircle052", "zCircle053")
    local out = ""
    fn pstr p = ("[" + ((p.x) as string) + "," + ((p.y) as string) + "," + ((p.z) as string) + "]")
    fn getPos o =
    (
        local p = [0,0,0]
        try ( p = (o.transform).row4 ) catch ( try ( p = o.pos ) catch () )
        p
    )
    fn snapAll = ( for o in objects collect (getPos o) )
    fn diffReport base cur =
    (
        local s = ""
        local hits = 0
        for i = 1 to objects.count do
        (
            local d = cur[i] - base[i]
            if (length d) > 0.002 do
            (
                hits += 1
                if hits <= 15 do s += "   " + objects[i].name + " d=" + ((d as string)) + "\n"
            )
        )
        "   hits=" + (hits as string) + "\n" + s
    )

    out += "scene=" + maxFileName + "\n"
    for nm in TARGETS do
    (
        local n = getNodeByName nm
        if n == undefined then out += "MISSING " + nm + "\n"
        else
        (
            -- A1/A2: 结构
            local par = "-"
            try ( par = n.parent.name ) catch ()
            out += "## " + nm + " (" + ((classOf n) as string) + ") parent=" + par + " wpos=" + (pstr (getPos n)) + "\n"
            try (
                local pc = n.parent.position.controller
                for i = 1 to pc.count do
                (
                    local sc = "-"
                    try ( sc = (classOf pc[i].controller) as string ) catch ( sc = (classOf pc[i]) as string )
                    out += "   pos[" + (i as string) + "] " + sc + "\n"
                    if (findString sc "Expression") != undefined do
                        try ( out += "      expr=" + (pc[i].controller.getExpression()) + "\n" ) catch ()
                )
            ) catch ()

            -- A3: 世界移动矩阵
            local base = snapAll()
            local saved = n.transform
            for ax in #([10,0,0], [0,10,0], [0,0,10]) do
            (
                move n ax
                out += "  +" + (pstr ax) + "\n" + (diffReport base (snapAll()))
                n.transform = saved
            )
        )
    )
    out
)
```

> 只读安全：脚本不改场景（移动后已复位）。`safeMode=true` 下可用。
> 可运行版：`Scripts/probe_template_a.ms`（改 `TARGETS` 后直接发）。

### 9.2 模板 B：UE 侧批量建控制

直接复制 `Scripts/build_brow_points.py`，改这四处：

| 常量 | 含义 |
|---|---|
| `PREFIX` | 本批节点的独立前缀（不要和其它脚本共用） |
| `POINTS` | `(控制名, 显示名, 目标骨骼, UE 世界坐标, Max 原名)` |
| `COEF` / `LIMIT` | 系数（按 3.4 实测）与滑条限值 |
| `SHAPE_SCALE` | 手柄显示尺寸（按部位间距调） |

脚本已经内置：

- 幂等清理（删 `PREFIX` 节点 + 本批控制）
- 控制创建（POSITION / 显示名 / 橙色 / 限值）
- `set_control_shape_transform` 双写（initial + current）
- 与已有控制相加（`break_link` + `VADD2`）
- 自检（单轴 × 方向、交叉、叠加）并打印 JSON

新增一批控制后，记得同步更新第 7 节索引和对应映射表。

> 如果目标控制器驱动多根骨骼、带旋转或分段（如 4.6 / 4.7 的眉毛弯曲），模板以
> `Scripts/build_brow_bend.py`（右）和 `build_brow_bend_left.py`（左，镜像数据）为准：
> 它们演示了 `float_limit` 值扫描、正负分支、`MathVectorSelectBool` / `MathFloatSelectBool`
> 和 `QuatMul` 旋转叠加。

### 9.3 反模式（这些都会导致返工）

| 反模式 | 后果 | 正确做法 |
|---|---|---|
| 没做移动测试就写构建脚本 | 建出来的控制驱动错对象，全部重来 | 先跑模板 A，拿到「对象 → 骨骼 → 系数」表 |
| 按命名/父子关系认定控制器 | 驱动反了（本次 `zCircle` 才是主动） | 1.3.1 三步判定 |
| 按名字猜左右镜像 | 少建或多建控制 | 按坐标配对（1.6） |
| 直接套用网上 FBX 坐标公式 | 控制绕头转 90° | 3.1 用骨骼坐标验证 |
| 系数沿用上一批控制 | 位移差 2 倍或 0 | 每个控制器实测（3.4） |
| 用控制 scale 压缩手柄 | 拖动灵敏度被放大 20 倍 | shape transform（2.3） |
| 串行 SetTransform 做叠加 | 覆盖或双倍 | MAKE 前插 VectorAdd（3.5） |
| 在 `Previewing Reference Pose` 下判断「拖不动」 | 误判资产有问题 | 切 `Previewing Rig` 再试 |
| 只跑数值不做截图 | 位置/朝向错了才发现 | 第 6 节「画面」确认 |
| 多骨骼控制只按单骨骼斜率做 | 端点对、中间全错 | 先扫值分清正/负分支与固定偏移（4.6） |
| 旋转类控制不看四元数顺序 | pitch/yaw/roll 串味 | 用 `probe_quat_order.py` 先验证 `Initial*Delta` 与轴 |

---

## 10. 一句话总结

**先证主动，再量系数，后验坐标，最后叠加。**
任何一步靠猜，返工都超过验证的 30 秒。
