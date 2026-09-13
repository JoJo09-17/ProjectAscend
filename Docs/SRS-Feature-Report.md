# Stylized Rendering System — Feature 与 BRDF 迁移报告

检查日期：2026-09-13。对象：`/Game/StylizedRenderingSystem`，ProjectAscend，JoJoEngine UE 5.7.4。

## 核心结论

这套系统的风格不只是 Toon 明暗分层，而是 **阴影 / 高光 / 轮廓光各自独立的遮罩、渐变、图案与颜色组合**，并为金属提供另一套参数。

建议迁移其视觉规则，而不是把整个后处理图搬进 BRDF：

- BRDF / 光照评估：明暗 Ramp、块面高光、逐灯颜色与真实阴影响应。
- 材质 / 独立风格合成：Pattern、视角轮廓光、风格参数与金属美术遮罩。
- 后处理 / 间接光阶段：描边、穿墙可见轮廓、AO 风格化、区域混合。

一个重要前提：**当前配置已开启 Substrate；只改 `ShadingModels.ush` 并不覆盖当前项目的全部路径。** 当前源码在 Substrate 评估中也已有 Toon 修改，应同步规划。

## 检查范围与证据强度

通过 UE 读取资产注册表，并将非 Demo 的 Blueprint、Material、MaterialFunction、MaterialLayer、Struct、Enum、MPC 导出为 T3D；读取实际节点连接、参数与蓝图字段说明，而非仅根据文件名推测。

目录共 230 个资产，其中 41 个 Material、39 个 MaterialInstanceConstant、30 个 MaterialFunction、2 个 MaterialLayer、15 个 Blueprint、50 个 Texture2D。这些数量包含 Demo 与 25 个描边包装材质，不代表 230 项功能。

以下描述为资产提供的功能，**不等于当前关卡全部启用**。本次未切换当前关卡、未修改资产、未做截图比对或 GPU 性能采样；Shader 编译兼容、实际帧耗与最终视觉一致性仍需后续测试。

原始导出：`D:/JoJoEngine/ProjectAscend/Saved/SRSInspection/`；资产索引：`index.json`。

## 系统组成与执行位置

1. `Blueprints/BP_StylizedRenderingSystem`：统一控制入口，创建后处理 MID、设置标量 / 向量 / 纹理参数，管理 Outline Types、相机所在 Bounds、功能模式和灯光校准。
2. `Materials/Master/M_SRS_MASTER`、`SRS_MaterialAttributes`、`MF_SRS_AttributeReader`：物体材质写入风格属性，屏幕材质读取这些属性。
3. `Materials/Private/M_SRS_MASTER_CelShader` → `MaterialsAsFunctions/SRS_MF_CelShader`：赛璐璐主后处理。实际 Blendable Location 为 **SceneColorBeforeDOF**。
4. `Materials/Private/Outlines/M_SRS_Outline01…25` → `SRS_MF_Outliner`：独立描边后处理。Outline01 为 **SceneColorAfterDOF**，Priority=1；不是 BRDF 内描边。
5. `BP_CelShadedLightColorManager`、`BP_LightCalibrationActor`、`MPC_LightColors`：注册、维护和校准灯光颜色。MPC 提供 `LightColor1/2/3`，Actor 另提供三组 Locked Light Colors。
6. `M_SRS_Translucent_MASTER`、`SRS_TranslucentMaterial`：透明物体的独立材质路径，不应假定能与不透明后处理完全统一。

## Feature 清单

### F01 — 可调阴影色与明暗过渡

支持阴影范围、过渡起点、渐变纹理与强度。`Shadow Color` 是乘到表面 Base Color 上的色彩控制，而不是简单固定灰色。对应参数：`Shadow Size`、`S Gradient Start`、`S Gradient`、`S Gradient Strength`。

基础光照估计位于 `MF_SRS_SceneTexture_Lighting`，其实际连接为：

`lightingApprox = saturate(desaturate(PostProcessInput0) / desaturate(DiffuseColor))`

这是屏幕空间近似值，包含已合成的光照信息；不能等同于单个光源的 `N·L`，也不能独立分离直接光、GI、高光与 Emissive。深色材质的除法与高亮处的 saturate 是需要验证的稳定性边界。

**迁移：高优先级。** 将明暗遮罩 / Ramp 改为由真实光照项生成，但要先决定“逐灯分层”还是“光照累加后分层”；两者不会天然得到相同结果。

### F02 — 块面高光

支持 `Highlight Max Size`、`H Gradient Start`、`H Gradient`、`H Gradient Strength`，并用物体材质的 Highlight Size Scale / Texture 做局部控制。

主图中存在 WorldNormal 与归一化“视线方向 + AtmosphericLightVector”的点积，即主方向光的半角向量式高光遮罩。它不是所有局部灯逐灯的完整 PBR 高光。

