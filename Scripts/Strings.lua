local strings =
{
	window_title = { "TombEngine" },
	all = { "All" },
	apply = { "Apply" },
	auto_target = { "Automatic Targeting" },
	back = { "Back" },
	binoculars = { "Binoculars" },
	close = { "Close" },
	cancel = { "Cancel" },
	caustics = { "Underwater Caustics" },
	choose_ammo = { "Choose Ammo" },
	choose_weapon = { "Choose Weapon" },
	combine = { "Combine" },
	combine_with = { "Combine With" },
	controls = { "Controls" },
	controls_action = { "Action" },
	controls_sprint = { "Sprint" },
	controls_draw_weapon = { "Draw Weapon" },
	controls_crouch = { "Crouch" },
	controls_inventory = { "Inventory" },
	controls_jump = { "Jump" },
	controls_look = { "Look" },
	controls_move_backward = { "Move Backward" },
	controls_move_forward = { "Move Forward" },
	controls_move_left = { "Move Left" },
	controls_move_right = { "Move Right" },
	controls_roll = { "Roll" },
	controls_step_left = { "Step Left" },
	controls_step_right = { "Step Right" },
	controls_pause = { "Pause" },
	controls_use_flare = { "Use Flare" },
	controls_walk = { "Walk" },
	controls_defaults = { "Reset to Defaults" },
	crossbow = { "Crossbow" },
	crossbow_normal_ammo = { "Crossbow Normal Ammo" },
	crossbow_poison_ammo = { "Crossbow Poison Ammo" },
	crossbow_explosive_ammo = { "Crossbow Explosive Ammo" },
	crossbow_lasersight = { "Crossbow + Lasersight" },
	crowbar = { "Crowbar" },
	diary = { "Diary" },
	disabled = { "Disabled" },
	display = { "Display Settings" },
	display_adapter = { "Display Adapter" },
	distance_travelled = { "Distance Travelled" },
	enable_sound = { "Enable Sounds" },
	enabled = { "Enabled" },
	equip = { "Equip" },
	examine = { "Examine" },
	exit_game = { "Exit Game" },
	exit_to_title = { "Exit to Title" },
	flares = { "Flares" },
	grenade_launcher = { "Grenade Launcher" },
	grenade_launcher_normal_ammo = { "Grenade Launcher Normal Ammo" },
	grenade_launcher_super_ammo = { "Grenade Launcher Super Ammo" },
	grenade_launcher_flash_ammo = { "Grenade Launcher Flash Ammo" },
	harpoon_gun = { "Harpoon Gun" },
	harpoon_gun_ammo = { "Harpoon Gun Ammo" },
	headset = { "Headset" },
	hk = { "HK" },
	hk_ammo = { "HK Ammo" },
	hk_burst_mode = { "Burst Mode" },
	hk_rapid_mode = { "Rapid Mode" },
	hk_with_silencer = { "HK With Silencer" },
	hk_sniper_mode = { "Sniper Mode" },
	lara_home = { "Lara's Home" },
	large_medipack = { "Large Medipack" },
	lasersight = { "Lasersight" },
	music_volume = { "Music Volume" },
	new_game = { "New Game" },
	none = { "None" },
	ok = { "OK" },
	options = { "Options" },
	other_settings = { "Sound and Gameplay" },
	output_settings = { "Output Settings" },
	passport = { "Passport" },
	pistol_ammo = { "Pistol Ammo" },
	pistols = { "Pistols" },
	player = { "Player" },
	render_options = { "Render Options" },
	reverb = { "Reverb" },
	revolver = { "Revolver" },
	revolver_ammo = { "Revolver Ammo" },
	revolver_laser = { "Revolver + Lasersight" },
	rocket_launcher_ammo = { "Rocket Launcher Ammo" },
	rocket_launcher = { "Rocket Launcher" },
	rumble = { "Rumble" },
	save_game = { "Save Game" },
	savegame_timestamp = { "%02d Days %02d:%02d:%02d" },
	screen_resolution = { "Screen Resolution" },
	secrets_found = { "Secrets Found" },
	select_level = { "Select Level" },
	separate = { "Separate" },
	sfx_volume = { "SFX Volume" },
	shadows = { "Shadows" },
	shotgun = { "Shotgun" },
	shotgun_normal_ammo = { "Shotgun Normal Ammo" },
	shotgun_wideshot_ammo = { "Shotgun Wideshot Ammo" },
	small_medipack = { "Small Medipack" },
	sound = { "Sound" },
	silencer = { "Silencer" },
	view = { "View" },
	thumbstick_camera = { "Thumbstick Camera" },
	time_taken = { "Time Taken" },
	statistics = { "Statistics" },
	torch = { "Torch" },
	empty = { "Empty" },
	use = { "Use" },
	ammo_used = { "Ammo Used" },
	used_medipacks = { "Medipacks Used" },
	uzi_ammo = { "Uzi Ammo" },
	uzis = { "Uzis" },
	volumetric_fog = { "Volumetric Fog" },
	antialiasing = { "Antialiasing" },
	low = { "Low" },
	medium = { "Medium" },
	high = { "High" },
	waiting_for_key = { "Waiting For Key" },
	windowed = { "Windowed" },
	load_game = { "Load Game" },
	test_level = { "Test Level" },
	waterskin_small_empty = { "Small Waterskin (Empty)" },
	waterskin_small_1l = { "Small Waterskin (1L)" },
	waterskin_small_2l = { "Small Waterskin (2L)" },
	waterskin_small_3l = { "Small Waterskin (3L)" },
	waterskin_large_empty = { "Large Waterskin (Empty)" },
	waterskin_large_1l = { "Large Waterskin (1L)" },
	waterskin_large_2l = { "Large Waterskin (2L)" },
	waterskin_large_3l = { "Large Waterskin (3L)" },
	waterskin_large_4l = { "Large Waterskin (4L)" },
	waterskin_large_5l = { "Large Waterskin (5L)" },
	torch2 = { "Torch 2" },
	mechanical_scarab = { "Mechanical Scarab With Key" },
	mechanical_scarab_1 = { "Mechanical Scarab" },
	mechanical_scarab_2 = { "Winding Key" },
	tut1_ba_cartouche = { "Ba Cartouche" },
	tut1_ba_cartouche_1 = { "Ba Cartouche Piece 1/2" },
	tut1_ba_cartouche_2 = { "Ba Cartouche Piece 2/2" },
	tut1_eye_horus = { "Eye of Horus" },
	tut1_eye_1 = { "Eye Piece 1/2" },
	tut1_eye_2 = { "Eye Piece 2/2" },
	tut1_hand_orion = { "The Hand of Orion" },
	tut1_hand_sirius = { "The Hand of Sirius" },
	GunCabinetKey = { "Gun Cabinet Key" },
	GoldRose = { "Gold Rose" },
	MachineCog = { "Machine Cog" },
	GoldIdol = { "Gold Idol" },
	SkullKey = { "Skull Key" },
	Scion = { "Scion" },
	NeptuneKey = { "Neptune Key" },
	AtlasKey = { "Atlas Key" },
	DamoclesKey = { "Damocles Key" },
	ThorKey = { "Thor Key" },
	gym = { "Lara's Home" },
	level1 = { "Caves" },
	level2 = { "City of Vilcabamba" },
	level3a = { "Lost Valley" },
	level3b = { "Tomb of Qualopec" },
	level4 = { "St. Francis Folly" },
	level5 = { "Colosseum" },
	level6 = { "Palace Midas" },
	level7a = { "The Cistern" },
	level7b = { "Tomb of Tihocan" },
	level8a = { "City of Khamoon" },
	level8b = { "Obelisk of Khamoon" },
	level8c = { "Sanctuary of the Scion" },
	level10a = { "Natla's Mines" },
	level10b = { "Atlantis" },
	level10c = { "The Great Pyramid" },
	title = { "Title" },
	accelerate = { "Accelerate" },
	reverse = { "Reverse" },
	speed = { "Speed" },
	slow = { "Slow" },
	brake = { "Brake" },
	fire = { "Fire" }
}

TEN.Flow.SetStrings(strings)

local languages =
{
	"English",
	"Italian",
	"German",
	"French",
	"Dutch",
	"Spanish",
	"Japanese",
	"Russian"
}

TEN.Flow.SetLanguageNames(languages)

