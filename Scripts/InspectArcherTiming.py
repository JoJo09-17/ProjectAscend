import unreal,json
from pathlib import Path
out={}
lib=getattr(unreal,'AnimationLibrary',getattr(unreal,'AnimationBlueprintLibrary',None))
for i in range(1,5):
 path='/Game/ArtAsset/Animations/Archer/Animation/Sequence/02_Attack/05_Combo_Attack_05/AS_Combo_Attack_05_'+str(i).zfill(2)+'_Seq'
 a=unreal.load_asset(path)
 row={'length':a.get_play_length(),'events':[]}
 if lib:
  for e in lib.get_animation_notify_events(a):
   row['events'].append(str(e))
 out[path]=row
Path(unreal.Paths.project_saved_dir(),'ArcherSourceTiming.json').write_text(json.dumps(out,indent=2))
unreal.log('ARCHER_SOURCE_TIMING_COMPLETE')