`Highlight / Rimlight Color` 可把高光推到高于原表面颜色的亮度；高光与轮廓光共享这一颜色参数，但有各自的范围与 Ramp。

**迁移：高优先级。** BRDF 使用 `N·H` 与独立 Size / Softness / Intensity 控制，不建议继续用物理 Roughness 同时承担全部美术语义。

### F03 — 视角轮廓光

根据相机方向与 WorldNormal 生成边缘遮罩，并支持 `Rimlight Max Size`、`R Gradient Start`、`RL Gradient`、`R Gradient Strength`。

`R Size Fade` 控制轮廓光接近阴影区时收缩；`Show Highlights / Rimlights in Shadow` 可允许阴影中仍存在高光和轮廓光。

**迁移：中高优先级，但不应无条件塞进逐灯 BRDF。** 纯视角 Rim 若每个灯都加一次，会随灯数变亮。建议区分“受灯光驱动的 Rim”与“每像素只合成一次的美术 Rim”。它也不是屏幕描边。

### F04 — 金属专属风格

不是仅把 Metallic 提高：金属另有阴影、高光、轮廓光颜色、尺寸、Ramp 和 Pattern。`MS Distance to Edge`、`MS Inner Bend`、`MS Outer Bend`、`MS Required Distance at Max RL Size` 调节金属阴影的视角形状与弯曲。

`SRS_MetallicShading` 还根据法线与相机方向调制表面颜色。换句话说，一部分“金属感”是美术造型，而不是 GGX 反射产生的结果。

**迁移：中优先级。** 区分物理 Metallic / F0 与 StylizedMetal Mask。建议先实现非金属，再添加金属风格分支；不能只复制当前窄高光函数期待得到相同金属效果。

### F05 — 独立 Ramp / Gradient 系统

阴影、高光、轮廓光、金属对应区域和 AO 可各自使用渐变纹理。`MF_SRS_ApplyGradient` 将 Low / High 重映射后采样渐变纹理，并提供范围钳制结果与 Clamp Mask。

内置纹理包括 Smooth、SmoothFlipped、1Step、Stripes2/3/4、NoGradient，因此可做柔和过渡、硬边与多条带，而不是固定两档。

**迁移：高优先级。** 第一版可先用分析式 Ramp；需要纹理 Ramp 时，应规划稳定的全局 / Profile 绑定，避免在每盏灯中重复采样大量独立纹理。

### F06 — 图案随明暗变化

支持 Lines、Dots、GridLines、Hexagons、HexagonalGrid、Bricks、Squares 等纹理，物体材质控制 Pattern Texture、UV Scale、Rotation；目录也提供 CameraAlignedTexture 工具。

阴影 / 高光 / 轮廓光与金属对应区域分别控制 Pattern Min / Max Thickness，让条纹或网点粗细随着相应 Ramp 变化。另有独立 Pattern Background：背景强度、渐变、大小与起点可覆盖前景参数。

`Gradient Strength=0` 仍可保留图案粗细变化而不改变渐变颜色。`Anti-Aliasing` 用于平滑图案边缘，不是替代 TSR / TAA 的完整方案。

**迁移：分层实现。** 图案坐标与纹理采样适合材质阶段；光照决定区域 / 密度，最终合成适合独立风格阶段。不要把屏幕 SceneTexture 读取搬入每灯 BRDF；后者也没有自动携带原物体 UV。

### F07 — 风格化 AO

读取 `PPI_AmbientOcclusion`，提供 `AO Color`、`AO Size`、`AO Gradient Start`、`AO Gradient` 与启用开关。

**迁移：不属于单灯 BRDF。** 放在 AO / 间接光或后合成阶段；若在每灯 BRDF 内重复乘，会改变多灯行为并混淆遮蔽与投影阴影。

### F08 — 彩色动态灯与校准

模式包括 Smooth Lights、Cel Shaded Lights、Disabled。支持 Smooth Light Strength、动态灯 Pattern Thickness、Light Color Threshold、亮 / 暗表面不同颜色强度，以及三组 Locked Light Colors。

LightColorManager 维护颜色注册表；MPC 只有三组颜色槽。这证明现有方案需要管理颜色类别，并非在后处理中拥有任意数量真实光源的独立数据。

`MF_SRS_SceneTexture_LightColor` 使用 PostProcessInput0、BaseColor 和 Light Color Filter 估算颜色；Calibration Actor / Mode 用于辅助校准。

**迁移：高优先级。** 原生逐灯评估能够直接使用真实灯色与衰减，有机会去掉颜色反推和槽位校准；但多灯累加仍需单独设计以避免色阶和高光叠加失控。三槽限制是这个颜色管理方案的限制，不代表整个 UE 只能使用三盏灯。

