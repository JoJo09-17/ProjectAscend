# JoJo legacy Toon BRDF: profiles and antialiasing

Uses Default Lit with Substrate disabled. Outlines, screen-space patterns and
postprocess effects remain outside this implementation.

## Authoring

`Scripts/SetupToonBRDF.py` creates missing assets under `/Game/Materials/Toon`
without replacing existing assets. Run using the newly compiled editor.

- `DA_ToonProfiles`: library; array entries 0..6 map to material profile IDs 1..7.
- `M_ToonSurface`: ordinary BaseColor, Roughness, Metallic, Specular inputs plus
  `ToonProfile` and `ToonDetailScale` scalar parameters.
- `MI_Toon_Default`, `MI_Toon_Soft`, `MI_Toon_Bands`, `MI_Toon_Metal`, `MI_Toon_PBR`:
  sample instances. Profile 0 uses the original Default Lit PBR BxDF and no rim.
- `C_Toon_Smooth`, `C_Toon_ThreeBands`, `C_Toon_Hard`: optional response curves.

Project Settings > Game > JoJo Toon BRDF selects the library loaded at startup.
The active library's property edits apply immediately. After editing a referenced
curve, invoke Apply Profiles on the library to rebuild its texture atlas.
Only one library is active per engine process, including editor worlds.

In any legacy Default Lit material, connect a scalar to `Toon Profile (0=PBR, 1-7)`.
IDs are discrete, not blended. `Toon Detail Scale (0.1=1x)` controls highlight/rim
size, with a value of 0 disabling these details. Dynamic material instances can
change these scalar parameters; no custom Roughness/Metallic encoding is needed.
Existing SRS materials using packed physical inputs require explicit conversion.

## Features

- Binary diffuse with independent light/shadow tints, threshold and softness.
- Screen-footprint-aware transition widths for diffuse, highlights, rim and metal.
  The base pass records normal variation; deferred pixel lighting also accounts
  for varying local-light directions. Nanite uses analytic geometric derivatives.
- `ShadowTerminatorBias`: VSM receiver correction for grazing-angle Toon pixels.
  Defaults to 5 in engine NormalBias units; 0 disables the additional correction.
  The correction fades out between N.L=0.1 and 0.35. PBR profile 0, other shading
  models, hair and Substrate keep the original bias. This addresses coarse-mesh
  self-shadow triangles that threshold AA cannot remove. Large values can cause
  light leaks or offset grazing-angle contact shadows; keep the lowest useful value.
- Optional CurveFloat responses sampled into a texture atlas, with an integral
  channel to filter hard ramp steps. Curves use normalized coordinates and values.
- Independent highlight tint, intensity, size, softness and dark-side visibility.
- Independent artistic rim emitted once in the opaque/masked base pass, rather
  than added once per light. This is an unshadowed stylized emissive contribution.
- Metal shadow tint/strength/edge, inner/outer bend and separate highlights.
  Pure metals receive a deliberately nonphysical direct-light base fill scaled
  by MetalShadowStrength, so their shadow shaping remains visible.
- Per-material profile selection and a PBR opt-out.

Real cast-shadow visibility still modulates direct light once. ShadowTint affects
the normal-based dark side; it does not replace all projected-shadow colors.
Threshold AA does not increase VSM resolution or smooth geometry silhouettes.
The separate VSM terminator correction is a bounded receiver-bias workaround,
not mesh subdivision and not a universal cure for arbitrary low-poly meshes.
Nanite normal-map derivatives, stereo and all engine permutations are not certified.
Material Roughness remains unchanged in the GBuffer and direct-light path. The
environment path can now use a separate, profile-controlled roughness floor.

## Environment reflections and indirect light

In `DA_ToonProfiles`, expand the material's profile and use these categories:

- **Environment Reflection**: `ReflectionStylization` (0 disables all reflection
  changes), `MetalReflectionRoughnessFloor` (default 0.55), `ReflectionBands`
  (default 3), `ReflectionIntensity` (0.75), `ReflectionSaturation` (0.35).
- **Indirect Light**: `IndirectStylization` (default 0.75; 0 bypasses),
  `IndirectBands` (3), `IndirectIntensity` (0.7), `IndirectSaturation` (0.65),
  `IndirectBrightnessLimit` (2, scene-linear luminance before intensity/blend).

For Metallic=1/Roughness=0, the environment-only floor removes sharp mirrored
scene detail while leaving the existing artistic direct-light highlight intact.
Lumen's material read keeps reflection classification, ray generation, rough
screen-probe integration and final composition on the same effective roughness.
The floor is weighted by metallic and reflection stylization. It does not rewrite
material assets. Reflection intensity/bands/color also gently affect nonmetallic
reflections (one quarter of the metal blend), without imposing a roughness floor.

Reflection luminance is softly quantized after combining traced and rough-probe
reflections. Material specular tint is approximately factored out before reducing
incident-light saturation, so colored metals retain their color. A 20% continuous
response preserves low-light readability instead of turning all dark metals black.
This is an artistic approximation, not a physically energy-conserving metal model.

