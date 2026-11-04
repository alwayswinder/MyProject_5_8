# 3ds Max 控制器迁移到 UE Control Rig 指南

适用范围：把 3ds Max 里的面部控制器（`K_` 点、滑条、Animcraft 曲线手柄）在 UE 5.8 Control Rig 里复现成同样的手感。

本文按 `CR_Seven_MorphSliders` 眉毛控制的两次重做整理，所有数值都是实测，不是推算。**先读第 5 节的踩坑表，能省掉大半返工。**

---

## 0. 总览

| 步骤 | 产出 | 判定标准 |
|---|---|---|
| 1. 查 Max | 控制器名、类型、父级、三轴各拖 10 后谁动、动多少 | 记录成「位移 → 位移」的系数 |
| 2. 搭 UE | 骨骼链、控制类型/形状/位置、RigVM 图 | 数值自检通过，且编辑器里拖得动 |
| 3. 对轴向 | Space 选择 + 分量映射 | 单轴单拖，模型方向对 |

顺序不能反。轴向问题必须在**单轴干净状态**下验证：一次只变一个分量。

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

脚本里发的是：

```json
{"type":"maxscript","command":"( ...maxscript... )"}
```

返回 `{"success":true,"result":"...","meta":{...}}`。`safeMode=true` 时 `doscommand` / `createfile` / `python.execute` 会被拦，其余照常执行。

### 1.3 探测脚本要点

1. 打印控制器自身：`classOf`、`parent`、`pos`、`rotation`、`scale`、`position.controller` 的 list 各项。
2. **用世界空间移动**再读结果，避免父级缩放干扰：
   ```maxscript
   move n [10,0,0]     -- 世界位移
   ```
   不要用 `n.pos += [10,0,0]`，那是父空间，遇到 `-0.94` 缩放会算错。
3. 移动前后遍历 `objects` 比较 `(o.transform).row4`（Biped 对象没有 `.pos`，会报错）。
4. 每次测完复位。

### 1.4 表达式控制器

`position_list` 的第 2 项常是 `Position_Expression`。取公式：

```maxscript
local expr = (getNodeByName "meibu_R")[#Transform][#Position][2].controller
expr.getExpression()
```

但**变量绑定读不到**：`getNumVariables` / `getVariableName` / `getVariableTarget` 在这版全报错。别猜绑定，直接移动控制器看骨骼世界位移，用实测反推。

### 1.5 记录表（填完再进 UE）

| 项 | 示例（Animcraft 右眉） |
|---|---|
| Max 对象名 | `Animcraft_Right_Brow_Z_Ctrl`（NURBSCurveShape） |
| 父级 | `Point471 → DAnimcraft_Master_GRP → … → BIPRoot` |
| 驱动方式 | 骨骼（`meibu_R` + `Bone010-012`） |
| 世界 X +10 | 整条右眉世界 X +5 |
| 世界 Z +10 | 整条右眉世界 Z +5 |
| 世界 Y +10 | 无效果 |
| 交叉影响 | 左眉完全不动 |
| morph 参与 | 无 |

写成系数就是 `0.5`，UE 里直接复用。

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
| 形状像一枚扁平条 | `Box_1mm`（线框盒），`shape_name='Box_1mm'` |
| 尺寸 | 见下方「手柄大小只能走 shape transform」 |
| 颜色 | `settings.shape_color = LinearColor(1.0, 0.42, 0.04, 1.0)`（橙） |

