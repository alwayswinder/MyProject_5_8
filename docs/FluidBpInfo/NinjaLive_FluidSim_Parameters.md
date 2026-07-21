# NinjaLive 流体仿真参数说明

> 实例：`NinjaLive_Area_Water`（UseCase_002_WorldSpaceWater_LIVE17）
> 
> ✅ = 此实例中**显式修改**的值（覆盖了默认值）
> （无标记）= 使用**默认值**的参数

---

## Live Performance（实时性能参数）

> 控制流体仿真的性能与质量平衡的核心参数组。

| #   | 参数名                                            | 中文翻译                   | 默认值   | 实例值         | 说明                                                     |
| --- | ---------------------------------------------- | ---------------------- | ----- | ----------- | ------------------------------------------------------ |
| 1   | **Resolution X**                               | 仿真分辨率 X                | 256   | **2048** ✅  | 流体模拟的横向分辨率（像素）。值越高细节越丰富，但 GPU 计算量呈平方增长。2048 适合大范围高质量水面 |
| 2   | **Resolution Y**                               | 仿真分辨率 Y                | 256   | **2048** ✅  | 流体模拟的纵向分辨率（像素）。与 Resolution X 共同决定总像素数，直接影响性能开销        |
| 3   | **Use Pressure Solver 1 (Default Is 2)**       | 使用压力求解器 1（默认 2）        | false | false       | 默认使用更高效的 Solver 2。勾选后会切换到旧版 Solver 1，一般不需要改动           |
| 4   | **Pressure Solver 1 Max Iterations**           | 压力求解器 1 最大迭代次数         | 5     | 5           | Solver 1 每次仿真的迭代次数。迭代越多压力传播越准确，但性能开销越大                 |
| 5   | **Pressure Solver 2 Max Iterations**           | 压力求解器 2 最大迭代次数         | 1     | 1           | Solver 2 的迭代次数。值越大流体压力传播越精确，默认 1 次已在大多数场景取得良好效果        |
| 6   | **Pressure Solver 2 Kernel Reduction**         | 压力求解器 2 核缩减            | 1     | 1           | Solver 2 计算核的缩减步长，影响压力计算的精度和性能平衡                       |
| 7   | **Max Sampling FPS**                           | 最高采样帧率                 | 60    | 60          | 流体仿真的最高采样帧率上限。即使游戏帧率更高，仿真也不会超过此值                       |
| 8   | **Min Sampling FPS**                           | 最低采样帧率                 | 30    | 30          | 流体仿真的最低采样帧率。当游戏帧率低于此值时，仿真的品质会开始降级                      |
| 9   | **Half Res Pressure and Divergence Buffers**   | 半精度压力和散度缓冲             | false | false       | 以半分辨率计算压力和散度缓冲区，可大幅降低 GPU 开销，但会损失一些流体细节                |
| 10  | **LOD1-Reduce Sim Quality**                    | LOD1 降低模拟质量            | false | false       | 第一级 LOD 时降低模拟的内部精度，减少 GPU 计算量                          |
| 11  | **LOD2-Reduce Sampling FPS**                   | LOD2 降低采样帧率            | false | false       | 第二级 LOD 时降低采样帧率，减少每帧的仿真计算频率                            |
| 12  | **LOD-Near Bound**                             | LOD 近边界                | 2000  | **6000** ✅  | 相机在此距离内保持最高模拟质量。此实例从 2000 扩大到 6000，适应大场景               |
| 13  | **LOD-Far Bound**                              | LOD 远边界                | 5000  | **16000** ✅ | 相机在此距离外模拟完全降级或停止。从 5000 扩至 16000 确保远处水面仍然可见            |
| 14  | **LOD-Steps**                                  | LOD 步数                 | 5     | 5           | LOD 从最近到最远的渐变阶梯级数。级数越多过渡越平滑                            |
| 15  | **LOD-Check Frequency**                        | LOD 检测频率               | 0.5   | 0.5         | 每间隔多少秒重新评估一次 LOD 状态。值越小响应越快但 CPU 开销略增                  |
| 16  | **Pause Sim when Not Visible**                 | 不可见时暂停仿真               | true  | true        | 流体仿真不在任何相机视野内时自动暂停，大幅节省性能                              |
| 17  | **Wait Before Pause**                          | 暂停前等待时间                | 0.2   | 0.2         | 从不可见到真正暂停的等待秒数。短暂离开视野（如快速转头）不会触发频繁启停                   |
| 18  | **Sim Speed Adjustment Latency**               | 仿真速度调整延迟               | 0.1   | 0.1         | 仿真速度参数变化时的响应延迟秒数，防止参数突变导致仿真不稳定                         |
| 19  | **Stop Using Painter Canvas when Idle**        | 空闲时停止使用绘制画布            | true  | true        | 没有交互输入时释放 Painter Canvas 资源，减少 GPU 开销                  |
| 20  | **Experimental PSolver 2 Kernel Index Offset** | 实验：求解器 2 核索引偏移         | 0     | 0           | 压力求解器 2 的实验性内核索引偏移量，用于微调求解器的计算模式                       |
| 21  | **Experimental Pressure Feedback**             | 实验：压力反馈系数              | 0.999 | 0.999       | 压力反馈的实验性调节系数，控制压力在帧之间的反馈强度，值越接近 1 反馈越持久                |
| 22  | **Exp Pressure Feedback Component**            | 实验：压力反馈分量              | 1     | 1           | 压力反馈的分解分量调节，进一步细调压力反馈行为                                |
| 23  | **Exp Divergence Feedback Component**          | 实验：散度反馈分量              | 1     | 1           | 散度反馈的分解分量调节，控制流体不可压缩性的反馈强度                             |
| 24  | **Force 8bit Simple Painter Buffers**          | 强制 8bit 简单绘制缓冲         | false | false       | 强制使用 8bit 精度的简单绘制缓冲区，减少显存占用但可能损失绘制精度                   |
| 25  | **Force 8bit Output Buffer**                   | 强制 8bit 输出缓冲           | false | false       | 强制输出缓冲区为 8bit 精度，降低显存带宽需求                              |
| 26  | **Force 2x Resolution Output Buffer**          | 强制 2 倍分辨率输出缓冲          | false | false       | 强制输出缓冲区为仿真分辨率 2 倍，输出更清晰但 GPU 开销显著增加                    |
| 27  | **Force Max Sampling FPS-To Niagara**          | 强制最大采样 FPS 同步到 Niagara | true  | true        | 将 Max Sampling FPS 设置同步到 Niagara 系统，保持流体仿真与粒子系统的帧率一致   |

