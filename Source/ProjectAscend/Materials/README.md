# MaterialController

Create a Data Asset in the Content Browser and select **MaterialController**.
One asset describes one material presentation (for example a hit flash or buff).

- `ScalarParameters`, `VectorParameters`, `TextureParameters`: names must match
  parameters exposed by the target material. This system does not modify shaders.
- `MaterialSlots`: empty targets all slots; otherwise specify zero-based slot indices.
- `Duration`: seconds, or zero for an effect that lasts until explicitly removed.
- `Priority`: higher values override shared parameter names; the most recently
  applied effect wins at equal priority. Different parameter names compose.

Every Ascend base character has a `MaterialController` component. In Blueprint,
call `ApplyController(DA)` on it and retain the returned handle. Zero means failure.
Call `RemoveController(handle)` when a buff ends, or `RemoveControllersByAsset(DA)`
to remove every application of that asset. Timed effects expire automatically.
Reapplication adds a separate layer, rather than refreshing an earlier handle.

The component lazily creates private dynamic material instances for the character's
main mesh on the first application, and reuses them. For another mesh (weapons,
accessories, etc.), use another component and call `InitializeMaterials(mesh)`.
After changing a mesh/material loadout, reinitialize explicitly; this clears effects.

`ClearControllers` restores baseline parameters while retaining the private MIDs.
`RestoreOriginalMaterials` also puts the original material objects back. Original
MID parameters are copied into the private instances; shared materials and Data
Assets are never changed. Do not write effect parameters directly into these private
MIDs: recomposition resets them. Use another controller layer instead.

No buff/damage event is automatically connected. Configure matching material
parameters and invoke the component from the relevant gameplay event first.

Automation test: `Ascend.Materials.ControllerLifecycle` covers per-component MID
isolation, independent effect handles, timed expiry, persistence, and restoration.
