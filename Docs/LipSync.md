# Seven 口型同步（UE 5.8）

给 `Content/_MyTest/Mouth/Seven`（52 个 ARKit morph target）接上
Runtime MetaHuman Lip Sync 的**真实模型**（`URealisticMetaHumanLipSyncGenerator`），
把 MetaHuman 的 81 个控制量映射成 ARKit blendshape 并每帧写进 mesh。

## 状态

| 项 | 值 |
|---|---|
| 引擎 | UE 5.8.3（`E:\Unreal\UE_5.8`） |
| 插件 | `Plugins/RuntimeMetaHumanLipSync_UE58`（v1.18+5.8，预编译二进制，依赖 `NNERuntimeORT`） |
| 测试关卡 | `/Game/_MyTest/Mouth/L_SevenLipSync`（已设为 EditorStartupMap / GameDefaultMap） |
| 测试 Actor | `ASevenLipSyncActor`（C++，已放在关卡里，名为 `SevenLipSyncCamera`） |
| 音频 | `TestAudio/SevenSpeech.wav`（16 kHz / 单声道 / 16 bit，**阿七原声**，循环播放） |
| 日志 | `SevenLipSync: generator created / playing / jawOpen=... | audio=on queuedKB=...`（Output Log 过滤 `SevenLipSync`） |

## 语音素材（阿七原声）

素材来自 `D:\AI\work\bili-audio\【刺客伍六七】伍六七语音包 320k.mp3`（时长 4:19），
用 whisper(faster-whisper small) 转出了逐句时间轴：`Tools/aqi_transcript.json`。

当前默认播放的是 **`0:106.6 – 0:113.2`**：

> 「我是刺客伍六七，刺客排行榜第 17369 位」

`TestAudio/` 里另有 3 条备选（改 Actor 的 `WavFile` 即可切换）：

| 文件 | 原片时间码 | 台词 |
|---|---|---|
| `aqi_alt_memory.wav` | 3:46.1 – 3:52.6 | 为什么我没有过去的记忆，我以前到底是一个怎么样的人 |
| `aqi_alt_assassin.wav` | 3:04.0 – 3:07.4 | 就算是刺客，也会有想保护的人 |
| `aqi_alt_shadow.wav` | 0:48.3 – 0:51.6 | 我是玄武国的暗影刺客 |
| `SevenSpeech_tts_backup.wav` | — | 之前的中文 TTS 测试音（备份） |

想换别的句子：在 `Tools/export_aqi_lines.py` 的 `CLIPS` 里加一条
`(起始秒, 结束秒)`，重跑脚本就会输出 16 kHz 单声道 wav 到 `TestAudio/`。
时间码从 `Tools/aqi_transcript.json` 里挑。

> 版权提醒：这是《刺客伍六七》的角色原声，做内部测试/风格验证没问题，
> 对外发布或商用前请替换成原创配音。

## 代码

| 文件 | 作用 |
|---|---|
| `Source/MyProject_5_8/LipSync/SevenLipSyncComponent.*` | 核心：建生成器、流式喂 PCM、控制量→ARKit 映射、`SetMorphTarget` |
| `Source/MyProject_5_8/LipSync/SevenLipSyncActor.*` | 测试 rig：`ACameraActor` 子类，相机 + 点光 + Seven mesh + 上面的组件 |

`MyProject_5_8.Build.cs` 增加了 `RuntimeMetaHumanLipSync` 依赖。

### 关键参数（都在 Actor / 组件 Details 里）

| 参数 | 默认 | 说明 |
|---|---|---|
| `WavFile` | `TestAudio/SevenSpeech.wav` | 相对项目根目录；也支持绝对路径 |
| `bAutoStart` | true | BeginPlay 自动播放 |
| `bLoopAudio` | true | 播完循环（demo 用） |
| `Gain` | 1.0 | 全局幅度，嘴张得不够大就加到 1.5~2 |
| `SmoothingSpeed` | 30 | 越大越跟手、越小越柔 |
| `LipSyncLookaheadMs` | 100 | 模型相对声卡时钟的前瞻补偿；嘴比声音**早**就调小，**晚**就调大 |
| `LipSyncGain` | 1.0 | 整体幅度（映射表里 `jawOpen` 已单独 ×2.2） |
| `LipSyncGateLowRms` | 0.015 | 静音门下限：RMS 低于它就闭嘴（语音 RMS 约 0.1，底噪约 0.007） |
| `LipSyncGateHighRms` | 0.035 | 静音门上限：高于它全量驱动，中间线性过渡 |
| `LoopGapSeconds` | 1.0 | 每轮之间的静音间隔 |
| `bMouthOnly` | true | 只驱动嘴（jaw/mouth/tongue/cheek），不碰眉眼 |
| `MeshLocation / MeshRotation` | (105,0,-158) / yaw 90 | 相对相机的取景（头在相机正前方约 1 m） |
| `CameraFOV` | 35 | |
| `KeyLightIntensity` | 250 cd | |