### 此实例的 Live Performance 修改要点

| 修改                             | 效果                       |
| ------------------------------ | ------------------------ |
| Resolution 256→**2048**（X 和 Y） | 总像素数提升 64 倍，大幅提高水面细节     |
| LOD-Near Bound 2000→**6000**   | 高质量区域扩大 3 倍              |
| LOD-Far Bound 5000→**16000**   | 可视距离扩大 3.2 倍             |
| 其余 24 项保持默认                    | 压力求解器、帧率限制、缓冲区精度等均使用标准配置 |

---

## Live Interaction（实时交互参数）

> 控制流体仿真如何与场景中的物体、角色和用户输入进行交互。

| #   | 参数名                                                  | 中文翻译               | 默认值            | 实例值                  | 说明                                                         |
| --- | ---------------------------------------------------- | ------------------ | -------------- | -------------------- | ---------------------------------------------------------- |
| 1   | **Continuous Interaction with Owner Actor**          | 与所有者 Actor 连续交互    | false          | false                | 是否持续追踪 Owner Actor 与流体的交互。开启后，组件所属的 Actor 只要在仿真范围内就会持续产生交互 |
| 2   | **Continuous Interaction Inclusive Obj Type**        | 连续交互包含的物体类型        | [ObjType3,1,2] | —                    | 哪些物体类型可以触发连续交互碰撞。通常包含 Pawn、WorldDynamic、PhysicsBody 等      |
| 3   | **Continuous Interaction Component Names Exact**     | 连续交互精确组件名          | []             | —                    | 只与列表中指定名称的组件产生连续交互，空列表表示匹配所有组件                             |
| 4   | **Continuous Interaction Bone Names Exact**          | 连续交互精确骨骼名          | []             | —                    | 只与列表中指定名称的骨骼产生连续交互，空列表表示匹配所有骨骼                             |
| 5   | **Use PAINTER V2 to Track Objects**                  | 使用 Painter V2 追踪物体 | true           | true                 | 启用 V2 版绘制系统追踪物体交互。V2 支持骨骼级和组件级精确追踪，比 V1 更高效准确              |
| 6   | **PV2 Connect Trackpoints with Lines**               | V2 追踪点连线           | false          | false                | 在 Painter V2 的追踪点之间绘制连线，产生笔触或拖尾效果。开启后物体移动会留下连续的线条痕迹        |
| 7   | **PV2 Generate Velocity**                            | V2 生成速度            | true           | true                 | 从追踪点的位移自动计算速度向量，使流体交互的方向和力度更真实                             |
| 8   | **PV2 Line Drawing Fail Cooldown Time**              | V2 连线失败冷却时间        | 0.01           | 0.01                 | 追踪点连线失败后的重试冷却时间（秒）。值越小重试越频繁，适合快速移动的物体                      |
| 9   | **PV2 Stop Line Drawing Above This Velocity**        | V2 高于此速度停止连线       | 500            | 500                  | 当交互物体移动速度超过此值时停止连线绘制。防止高速移动时产生过长的拖尾                        |
| 10  | **Simple Painter Mode**                              | 简单绘制模式             | false          | false                | 使用简化的绘制模式，降低 GPU 开销但交互痕迹精度下降。适合性能受限的平台                     |
| 11  | **Camera Facing Trace Mesh**                         | TraceMesh 面向摄像机    | false          | false                | TraceMesh 是否始终面向摄像机旋转（公告板模式）。关闭时 TraceMesh 保持世界空间固定方向      |
| 12  | **Camera Facing Lock Y-Axis**                        | 面向摄像机锁定 Y 轴        | false          | false                | 在 Camera Facing 模式下是否锁定 Y 轴旋转。锁定后 TraceMesh 只围绕垂直轴旋转       |
| 13  | **Use Custom Trace Source**                          | 使用自定义追踪源           | false          | **true** ✅           | 是否使用自定义的射线追踪源位置而非默认的摄像机位置。开启后可从任意位置发射交互射线                  |
| 14  | **Custom Trace Source Position**                     | 自定义追踪源位置           | (0,0,5000)     | **(0,0,15000)** ✅    | 自定义交互射线的起始世界坐标。此实例 Z=15000 表示从高空垂直向下追踪，适合俯视水面场景            |
| 15  | **Trace Mesh Moving in World Space**                 | TraceMesh 世界空间移动模式 | NewEnumerator5 | —                    | TraceMesh 在世界空间中移动的追踪模式枚举。决定如何将世界空间中的位置变化映射到仿真 UV 空间       |
| 16  | **Movement Not Quantized to Steps on Axis**          | 轴上非量化移动            | NewEnumerator2 | —                    | 在指定的轴上不进行步进量化，保持亚像素级别的平滑移动精度                               |
| 17  | **Movement Is Locked on This Axis**                  | 轴锁定模式              | NewEnumerator5 | **NewEnumerator2** ✅ | 锁定流体仿真在某个轴上的运动。此处为 NewEnumerator2（通常对应 Z 轴），实现水平水面效果       |
| 18  | **Force Trace Mesh to Custom Vertical Pos**          | 强制 TraceMesh 垂直位置  | false          | **true** ✅           | 将 TraceMesh 强制锁定到指定的垂直高度，忽略场景中实际的 TraceMesh 位置             |
| 19  | **Force Trace Mesh Vertical Position**               | 强制 TraceMesh 垂直位置值 | 0              | **-470** ✅           | TraceMesh 被锁定的 Z 轴高度值。此实例设为 -470，使交互面固定在水面高度               |
| 20  | **Single Target Mode LEGACY**                        | 单目标模式（旧版）          | false          | false                | 旧版单目标交互模式。开启后仿真只与一个特定目标交互，适用于聚焦式交互场景                       |
| 21  | **Single Target Type LEGACY**                        | 单目标类型（旧版）          | NewEnumerator1 | —                    | 旧版单目标的类型枚举，定义如何选择交互目标                                      |
| 22  | **Single Target Mode Skeletal Mesh Index LEGACY**    | 单目标骨骼网格索引（旧版）      | 0              | —                    | 旧版单目标模式下，指定跟踪第几个骨骼网格组件（当 Actor 有多个骨骼网格时）                   |
| 23  | **Single Target Mode Set Sim Speed LEGACY**          | 单目标设置仿真速度（旧版）      | false          | —                    | 旧版单目标模式下是否根据目标的运动速度动态调整仿真速度                                |
| 24  | **Single Target Mode Speed Influence Factor LEGACY** | 单目标速度影响因子（旧版）      | 0.5            | —                    | 旧版单目标模式下目标速度对仿真速度的影响系数，值越大速度影响越明显                          |
| 25  | **Enable Painter Double Buffering**                  | 启用绘制双缓冲            | false          | **true** ✅           | 双缓冲机制，避免绘制操作与仿真读取之间的帧冲突。开启后交互更稳定，无撕裂                       |
| 26  | **Use Legacy Camera Facing**                         | 使用旧版面向摄像机          | false          | false                | 使用旧版 Camera Facing 计算方式。一般无需开启，除非需要兼容旧项目行为                 |
| 27  | **Trace Mesh Translucent Sort Prio**                 | TraceMesh 半透明排序优先级 | 0              | 0                    | TraceMesh 在半透明渲染时的排序优先级。值越大越晚渲染，影响与其它半透明物体的前后遮挡关系          |

