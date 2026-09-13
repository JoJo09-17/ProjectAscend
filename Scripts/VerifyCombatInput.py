"""Read back persisted Enhanced Input assets without modifying them."""
import unreal

context = unreal.load_asset('/Game/Input/IMC_Default')
mappings = context.get_editor_property('default_key_mappings').get_editor_property('mappings')
expected = [
    ('IA_LightAttack', 'LeftMouseButton'), ('IA_HeavyAttack', 'RightMouseButton'),
    ('IA_GamepadMove', 'Gamepad_Left2D'), ('IA_GamepadSwitchTarget', 'Gamepad_Right2D'),
    ('IA_GamepadLightAttack', 'Gamepad_LeftShoulder'),
    ('IA_GamepadHeavyAttack', 'Gamepad_RightTriggerAxis'),
    ('IA_GamepadDodge', 'Gamepad_FaceButton_Right'),
    ('IA_GamepadSkill1', 'Gamepad_FaceButton_Left'),
    ('IA_GamepadSkill2', 'Gamepad_FaceButton_Top'),
    ('IA_GamepadSkill3', 'Gamepad_LeftTriggerAxis'),
    ('IA_Jump', 'SpaceBar'),
    ('IA_Jump', 'Gamepad_FaceButton_Bottom'),
]
for name, key in expected:
    matches = [m for m in mappings if (
        m.get_editor_property('action').get_name() == name and
        str(m.get_editor_property('key').get_editor_property('key_name')) == key
    )]
    assert len(matches) == 1, name
    assert str(matches[0].get_editor_property('key').get_editor_property('key_name')) == key, name
    action = matches[0].get_editor_property('action')
    if name in ('IA_GamepadMove', 'IA_GamepadSwitchTarget'):
        assert action.get_editor_property('value_type') == unreal.InputActionValueType.AXIS2D
        assert len(action.get_editor_property('modifiers')) == 1
    unreal.log('INPUT_VERIFIED ' + name + ' = ' + key)
assert not any(
    m.get_editor_property('action').get_name() != 'IA_Jump' and
    str(m.get_editor_property('key').get_editor_property('key_name')) == 'Gamepad_FaceButton_Bottom'
    for m in mappings
)
unreal.log('COMBAT_INPUT_VERIFIED: 12 persisted mappings')