### F09 — Emissive 保护与混合

材质写入 Emissive Marker；后处理根据像素亮度与 `Emissive Blending Threshold` 混合，避免发光区域完全被 Cel 效果替换。

**迁移：保留独立 Emissive。** 原生方案无需从最终亮度猜 Emissive。Buff / 受击的发光和颜色表现可由 MaterialController 修改材质参数，但 DA / MID 不会自动成为逐灯 BRDF 可读取的额外数据。

### F10 — 对象筛选与区域风格控制

Actor 可整体 Enable / Disable，模式为 Cel only、Cel + Outlines、Outlines only。`Only on Custom Depth` 与 Stencil Mask Min / Max 限定 Cel 对象；描边使用自己的 Stencil 范围。

`Limited to Bounds` 的字段说明明确是 **相机进入 Bounds 后该 Actor 生效**，不是逐物体按世界区域切换材质。

**迁移：保留控制面，重定义数据路径。** BRDF 必须能区分哪些表面启用风格及其 Profile，不能仅用全球 DefaultLit 替换满足所有对象筛选。区域混合和相机区域状态应在 BRDF 外管理。

### F11 — 多类型、可穿墙的屏幕描边

支持最多 25 个 Outline Types；每类配置名称、RGBA 颜色、厚度、Stencil 范围、直视可见 / 遮挡可见。Advanced 支持最小深度差、遮挡距离阈值、随距离改变透明度和尺寸。

`SRS_MF_Outliner` 使用邻域内 / 外采样、CustomDepth 检查、SceneDepth 比较、可见性判断和最终屏幕叠加。它是深度 / CustomDepth 驱动的轮廓，不应误称为完整法线细节描边。

**迁移：继续独立后处理。** BRDF 只评估当前表面，无法通过自身生成邻域轮廓或遮挡物后方的像素。多个描边类型存在多次后处理与邻域采样成本；25 是资产系统支持上限，不是推荐全部启用。

### F12 — 透明材质风格分支

独立 `SRS_TranslucentMaterial` 输出 Base Color、Emissive Color、Opacity；提供阴影、高光、轮廓光、Pattern、Fresnel Strength、Highlight Opacity / Non-Highlight Opacity。

**迁移：后续单独开发。** 不透明 Deferred / Substrate 修改不自动覆盖透明路径，透明物体的深度、阴影、GI 和后处理数据来源需要另行验证。

## 最大的结构风险：GBuffer 通道被当作数据包

`SRS_MaterialAttributes` 输入 Pattern Gradient、Highlight Size Scale、Rimlight Size Scale、Metallic、Emissive Color，通过系数、二进制提取和 `/255` 运算输出到 Roughness、Specular、Anisotropy 等通道。

- Roughness / Anisotropy 承载打包的尺寸、图案和标记，而非只表达物理参数。
- Specular 承载 Emissive Marker；实际编码与普通非金属 F0 语义不同。
- 该函数的 Metallic 输出连接常量 0，输入的金属分类另外打包供后处理读取。
- Reader 实际采样 Roughness、StoredSpecular、Anisotropy；图中包含 `Pattern Gradient (3 Bits)` 注释，但本报告不把注释作为完整位布局规范。

因此“继续使用 SRS 物体材质，只移除后处理，然后加强当前 Toon BRDF”存在明显冲突风险：现有 Roughness 控制过渡 / 高光、Metallic / F0 控制金属分类，可能读到 SRS 的编码而非预期参数。

建议先定义新的风格参数契约（Enable / Profile、Shadow Ramp、Highlight、Rim、MetalStyle、Pattern），再确认哪些留在材质、哪些进入已有可用通道或 Profile lookup。**不要提前增加多个 GBuffer 或直接占用全部 CustomData；先量化参数数量、精度与实际存储成本。** DefaultLit 的现有通道也不能被假定为随意空闲。

## 与当前 JoJoEngine BRDF 的差距

当前源码已实现：`N·L` 映射后单段 smoothstep 明暗；Roughness 调节过渡宽度；窄带 `N·H` 高光；Metallic 收窄高光；视角项调节高光强度。

这与 SRS 的独立美术 Rim、彩色阴影、纹理 Ramp、图案背景、金属阴影造型、AO 合成与描边并不等价。当前 BRDF 并非从零开始，但参数控制能力远少于 SRS。

当前检查位置：