### 此实例的 Live Interaction 修改要点

| 修改                                                                | 效果                    |
| ----------------------------------------------------------------- | --------------------- |
| Use Custom Trace Source false→**true**                            | 启用自定义追踪源，从俯视角追踪       |
| Custom Trace Source Position Z=5000→**15000**                     | 追踪源升高到 15000，覆盖更大水面范围 |
| Movement Is Locked on This Axis NewEnumerator5→**NewEnumerator2** | 切换轴锁定模式，适配水平水面        |
| Force Trace Mesh to Custom Vertical Pos false→**true**            | 锁定 TraceMesh 垂直位置     |
| Force Trace Mesh Vertical Position 0→**-470**                     | 交互面锁定在水面高度 Z=-470     |
| Enable Painter Double Buffering false→**true**                    | 开启双缓冲，消除帧冲突           |
| 其余 21 项保持默认                                                       |                       |

---

## Live Brush Settings（实时笔刷设置）

> 控制流体交互笔刷的大小、缩放、噪声、速度衰减等行为，决定交互痕迹的外观和物理表现。

| #   | 参数名                                           | 中文翻译               | 默认值                   | 实例值        | 说明                                                          |
| --- | --------------------------------------------- | ------------------ | --------------------- | ---------- | ----------------------------------------------------------- |
| 1   | **Brush Scaled Inversely by Trace Mesh Size** | 笔刷随 TraceMesh 反向缩放 | true                  | true       | 笔刷大小是否与 TraceMesh 尺寸成反比。TraceMesh 越大笔刷反而越小，保持笔刷在 UV 空间中的一致性 |
| 2   | **Brush Scaled by Interacting Obj Size**      | 笔刷随交互物体大小缩放        | false                 | **true** ✅ | 交互物体的实际大小是否影响笔刷范围。开启后大象踩水波纹比老鼠大                             |
| 3   | **Use Obj Bounds Instead Of Size**            | 使用物体包围盒而非尺寸        | false                 | false      | 使用物体的包围盒（Bounding Box）来计算笔刷大小，而非物体的尺寸属性                     |
| 4   | **Global Brush Scale**                        | 全局笔刷缩放             | 1                     | **2** ✅    | 所有笔刷的全局缩放倍率。此实例设为 2，所有交互痕迹整体放大一倍                            |
| 5   | **User Input Brush Scale**                    | 用户输入笔刷缩放           | 1                     | **2** ✅    | 用户直接输入（鼠标/触屏/按键）产生的笔刷缩放倍率                                   |
| 6   | **Primitive Obj Brush Scale**                 | 基础物体笔刷缩放           | 1                     | **4.3** ✅  | 基础几何物体（Box、Sphere 等）交互时的笔刷缩放。4.3 倍使物体产生强烈的波纹效果              |
| 7   | **Skeletal Mesh Brush Scale**                 | 骨骼网格笔刷缩放           | 1                     | **1.7** ✅  | 骨骼网格体（角色模型）交互时的笔刷缩放，控制角色产生的波纹大小                             |
| 8   | **Brush Noise in World Space**                | 世界空间笔刷噪声           | false                 | false      | 笔刷噪声是否在世界空间中计算。开启后噪声贴图固定在世界坐标上，移动笔刷会产生变化的噪声图案               |
| 9   | **Brush Density Noise Scale**                 | 笔刷密度噪声缩放           | 1                     | —          | 笔刷绘制时密度噪声的整体缩放系数，影响噪声对流体密度的影响幅度                             |
| 10  | **Brush Density Noise Freq**                  | 笔刷密度噪声频率           | 40                    | —          | 笔刷密度噪声贴图的纹理频率，值越高噪声细节越细密                                    |
| 11  | **Brush Velocity Noise Scale**                | 笔刷速度噪声缩放           | 1                     | **30** ✅   | 笔刷运动速度附加噪声的强度。30 倍使速度产生的波纹非常丰富自然                            |
| 12  | **Brush Velocity Noise Freq**                 | 笔刷速度噪声频率           | 0.1                   | —          | 笔刷速度噪声的频率，控制速度扰动波纹的疏密程度                                     |
| 13  | **Dampen Brush Below This Velocity**          | 低于此速度时衰减笔刷         | 0                     | —          | 交互速度低于此阈值时开始衰减笔刷效果。0 表示不衰减，任何速度都产生相同强度的交互                   |
| 14  | **Dampen Ignores Static Meshes**              | 衰减忽略静态网格           | false                 | —          | 是否对静态网格物体跳过速度衰减。开启后静态物体即使移动缓慢也产生完整交互                        |
| 15  | **Dampen Brush Factor**                       | 笔刷衰减因子             | 0.5                   | —          | 速度低于阈值时笔刷强度的衰减系数。0.5 表示每级衰减一半                               |
| 16  | **Brush Velocity Pow**                        | 笔刷速度幂次             | 1.6                   | —          | 速度对笔刷影响力的幂次曲线。1.6 使中速和高速的差异比线性更明显                           |
| 17  | **Brush Velocity Clamp**                      | 笔刷速度钳制             | 1                     | **0.5** ✅  | 笔刷速度的上限钳制值。0.5 表示速度超过此值后不再增加笔刷效果，防止交互痕迹过于夸张                 |
| 18  | **Sim Area Motion Effects Brush Puncture**    | 仿真区域运动影响笔刷穿刺       | 0                     | —          | 仿真区域自身运动对笔刷穿刺深度的影响系数。0 表示不受影响                               |
| 19  | **Adjust Painter V2 Brush Strength**          | V2 笔刷强度调节          | 1.4                   | —          | Painter V2 的笔刷强度额外调节因子。值越大 V2 绘制的密度和速度痕迹越强                  |
| 20  | **Adjust Painter V2 Edge Mask**               | V2 边缘遮罩调节          | 1                     | —          | Painter V2 的边缘遮罩强度。控制笔刷边缘从中心到边缘的衰减曲线                        |
| 21  | **Adjust Painter V2 Brush Velo Noise**        | V2 笔刷速度噪声调节        | 1                     | —          | Painter V2 的速度噪声强度调节，在 V2 绘制系统中额外叠加的噪声量                     |
| 22  | **Painter V2 Brush Velo Noise Texture**       | V2 笔刷速度噪声纹理        | T_NoiseTemplate2_blur | —          | V2 速度噪声使用的纹理贴图。不同的噪声纹理产生不同的波纹图案                             |

