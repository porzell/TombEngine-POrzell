# Changelog

Here you will find the full changelog of TEN's releases from Version 1.0 and up 

The dates are in European standard format where date is presented as **YYYY-MM-DD**

TombEngine releases are located in this repository (alongside with Tomb Editor): https://github.com/TombEngine/TombEditorReleases

## Version 1.5 - xxxx-xx-xx

### Bug fixes
* Fixed original issue with classic switch off trigger wrongly activating some trigger actions.
* Fixed incorrect diving animation when swandiving from a high place.
* Fixed camera rotating with Lara's hips when climbing out of water.
* Fixed AI for skidoo driver and worker with shotgun TR2 enemies.
* Fixed Ember emitter crashing when ocb is between -1 and -10 
* Fixed Electric cleaner and Squishy block not detecting collision with certain block heights.
* Fixed Squishy blocks crashing the level.
* Fixed the path finding zones of Larson and Pierre.
* Fixed the torch flame delay in disappearing when Lara threw or dropped the torch object.

### Features/Amendments

### Lua API changes
* Added Inventory.GetUsedItem(), Inventory.SetUsedItem() and Inventory.ClearUsedItem() functions.

## [Version 1.4](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.7.1) - 2024-04-21

### Bug Fixes
* Fixed drawing of display sprites in title level.
* Fixed drawing of smoke sprites and various other sprites.
* Fixed drawing of transparent surfaces when debris are present in scene.
* Fixed player holster state and current vehicle not preserved correctly on level jump.
* Fixed diving when swimming over sinks.
* Fixed fire item effect not extinguishing in water.
* Fixed fade-in and fade-out effects not canceling correctly when next level is loaded.
* Fixed shadows still being visible after shattering a moveable.
* Fixed FOV interpolation at the end of the flyby sequence.
* Fixed sounds resuming in pause mode while switching between apps.
* Fixed slide directions.
* Fixed occasional collision warnings in a log when teeth spikes object was activated.
* Fixed climbable pushables collision during continuous pulling action.
* Fixed collision for solid static meshes with zero collision box.
* Fixed bottom collision for solid static meshes.
* Fixed T-Rex's head rotation.

### Features/Amendments
* Auto-switch to a crawl state if player start position is in a crawlspace.
* Allow directional flame emitter (negative OCBs) to be rotated at any angle.
* Revise wall spikes:
  - Wall spikes now stop when they touch a pushable, another spike wall or a normal wall.
  - Wall spikes will shatter any shatter in its path.
  - Wall spikes can be stopped by normal antitrigger or with a volume.
* Added hub system to preserve level state on level jumps.
* Added ember emitter.
* Added fish emitter.
* Added laser beam object.
* Added TR2 dragon.
* Added TR3 Winston (requires updated TEN .wad2 on TombEngine.com).
* Added TR4 squishy blocks (requires updated TEN .wad2 on TombEngine.com).

### Lua API changes
* Added resetHub flag to Flow.Level, which allows to reset hub data.
* Added Flow.GetFlipMapStatus() function to get current flipmap status.
* Added Moveable:GetMeshCount() function to get number of moveable meshes.
* Added timeout parameter for Moveable:Enable() function.
* Added Static:GetHP() and Static:SetHP() functions to change shatterable static mesh hit points.
* Fixed Moveable:SetOnCollidedWithObject() callback.

## [Version 1.3](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.7) - 2024-01-06

### Bug Fixes
* Fixed crash if title logo is removed from Textures folder.
* Fixed crash if unknown player state ID is encountered.
* Fixed bug with OCB 2 on pushables, and some other pushable bugs.
* Fixed pushable camera bug during edge slip.
* Fixed lever switch turn off alignment animation. 
* Fixed lack of water splash in certain scenarios.
* Fixed hydra flame not showing when charging.
* Fixed shockwave light for hammer god.
* Fixed camera shaking in some cases when the player is in quicksand room.
* Fixed certain flame emitter OCBs emitting fire in wrong directions.
* Fixed player not being able to pick up a torch when crouching.
* Fixed jittery camera when performing crawl-to-hang.
* Fixed several issues with limited pistol ammo.
* Fixed player not being able to crawl if two-handed weapon is currently equipped.
* Fixed playback issues with audio tracks placed in subfolders.
* Fixed thin caustics outline on the edge of the blocks.
* Fixed big static objects affected wrongly by dynamic lights.
* Fixed legacy trigger leveljumps ignoring provided level index.
* Fixed incorrect light collection in some cases.
* Fixed normal mapping for rooms, items, and statics.'

