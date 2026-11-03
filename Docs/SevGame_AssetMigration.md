# SevGame 资产迁移：5.8.4 改版引擎 → 原厂 5.8.3

2026-10-30。从 `D:\Work\SSMainline`（`Sev.uproject`）merge 过来的 `Content/SevGame`
整批打不开，报「saved with a newer engine version」。本文件记录根因、修法和复用手法。

## 现象

Content Browser 里资产都在，双击报错；`Seven`、`Seven_Skeleton`、`Seven_PhysicsAsset`、
`Atk_Idle01` 全部失败。日志（`Saved/Logs/MyProject_5_8.log`）：

```
LogLoadErrors: Warning: Package '/Game/SevGame/Art/Player/Seven/Mesh/Seven_Skeleton'
has been saved with a newer engine version and can't be loaded.
Current EngineVersion: 5.8.3-58210709+++UE5+Release-5.8 (Licensee=0)
Package EngineVersion: 5.8.4-166886+++ScissorSeven+Game+Mainline (Licensee=1)
```

`Seven_Hair_AnimBlueprint` 另有单独提示：

```
LogAssetRegistry: Package .../Seven_Hair_AnimBlueprint.uasset uses an unknown custom
version and cannot be loaded for the AssetRegistry
```

（那是 KawaiiPhysics 插件的 custom version，插件没装。）

## 根因

两边的引擎不是一回事：

| | 本项目 | 源项目 `D:\Work\SSMainline` |
|---|---|---|
| 引擎 | 5.8.3 | 5.8.4 |
| CL | 58210709 | 167004（兼容 CL 166886） |
| 分支 | `++UE5+Release-5.8` | `++ScissorSeven+Game+Mainline` |
| Licensee | 0（原厂） | 1（改版构建） |

`FLinkerLoad` 的判断（`LinkerLoad.cpp:1572`）：

```cpp
if (bLoaderNeedsEngineVersionChecks && IsEnforcePackageCompatibleVersionCheck() &&
    (Summary.IsFileVersionTooNew() || !FEngineVersion::Current().IsCompatibleWith(Summary.CompatibleWithEngineVersion)))
```

资产表头里存的 `CompatibleWithEngineVersion = 5.8.4-166886+++ScissorSeven+Game+Mainline`，
和本机的 5.8.3 比 patch 号就挂了（`EngineVersion.cpp:123` 的 `IsCompatibleWith`）。
注意报错里打的是源码里 `NSLOCTEXT` 的 `{3}`，实际值来自 `CompatibleWithEngineVersion`。

**关键：二进制格式其实是一致的**，只是版本号标签不同。用 `Scripts/uasset_engver.py` 读表头：

| 字段 | 源资产 | 本机存的资产 |
|---|---|---|
| LegacyFileVersion | -9 | -9 |
| FileVersionUE4 | 522 | 522 |
| FileVersionUE5 | 1018 | 1018 |
| FileVersionLicenseeUE | 0 | 0 |
| CustomVersions | 与 5.8.3 注册表逐条吻合（268/123/15/37/68/40…） | 同 |

所以不需要改引擎、也不需要找 5.8.4 那份引擎，**在本机重存一遍**就能永久修好。

## 修法（三步）

1. 绕过版本校验（只在重存这个进程里生效，不动 ini）：

   ```python
   unreal.SystemLibrary.execute_console_command(None, "s.EnforcePackageCompatibleVersionCheck 0")
   ```

   对应的 CVar 在 `LinkerLoad.cpp:204`，`s.EnforcePackageCompatibleVersionCheck`。

2. 逐个 `load_asset` + `save_loaded_asset(..., only_if_is_dirty=False)`，表头被改写成
   `5.8.3-58210709+++UE5+Release-5.8` / `CompatibleWith = 5.8.0-55116800+++UE5+Release-5.8`。

3. 关掉 bypass，再跑一次纯 load 校验，确认原厂校验能过。

## 可复用脚本

| 脚本 | 作用 |
|---|---|
| `Scripts/uasset_engver.py` | 纯 Python 读 `.uasset`/`.umap` 表头：包版本、Licensee、CustomVersion、`SavedBy` / `CompatibleWith` 两个引擎版本串。批量跑：`python Scripts\uasset_engver.py "Content\SevGame\**\*.uasset"` |
| `Scripts/resave_sevgame.py` | 在引擎里重存 `ROOT` 下的资产（默认 `/Game/SevGame`，改脚本顶部常量即可换目录） |
| `Scripts/verify_sevgame.py` | 不开 bypass，纯校验 `ROOT` 下每个资产能否加载 |
| `Scripts/inspect_seven.py` | 打印 `Seven` 的骨架/物理资产/morph/材质槽/后处理 AnimBP 引用，迁移后体检用 |