### 此实例的 Live Brush Settings 修改要点

| 修改                                                  | 效果                  |
| --------------------------------------------------- | ------------------- |
| Brush Scaled by Interacting Obj Size false→**true** | 物体大小影响笔刷，大型物体产生更大波纹 |
| Global Brush Scale 1→**2**                          | 所有笔刷痕迹整体放大 2 倍      |
| User Input Brush Scale 1→**2**                      | 用户输入交互痕迹放大 2 倍      |
| Primitive Obj Brush Scale 1→**4.3**                 | 物体交互产生强烈大波纹         |
| Skeletal Mesh Brush Scale 1→**1.7**                 | 角色交互痕迹适度放大          |
| Brush Velocity Noise Scale 1→**30**                 | 速度噪声大幅增强，产生丰富自然波纹   |
| Brush Velocity Clamp 1→**0.5**                      | 速度钳制减半，交互更柔和        |
| 其余 15 项保持默认                                         |                     |

---

## Live Generic（通用参数）

> 涵盖预设、输出材质管线、输入源、碰撞遮罩、核心仿真材质与 Niagara 系统等综合性参数。

| #   | 参数名                                                  | 中文翻译               | 默认值                                     | 实例值                                | 说明                                                |
| --- | ---------------------------------------------------- | ------------------ | --------------------------------------- | ---------------------------------- | ------------------------------------------------- |
| 1   | **Default Preset**                                   | 默认预设               | DT_NinjaLive_Default                    | **DT_NinjaLive_Coastline_Quiet** ✅ | 初始化时自动加载的预设数据表，包含密度、速度、压力等全部仿真参数。此实例使用"海岸线平静水面"预设 |
| 2   | **Preset Search Paths**                              | 预设搜索路径             | ["/Game/FluidNinjaLive"]                | —                                  | 扫描并加载预设文件的目录路径列表。插件会在这些目录下查找匹配的预设数据表              |
| 3   | **Preset Name Filter Criteria**                      | 预设名称过滤条件           | "NinjaLive"                             | —                                  | 只加载名称中包含此字符串的预设文件，避免误加载其他不相关的数据表                  |
| 4   | **Trace Mesh Invisible**                             | 追踪网格不可见            | false                                   | **true** ✅                         | 运行时隐藏 TraceMesh。世界空间水面场景中 TraceMesh 只做交互检测，不参与渲染  |
| 5   | **Output Material Selected**                         | 输出材质选择索引           | 1                                       | —                                  | 当前选用的一级输出材质索引。切换不同的输出材质可查看不同的仿真缓冲区（密度、速度、压力等）     |
| 6   | **Output Materials**                                 | 输出材质数组             | [9 个调试材质]                               | **[]** ✅                           | 直接渲染仿真缓冲区（密度/速度/压力等）的材质列表。此实例清空，改为通过二级输出渲染到世界空间   |
| 7   | **Apply 1st Out Mat to Actors with Tag**             | 按标签应用一级材质到 Actor   | "None"                                  | —                                  | 将一级输出材质自动赋予场景中带此标签的所有 Actor                       |
| 8   | **Apply 1st Out Mat to Components with Tag**         | 按标签应用一级材质到组件       | "None"                                  | —                                  | 将一级输出材质自动赋予场景中带此标签的所有组件                           |
| 9   | **Feed Tagged Actor Niagara Component**              | 输入到带标签的 Niagara 组件 | "None"                                  | —                                  | 将流体仿真数据直接输入到场景中带此标签的 Niagara 组件，驱动粒子效果            |
| 10  | **Make 1st Output Available for 2nd Output**         | 一级输出供二级使用          | false                                   | —                                  | 将一级输出材质的结果传递给二级输出管线，实现材质链式处理                      |
| 11  | **Make 1st Output Available for Niagara**            | 一级输出供 Niagara 使用   | false                                   | —                                  | 将一级输出材质的仿真缓冲区数据暴露给 Niagara 粒子系统                   |
| 12  | **Make Pressure Available for Niagara**              | 压力场供 Niagara 使用    | false                                   | **true** ✅                         | 将流体压力场数据传递给 Niagara。开启后 Niagara 可读取压力值驱动粒子密度、颜色等  |
| 13  | **Secondary Output Material Selected**               | 二级输出材质选择索引         | 0                                       | —                                  | 当前选用的二级输出材质索引                                     |
| 14  | **Secondary Output Materials**                       | 二级输出材质数组           | []                                      | **[3 个水面材质]** ✅                    | 用于将流体仿真渲染到世界空间表面的材质实例列表。此实例配置了 3 种水面效果（平静/焦散/微风）  |
| 15  | **Apply 2nd Out Mat to Actors with Tag**             | 按标签应用二级材质到 Actor   | "None"                                  | **"seasurface681"** ✅              | 将二级输出材质自动赋予带此标签的 Actor（即场景中的水面网格）                 |
| 16  | **Apply 2nd Out Mat to Components with Tag**         | 按标签应用二级材质到组件       | "None"                                  | —                                  | 将二级输出材质自动赋予带此标签的组件                                |
| 17  | **Tertiary Output Material Selected**                | 三级输出材质选择索引         | 0                                       | —                                  | 当前选用的三级输出材质索引                                     |
| 18  | **Tertiary Output Materials**                        | 三级输出材质数组           | []                                      | —                                  | 第三组输出材质列表，较少使用。可用于额外的渲染层或特效                       |
| 19  | **Apply 3rd Out Mat to Actors with Tag**             | 按标签应用三级材质到 Actor   | "None"                                  | —                                  | 将三级输出材质自动赋予带此标签的 Actor                            |
| 20  | **Apply 3rd Out Mat to Components with Tag**         | 按标签应用三级材质到组件       | "None"                                  | —                                  | 将三级输出材质自动赋予带此标签的组件                                |
| 21  | **Use Render Target as Input**                       | 使用渲染目标作为输入         | false                                   | —                                  | 使用外部渲染目标作为流体仿真的输入源，可将场景渲染结果注入流体                   |
| 22  | **Input Render Target**                              | 输入渲染目标             | null                                    | —                                  | 用作输入的外部渲染目标引用                                     |
| 23  | **RGB-Input Material**                               | RGB 输入材质           | false                                   | —                                  | 输入材质是否使用 RGB 彩色数据而非单通道灰度。开启后可输入彩色纹理驱动流体           |
| 24  | **Input Material Clamp**                             | 输入材质钳制             | true                                    | —                                  | 是否将输入材质的数值钳制到 [0,1] 范围，防止超出范围导致仿真异常               |
| 25  | **Input Material Selected**                          | 输入材质选择索引           | 0                                       | —                                  | 选中的输入材质索引                                         |
| 26  | **Input Materials**                                  | 输入材质数组             | []                                      | —                                  | 可选的输入材质列表，用于从外部材质驱动流体密度或速度                        |
| 27  | **Sim Area Clamp**                                   | 仿真区域钳制             | true                                    | —                                  | 是否将流体物理量钳制在仿真区域内。关闭后流体可扩散到区域外，但可能导致边缘伪影           |
| 28  | **Set Internal Params to Material Param Collection** | 设置内部参数到材质参数集       | null                                    | —                                  | 将仿真内部参数同步到指定的材质参数集（MPC），供场景中的其他材质使用               |
| 29  | **Draw Internal Render Target to External**          | 绘制内部 RT 到外部        | false                                   | —                                  | 将内部渲染目标绘制到外部指定的渲染目标上，用于导出仿真结果                     |
| 30  | **Internal Render Targets to Export**                | 要导出的内部 RT          | [NewEnumerator0]                        | —                                  | 选择要导出的内部渲染目标。不同枚举值对应不同的仿真缓冲区                      |
| 31  | **External Render Targets**                          | 外部渲染目标             | []                                      | —                                  | 外部渲染目标引用列表，用于接收内部仿真缓冲区的导出数据                       |
| 32  | **Collision Mask**                                   | 碰撞遮罩纹理             | T_maskframe_256                         | —                                  | 用于初始化碰撞检测的遮罩纹理。黑色区域检测碰撞，白色区域忽略                    |
| 33  | **Input Scene Capture Camera**                       | 输入场景捕获摄像机          | null                                    | —                                  | 使用场景捕获 2D 组件捕获的场景作为流体输入源                          |
| 34  | **Input Media Player**                               | 输入媒体播放器            | null                                    | —                                  | 使用媒体文件（视频）作为流体仿真的动态输入源                            |
| 35  | **Media Texture**                                    | 媒体纹理               | T_Live_MediaTexture                     | —                                  | 媒体播放器的输出纹理，作为流体输入时的默认纹理                           |
| 36  | **Input Media Source**                               | 输入媒体源              | null                                    | —                                  | 媒体文件的来源，可以是本地文件路径或 URL                            |
| 37  | **Input Media Loop Length**                          | 输入媒体循环长度           | 9                                       | —                                  | 媒体输入循环播放的时长（秒），超过此长度后重新开始                         |
| 38  | **Randomize Noise Offsets**                          | 随机化噪声偏移            | false                                   | —                                  | 每次启动时随机化噪声纹理的偏移量，使每次仿真的噪声图案不同                     |
| 39  | **Randomize Density Texture Offset**                 | 随机化密度纹理偏移          | false                                   | —                                  | 随机化密度输入纹理的 UV 偏移，使密度图案每次启动时在不同的位置                 |
| 40  | **Allow Absolute Black Density**                     | 允许绝对黑色密度           | false                                   | —                                  | 是否允许密度值降到绝对黑（0）。关闭时将自动保持最低可见密度                    |
| 41  | **Pressure Edge Masking**                            | 压力边缘遮罩             | 0                                       | **1** ✅                            | 在仿真区域边缘逐渐衰减压力的强度。1 表示完全遮罩，防止边缘压力堆积产生的伪影           |
| 42  | **Core Sim Materials**                               | 核心仿真材质             | [18 个材质]                                | **索引 2、3 替换为 HDnoise** ✅           | 流体仿真管线使用的 18 个核心材质实例。此实例将复合梯度材质替换为高清噪声版本          |
| 43  | **Core Niagara Systems**                             | 核心 Niagara 系统      | [NS_Painter_v2_Dot, NS_Painter_v2_Line] | —                                  | 用于绘制交互痕迹的两个核心 Niagara 粒子系统（点绘制和线绘制）               |

