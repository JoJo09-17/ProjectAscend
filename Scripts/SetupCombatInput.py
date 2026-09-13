"""Create editable Enhanced Input assets; preserve existing keyboard mappings."""
import unreal

tools = unreal.AssetToolsHelpers.get_asset_tools()
context = unreal.load_asset('/Game/Input/IMC_Default')
assert context
data = context.get_editor_property('default_key_mappings')
mappings = list(data.get_editor_property('mappings'))

specs = [
    ('IA_LightAttack', 'LeftMouseButton', False),
    ('IA_HeavyAttack', 'RightMouseButton', False),
    ('IA_GamepadMove', 'Gamepad_Left2D', True),
    ('IA_GamepadSwitchTarget', 'Gamepad_Right2D', True),
    ('IA_GamepadLightAttack', 'Gamepad_LeftShoulder', False),
    ('IA_GamepadHeavyAttack', 'Gamepad_RightTriggerAxis', False),
    ('IA_GamepadDodge', 'Gamepad_FaceButton_Right', False),
    ('IA_GamepadSkill1', 'Gamepad_FaceButton_Left', False),
    ('IA_GamepadSkill2', 'Gamepad_FaceButton_Top', False),
    ('IA_GamepadSkill3', 'Gamepad_LeftTriggerAxis', False),
    ('IA_Jump', 'Gamepad_FaceButton_Bottom', False),
]
for name, key, axis in specs:
    action = unreal.load_asset('/Game/Input/Actions/' + name)
    if not action:
        action = tools.create_asset(name, '/Game/Input/Actions', unreal.InputAction, unreal.InputAction_Factory())
        action.set_editor_property('value_type', unreal.InputActionValueType.AXIS2D if axis else unreal.InputActionValueType.BOOLEAN)
        if axis:
            modifier = unreal.new_object(unreal.InputModifierDeadZone, outer=action)
            action.set_editor_property('modifiers', [modifier])
        unreal.EditorAssetLibrary.save_loaded_asset(action)
    if key in ('Gamepad_LeftTriggerAxis', 'Gamepad_RightTriggerAxis') and not action.get_editor_property('triggers'):
        action.set_editor_property('value_type', unreal.InputActionValueType.AXIS1D)
        action.set_editor_property('triggers', [unreal.new_object(unreal.InputTriggerDown, outer=action)])
        unreal.EditorAssetLibrary.save_loaded_asset(action)
    # Keep existing mappings and add only the exact action/key pair requested.
    if not any(
        m.get_editor_property('action') == action and
        str(m.get_editor_property('key').get_editor_property('key_name')) == key
        for m in mappings
    ):
        entry = unreal.EnhancedActionKeyMapping()
        entry.set_editor_property('action', action)
        input_key = unreal.Key()
        input_key.set_editor_property('key_name', key)
        entry.set_editor_property('key', input_key)
        mappings.append(entry)

# The old click-to-move action must not also consume the attack mouse button.
mappings = [m for m in mappings if not (
    m.get_editor_property('action') and
    m.get_editor_property('action').get_name() == 'IA_SetDestination_Click' and
    str(m.get_editor_property('key').get_editor_property('key_name')) == 'LeftMouseButton')]
# A is jump. Remove any stale A binding from other actions so one press cannot
# trigger both jumping and combat.
mappings = [m for m in mappings if not (
    m.get_editor_property('action') and
    m.get_editor_property('action').get_name() != 'IA_Jump' and
    str(m.get_editor_property('key').get_editor_property('key_name')) == 'Gamepad_FaceButton_Bottom')]
data.set_editor_property('mappings', mappings)
context.set_editor_property('default_key_mappings', data)
assert unreal.EditorAssetLibrary.save_loaded_asset(context, only_if_is_dirty=False)
for m in mappings:
    unreal.log('COMBAT_MAPPING ' + m.get_editor_property('action').get_name() + ' ' + str(m.get_editor_property('key')))
unreal.log('COMBAT_INPUT_ASSETS_SAVED')