### 换音频

1. 把 wav 放到 `TestAudio/` 下（任意采样率/位深都行，代码会解码并按 16 kHz 重采样；
   支持 PCM 8/16/24/32bit 和 float32，单/双声道）。
2. 选中关卡里的 `SevenLipSyncCamera` → Details → `Seven / Audio / Wav File` 改成新路径。
3. PIE 即可。

运行时也可以从蓝图调 `StartPlayback(Path)` / `StopPlayback()`。

## 映射表在哪

`SevenLipSyncComponent.cpp` 里的 `GMorphMappings`：

```cpp
{ TEXT("CTRL_C_jaw.ty"),             TEXT("jawOpen"),        2.20f },
{ TEXT("CTRL_L_mouth_cornerPull.ty"),TEXT("mouthSmileLeft"), 1.00f },
...
```

一条记录 = 一个控制量（`GetControlValues()` 的 key）→ 一个 ARKit morph + 系数；
同一个 morph 可以有多条（累加后 clamp 0..1）。要加通道就直接加行、重编译。

当前覆盖：jaw（open/forward/left）、上下唇、嘴角（smile/frown/stretch/dimple/sharpCornerPull）、
funnel、pucker、lipsTogether→close/press、nasolabial→noseSneer、tongueOut；
`bMouthOnly=false` 时额外驱动 brow / eyeBlink / eyeSquint / cheekSquint。

## 声音

口型模型只吃 PCM、不出声，所以组件里另外挂了一个 `USoundWaveProcedural` +
2D `UAudioComponent`（`bIsUISound = true`，不吃距离衰减）：`StartAudio()` 把整段
16 kHz 单声道 PCM 一次性 `QueueAudio` 再 `Play()`，循环时 `Stop → ResetAudio → 重新排队`。

验证办法（Output Log 过滤 `SevenLipSync`）：

```
SevenLipSync: audio queued 6.65s, playing=true
SevenLipSync: jawOpen=... | audio=on queuedKB=-100   ← 这个数每 0.5s 掉 16KB，就是 32KB/s 的播放速率
```

如果还是没声音，按顺序查：

1. PIE 工具栏那个喇叭图标（`Mute Audio`）——它会被记住，之前点过就一直静音；
2. Windows 音量合成器里 `UnrealEditor.exe` 的音量/输出设备；
3. 控制台 `au.MuteAudio` 应为 0（默认就是 0）。

## 已知问题 / 待办

1. **材质**：2026-10-30 修完版本迁移（见 [SevGame_AssetMigration.md](SevGame_AssetMigration.md)）后，
   `SevGame/.../Seven` 的贴图和 11 个 MI 都能正常加载了，之前的「纯色」是因为整批资产因
   引擎版本不匹配而根本没加载。仍有两个槽是默认材质：`Seven_Face_ncl_1`、`09_-_Default`
   （源项目里就没挂，`09 - Default` 是 3ds Max 默认材质名）。
2. **没有身体动画**：mesh 直接播 Ref Pose（A-pose）。要站姿/呼吸/待机，需要给 Seven 建
   AnimBP（Target Skeleton = `Seven_Skeleton`），再把 mesh 的 Anim Class 指过去；
   口型是直接写 morph，**不会**被 AnimBP 覆盖。
   注：`Seven_Hair_AnimBlueprint` 只是头发的 Post Process AnimBP（KawaiiPhysics），不算身体动画。
3. **没有麦克风/实时 TTS**：现在只有「wav 文件 → 口型」。要麦克风就装
   `RuntimeAudioImporter`（D 盘有 5.8 版），把 PCM 回调接到
   `StartPlayback` 同样的 `ProcessAudioData` 路径；要 TTS 就装 `RuntimeTextToSpeech` 5.8。
4. **眨眼/视线没做**：`bMouthOnly=true` 时眼睛是静止的；真实模型本身有 eyeBlink 控制量，
   想要的话把 `bMouthOnly` 关掉即可。
5. **LOD**：morph 只在有 morph 的 LOD 上生效，远处切 LOD 后嘴会不动，属正常。

## 命令备忘

```powershell
# 编译
& 'E:\Unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat' MyProject_5_8Editor Win64 Development `
  -Project="E:\Unreal\Projects\MyProject_5_8\MyProject_5_8.uproject" -WaitMutex

# 启动
& 'E:\Unreal\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' `
  "E:\Unreal\Projects\MyProject_5_8\MyProject_5_8.uproject"

# MCP（编辑器开着时）
.\Scripts\ue-mcp.ps1 -Toolsets
.\Scripts\ue-mcp.ps1 -Toolset EditorToolset.EditorAppToolset -Tool StartPIE `
  -Arguments '{"options":{"bSimulate":false,"playMode":"PlayMode_InViewPort","warmupSeconds":4}}'
```

> 编译前必须关掉编辑器（模块 DLL 被占用）。改完 C++ 后重启编辑器才会加载新代码。