- `UnrealEngine/Engine/Shaders/Private/ShadingModels.ush`：`JoJoToonStylizedDiffuseAtten`、`CelShadingToonSpecMask`、`CelShadingAnimeSpecular`、`JoJoToonBxDF`；DefaultLit 返回 Toon BxDF。
- `UnrealEngine/Engine/Shaders/Private/Substrate/SubstrateEvaluation.ush`：已有相同 Toon diffuse / specular 的接入。
- `ProjectAscend/Config/DefaultEngine.ini`：`r.Substrate=True`，`r.Substrate.ProjectGBufferFormat=0`，`r.Lumen.TraceMeshSDFs=0`。

Shader / ini 是本次重新读取的结果。ini 不证明运行中 CVar 没有被覆盖；本次也未断言已渲染所有 Shader permutation。

## 建议迁移顺序

### 第一阶段 — 核心风格与参数契约

单个主方向光、普通不透明非金属。实现可调 Shadow Tint / Threshold / Softness / Ramp、独立 Highlight Size / Softness / Color / Intensity。定义样式 Profile 与保留物理参数的材质接口，同步 Legacy 与当前 Substrate 路径。

要明确有色暗面的能量来源：真实投影阴影将直接光遮断后，单纯给 BRDF diffuse 乘一个 ShadowTint 并不能自动产生亮的暗面；可由受控环境 / GI 或独立风格底色承担。不要无依据重复乘 Shadow.SurfaceShadow，现有 Deferred 累加路径也会使用真实阴影。

### 第二阶段 — Rim、局部灯与金属

单次美术 Rim 与阴影可见性选项；真实彩色局部灯；多灯累加规则；金属专属参数和视角造型。不沿用三槽反推颜色方案。

### 第三阶段 — Pattern 与环境光

材质内准备稳定的图案坐标 / 遮罩；光照或合成阶段调节图案密度和背景；分别处理 AO / Lumen 间接光与自发光。测试图案远距离与运动中的稳定性。

### 第四阶段 — 保留屏幕描边，扩展透明

继续使用或精简 Outline Types；单独验证透明材质与切换 / 混合。逐项关闭被原生替换的 SRS Cel 功能，避免同一个效果被 BRDF 与后处理各做一次。

## 最小验收场景

- 灰、深色、饱和色非金属；金、银；Emissive；透明各一组。
- 主方向光、真实投影阴影、彩色 Point / Spot、多灯叠加、灯全关仅 GI。
- 正视 / 掠射角、正常与反向法线、角色移动、相机远近、不同曝光。
- 对象风格开关 / Profile，Stencil 划分和遮挡描边。
- 固定相机与曝光，分别记录 SRS baseline、新原生效果、两者同时开启的图像和 GPU 数据。

目标应先是复现可识别的核心风格，不承诺逐像素复制屏幕近似算法；只有明确累加顺序与数据语义，才能讨论视觉等价与性能收益。

## 参考与原始证据

本项目节点连接是功能结论的主要依据。关键导出文件位于 `Saved/SRSInspection`，可按资产路径对应 `_Game_… .t3d` 文件检查。

- [屏幕光照反推节点](D:/JoJoEngine/ProjectAscend/Saved/SRSInspection/_Game_StylizedRenderingSystem_MaterialFunctions_Private_MF_SRS_SceneTexture_Lighting.t3d:56)。
- [SRS 材质属性编码输出](D:/JoJoEngine/ProjectAscend/Saved/SRSInspection/_Game_StylizedRenderingSystem_MaterialFunctions_SRS_MaterialAttributes.t3d:400)。
- [后处理读取的材质通道](D:/JoJoEngine/ProjectAscend/Saved/SRSInspection/_Game_StylizedRenderingSystem_MaterialFunctions_Private_MF_SRS_AttributeReader.t3d:897)。
- [Cel 后处理位置](D:/JoJoEngine/ProjectAscend/Saved/SRSInspection/_Game_StylizedRenderingSystem_Materials_Private_M_SRS_MASTER_CelShader.t3d:23)；[描边后处理位置](D:/JoJoEngine/ProjectAscend/Saved/SRSInspection/_Game_StylizedRenderingSystem_Materials_Private_Outlines_M_SRS_Outline01.t3d:35)。
- [当前 Toon BRDF](D:/JoJoEngine/UnrealEngine/Engine/Shaders/Private/ShadingModels.ush:208)；[当前 Substrate Toon 接入](D:/JoJoEngine/UnrealEngine/Engine/Shaders/Private/Substrate/SubstrateEvaluation.ush:517)；[当前项目配置](D:/JoJoEngine/ProjectAscend/Config/DefaultEngine.ini:23)。

Epic 官方参考：[Post Process Materials](https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-materials-in-unreal-engine)、[Programming with Substrate GBuffer Formats](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-substrate-gbuffer-formats)。用于交叉核对后处理位置、GBuffer 与 Substrate 分工；在线文档默认版本可能更新，当前实现判断以本地 UE 5.7.4 源码为准。