### 此实例的 Live Generic 修改要点

| 修改                                                         | 效果              |
| ---------------------------------------------------------- | --------------- |
| Default Preset → **Coastline_Quiet**                       | 使用海岸线平静水面预设     |
| Trace Mesh Invisible false→**true**                        | 运行时隐藏 TraceMesh |
| Output Materials [9 个]→**[]**                              | 清空一级输出，改用二级输出管线 |
| Make Pressure Available for Niagara false→**true**         | 压力数据传递到 Niagara |
| Secondary Output Materials []→**[3 个水面材质]**                | 配置世界空间水面渲染      |
| Apply 2nd Out Mat to Actors with Tag → **"seasurface681"** | 水面材质自动应用到场景网格   |
| Pressure Edge Masking 0→**1**                              | 完全遮罩边缘压力伪影      |
| Core Sim Materials 索引 2、3 → **HDnoise**                    | 使用高清噪声提升流体细节    |
| 其余 35 项保持默认                                                |                 |

---

## Live Compatibility（兼容性参数）

> 控制流体仿真与不同平台、UE 版本、输入系统的兼容性行为。

| #   | 参数名                                 | 中文翻译              | 默认值                   | 实例值 | 说明                                                        |
| --- | ----------------------------------- | ----------------- | --------------------- | --- | --------------------------------------------------------- |
| 1   | **Preferred Trace Channel Name**    | 首选追踪通道名称          | "FluidTrace"          | —   | 自定义碰撞通道的名称标识，用于在项目中标识流体交互的专用通道                            |
| 2   | **Trace Channel**                   | 追踪通道类型            | TraceTypeQuery3       | —   | 交互射线使用的 UE 追踪通道。决定射线能与哪些物体发生碰撞                            |
| 3   | **Collision Channel**               | 碰撞通道              | ECC_GameTraceChannel1 | —   | 交互使用的 UE 碰撞通道，与物体上的碰撞预设匹配来决定是否触发交互                        |
| 4   | **Sim Precision**                   | 模拟精度              | NewEnumerator2        | —   | 流体仿真的数值精度模式。不同枚举值对应不同的精度与性能平衡策略                           |
| 5   | **Flip Render Targets for Mobile**  | 移动平台翻转渲染目标        | false                 | —   | 移动端是否需要翻转渲染目标。部分移动 GPU 的纹理坐标方向与桌面不同，需要翻转适配                |
| 6   | **Use Unreal Native Event Tick**    | 使用 UE 原生事件 Tick   | false                 | —   | 使用 UE 原生的 Tick 事件驱动仿真，而非插件内部的自定义时间步。开启后与 UE 生命周期更一致但控制力降低 |
| 7   | **Limit Unreal Native Event Tick**  | 限制 UE 原生 Tick 频率  | 120                   | —   | 使用原生 Tick 时限制每秒最大 Tick 次数，防止仿真更新过快导致性能问题                  |
| 8   | **Overwrite Preset Density Input**  | 覆盖预设密度输入          | null                  | —   | 手动指定一个纹理或材质覆盖预设中的密度输入，用于动态改变流体外观                          |
| 9   | **Overwrite Preset Velocity Input** | 覆盖预设速度输入          | null                  | —   | 手动指定一个纹理或材质覆盖预设中的速度输入，用于动态改变流体运动方向                        |
| 10  | **Supress UE51 Texture Smearing**   | 抑制 UE5.1 纹理拖影     | true                  | —   | 解决 UE5.1 版本中特定情况下渲染目标纹理出现的拖影问题。5.8 项目中保持开启以兼容             |
| 11  | **LWC Support**                     | 大型世界坐标支持          | true                  | —   | 启用大型世界坐标（Large World Coordinates）支持。在大型场景中避免远距离精度损失       |
| 12  | **LWC Avoid Niagara Warnings**      | LWC 避免 Niagara 警告 | true                  | —   | 在启用 LWC 时抑制 Niagara 系统中的坐标精度警告信息，减少日志噪音                   |