内置形状清单在 `Engine\Plugins\Animation\ControlRig\Content\Controls\`，名字去掉 `ControlRig_` 前缀就是 `shape_name`：`Box`、`Box_1mm`、`Box_3mm`、`Box_solid`、`Arrow`、`Arrow4`、`Circle`、`Diamond`、`Hexagon`、`Octagon`、`Pyramid`、`RoundedSquare`、`Sphere`、`Square`、`Star4`、`Triangle`、`Wedge`、`Line`……

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

**尺寸坑**：`Box`（无后缀）默认是 1 米级的盒子，`scale 1.9` 会直接糊满半个屏幕；`Box_1mm` 才是 10 cm 级。摆之前先想清楚基准尺寸。

**位置坑**：不要假设「Y 就是左右」。这具骨架 `meibu_L` 在 `(4.46, 7.49, 161.39)`、`meibu_R` 在 `(-4.46, 7.49, 161.39)` —— **X 才是左右，Y 是前后，Z 是上下**。按坐标轴直觉放会把两个手柄都甩到脑后。

### 2.4 控制挂哪儿

父级用 `Root` 骨骼（世界变换是单位矩阵），再用 `set_control_offset_transform` 把 `initial/current` 都设成目标世界变换：

```python
off = h.get_global_control_offset_transform(k)
off.translation = unreal.Vector(*pos)
off.rotation = yaw_quat
off.scale3d = unreal.Vector(*scale)
h.set_control_offset_transform(k, off, True, True)   # initial
h.set_control_offset_transform(k, off, False, True)  # current
```

---

## 3. 第三步：驱动图与轴向

### 3.1 两个引擎的坐标系

| | 3ds Max | UE |
|---|---|---|
| 上 | Z | Z |
| 前 | −Y | +X |
| 右 | +X | +Y |
| 手性 | 右手 | 左手 |

FBX 导入通常是 `UE = (−Max.y, −Max.x, Max.z)`，但**以实际骨骼坐标为准**。

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

Max 里拖 10 cm → 骨骼 5 cm，UE 控制也设 `±10`、系数 `0.5`，手感一比一。

---

## 4. 实战：Animcraft 眉毛左右整体控制

### 4.1 Max 侧

`Animcraft_Right_Brow_Z_Ctrl` / `Animcraft_Left_Brow_Z_Ctrl`，各驱动本侧 `meibu_*` + 3 根眉骨，世界 X/Z 各 0.5 倍，Y 无效，左右互不影响。

子对象 `Animcraft_*_Brow_Ctrl` 位置被表达式锁死，拖动只有 0.1–0.8 cm 形状差，**不是可交互主控**，不要照它建控制。

### 4.2 UE 侧

| 项 | 值 |
|---|---|
| Rig | `/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders` |
| 控制 | `CTRL_BROW_L` / `CTRL_BROW_R`，`POSITION`，±10 |
| 显示名 | 左眉整体 / 右眉整体 |
| 形状 | `Box_1mm`，橙色；尺寸走 shape transform `(0.42, 0.22, 0.05)`，控制自身缩放 `(1,1,1)` |
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
| `Box` 直接摆 | 手柄变成糊满屏幕的巨大盒子 | 用 `Box_1mm`，或把 shape scale 缩到 0.1 量级 |
| `GetTransform.Transform.Scale` | 报 `Cannot find pin` | 写 `Scale3D` |
| `SetTransform` 连“Transform” | 找不到引脚 | 输入引脚叫 `Value` |
| `RigHierarchy.set_translation` | `AttributeError` | 用 `set_global_transform` / `set_local_transform` |
| 在脚本里 `inst.execute('Forwards Solve')` 于 debugged rig | 编辑器预览骨骼被推到世界某处 | 用 `create_control_rig()` 建临时实例自检 |
| 只看返回值不看画面 | 数值对、方向错 | 数值自检 + 编辑器截图双确认 |
| 只测一侧 | 另一侧符号反了没发现 | 左右各测一遍 |
| 拿 `import_bones` 返回值当骨骼总数 | 数量对不上 | 用 `get_all_keys()` 统计 |

---

## 6. 验证流程

1. **数值**：`rig.create_control_rig()` → `request_init()` → 设控制值 → `execute('Forwards Solve')` → 读 `get_global_transform(bone).translation`，对比静止状态。
2. **单轴**：一次只给 X、再只给 Z，左右各一组，共 4 组。
3. **画面**：`CaptureEditorImage` 截图，确认手柄位置、形状、眉毛方向。
4. 保存后重开资产，数值和效果都保持。

---

## 7. 脚本索引

| 文件 | 用途 |
|---|---|
| `Scripts/max_run.ps1` | 向运行中的 3ds Max 发 MaxScript（命名管道） |
| `Scripts/probe_animcraft_brow*.ms`、`probe_animcraft_world.ms` | Max 侧结构 / 世界位移探测 |
| `Scripts/inspect_cr7_state.py` | 打印 rig 当前控制、骨骼、节点 |
| `Scripts/clear_rig.py` | 清空 rig + 全量重导骨骼 |
| `Scripts/build_brow_two.py` | 建左右眉整体控制 + 图 + 自检（幂等） |
| `Scripts/probe_axes.py` | 打印骨骼世界变换与局部轴 |
| `Scripts/probe_translation_space.py` | 实测 Global / Local 位移差 |
| `Scripts/ue_run.py` | 在运行中的编辑器里执行 Python |
| `Saved/Codex/Backup/` | 改动前的 `.uasset` 备份 |

> 改资产前先 `Copy-Item` 备份 `.uasset`，再把脚本包在 `unreal.ScopedEditorTransaction` 里，方便撤销。
