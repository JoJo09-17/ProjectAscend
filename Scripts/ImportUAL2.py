"""Import the CC0 UAL2 Standard GLBs into isolated project content folders."""
import json
import struct
from pathlib import Path
import unreal

PROJECT = Path(unreal.Paths.project_dir()).resolve()
SOURCE = PROJECT / 'ThirdPartyAssets/Quaternius/UniversalAnimationLibrary2/Universal Animation Library 2[Standard]/Unreal-Godot'
DEST = '/Game/ThirdParty/Quaternius/UAL2'
REPORT = PROJECT / 'Saved/UAL2ImportReport.json'


def source_names(path):
    with path.open('rb') as stream:
        assert stream.read(4) == b'glTF'
        stream.read(8)
        size, kind = struct.unpack('<II', stream.read(8))
        assert kind == 0x4E4F534A
        return [a['name'] for a in json.loads(stream.read(size))['animations']]


def import_variant(filename, folder, skeleton=None):
    source = SOURCE / filename
    destination = DEST + '/' + folder
    assert not unreal.EditorAssetLibrary.list_assets(destination, recursive=True), 'Destination must be empty: ' + destination
    pipeline = unreal.InterchangeGenericAssetsPipeline()
    pipeline.set_editor_property('asset_type_sub_folders', True)
    pipeline.animation_pipeline.set_editor_property('import_animations', True)
    pipeline.animation_pipeline.set_editor_property('use30_hz_to_bake_bone_animation', True)
    if skeleton:
        settings = pipeline.common_skeletal_meshes_and_animations_properties
        settings.set_editor_property('skeleton', skeleton)
        settings.set_editor_property('import_only_animations', True)
    stack = unreal.InterchangePipelineStackOverride()
    stack.add_pipeline(pipeline)
    task = unreal.AssetImportTask()
    task.set_editor_property('filename', str(source))
    task.set_editor_property('destination_path', destination)
    task.set_editor_property('automated', True)
    task.set_editor_property('replace_existing', False)
    task.set_editor_property('save', True)
    task.set_editor_property('options', stack)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    assets = [unreal.load_asset(p) for p in unreal.EditorAssetLibrary.list_assets(destination, recursive=True)]
    animations = [a for a in assets if isinstance(a, unreal.AnimSequence)]
    assert len(animations) == len(source_names(source)), (filename, len(animations), len(source_names(source)))
    entries = []
    for anim in animations:
        assert anim.get_editor_property('sequence_length') > 0, anim.get_path_name()
        assert anim.get_editor_property('skeleton'), anim.get_path_name()
        if folder == 'RootMotion':
            anim.set_editor_property('enable_root_motion', True)
        assert unreal.EditorAssetLibrary.save_loaded_asset(anim, only_if_is_dirty=False)
        entries.append({'path': anim.get_path_name(), 'seconds': anim.get_editor_property('sequence_length'), 'skeleton': anim.get_editor_property('skeleton').get_path_name(), 'root_motion': anim.get_editor_property('enable_root_motion')})
    assert unreal.EditorAssetLibrary.save_directory(destination, only_if_is_dirty=False, recursive=True)
    return animations[0].get_editor_property('skeleton'), {'source': str(source), 'source_animations': source_names(source), 'animations': entries, 'assets': [a.get_path_name() for a in assets]}


try:
    skeleton, standard = import_variant('UAL2_Standard.glb', 'InPlace')
    _, root_motion = import_variant('UAL2_Standard_RM.glb', 'RootMotion', skeleton)
    REPORT.write_text(json.dumps({'status': 'complete', 'in_place': standard, 'root_motion': root_motion}, indent=2), encoding='utf-8')
    unreal.EditorAssetLibrary.sync_browser_to_objects([skeleton.get_path_name()])
    unreal.log('UAL2_IMPORT_COMPLETE: ' + str(REPORT))
except Exception:
    import traceback
    REPORT.write_text(json.dumps({'status': 'failed', 'traceback': traceback.format_exc()}, indent=2), encoding='utf-8')
    raise
