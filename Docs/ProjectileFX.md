# 子弹本体配置

默认资产在 `/Game/Combat/Projectiles/Profiles`：`DA_Archer_Projectile_Light`（普通攻击、远程敌人）和 `DA_Archer_Projectile_Heavy`（重击）。在 `DA_Archer_Manny` 的各个 Archer 攻击设置中修改 `ProjectileProfile`，可以给不同攻击指定不同子弹表现。

`AscendProjectileProfile` 类型的 DA 支持：

- Mesh、MaterialOverride、MeshTransform：本体模型、材质以及相对于碰撞球的偏移、旋转、缩放。Mesh 留空可使用纯粒子子弹。
- FlightNiagara、FlightFXTransform：跟随子弹的飞行特效和局部变换；子弹销毁时清理特效。
- DirectionParameter：默认 `User.ProjectileDirection`，Niagara 激活前写入归一化发射方向。Niagara 中需要创建同名 Vector 用户参数并绑定到初始方向输入。
- Direction In Local Space：方向输入使用组件局部空间时启用。
- LifeSpan、GravityScale：子弹最长存活时间（秒）和重力倍率。

子弹 Actor 的前方是局部 +X。导入的 Arrow 模型沿 +Y，因此重击默认模型旋转 Yaw=-90、缩放 0.9、位置 X=-40。

伤害、飞行速度、碰撞球半径和穿透开关仍在 `DA_Archer_Manny` 对应攻击条目中。模型和特效缩放独立于伤害判定半径。飞行 Niagara 初始方向是发射时的快照；需要随弯曲轨迹更新的特效可以读取组件朝向。

受击特效仍由 `DA_EnemyImpact_Default` 和受击 Notify 管理。默认子弹 DA 的 FlightNiagara 留空，供制作特效后填写。
