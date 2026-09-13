# JoJo BRDF — phase 1

Historical phase-1 notes. The profile/AA implementation supersedes these controls;
see [JoJo-BRDF-Profiles.md](JoJo-BRDF-Profiles.md) for the current workflow.

Uses legacy Default Lit (`r.Substrate=False`). This iteration changes analytical
direct lighting only, not Lumen GI, reflections, AO, emissive, transparency,
outlines, texture patterns, or post-processing. It does not reproduce the entire
screen-space Stylized Rendering System.

## Style controls

Edit `UnrealEngine/Engine/Shaders/Private/JoJoToonBRDF.ush` and recompile shaders.
These are global shader constants, not runtime console variables or per-material
parameters. No additional GBuffer is allocated, and no physical channel is packed
with effect data. MaterialController DA cannot override these constants.

- `JoJoShadowThreshold`: N.L threshold, in [-1, 1]. Higher means more dark area.
- `JoJoShadowSoftness`: half-width of the analytic smoothstep transition.
- `JoJoShadowTint`, `JoJoLitTint`, `JoJoDiffuseIntensity`: linear-space colors and
  strength for the normal-based dark/lit sides of each light.
- `JoJoHighlightSize`: 1 - N.H at transition center. Higher means a larger patch.
- `JoJoHighlightSoftness`, `JoJoHighlightTint`, `JoJoHighlightIntensity`: independent
  block-highlight controls. Material Specular/F0 remains its color/reflectance input.
- `JoJoHighlightInDark`: highlight visibility on the normal-based dark side, not
  permission to bypass cast shadows.
- `JoJoMetalHighlight*`: separate highlight controls blended using real Metallic.

Physical Roughness no longer adjusts direct Toon diffuse/highlight shape; it still
has its normal meaning in unchanged reflection and indirect-light paths. Metallic
and Specular/F0 remain ordinary material inputs. Existing SRS surface materials
that pack custom data into these inputs must not be treated as ordinary PBR inputs.

## Light and shadow semantics

Diffuse uses `Lambert(albedo) * lerp(ShadowTint, LitTint, ramp(N.L))` with the real
light's attenuation and color. Highlights use a safe half-vector and independent
N.H ramp, suppressing back-lit highlights and the degenerate L=-V case.

UE's existing accumulation applies cast-shadow visibility once; this code neither
squares it nor divides by it. Fully shadowed direct light is still zero, with
existing GI supplying the dark-side environment. ShadowTint colors the normal-based
dark region of visible lights; it is NOT a color replacement for all cast-shadow
pixels. Colored cast-shadow fill requires a separately designed indirect/base layer.

This deliberately uses per-light ramps. Multiple lights still add, including the
nonzero dark-side response, so it is not equivalent to SRS's final-screen ramp.
There is no standalone artistic rim term repeated for every light. A single-pass
rim, per-object style Profile, texture Ramp and metal shadow shaping are future work.

The older helper functions remain intact for the disabled Substrate path. The new
legacy BxDF alone consumes these phase-1 controls. Other PBR shading models remain
on their existing BxDF implementations.

## Validation

Compile the editor normally, restart it, and confirm `r.Substrate=0`. Test ordinary
Default Lit gray/colored surfaces, dielectrics and metals, direction/point/spot
lights, real cast shadows, and varying view angles. CPU numeric tests in
`Saved/TestJoJoBRDF.ps1` mirror the analytic formulas; actual GPU compilation and
visual acceptance are separate checks.

2026-09-13 checks: normal UBT build succeeded (C++ target up to date); numeric
tests passed; DX12 editor started and loaded DevelopmentMap, with runtime
`r.Substrate=0` (ProjectSetting). No shader/material compile errors were found in
`Saved/Logs/JoJoBRDFStartup.log` at handoff. Final art direction still needs visual
acceptance; this is not a claim that every engine shader permutation was tested.