### 此实例的 Live Compatibility 修改要点

所有 12 项均保持默认值，未作修改。

---

## Live Debug（调试参数）

> 控制流体仿真调试信息的输出、可视化、日志记录等行为。

| #   | 参数名                                                 | 中文翻译         | 默认值   | 实例值 | 说明                                         |
| --- | --------------------------------------------------- | ------------ | ----- | --- | ------------------------------------------ |
| 1   | **Show Debug Messages-Trace Channels**              | 调试：追踪通道信息    | true  | —   | 在屏幕上打印追踪通道的初始化和检测信息，用于确认碰撞通道配置是否正确         |
| 2   | **Show Debug Messages-Memory Management**           | 调试：内存管理信息    | false | —   | 打印内存分配和释放的调试信息，用于排查显存和内存泄漏问题               |
| 3   | **Show Debug Messages - Collision and Tracing**     | 调试：碰撞与追踪信息   | false | —   | 打印碰撞检测和射线追踪的详细信息，用于调试物体交互检测                |
| 4   | **Show Debug Messages- LODInitial**                 | 调试：LOD 初始化信息 | false | —   | 打印 LOD 系统初始化时的配置信息，包括边界距离、步数等              |
| 5   | **Show Debug Messages - LODRuntime**                | 调试：LOD 运行时信息 | false | —   | 打印 LOD 在运行时的变化信息，如何时升降级                    |
| 6   | **Show Debug Messages - Interface Control**         | 调试：接口控制信息    | false | —   | 打印外部接口（蓝图、Niagara、材质）对仿真参数的实时控制信息          |
| 7   | **Show Debug Messages-Render Target Export**        | 调试：渲染目标导出信息  | false | —   | 打印渲染目标导出时的详细信息，用于排查导出管线问题                  |
| 8   | **Show Debug Messages - Number Of Tracked Objects** | 调试：追踪物体数量    | false | —   | 在当前帧追踪的物体数量，用于监控交互性能开销                     |
| 9   | **Show Warning- Number Of Bones to Track**          | 警告：骨骼追踪数量    | true  | —   | 当被追踪的骨骼数量超过推荐上限时弹出警告，防止性能过度消耗              |
| 10  | **Visualize Custom Trace Source**                   | 可视化自定义追踪源    | false | —   | 在场景中绘制自定义追踪源的视觉指示器（线框球体），方便调试追踪源位置         |
| 11  | **Save Debug Messages to Default Log**              | 保存调试信息到日志文件  | false | —   | 将屏幕调试信息同时写入 UE 的默认日志文件（Saved/Logs/），用于事后分析 |
| 12  | **Debug Messages Lifetime**                         | 调试信息显示时长     | 4     | —   | 屏幕调试信息在画面上的停留时间（秒），超过后自动消失                 |
| 13  | **Show Mouse Cursor**                               | 显示鼠标光标       | true  | —   | 在交互过程中是否显示操作系统鼠标光标。关闭后交互区域可能隐藏光标           |
| 14  | **Show Velocity Debug Cone**                        | 显示速度调试锥体     | false | —   | 在交互点绘制速度方向锥体可视化，用于调试速度场的方向和强度              |
| 15  | **Ignore Interface Commands**                       | 忽略接口命令       | false | —   | 忽略来自外部接口（蓝图、Niagara）的仿真控制命令。用于排查外部控制是否引起问题 |