### Features/Amendments
* Added ambient occlusion (SSAO).
* Added new post-process workflow (monochrome, negative, exclusion) with tinting.
* Added SMAA antialiasing instead of MSAA.
* Added previously missing player start position object functionality.
* Added fast speed for fly cheat by holding Sprint input action.
* Added speedometer to vehicles.
* Added global node events.
* Totally revised transparency handling.
* Increased the maximum frames for animated sequences from 128 to 256.
* Optimized the renderer.
* Separate underwater wall and ceiling switch objects into two slots each.
* Accurately rotate display sprites around the pivot defined by the align mode.
* Allow walking on slopes when wading in water (similar to quicksand).
* Allow player to pull certain levers with both hands when holding a flare.
* Ported twin auto gun from TR3.
* Revised keyhole OCBs to account for keeping or losing keys:
  - OCB 0: Play default animation and lose key.
  - Positive OCB: Play anim number and keep key.
  - Negative OCB: Play anim number and lose key.
* Revised Wolf OCBs:
  - OCB 0: Wolf starts in walking animation, ready to chase Lara.
  - OCB 1: Wolf starts in sleeping animation.

### Lua API changes
* Added Lara:GetInteractedMoveable() which returns currently interacted moveable by Lara.
* Added Moveable:SetStatus() to set the current status of the moveable.
* Added Room:GetColor() to get room's ambient light color.
* Added Util.PickMoveableByDisplayPosition() and Util.PickStaticByDisplayPosition() functions.
* Added View.GetCameraPosition(), View.GetCameraTarget() and View.GetCameraRoom() functions.
* Added View.SetPostProcessMode(), View.SetPostProcessStrength() and View.SetPostProcessTint() functions.

## [Version 1.2](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.9) - 2023-11-11

### Bug Fixes
* Fix burning torch not working properly if there are more than 256 objects in a level.
* Fix grenade and rocket projectiles smoke offset in certain directions.
* Fix projectiles flying through animating objects.
* Fix harpoon gun doing enormous damage on enemies.
* Fix train death animation.
* Fix various camera issues with unique death animations.
* Fix zipline not following correct trajectory.
* Fix TR1 wolf damage value inflicted on a close bite attack.
* Fix TR1 bear various original AI issues.
* Fix TR2 knife thrower AI.
* Fix TR2 doberman crashing the game when killed by explosive weapons.
* Fix random crashes when killing exploding enemies.
* Fix random crashes in battle with more than 8 enemies.
* Fix volume change in settings not affecting voice track.
* Fix several lighting bugs.
* Fix double drawing additive faces.
* Fix savegame count not properly increasing.
* Fix regeneration of non-ammo pickups with OCB 128.
* Fix vault bug preventing the player from climbing onto ledges out of deeper sections of wade-depth water.
* Fix cold exposure status not recovering in non-cold wade-depth water.
* Fix non-elevated combat camera.
* Fix camera snap when disengaging the look-around mode.
* Fix TR4 mapper not being visible.

### Features/Amendments
* Improve head-on wall collision.
* Overhaul pushables:
  - Separate climbable and non-climbable pushable object slots.
  - Add new pushable OCB to manipulate pushable properties.
  - Add new animations for pushing pushables off edgees (TR1-3 and TR4-5 versions).
  - Fix pushables not working with raising blocks.
  - Fix miscellaneous pushable bugs.
* Overhaul look-around feature:
  - Allow for more consistent and wider viewing angles while crawling, crouching, and hanging.
  - Improve look camera movement and control.
  - Re-enable looking while performing up jump, backward jump, or backward crawl.
  - Add functionality to rotate slowly when holding Walk while using binoculars or lasersight.
* Add target highlighter system with toggle in Sound and Gameplay settings.
* Add sprint slide state 191.
* Add swinging blade.
* Add crumbling platform and add new OCBs for behaviour:
  - OCB 0: Default behaviour. When the player steps on the platform, it will shake and crumble after 1.2 seconds.
  - OCB > 0: When the player steps on the platform, it will crumble after the number of frames set in the OCB.
  - A positive value results in activation via player collision.
  - A negative value requires a trigger to activate.
* Add basic mouse input handling. Allows for binding of mouse inputs in control settings.
* Add settings for Mouse Sensitivity and Mouse Smoothing (not used in-game yet).

### Lua API changes
* Split and organize functions in `Misc` namespace to appropriate new namespaces.
* Make Vec2 and Vec3 objects float-based instead of integer-based.
* Add DisplaySprite object.
* Add Flow.EnableLoadSave() function to disable savegames.
* Add Flow.EnablePointFilter() function to disable bilinear filtering.
* Add View.GetAspectRatio() function to get the screen resolution's aspect ratio.
* Add Logic.HandleEvent() function to call node events.
* Add Input.GetCursorDisplayPosition() function to get the cursor's position.
* Add functions to load, save, delete and check existence of savegames.
* Add Lara:GetAmmoType() function to read the ammo that player is using.
* Add Moveable:GetEndFrame() function to get the end frame number of a moveable's current animation.
* Add extra parameter to GiveItem() function to optionally display an inventory item given to the player in the pickup summary.
* Add DisplayStringOption.RIGHT and DisplayStringOption.BLINK flags for DisplayString.
* Add log messages warnings to functions AddCallback and RemoveCallback.
* Add Vec2 and Vec3 arithmetic for division with a number and multiplication with another Vec2 or Vec3.
* Add various Vec2 and Vec3 operators and methods, such as Normalize() and Lerp().
* Fix InventoryItem constructor, now it will accept compound values in Item Action parameter.
* Fix Moveable constructor forcing initial animation to 0 and hit points to 10, even if not specified.
* Fix activation of a flipped room when using SetPos() script command or position change node.
* Fix Sound:PlaySoundEffect() function when used without optional position argument.
* Update DisplayString constructor to take a "scale" parameter, allowing for the resizing of text.
* Make DisplayString constructor's "color" parameter optional.
* Add DisplayString::SetScale() function to resize text.
* Add DisplayString::GetScale() function to get text scale.