跑法（**先关掉所有开着本项目的编辑器**）：

```powershell
$exe = "E:\Unreal\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$proj = "E:\Unreal\Projects\MyProject_5_8\MyProject_5_8.uproject"

# 重存
& $exe $proj -run=pythonscript -script="E:\Unreal\Projects\MyProject_5_8\Scripts\resave_sevgame.py" `
  -unattended -nosplash -nopause -abslog="E:\Unreal\Projects\MyProject_5_8\Saved\Logs\resave.log"

# 校验（日志里找 SEVCHECK: ===== ok=60 failed=0)
& $exe $proj -run=pythonscript -script="E:\Unreal\Projects\MyProject_5_8\Scripts\verify_sevgame.py" `
  -unattended -nosplash -nopause -abslog="E:\Unreal\Projects\MyProject_5_8\Saved\Logs\verify.log"
```

日志里搜 `SEVGAME`（重存）和 `SEVCHECK`（校验）即可，脚本会打 `OK` / `FAIL` 逐条清单。

## 坑

- **编辑器开着会锁文件**。第一次跑时 56/60 成功，剩下 4 个大的（`Seven`、`Seven_Skeleton`、
  `Seven_PhysicsAsset`、`Atk_Idle01`）全是 `MoveFile ... Error Code 32`（sharing violation）→
  `save returned False`。原因就是当时 `MyProject_5_8` 的编辑器还开着、这 4 个包被它加载着。
  关掉编辑器重跑，60/60 通过。
- 重存会重写 `Content/` 里的文件，动手前备份：`Saved/Backup/SevGame_before_resave_20261030.zip`。
- 每次再从 `SSMainline` merge 新资产，都要重跑一遍 `resave_sevgame.py`。想彻底省事，就让对方
  用原厂 5.8.3/5.8.4 存一份；或者约定「merge 完就跑这个脚本」。

## KawaiiPhysics 插件

`Seven_Hair_AnimBlueprint` 是 `Seven` 的 Post Process Anim Blueprint（头发骨骼物理），
依赖 KawaiiPhysics。已从 `D:\Work\SSMainline\Game\Plugins\KawaiiPhysics`（v1.21.0）复制到
`Plugins/KawaiiPhysics`，去掉 `Binaries`/`Intermediate` 后用 5.8.3 重新编译（3 个模块：
`KawaiiPhysics`、`KawaiiPhysicsSequencer`、`KawaiiPhysicsEd`），并在
`MyProject_5_8.uproject` 里 `Enabled: true`。

- 编译：`Build.bat MyProject_5_8Editor Win64 Development -Project="...\MyProject_5_8.uproject" -WaitMutex`（约 70 s）
- 插件 Content 下只有本地化和 Python 工具，没有 `.uasset`，所以不存在同样的版本问题。
- 插件带 `Content/Python/kawaii_physics_toolset`，会注册成 MCP/Agent 工具集。
- **注意：`.gitignore` 里有 `/plugins/`，所以这个插件和 `Oceanology_Plugin`、
  `RuntimeMetaHumanLipSync_UE58` 一样只存在于本地**，换机器/重新 clone 要再复制一份并编译，
  否则 `Seven_Hair_AnimBlueprint` 又会报 missing module。想纳入版本管理就得改 `.gitignore`
  单独放行 `Plugins/KawaiiPhysics`（排除 `Binaries`/`Intermediate`）。

## 结果（2026-10-30 18:56）

| 项 | 状态 |
|---|---|
| `SevGame` 60 个资产表头 | 全部 `5.8.3-58210709+++UE5+Release-5.8` |
| 原厂校验下加载 | 60/60 成功（`SEVCHECK: ===== ok=60 failed=0`） |
| `Seven` mesh | SkeletalMesh 正常，13 个材质槽，骨架 187 根骨骼，`Seven_PhysicsAsset` 已挂 |
| `Seven_Hair_AnimBlueprint` | 正常加载（KawaiiPhysics 就位），已作为 `PostProcessAnimBlueprint` 生效 |
| 备份 | `Saved/Backup/SevGame_before_resave_20261030.zip`（31 MB） |

遗留（迁移前就存在、与版本无关）：

1. `Seven` 的 morph target 数量为 **0**（`inspect_seven.py` 报 `morph_count = 0`）。口型那套
   52 个 ARKit morph 在 `/Game/_MyTest/Mouth/Seven` 那份资产上，不是这一份。要用 new mesh 做口型，
   得从 `D:\Download\Seven.fbx` 重新导入带 morph 的版本，或把 morph 合到这个 mesh 上。