### 此实例的 Live Debug 修改要点

所有 15 项均保持默认值，未作修改。

---

## Live Raymarching（光线步进参数）

> 控制流体表面的光线步进渲染、光照方向和着色效果，用于实现流体与场景光照的交互。

| #   | 参数名                                            | 中文翻译                 | 默认值     | 实例值 | 说明                                       |
| --- | ---------------------------------------------- | -------------------- | ------- | --- | ---------------------------------------- |
| 1   | **Enable Ray Marching**                        | 启用光线步进               | false   | —   | 开启流体表面的光线步进渲染，实现更逼真的光照和折射效果。需要额外的 GPU 开销 |
| 2   | **Light Direction Provider**                   | 光源方向提供者              | null    | —   | 指定提供光源方向的组件或 Actor。为空时使用默认场景主光源          |
| 3   | **Light Direction Source Is Rotation NOT Pos** | 光源方向使用旋转而非位置         | true    | —   | 使用光源 Actor 的旋转角度而非世界位置来计算光照方向。适合方向光（太阳光） |
| 4   | **Distance Based Light Attenuation**           | 基于距离的光衰减             | false   | —   | 开启后光源强度随距离衰减，适合点光源和聚光灯。关闭时全局光照强度一致       |
| 5   | **Attenuation Power**                          | 光衰减幂次                | 1       | —   | 光源距离衰减的幂次曲线指数。值越大衰减越快                    |
| 6   | **Point Light Movement Multiplier**            | 点光源移动倍率              | 1       | —   | 点光源移动对流体光照影响的倍率。值越大光源运动产生的光照变化越明显        |
| 7   | **Two Sided Shading**                          | 双面着色                 | false   | —   | 流体材质是否双面渲染。开启后从背面看也能看到着色效果，常用于薄的水面或半透明流体 |
| 8   | **Two Side Blend Pow**                         | 双面混合幂次               | 0.25    | —   | 双面渲染时正反面颜色的混合曲线控制。值越小背面越淡                |
| 9   | **Offset Light Vector**                        | 光源向量偏移               | (0,0,0) | —   | 对计算出的光源方向进行手动偏移，用于微调光照角度                 |
| 10  | **Force Manual Sun Position**                  | 强制手动太阳位置             | false   | —   | 开启后不使用场景中的太阳光，而是使用手动指定的经纬度计算太阳方向         |
| 11  | **Sun Latitude**                               | 太阳纬度                 | 0       | —   | 手动太阳位置的纬度角度（度），与 longitude 共同决定太阳在天空的位置  |
| 12  | **Sun Longitude**                              | 太阳经度                 | 0       | —   | 手动太阳位置的经度角度（度）                           |
| 13  | **Sun Height**                                 | 太阳高度                 | 0       | —   | 手动太阳的高度角（度），影响光照的俯仰角度                    |
| 14  | **Show Light Direction Vector (Yellow)**       | 显示光源方向（黄色）           | false   | —   | 在场景中绘制黄色箭头可视化当前光源方向，用于调试光照计算             |
| 15  | **Show Niagara Sys Upvector (Red)**            | 显示 Niagara 系统上向量（红色） | false   | —   | 在场景中绘制红色箭头显示 Niagara 系统的上向量方向，用于调试坐标对齐   |
| 16  | **Show Facing Plane**                          | 显示面向平面               | false   | —   | 在场景中绘制当前面向平面的线框可视化，用于调试平面朝向              |
| 17  | **Print Facing**                               | 打印面向信息               | false   | —   | 在屏幕或日志中输出当前面向平面的数值信息，用于精确调试              |

### 此实例的 Live Raymarching 修改要点

所有 17 项均保持默认值，未作修改。光线步进相关功能在此实例中未启用。

---

## Live Activation（激活参数）

> 控制流体仿真自动启用/禁用的条件，包括蓝图控制、Pawn 接近检测、激活体积和 TraceMesh 非活跃行为。

| #   | 参数名                                     | 中文翻译            | 默认值            | 实例值                  | 说明                                                         |
| --- | --------------------------------------- | --------------- | -------------- | -------------------- | ---------------------------------------------------------- |
| 1   | **Disable Blueprint**                   | 禁用蓝图            | false          | —                    | 完全禁用此 Actor 的蓝图逻辑。开启后所有蓝图事件和函数都不会执行，仅保留 C++ 逻辑             |
| 2   | **Sim Activated by Pawn Proximity**     | Pawn 接近时激活仿真    | false          | —                    | 当有 Pawn（角色）进入激活体积范围时自动启动流体仿真。关闭后需要手动触发                     |
| 3   | **Show Activation Volume in Editor**    | 编辑器中显示激活体积      | false          | —                    | 在编辑器中显示激活体积的线框范围，方便调整大小和位置                                 |
| 4   | **Activation Volume Size**              | 激活体积大小          | (50,50,50)     | —                    | 激活体积的缩放值。实际碰撞范围由 BoxExtent 配合此缩放共同决定                       |
| 5   | **Activator Proximity Check Frequency** | 激活器接近检测频率       | 0.1            | —                    | 每隔多少秒检测一次是否有激活器（Pawn）进入激活范围。值越小响应越快但 CPU 开销略增              |
| 6   | **Activator Type**                      | 激活器类型           | ECC_Pawn       | —                    | 哪种碰撞类型的 Actor 可以触发激活。默认只有 Pawn 角色可触发                       |
| 7   | **Activator**                           | 当前激活器           | null           | —                    | 当前已触发激活的 Actor 引用。运行时自动赋值                                  |
| 8   | **Trace Mesh Inactive Behaviour**       | TraceMesh 非活跃行为 | NewEnumerator1 | **NewEnumerator2** ✅ | 当流体仿真未激活时 TraceMesh 的行为模式枚举。此实例选择 NewEnumerator2 改变非活跃时的表现 |

