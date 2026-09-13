"""Transient A/B tests: restore the sphere, selection, camera and active library."""
import unreal
import time

_env_library = unreal.load_asset('/Game/Materials/Toon/DA_ToonProfiles')
assert _env_library
assert unreal.SystemLibrary.get_console_variable_int_value('r.Substrate') == 0
_env_test_library = unreal.JoJoToonProfileLibrary()
_env_test_library.set_editor_property('profiles', _env_library.get_editor_property('profiles'))
_env_original_profiles = str(_env_library.get_editor_property('profiles'))
_env_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
_env_selected = _env_actors.get_selected_level_actors()
_env_spheres = [a for a in _env_actors.get_all_level_actors() if isinstance(a, unreal.StaticMeshActor) and a.static_mesh_component.static_mesh and a.static_mesh_component.static_mesh.get_path_name() == '/Engine/BasicShapes/Sphere.Sphere']
assert len(_env_spheres) == 1, str(_env_spheres)
_env_sphere = _env_spheres[0]
_env_component = _env_sphere.static_mesh_component
_env_material = _env_component.get_material(0)
_env_view = unreal.EditorLevelLibrary.get_level_viewport_camera_info()
_env_mid = unreal.MaterialLibrary.create_dynamic_material_instance(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(), unreal.load_asset('/Game/Materials/Toon/M_ToonSurface'), creation_flags=unreal.MIDCreationFlags.TRANSIENT)
assert _env_mid
_env_stage = 0
_env_next = time.monotonic() + 2
_env_cases = [
    ('ToonEnv_Metal_Off.png', 1, (0.7,0.7,0.7), 0, 0),
    ('ToonEnv_Metal_On.png', 1, (0.7,0.7,0.7), 1, .75),
    ('ToonEnv_Gold_On.png', 1, (0.83,0.55,0.15), 1, .75),
    ('ToonEnv_Indirect_Off.png', 0, (.45,.45,.45), 0, 0),
    ('ToonEnv_Indirect_On.png', 0, (.45,.45,.45), 0, .75),
]

def _env_restore():
    _env_component.set_material(0, _env_material)
    _env_library.apply_profiles()
    _env_actors.set_selected_level_actors(_env_selected)
    unreal.EditorLevelLibrary.set_level_viewport_camera_info(*_env_view)

def _env_tick(dt):
    global _env_stage, _env_next
    if time.monotonic() < _env_next:
        return
    stage = _env_stage
    _env_stage += 1
    _env_next = float('inf')
    try:
        if stage == 0:
            unreal.AutomationLibrary.finish_loading_before_screenshot()
            unreal.log('TOON_ENV_DEFAULTS ' + str(_env_library.get_editor_property('profiles')[0]))
            unreal.log('TOON_ENV_ORIGINAL_MATERIAL ' + _env_material.get_path_name())
            _env_actors.clear_actor_selection_set()
            center, extent = _env_sphere.get_actor_bounds(False)
            rotation = unreal.Rotator(-12, -130, 0)
            unreal.EditorLevelLibrary.set_level_viewport_camera_info(center - unreal.MathLibrary.get_forward_vector(rotation) * max(extent.x, extent.y, extent.z) * 5, rotation)
            _env_component.set_material(0, _env_mid)
        elif stage <= len(_env_cases) * 2:
            case = _env_cases[(stage - 1) // 2]
            if stage % 2 == 1:
                profiles = _env_test_library.get_editor_property('profiles')
                profile = profiles[0]
                profile.set_editor_property('reflection_stylization', case[3])
                profile.set_editor_property('indirect_stylization', case[4])
                profiles[0] = profile
                _env_test_library.set_editor_property('profiles', profiles)
                assert abs(_env_test_library.get_editor_property('profiles')[0].get_editor_property('reflection_stylization') - case[3]) < .001
                assert abs(_env_test_library.get_editor_property('profiles')[0].get_editor_property('indirect_stylization') - case[4]) < .001
                _env_test_library.apply_profiles()
                _env_mid.set_scalar_parameter_value('Metallic', case[1])
                _env_mid.set_scalar_parameter_value('Roughness', 0 if case[1] else .6)
                _env_mid.set_scalar_parameter_value('ToonProfile', 1)
                _env_mid.set_vector_parameter_value('BaseColor', unreal.LinearColor(*case[2], 1))
                unreal.log('TOON_ENV_CASE ' + str(case))
                unreal.log('TOON_ENV_READBACK ' + str(_env_test_library.get_editor_property('profiles')[0]))
                center, extent = _env_sphere.get_actor_bounds(False)
                unreal.EditorLevelLibrary.set_level_viewport_camera_info(center - unreal.MathLibrary.get_forward_vector(_env_view[1]) * max(extent.x, extent.y, extent.z) * 4, _env_view[1])
            else:
                unreal.AutomationLibrary.take_high_res_screenshot(1600, 900, case[0])
        else:
            _env_restore()
            unreal.unregister_slate_post_tick_callback(_env_handle)
            assert str(_env_library.get_editor_property('profiles')) == _env_original_profiles, 'Original profile asset changed during transient test'
            assert _env_component.get_material(0) == _env_material
            unreal.log('TOON_ENV_VERIFIED_AND_RESTORED')
    except Exception:
        _env_restore()
        unreal.unregister_slate_post_tick_callback(_env_handle)
        raise
    _env_next = time.monotonic() + 6

_env_handle = unreal.register_slate_post_tick_callback(_env_tick)