2. 材质槽 `Seven_Face_ncl_1` 和 `09_-_Default` 指向 `WorldGridMaterial`（源项目里就没挂 MI，
   `09 - Default` 是 3ds Max 的默认材质名），其余 11 个槽正常指向 `Seven_*_Inst`。
3. 贴图资产里有 `/Game/Developers/jinzhang/Seven_1` 这类**源文件路径**残留（贴图的
   Source File 记录），运行时不加载，无害；想要干净可以重新导入贴图。

## 第二轮：2026-11-02 重新 merge

用户在 `SSMainline` 里更新了内容后重新 merge 了一次，于是：

1. 重存结果被覆盖。带上来的 55 个小资产又全部变回 `5.8.4-2147650534+++ScissorSeven+Game+Mainline`，
   一条都加载不了。**这是预期行为：merge 会覆盖本地的重存结果，每次 merge 都要重跑
   `resave_sevgame.py`。**
2. merge 不完整，缺 5 个关键文件（`Animations/` 整个目录都没带过来）：

   | 缺的文件 | 源文件大小 | 源文件自己的版本标签 |
   |---|---|---|
   | `Mesh/Seven.uasset` | 7 671 630 | 5.7.4-dev_fight |
   | `Mesh/Seven_Skeleton.uasset` | 75 997 | 5.7.4-dev_fight |
   | `Mesh/Seven_PhysicsAsset.uasset` | 62 195 | 5.7.4-dev_fight |
   | `Mesh/Seven_Hair_AnimBlueprint.uasset` | 337 481 | 5.7.4-dev_fight |
   | `Animations/Atk/Atk_Idle01.uasset` | 928 045 | 5.7.4-dev_fight |

   结论：源项目里角色本体这几个文件**一直没重存**，还停在 `++ScissorSeven+Game+dev_fight` 的
   5.7.4 上。它们比 5.8.3 老，本来就能加载；真正的阻塞只有那 55 个被 5.8.4 存过的小资产。

3. **新坑：只读属性。** 从源项目拷过来的这 5 个文件带 `ReadOnly`（源项目受源码管理），
   重存时 `SavePackage` 直接报：

   ```
   LogSavePackage: Error: Cannot remove '...Seven_Skeleton.uasset' as it is read only!
   ```

   修法：拷完先清属性再重存。

   ```powershell
   Get-ChildItem Content\SevGame -Recurse -File -Include *.uasset,*.umap | Where-Object IsReadOnly | ForEach-Object { $_.IsReadOnly = $false }
   ```

4. 补齐 + 清属性后重跑：`ok=60 failed=0`，原厂校验下 `SEVCHECK: ===== ok=60 failed=0`。
   备份：`Saved/Backup/SevGame_before_resave_20261102.zip`（28 MB）。

5. **散落资产也跟着修了**：`Content/Developers/fulinhou/Skele/StaticMesh.uasset` 同样是这一批
   进来的 5.8.4 资产（Seven 本体拆出来的**非蒙皮静态网格**，引用 9 个 `Seven_*_Inst` 材质）。
   脚本支持换根目录，不用改代码：

   ```powershell
   $env:RESAVE_ROOT="/Game/Developers/fulinhou/Skele"
   # 然后照下面跑 resave_sevgame.py / verify_sevgame.py
   ```

   修完 `Content` 全树 747 个包里「太新」的数量 = **0**。

### 标准流程（每次 merge 后照抄）

```powershell
# 0. 关掉所有打开本项目的编辑器
# 1. 清只读
Get-ChildItem Content\SevGame -Recurse -File -Include *.uasset,*.umap | Where-Object IsReadOnly | ForEach-Object { $_.IsReadOnly = $false }
# 2. 先看版本分布，确认哪些是新引擎存的
python Scripts\audit_engver.py Content\SevGame
# 3. 重存（日志找 SEVGAME: ===== SUMMARY ok=60 failed=0）
# 4. 原厂校验（日志找 SEVCHECK: ===== ok=60 failed=0）
```

### 范围提醒

源项目 `D:\Work\SSMainline\Game\Content\SevGame` 有 **30 656** 个资产（整套 UI / VFX / GAS /
Montage / BP / 关卡序列…），本项目 `SevGame` 目前只有 **60** 个（`Art/MaterialLibrary` 全部 +
`Art/Player/Seven/Mesh` 的 `Material`/`Texture`/本体 + 一个 `Atk_Idle01` 动画）。
源里另有 3 个同类资产从没带过来（`Mesh/Material/Seven_Outline_Inst`、`Mesh/Texture/Seven_BodyCloth_Nor`、
`Mesh/Texture/Seven_Face_Mask`），要的话单独拷。