## [Version 1.1.0](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.8) - 2023-07-29

### Bug Fixes
* Fix enemies shooting Lara through static meshes and moveables.
* Fix skeletons and mummies not being affected by explosive weapons.
* Fix crash on loading if static meshes with IDs above maximum are present.
* Fix various crashes specific to 64-bit build.
* Fix random crashes when playing audio tracks with names longer than 15 symbols.
* Fix crashes when trying to play .wav audio tracks on some Windows 11 systems.
* Fix last selected gun type not preserved after level jump.
* Fix incorrect vertical position after reloading a savegame made while standing on a bridge.
* Fix sprint value going below zero.
* Fix fog bulb density formula.
* Fix clockwork beetle activation crashing the game.
* Fix corrupted vehicle positions after savegame reload.
* Fix default ambience overlapping current one when loading a savegame.
* Fix doppelganger being limited to a single room.
* Fix bat AI, damage value, and incorrect collision after death.
* Fix regeneration for pickups with OCB 128.
* Fix raising blocks still shaking without OCB.
* Fix spiky ceiling, improve collision, and allow setting velocity via OCB.
* Fix TR1 winged mutant pathfinding and damage issues and add new OCBs.
* Fix TR1 Natla facing angle, bomb and shard projectiles, shooting anim in the second phase.
* Fix last inventory item position not being saved.
* Fix some puzzle hole objects crashing the game on item insertion.
* Fix incorrect harpoon bolt speed and angle when shooting vertically.
* Fix black shatter debris.
* Fix Lara's shadow projecting only her joints on some occasions.
* Fix sun and spot bulbs direction and sheen casts.
* Fix room collector freezing game on some occasions.
* Fix incorrect culling for scaled static meshes.
* Fix normal mapping.

### Features/Amendments
* Add ability to save screenshot in the "Screenshots" subfolder by pressing the "Print screen" key.
* Implement separate audio track channel for playing voiceovers with subtitles in .srt format.
* Don't stop ambience when Lara dies.
* Pause all sounds when entering inventory or pause menu.
* Preserve hit points on level jump.
* Improve deflection against slopes.
* Move and rotate Lara and activated pickups together with dynamic bridge objects.
* Reduce camera bounce.
* Improve spiky wall collision accuracy.
* Expand control settings page.
* Allow key bindings for previously hardcoded actions (weapon hotkeys, vehicle controls).
* Add input actions for weapon scroll.
* Add splash effect to rockets and grenades when they enter water.
* Allow multiple doppelgangers by using the same OCB for the origin nullmesh and doppelganger.
* Add TR1 skateboard kid.
* Add TR1 Kold.

### Lua API changes
* Add soundtrack functions:
  - Misc::GetAudioTrackLoudness() for getting current loudness of a given track type.
  - Misc::IsAudioTrackPlaying() for checking if a given track type is playing.
  - Misc::GetCurrentSubtitle() for getting current subtitle string for the voice track.

## [Version 1.0.9](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.7) - 2023-06-03

### Bug fixes
* Fix cold bar triggered in non-water rooms.
* Fix spiky wall speed value and change it via OCB number or Lua (Moveable::SetItemFlags[0]).
* Fix bats emitter crashing the game if little beetle object does not exist in wad.
* Fix gunflash rendering and position for entities.
* Fix snowmobile driver crashing the game.
* Fix knifethrower not throwing knife.
* Fix classic rollingball rolling in place into some closed doors.
* Fix zipline not working properly.
* Fix missing heavytrigger checks for node events activated from classic triggers.
* Fix death flag burning enemies underwater.
* Fix pickups and object collision not working properly in flipped rooms without portals.
* Fix footprints not being cleared after level change.
* Fix thumbstick camera option sometimes producing jerky camera movements during object interaction.
* Fix soundtrack position not restoring if same track is already playing.
* Fix inventory input interference when entering inventory via puzzle.
* Fix gamepad still vibrating if Lara was poisoned prior to death.
* Fix flare brightness.
* Fix grenade firing angle.
* Fix rendering for static meshes with custom blending modes and alpha transparency.
* Fix inconsistent multiline string spacing on different display modes.