### 此实例的 Live Activation 修改要点

| 修改                                                              | 效果                     |
| --------------------------------------------------------------- | ---------------------- |
| Trace Mesh Inactive Behaviour NewEnumerator1→**NewEnumerator2** | 改变非活跃时 TraceMesh 的行为模式 |
| 其余 7 项保持默认                                                      |                        |

---

## Live Interaction（Actor 级交互参数）

> 控制 Actor 层面的交互设置，包括 TraceMesh、交互体积、重叠过滤、骨骼追踪等。这些参数位于 NinjaLive Actor 上，而非 NinjaLiveComponent。

| #   | 参数名                                               | 中文翻译                | 默认值                | 实例值                       | 说明                                                             |
| --- | ------------------------------------------------- | ------------------- | ------------------ | ------------------------- | -------------------------------------------------------------- |
| 1   | **Show Trace Mesh in Editor**                     | 编辑器中显示 TraceMesh    | true               | **false** ✅               | 编辑器的视口中隐藏 TraceMesh 线框，避免遮挡场景视图                                |
| 2   | **Trace Mesh Size**                               | TraceMesh 缩放        | (1,1,1)            | **(80,80,1)** ✅           | TraceMesh 在三维空间中的缩放值。此实例设为 X=80, Y=80 的大平面覆盖水面区域，Z 仅 1 保持扁平    |
| 3   | **User Input Based Interaction**                  | 基于用户输入的交互           | NewEnumerator1     | —                         | 用户输入方式枚举。定义使用鼠标、触摸还是其他输入设备与流体交互                                |
| 4   | **Overlap Based Interaction**                     | 基于重叠的交互             | true               | —                         | 是否通过物理重叠检测触发交互。开启后，物体进入交互体积时会自动产生流体交互                          |
| 5   | **Use Trace Mesh as Interaction Volume**          | 使用 TraceMesh 作为交互体积 | false              | —                         | TraceMesh 是否同时充当交互触发体积。开启后无需单独的 InteractionVolume              |
| 6   | **Show Interaction Volume in Editor**             | 编辑器中显示交互体积          | false              | **true** ✅                | 编辑器中显示交互体积的线框，方便调整交互区域的范围和位置                                   |
| 7   | **Interaction Volume Size**                       | 交互体积缩放              | (1,1,1)            | **(80,80,0.2)** ✅         | 交互体积的缩放值。此实例配合 BoxExtent 形成 8000×8000×20 的扁平区域                 |
| 8   | **Track Actor Primitive Components with Tag**     | 追踪带标签的基础组件          | "None"             | —                         | 只追踪场景中带此标签的基础组件（StaticMeshComponent 等），用于精确控制交互对象              |
| 9   | **Track Actor Skeletal Mesh Components with Tag** | 追踪带标签的骨骼组件          | "None"             | —                         | 只追踪场景中带此标签的骨骼网格组件（SkeletalMeshComponent），用于角色交互                |
| 10  | **Overlap Filter Inclusive Obj Type**             | 重叠过滤包含物体类型          | [ObjType1,2,3,4,5] | **[ObjType2,3,4]** ✅      | 哪些物体类型可以触发重叠交互。此实例仅保留 PhysicsBody、Pawn、Vehicle，移除 WorldDynamic |
| 11  | **Overlap Filter Inclusive Bone Name Exact**      | 重叠过滤精确骨骼名           | []                 | **["calf_l","calf_r"]** ✅ | 只与指定名称的骨骼产生交互。此实例限定左右小腿骨骼，实现角色踩水                               |
| 12  | **Overlap Filter Inclusive Bone Name Partial**    | 重叠过滤骨骼名部分匹配         | []                 | —                         | 使用部分名称匹配骨骼（如输入"hand"可匹配"hand_l"和"hand_r"）                      |
| 13  | **Exclude Specific Actors from Overlap**          | 排除特定重叠 Actor        | []                 | —                         | 从交互检测中排除的特定 Actor 列表，用于忽略不需要交互的物体                              |
| 14  | **Auto Exclude Large Overlapping Objects**        | 自动排除大型重叠物体          | true               | —                         | 自动忽略体积过大的重叠物体，防止大型静态物体意外触发流体交互                                 |
| 15  | **Force Track Objects with No Collision Flag**    | 强制追踪无碰撞标记物体         | true               | —                         | 是否强制追踪碰撞响应设为"无"的物体。开启后即使物体关闭碰撞也会被追踪                            |
| 16  | **Force Track Bones with Similar Name**           | 强制追踪相似名称骨骼          | false              | —                         | 是否自动追踪与被追踪骨骼名称相似的其它骨骼，扩展骨骼匹配范围                                 |

### 此实例的 Live Interaction（Actor）修改要点

| 修改                                                                  | 效果                           |
| ------------------------------------------------------------------- | ---------------------------- |
| Show Trace Mesh in Editor true→**false**                            | 编辑器中隐藏 TraceMesh             |
| Trace Mesh Size (1,1,1)→**(80,80,1)**                               | 80×80 大平面覆盖水面                |
| Show Interaction Volume in Editor false→**true**                    | 编辑器中显示交互体积                   |
| Interaction Volume Size (1,1,1)→**(80,80,0.2)**                     | 扁平化交互区域                      |
| Overlap Filter Inclusive Obj Type 5 项→**[ObjType2,3,4]**            | 仅响应 Pawn/PhysicsBody/Vehicle |
| Overlap Filter Inclusive Bone Name Exact []→**["calf_l","calf_r"]** | 仅小腿骨骼触发                      |
| 其余 10 项保持默认                                                         |                              |

---

*文档生成时间：2026-07-21 | 数据来源：蓝图 CDO（NinjaLiveComponent_C / NinjaLive_C）+ 实例地图文件*
