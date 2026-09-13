# 受击特效与贴花

默认 DA：`/Game/Combat/ImpactFX/Profiles/DA_EnemyImpact_Default`。
它是 `AscendImpactFXProfile` 类型。可在内容浏览器新建同类型 Data Asset，供不同角色、武器或动画复用。

## 配置资源

- VFX：NiagaraSystem 与 ParticleSystem 均可选，同时填写会同时生成。设置缩放、偏移和最大存活时间。
- 使用 Mesh Socket：启用后填写骨骼或 Socket 名。Attach VFX 控制是否跟随模型；关闭时留在生成位置。
- Decal：填写 Deferred Decal 材质。DecalSize 的 X 是投射深度，Y/Z 是贴花覆盖范围。
- Ground 默认向下检测地面；ImpactSurface 使用本次命中的位置与法线。
- FadeDelay 控制保持时间，FadeDuration 控制消退时间。消退后清理组件，不销毁被击中的角色。

默认粒子与暗红圆形贴花仅为可替换示例。

## Niagara 初始方向

当前 `NS_EnemyImpact_Default` 已添加 Vector 用户参数 `User.ProjectileDirection`。在 Niagara 的初始速度或方向模块中，把方向输入绑定到这个用户参数，再用独立的速度值控制粒子速度。

命中时保存子弹的归一化飞行方向，受击 Notify 延迟触发时仍可读取，即使子弹已经销毁。系统先创建未激活的 Niagara 组件，写入方向后再激活，供 burst 首帧使用。

DA 的 `VFX / Direction` 中可以修改参数名。默认传世界空间方向；如果 Niagara 方向输入使用特效局部空间，启用 `Niagara Direction In Local Space`，会按生成后的组件变换转换方向。近战没有子弹时使用命中方向，普通无受击上下文的 Notify 使用模型朝向。

## 角色默认配置

在 `DA_Saber_Manny` 或 `DA_Archer_Manny` 的 `HitReaction / HitFXProfile` 指定默认 DA。
受击发生时记录本次命中位置与方向，实际生成时间交给动画 Notify。

## 动画时点

打开 `/Game/Combat/HitReaction/Montages` 中的受击 Montage，在 `ImpactFX` 通知轨道添加 `Ascend Impact FX`。

- Spawn VFX、Spawn Decal：可同时启用，或用两个 Notify 分别控制。
- Profile：指定时覆盖角色 DA；留空则使用角色的 HitFXProfile。
- Use Hit Reaction Context：敌人受击默认启用。关闭后可将该 Notify 用于其他动画，但必须指定 Profile；位置使用模型位置或 DA 中的 Socket。

当前示例在 0.06 秒生成 VFX，在 0.12 秒生成贴花。拖动两个 Notify 即可修改时间，无需修改 C++。
重播受击动画时记录新的命中；同一受击实例的每类效果最多生成一次。被打断、已经结束或死亡后的受击 Notify 不会补生成效果。

## 脚本

`Scripts/SetupImpactFX.py` 创建默认示例，并重设八个受击 Montage 的时点为 0.06 / 0.12 秒。
重复运行会保留已有 DA 与材质配置，但会重设 Notify 时间，因此手工调好时点后无需重跑该脚本。