### Features/Amendments
* Remove search object 4 hardcoded meshswap activated with a flipmap.
* Add TR1 cowboy.
* Add TR3 wall mounted blade.
* Add TR3 claw mutant.
* Add TR5 lasers:
	- Choose colour for the lasers via tint menu.
	- Laser OCB means width of the laser in sectors.
	- Negative OCB laser will trigger heavy trigger.
	- Positive OCB kills Lara.
* Add removable puzzles from puzzle holes and puzzle dones:
	- Employed by setting the trigger type as "Switch" for either puzzle hole or puzzle done.
	- Can be mixed with puzzle done and puzzle holes of the same or different type.
* Add reusable keys for key holes:
	- Employed by setting the trigger type as "Switch" for key hole.
* Allow key hole animation to be played via OCB number:
	- Default OCB 0 will play Lara use key animation. 
	- Any positive OCB number will play the animation according to the OCB number.
* Reimplement fog bulbs.
* Add missing gunflash for some entities, also include dynamic light and smoke to all gunflashes.
* Add ability to pick up a single inactive flare as an inventory item.
* Add log reports if title level or other levels don't exist.
* Add better error handling for missing font, sprites or shaders.
* Add "Reset to defaults" entry to controls menu and automatically bind XBOX gamepad profile if connected.
* Add 64-bit executable and place both 32-bit and 64-bit versions into /Bin subdirectory.

### Lua API changes
* Add Vec2 class.
* Add function String::SetTranslated().
* Add function Misc::IsStringDisplaying().
* Add the following for use in AddCallback and RemoveCallback:
	- PRESTART, POSTSTART
	- PREEND, POSTEND
	- PRESAVE, POSTSAVE
	- PRELOAD, POSTLOAD

## [Version 1.0.8](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.6) - 2023-04-10

### Bug fixes
* Fix bubbles phasing through ceilings.
* Fix object camera not clearing at level end.
* Fix double breath sound effect when coming up for air.
* Fix flickering hair.
* Fix harpoon gun triggering water and dry sounds when shooting and reholstering.
* Fix Z-fighting in inventory rendering.
* Fix transparent objects not displaying correctly in the Inventory.
* Fix dozy cheat always giving uzi weapons even if not present in WAD.
* Fix player getting launched when landing close to an edge.
* Fix player going through trapdoor/bridge while climbing up a climbable wall.
* Fix TR3 Sophia's charge ring drawing below floor.
* Fix TR5 imp collision handling and animations:
	- OCB 1: Climbs up to player when triggered.
	- OCB 2: Starts rolling on the floor when triggered.
	- OCB 3: Will throw stones at player.
	- Imp is also scared of of the player if holding a lit torch.
	- Please note you must use the patched version found here: https://github.com/TombEngine/Resources/blob/main/Wad2%20Objects/tr5_Imp.wad2
* Fix and improve wraith tails.

### Features/Amedments
* Add dedicated WRAITH_TRAP object with enhanced effects.
	- OCB 0: Effect disabled.
	- OCB 1: Effect enabled.
