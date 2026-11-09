# Control Rig 验证规范

适用范围：本项目 UE 5.8.3 的 Seven 控制器迁移。本文集中维护验证要求，迁移指南和配方引用本文。

## 1. 先确认在测试什么

记录 Max 场景、时间、控制值、UE Rig、预览网格、骨架版本和控制映射。原始控制单位、归一化参数、手柄显示尺寸分开记录；脚本增益不能直接当作真实拖动距离。

验证状态分为：

| 状态 | 需要的证据 |
|---|---|
| 结构核对 | 资产、目标骨骼/控制、节点连接、执行链存在 |
| 数值验证 | 有效实例、基准姿势、多个输入样本的完整变换对照 |
| 画面验收 | Previewing Rig 下查看中间过程、两侧和组合姿势 |
| 持久化验证 | 保存并重新加载后，结构、编译、数值仍一致 |

历史报告只对当时的场景与样本有效。`BS_UP_TO_DATE`、`execute()` 返回 true、节点数量一致，都不能替代这些验证。

## 2. 测试实例必须有真实层级

2026-11-06 会话中出现过 `create_control_rig()` 返回的实例层级为空的情况（视调用时机而定）；读取不存在的骨骼会返回零/单位变换，而求解仍可返回 true，导致自检假通过。实测在 `request_init()` 之后层级通常已有内容，但仍应显式断言，不要依赖时序。

测试 POSITION 骨骼控制时，可使用以下初始化片段（仅修改新建临时实例）：

```python
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
assert rig is not None
source = rig.get_hierarchy()
inst = rig.create_control_rig()
inst.request_init()
rh = inst.get_hierarchy()
if not rh.get_all_keys():
    rh.copy_hierarchy(source)
    inst.request_init()

def key(name, kind):
    return unreal.RigElementKey(type=kind, name=name)

control = key('CTRL_EYE_LID_R', unreal.RigElementType.CONTROL)
bone = key('Bone028', unreal.RigElementType.BONE)
assert rh.contains(control) and rh.contains(bone), '实例缺少测试元素'

# 本例适用于本资产的 POSITION 控制；明确清零所有输入，避免预览值混入测试。
for k in rh.get_all_keys():
    if k.type == unreal.RigElementType.CONTROL:
        rh.set_control_value(k, rh.make_control_value_from_vector(unreal.Vector(0, 0, 0)))

def sample(z):
    rh.set_control_value(
        control, rh.make_control_value_from_vector(unreal.Vector(0, 0, z)))
    assert inst.execute('Forwards Solve')
    assert rh.contains(bone), '求解后层级失效'
    return rh.get_global_transform(bone)

baseline = sample(0.0)
closed = sample(-10.0)
assert (closed.translation - baseline.translation).length() > 0.01, '目标骨骼没有有效响应'
```

这是“实例能否有效测试”的检查，不是“眼睑已经正确”的判据。多个控制参与时，先把相关控制设为明确基准值，再依次求解；不要沿用编辑器中未记录的预览值。测试实例复制层级后只证明测试实例有效，不能据此宣称正式运行实例已正确初始化。

## 3. 对照完整变换

- 位移比较 `p(s)`，旋转比较完整 `q(s)` 或旋转矩阵，必要时比较缩放。
- 不能只比较位移差、欧拉某一轴、总旋转角，或只看端点。眼睑诊断中 Bone028 端点位置误差约 0.005 mm，但朝向误差约 42°。
- 至少覆盖中性、正负有效范围、中间值、限位与回到中性。单侧控制的无效方向应被夹住。
- 四元数 `q` 与 `-q` 表示相同旋转。对单位四元数可用 `2*acos(clamp(abs(dot(q_test,q_ref)),0,1))` 计算朝向误差，输出时注明弧度或度。
- Max 与 UE 的矩阵/四元数约定及骨骼局部基不同，先验证静止姿态映射，再比较动画增量；点坐标换算不能直接当成旋转轴换算。

项目的骨骼 `GlobalSpace` 是 Rig/组件空间，不是关卡的绝对世界空间。还要测试角色整体移动、转身和头部旋转；静止头部通过不代表能跟随动画。

## 4. 空间与组合顺序

按 UE 四元数约定：

```text
世界/组件空间增量：q_new = q_delta_global * q_initial
骨骼局部空间增量：q_new = q_initial * q_delta_local
```

这里“世界”只指上述 Rig 全局空间。选择乘法顺序前，先确定增量实际属于哪个空间，并通过 `RotateVector` 或矩阵验证方向。把局部轴拿去左乘仍然会错；`-dEuler` 不是所有控制器通用的符号规则。

纯平移偏移可以按向量求和。旋转、父级枢轴和表达式修正必须按源 Rig 的变换顺序合成，不能要求所有控制的最终位移都线性相加。

## 5. 交叉、执行流、画面与保存

1. 单控测试：目标动，其他独立部位保持基准。
2. 两侧测试：左右分别求解，验证符号与限位。
3. 组合测试：整体 + 点控 + 弯曲；眼睑 + 眼周点控；头部动画 + 面部控制。比较组合结果与 Max。
4. 重建后遍历 Execute 连接，确保眉毛和眼周 SetTransform 都从入口可达。不要假定对同一个 Execute 输出连续 `add_link` 就能保留两个分支。
5. 编辑器切到 `Previewing Rig`，选择目标控制，观察开合全过程并截图。`Previewing Reference Pose` 画面不能验收 Rig 驱动。
6. 恢复记录下来的预览值；保存并重开资产，再测。读取资产列表或编译成功不等于持久化功能验证。

## 6. 证据保存

2026-11-06 左右眼球组合复核发现，`FRigComputedTransform::Equals` 默认以 `0.0001` 比较变换各分量；微小输入可能不触发层级写入。应另测连续切换与独立实例，记录缓存残差，并明确检查其符合引擎容差。独立实例的公式对照仍保持严格精度，不能直接放宽全部测试。实测及代码入口见 [眼球控制](EyeballControls.md)。

每份报告写明日期、源资产、输入、预期值、实际值、误差与未测范围。需要交接的 JSON/图片放 `Docs/Evidence/` 或 `Docs/Images/`；`Saved/Codex/` 可保留原始调试过程，但该目录被 Git 忽略。

当前眼睑的逐样本证据见 [Evidence/SYP_R001](Evidence/SYP_R001/README.md)，功能结论见 [SYP_R001Diagnosis.md](SYP_R001Diagnosis.md)。

2026-11-06 12:24 对正式 Rig 的临时实例复核见 [复核快照](Evidence/control_rig_recheck_2026-11-06.json)；同目录 [采集脚本](Evidence/recheck_control_rig.py) 可从项目根目录用 `python Scripts/ue_run.py Docs/Evidence/recheck_control_rig.py` 执行。脚本仅查询内存资产和求解临时实例，重跑会覆盖该复核 JSON；不保存、重载或修改 Rig 资产，不属于画面/持久化验收。