Diffuse indirect irradiance and Lumen short-range GI are shaped before multiplying
by albedo and occlusion. Both helpers undo pre-exposure before shaping and restore
it afterward; they do not posterize final SceneColor, direct light or UI. Profile 0
returns the original values, and disabling both style blends restores the original
environment paths for a Toon material. The lighting solve, ray hit distances, AO
and cast-shadow visibility are retained. Rapid profile changes can require Lumen
history to settle; soft transitions reduce, but do not eliminate, temporal artifacts.

Primary implementation target: opaque/masked legacy Default Lit + Lumen in deferred
DX12. Reflection-capture/standalone reflection composition also has the helper;
SSR tracing itself is not roughened, and static lightmaps, forward/mobile lighting,
translucency, hair, clear coat and Substrate are not a complete stylized-GI solution.
Pure metals primarily show changes from specular environment lighting, not diffuse GI.

Reference direction: [YivanLee's Toon Lumen showcase](https://superpandaman.artstation.com/projects/KOQ3V9)
and [official UOD2022 presentation](https://www.bilibili.com/video/BV18Y411R7xZ/).
The public showcase does not provide implementation code; these are references for
scope/art direction, not a claim to reproduce his algorithm.

## Cost and validation

No new GBuffer render target is allocated. Legacy Default Lit now writes existing
GBuffer custom data and SceneColor for the rim, so bandwidth is not free. The
256x32 RGBA32F curve atlas costs 128 KiB; float precision protects integral filtering.
The view also carries 80 float4 profile values. Curve-enabled responses cost texture
reads; the default analytic direct-light profile avoids curve reads. Environment
settings reuse the atlas's previously-unused B/A channels, keeping its 128 KiB size
and the existing view-uniform layout. Enabled environment paths add settings
texture reads and arithmetic; this is not a measured zero-cost change.

Standalone DXC compilation of the actual helper covers pixel and compute stages.
Normal editor-target compilation, asset creation and a real DX12 startup/shader
compile are separate checks. Visual acceptance requires comparing near/distant
curved surfaces, moving lights and cameras, metals and cast shadows in the level.

### Verified 2026-09-14

Normal ProjectAscendEditor UBT build succeeded; DX12 editor reopened DevelopmentMap
with Substrate=0 and the profile library active. No shader compile errors or fatal
errors occurred in `Saved/Logs/JoJoToonTerminatorVerified.log` at handoff.
Parallel shadow-view initialization is supported: shared profile upload/read state
is protected by a mutex rather than restricted to the main render thread.

Editor A/B captures (`Saved/Screenshots/WindowsEditor/ToonShadowOn.png` and
`ToonShadowOff.png`) established that the remaining triangle-shaped sphere seam
was self-shadowing rather than the analytic ramp. Global bias tests at 1, 2 and 5
were temporary and restored to 0.5. The final targeted correction at profile bias 5
removed the visible triangle seam in `ToonTerminatorFixed.png`, while retaining the
sphere's ground shadow; global bias remained 0.5. This validates the tested sphere
and lighting, not every mesh/light/scale combination. Extreme low-poly meshes and
grazing contact shadows still need asset-specific tuning.

### Environment validation 2026-09-14

Normal UBT build succeeded, followed by a DX12 editor restart with the final Lumen
shader changes. `Saved/Logs/JoJoToonEnvironmentFinal.log` contains no shader compile
errors, Python errors or fatal errors at handoff. Standalone DXC also compiled the
actual environment helper in pixel and compute contexts.

`Scripts/VerifyToonEnvironment.py` is an editor-only transient A/B harness for this
DevelopmentMap (requires exactly one Engine BasicShapes sphere). It uses a transient
profile library and MID, asserts profile readback, and restores the original library,
sphere material, selection and camera. It does not save test materials or the level.
Do not operate the viewport during capture. Python array structs require assigning
the edited struct back to the array; the final harness explicitly does this.

Final fixed-view captures are in `Saved/Screenshots/WindowsEditor`:

- `ToonEnv_Metal_Off.png` / `ToonEnv_Metal_On.png`: same Metallic=1, Roughness=0
  material and camera. The first retains mirrored buildings/grid; the second removes
  those sharp details while retaining the artistic direct highlight and ground shadow.
- `ToonEnv_Gold_On.png`: golden color is retained instead of desaturating to silver.
- `ToonEnv_Indirect_Off.png` / `ToonEnv_Indirect_On.png`: nonmetallic sphere with
  reflection styling disabled in both. Indirect styling reduces the smooth dark-side
  gradient and brightness; direct-light boundary/highlight remain in place.

These checks validate the current scene, not moving-camera temporal stability,
every HDR lighting range, alternate platforms or all reflection permutations.
The default metal floor is deliberately strong; reduce it to retain more environment
structure. No GPU timing/performance certification was performed.
