# 冰弹 Niagara

源资产：`/Game/StylizedToonShotFX04/Particles/P_ky_shotIce1`。

新系统：`/Game/Combat/Projectiles/FX/NS_Archer_IceShot`。
子弹 DA：`/Game/Combat/Projectiles/Profiles/DA_Archer_Projectile_IceShot`，FlightNiagara 已填写，Mesh 留空。

在 `DA_Archer_Manny` 对应攻击的 `ProjectileProfile` 选择这份冰弹 DA 即可使用；当前普通攻击和重击的引用没有自动切换。

系统有四个发射器：shot（冰弹外层）、shotCore（核心）、dustS（冰晶碎屑）、smoke（烟雾）。保留原材质和模型、生命周期、颜色、动态材质参数、旋转、尺寸曲线以及按移动距离生成拖尾的设置。重复的 Size Over Life 在 Niagara 中合并相乘，避免转换工具的覆盖行为。

弹体与核心使用局部空间跟随子弹；碎屑与烟雾使用世界空间，已经生成的粒子留在飞行轨迹上。静止预览时拖尾会少一些，移动子弹才能看到完整效果。

DA 的 FlightFXTransform 可调整整体缩放和朝向。`User.ProjectileDirection` 已暴露并由子弹激活前写入；当前方向通过跟随子弹的组件变换控制。需要额外定向发射时，可以在 Niagara 对应方向输入中绑定这个参数。

旋转动态输入脚本已复制到项目的 `FX/Modules`，运行时无需启用 CascadeToNiagaraConverter 插件。原 Cascade 保留。

验证：在开启渲染、未启用转换插件的环境中进行实际 Niagara 模拟与 SimCache 检查，四层都有粒子，并确认移动时碎屑和烟雾生成。记录在 `Saved/IceShotNiagaraVerification.json`。