* Add TR1 slamming doors.
* Add TR3 mutant wasp (AI_MODIFY object won't allow it to land, the wasp will always fly).
* Add TR3 Corpse
	- OCB 0: used for coprses targeted by the compsognathus dinosaur. 
	- OCB 1: used for corpses hung in the air to be used as piranha bait. Will fall when shot.
	- Please note you must use the patched version found here: https://github.com/TombEngine/Resources/blob/main/Wad2%20Objects/tr3_Compsognathus_Cadavar.wad2
* Add cold exposure bar (employed by setting the "cold" flag in water rooms in Tomb Editor).
* Add water wakes for vehicles.
* Restored light effect nullmeshes (color, electrical, pulse, and strobe):
	- Select the light color as object tint in the OCB menu in Tomb Editor.
	- ELECTRICAL_LIGHT:
		- Can have multiple meshes. Add mesh number to OCB to be renderd with the light.
		- OCB + (mesh number): Light behaves like a neon light.
		- OCB – (mesh number): Light flickers.
* Restored inventory compass.
* Allow dynamic segment count for hair object.

### Lua API changes
* Add function Misc::IsSoundPlaying() 
* Add function DisplayString::SetFlags()

## [Version 1.0.7](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.5) - 2023-02-26

### Bug fixes
* Fix spark particles not being cleared on level reload.
* Fix visible but inactive enemies (e.g. Shiva or Xian guardians) taking damage.
* Fix blockable LOT type enemies (e.g. T-Rex and Shiva) not being able to step up 1 click or drop 2 clicks.
* Fix valve switch unable to untrigger objects.
* Fix valve and wall hole switches (OCB 5 and 6) not working as a timed switches.
* Fix incorrect Lara alignment on monkeyswing autojump.
* Fix silent crash if hair object or skin joints are missing.
* Fix holster meshes not displaying for Lara's shadow.
* Use Lara object's own meshes if Lara skin object does not exist in level file.
* Fix TR3 Shiva not taking explosive damage (should still block it with the sword).
* Fix TR3 Puna boss not blocking projectiles (grenade, rocket, harpoon, bolt) with the shield.
* Fix TR3 Puna boss and TR5 Roman statue taking fire when shot by explosive ammo.
* Fix TR2 sword and spear guardian:
	- Not doing transition from alive to stone and stone to alive.
	- Taking damage in stone mode.
	- Wrong joint index for rotating the head and torso.
	- Spear guardian not using left and right spear attack when walking (had wrong state ID set as target).
	- Killing move for spear used wrong value.
* Fix TR3 big gun spawning rocket with 0 life which caused an immediate explosion.
* Fix TR3 Tony and add boss effect for him.

### Features/Amendments
* Add TR3 civvy.
* Add TR3 electric cleaner.
* Add TR3 Sophia Leigh with following OCBs:	
	- 0 – Normal mode. Sophia behaves like a regular enemy.
	- 1 – Tower mode. Behaviour matched from Tomb Raider III.
	- 2 – Tower mode with volumes. Same as tower mode, but Sophia's ascent can be controlled using volumes in Tomb Editor.
* Add airlock switch object. Achieved by putting valve switch trigger and door trigger on the same sector (TR5 submarine door setup).
* Add OCB 7 for switches to be used with SetItemFlags Lua commands for customizing on/off animations and reachable distance.
* Add instant headshot to guards.
* Polish the distance position between Lara and switch objects.
* Adjust rocket ammo pickup from 10 to 1.
* Improve behaviour of tiger and lion enemies. 
* Implement more realistic water bubble effects.
* Implement a new stacked pickup display inspired by OpenLara.
* Prevent Lara from drawing weapons during parallel bar swinging.
* Further renderer performance optimizations and bugfixes.

### Lua API changes
* Fix Camera:SetPosition not updating camera position when it is played simultaneously.
* Add Moveable:GetAirborne and Moveable:SetAirborne.
* Add Moveable:GetLocationAI and Moveable:SetLocationAI.

## [Version 1.0.6](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.4) - 2023-01-29

### Bug FIxes
* Fix major pathfinding bug which could have caused lots of issues with enemy behaviour.
* Fix potential random crashes due to incorrect rendering behaviour.
* Fix savegame crash for disabled enemies with partially set activation mask.
* Fix certain enemies not damaging Lara if binoculars or lasersight mode is active.
* Fix invisible Lara after starting a new game from title flyby with hidden Lara.
* Fix backholster weapons not updating their sound position together with player.
* Fix black screen bug when there was an obstacle between the fixed camera and the target.
* Fix underwater caustics not appearing without visiting options menu beforehand.
* Fix TR1 ape climbing.
* Fix TR1 rat which crashed the game when it was killed.
* Fix TR2 small spider climbing and pathfinding.
* Fix TR3 Shiva and TR4 baddy 2 not blocking bullets.
* Fix TR4 harpy's sting attack which was neither hurting nor poisoning Lara.
* Fix TR4 SAS teleporting over the blocks he walks by.
* Fix TR4 seth blades that were doing a double activation when used an OCB different than 0.
* Fix TR4 skeleton spawn when used with OCB 3.
* Fix TR4 sphinx solving his bugged behaviour that happened if it received a lot of damage.
* Fix TR5 Roman statue and its meshswap.
* Fix TR5 twogun laser guard.
* Fix enemy projectile effect colours.
* Fix enemy shadow position.
* Fix sound positions not updated during flybys.
* Fix grenade launcher super ammo emitting too many fragments.
* Fix grenade and rocket launcher lighting.
* Fix ceiling trapdoor and floor trapdoor that Lara couldn't open manually.

### Features/Amendments
* Make enemies drop pickups at first available bounding box corner point, not centerpoint.
* Restore original volumetric explosion effects.
* Add TR3 lizard and Puna.
* Add TR3 boss effects in ID_BOSS_SHIELD and ID_BOSS_SHOCKWAVE_EXPLOSION slots.
* Add an option to activate Lua or node events from legacy triggers.
* Add more warnings in logs to enemies which animation or required slot is missing.
* Antitriggering an enemy will now cause it to vanish and pause.
* Re-triggering an enemy will cause it to reappear and unpause.
* Lua Moveable functions Enable and Disable now correctly trigger and antitrigger the moveable.
* Improve level loading speed a lot.

### Lua API changes
* Moveable:SetVisible has been added. MakeInvisible is now an alias for SetVisible(false).
* Moveable:MeshIsVisible is now GetMeshVisible.
* Moveable:SetMeshVisible has been added to replace ShowMesh/HideMesh.
* Moveable:MeshIsSwapped is now GetMeshSwapped
* Camera:SetPosition now updates the position if it's called while it's being played.
* Primitive Classes (Color, Rotation, Vec3) can now be saved via Levelvars and Gamevars variables.
* OnSave function now gets called just before data is saved, rather than just after.
* Add new function CameraObject::PlayCamera()
* Add new function Misc::GetCameraType()
* Add new functions Moveable:GetAirborne() and Moveable:SetAirborne(bool input)

## [Version 1.0.5](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.3) - 2022-12-30

### Bug fixes
* Fix combined items not existing in inventory upon game reload.
* Fix classic rollingball not behaving properly in rooms beyond the distance of 32 blocks.
* Fix rollingball not killing Lara under certain movement angles.
* Fix savegame crashes when rooms with static meshes are flipped.
* Fix discrepancies between statistics and save / load game time units.
* Fix draw key incorrectly working in binoculars / lasersight mode.
* Fix incorrect picking up from plinths concealed by raising blocks.
* Fix reversed grenade rotation.
* Fix flame emitter 3 not burning player.
* Fix TR2 yeti, TR3 civvy and MP with stick vault bugs.
* Fix TR2 worker with flamethrower and TR3 flamethrower baddy attack ranges and aiming issues.
* Fix TR3 tribeman with axe not attacking Lara.
* Fix TR3 tribeman with dart not shooting at Lara's direction.
* Fix TR3 crow and TR4 harpy death animations not performing correctly.
* Fix TR4 crocodile attack range and joint rotation speed.
* Fix TR4 Von Croy not using animation for 2 and 3 step down vaults.
* Fix TR4 baddies not using animation for 3 step down vaults.
* Fix TR5 laserhead guardian.
* Fix crash, water death and meshswap issues for TR5 cyborg.
* Fix pathfinding problems for first initialized enemy of every slot.
* Fix pathfinding of flying and water creatures (partial).
* Fix rare crash when smash item is inside a wall and add warning log for the scenario.
* Fix bone rotations of some entities.
* Fix Lara's animation for cog switch release.

### Features/Amendments
* Added new OCB to cog switch object: 
	- Use OCB 0 to have the traditional behaviour.
	- Use any other OCB to can use the Cog Switch without need of any door linked.
* Allow to freely rotate dart emitter in all directions.
* Customize dart emitter damage with OCB. Negative number will additionally poison Lara.
* Draw real mesh for darts.
* Added warning log when one slot requires another slot which is missing.

### Lua API changes
* Add new Room class and several methods for it.

## [Version 1.0.4](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.2) - 2022-12-16

### Features/Amendments
* Add generic assignable effects for moveables - fire, sparks, smoke and laser / electric ignite.
* Add ability to burn enemies with FLAME_EMITTER_1 and death blocks.
* Add wireframe mode and other visual debug information (switch by F10/F11 debug page scroll hotkeys).
* Activate Lara-bound volume triggers with mounted vehicles.
* Allow multiple individual activators for the same volume at the same time.
* Remove TRC remnant which added HK to inventory if pistols weren't available.
* Change default shatter sound to TR4_SMASH_ROCK (tomb4 default).
* Reduce idle pose time from 30 to 20 seconds.
* Automatically align pickups to floor surface.
* Minecart enhancements:
    - Add landing / jump sound.
    - Add falldamage.
    - Explode in water below 2 click surface.
    - Fix jump.
* Templar Knight enhancements:
    - Restored spark effects.
    - Can destroy statics in shatter slots.
	- Fix crash when attacking.
* SAS enhancements:
    - Fix grenade shooting.
    - Fix AI_MODIFY and AI_GUARD behaviour.
 
### Bug fixes
* Fix choppy camera movement in several cases.
* Fix Lara's vertical position when shimmying around steep slope corners.
* Fix legacy pickup triggers not working in certain cases.
* Fix crawl pickup not actually doing any pickups.
* Fix demigod and harpy shooting in incorrect directions.
* Fix particle effects for seth and harpy magic attacks.
* Fix lasersight always displaying with HK, revolver and crossbow.
* Fix rapid ammo spending in HK lasersight mode.
* Fix incorrect string IDs for item combine, HK and revolver with lasersight.
* Fix puzzle holes not swapping to puzzle done objects.
* Fix several collision and sound source issues in flipped rooms.
* Fix several pushable sound and object collision bugs.
* Fix original bug with incorrect climb up behaviour on ladders under sloped ceilings.
* Fix original bug with reassigned control keys still triggering default events.
* Fix TR1 centaur bubble targeting.
* Fix TR5 autogun rotation.
* Fix occasional wrong rollingball collision in narrow pits.
* Fix classic rollingball and big rollingball not behaving properly.
* Fix caustics not turning off in display settings.
* Fix windowed mode not using real resolution when DPI scaling is active.
* Fix control lock not working in flyby sequences.
* Fix empty inventory when using build and play feature in TE.
* Fix non-pickupable thrown flares.
* Fix throwing flare without drawing any weapons if there are no weapons present.
* Fix several incorrect FOV reset issues.
* Fix current soundtrack fading into silence if incoming one doesn't exist.
* Fix crash if there is an attempt to display a string with missing characters.

### Lua API changes
* Add new Volume class and several methods for it.
* Add new Moveable functions: GetEffect, SetEffect and SetCustomEffect (for colored fire).
* Add new Lara functions: GetTarget, GetVehicle and TorchIsLit.
* Remove Lara functions: SetOnFire and GetOnFire (replaced with GetEffect and SetEffect).
* Add Gameflow.lua options:
  - Flow.EnableMassPickup for mass pickup functionality.
  - Flow.EnableLaraInTitle for displaying Lara in title flyby.
  - Flow.EnableLevelSelect for title flyby level selection.
  - level.secrets for level-specific secret count.
  
* Fix level.ambientTrack property not working for title flyby.
* Fix action key script functions not fully working in some cases.
* Fix mounted vehicles ignoring Disable, Shatter and Explode script commands.
* Fix SetPosition command not updating room number correctly.
* Fix Rotation class using integers under the hood which prevented using fractional rotation values.
* Fix distance tests failing on a very high distances.

## [Version 1.0.3](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6.1) - 2022-11-18

### Features/Amendments
* Add ledge jumps (Lara object must be updated with new animations to make it work).
* Allow any object slot to be used as a meshswap.
* Add OCB 1 for rollingball to make it silent.
* Customize waterfall mist colour and OCB (XXYY, where XX is width, and YY is size).
* Implement HK shooting modes from TR5.
* Implement sprite instancing to speed up rendering.
* Enable dynamic lights for swarm enemies (beetles, rats and bats).
* Re-enable underwater caustics.
* Increase amount of maximum secrets per level from 8 to 32.
* Improve game and inventory input handling.
* Adjust sprint jump timing.
* Backport DAMOCLES_SWORD from TR1.

### Bug fixes
* Fix going into inventory and load/save dialogs during fade-ins and fade-outs.
* Fix savegames not preserving save number and game timer.
* Fix dodgy weapon lock angle constraints.
* Fix wrong shotgun ammo pickup amount.
* Fix shotgun using 6 units of ammo with each shot.
* Fix rocket explosions near statics.
* Fix explosive crossbow bolts not damaging player.
* Fix poisoned crossbow bolts not damaging enemies.
* Fix TR3 monkey level crash.
* Fix occasional ejections when landing on a slope.
* Fix occasional ejections when climbing up on a ledge under a slope.
* Fix pushables not being pushable on top of bridges and other pushables.
* Fix pushables having incorrect collision when bounding box is bigger than 1 sector.
* Fix grabbing narrow ledges below ceilings.
* Fix slow centaur projectile velocity.
* Fix search animations - allow chest and shelf animations to play properly.
* Fix sarcophagus and its item pickup.
* Fix underwater door and double doors continuing to be interactable after opening.
* Fix underwater door being interactable when underwater switch is on the same square.
* Fix ability to turn when aligning to an object while standing.
* Fix left arm lock while picking up an item with a flare in hand.
* Fix potential crashes when exiting game.
* Fix secret soundtrack (which filename number should be the last) not playing.
* Fix distance fog not applying properly to additive and subtractive surfaces.
* Fix swarm enemies and projectiles occasionally drawn using incorrect meshes.
* Fix single-hand weapons not having a sound on draw and undraw.
* Fix waterfall mist - it can now be disabled with antitrigger.
* Fix underwater lever pull animation playing after interacting with underwater ceiling switch.
* Fix SAS_DRAG_BLOKE object interaction.
* Fix KILLER_STATUE not triggering.

### Lua API changes
* A new class has been added, LaraObject, for Lara-specific functions. The built-in "Lara" variable now uses this class.
* Add functions for Lara object:
  - GetPoison / SetPoison
  - GetAir / SetAir
  - GetOnFire / SetOnFire
  - GetSprintEnergy / SetSprintEnergy
  - GetWet / SetWet
  - GetWeaponType / SetWeaponType
  - UndrawWeapon
  - GetAmmoCount
  - GetHandStatus
  - ThrowAwayTorch
  
* Add FlipMap and PlayFlyBy script commands and node functions.
* Add GetMoveablesBySlot and GetStaticsBySlot script commands to select groups of items.
* Add FlyCheat option to gameflow script for disabling dozy mode.
* Add SetTotalSecretCount option to gameflow script to set overall amount of secrets.
* Raised the maximum value on Moveable.SetHP to 32767 (its internal numeric maximum).

## [Version 1.0.2](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.6) - 2022-09-16

### Features/Amendments
* Fix removing Pistols with TakeItem and SetItemCount.
* Allow saving and loading of Vec3s in LevelVars and GameVars.
* Support volume triggers made with node editor.
* Adjust max turn rate of idle state.
* Align Lara on slopes when crouching, crawling, and dying.
* Better slope alignment for large, flat enemies (i.e. big scorpion and crocodile).
* Lock turn rate when automatically aligning Lara to objects.
* Don't play Lara alignment animations if the interacted object is too close.
* Allow vertical pole mounts only when facing one directly.
* Allow vertical pole mounts while turning, walking, or running.
* Update monkey swing 180 turn animation.
* Update backward monkey swing animations.
* Added standing 180 turn. Activated with WALK+ROLL on dry ground, automatic when in wade-height water or swamp.
* Added crouch turn and crawl turn animations.
* Added new switch OCBs:
  - 0 for wall switch
  - 1 for small wall switch
  - 2 for small button
  - 3 for big button
  - 4 for giant button (sequence switch)
  - 5 for valve turn
  - 6 for hole switch
  - any other OCBs play corresponding switch on anim or OCB+1 switch off anim.

### Bug fixes
* Fix incorrect pole mounting.
* Fix zeroed forward velocity upon landing.
* Fix incorrect behaviour when falling on statics from the top after monkeyswing.
* Fix missing animcommand calls on first animation frame.
* Fix 1-frame turn rate delays.
* Fix occasional leave event calls when moving closer to volumes.
* Fix incorrect viewport size in windowed mode.
* Fix late landing animation dispatch in rare cases.
* Fix incorrect velocity calculations for death animations.
* Fix horseman's axe attack using his left foot as the damaging joint.
* Fix stargate blades needlessly pushing the player around while hardly doing any damage.
* Fix weapon hotkeys and add missing crossbow hotkey.

### Lua API changes
* Util.ShortenTENCalls no longer needs to be called; it is now automatic for both level scripts and Gameflow.lua.
* Flow.InvID has been removed; any function taking a pickup (e.g. GiveItem) now takes an Objects.ObjID instead.
* Add Enable, Disable, GetActive, Get/SetSolid functions for static meshes.
* Add FadeOutComplete, StopAudioTrack and StopAudioTracks functions.
* Account for objects in HasLineOfSight tests.
* Move Timer.lua, EventSequence.lua and Util.lua to a subfolder named "Engine".
* LevelFuncs can now contain tables as well as functions. These tables can contain functions and other tables, and so forth.
* Moveable functions SetOnHit, SetOnKilled, SetOnCollidedWithObject and SetOnCollidedWithRoom no longer take strings, and instead take function objects themselves.
* Don't require EventSequence and Timer to call Timer.UpdateAll in OnControlPhase.
* Add TEN.Logic.AddCallback and TEN.Logic.RemoveCallback.
* Rework GiveItem, TakeItem, and SetItemCount (e.g. SetItemCount with a value of -1 can give infinite ammo/consumables).


## [Version 1.0.1](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.5.2) - 2022-08-16

### Features
* Added antialiasing support.
* Added static mesh scaling support.
* Added free rotation for teeth spikes instead of using OCB codes.

### Bug fixes
* Fix some issues with shimmying between diagonal ledges and walls.
* Fix rope transparency.
* Fix objects disappearing under certain angles at the edges of the screen.
* Fix incorrect polerope and jumpswitch grabbing.
* Fix camera behaviour with pushable blocks.
* Fix minecart unduck on inclines.
* Fix quadbike dismount with jump key and allow to shoot big gun with action key.
* Fix static meshes having wrong colors on savegame reload.
* Fix rollingball incorrectly killing Lara in water and in jump.
* Fix resurfacing on underwater death.
* Fix water to ladder animation not activating in all cases.
* Fix ripples not appearing on water connections higher than room bottom.
* Fix several problems with ropes (stumbling, rope length, etc).
* Fix several problems with teeth spikes.
* Fix falling through twoblock platform on room number change.
* Fix falling block breaking too early if placed on a vertical portal.
* Fix crashes when loading image files are missing.

### Amendments
* Disable trigger check for puzzle holes.
* Clear locusts and other swarm enemies on level reload.
* Enhance cobra AI and fix targeting.
* Fully decompile HAMMER object from TR4.
* Prevent title music audio from starting in a random place.
* Update harpoon speed on room change.
* Enable second sky layer rendering.
* Preserve inventory and flare on level jumps.
* Timer.Create now lets you choose the units to display remaining time.
* Fatal script errors now boot you to the title (it will crash if the title itself has these errors).
* SetFarView has been removed, and Flow.Level.farView is now uncapped.
* DisplayString text will now be cleared when a level is exited or reloaded.
* EventSequence.lua has been added and documented.


## [Version 1.0](https://github.com/TombEngine/TombEditorReleases/releases/tag/v1.5.1) - 2022-08-06

First beta release.