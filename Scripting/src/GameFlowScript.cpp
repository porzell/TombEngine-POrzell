#include "frameworkandsol.h"
#include "GameFlowScript.h"
#include "Sound/sound.h"
#include "Game/savegame.h"
#include "GameScriptInventoryObject.h"
#include "InventorySlots.h"
#include "Game/gui.h"

/***
Scripts that will be run on game startup.
@files Pre-game
@pragma nostrip
*/

using std::string;
using std::vector;
using std::unordered_map;

ScriptInterfaceFlow* g_GameFlow;
ScriptInterfaceGame* g_GameScript;
ScriptInterfaceEntity* g_GameScriptEntities;

GameFlow::GameFlow(sol::state* lua) : LuaHandler{ lua }
{
	GameScriptLevel::Register(m_lua);
	GameScriptSkyLayer::Register(m_lua);
	GameScriptFog::Register(m_lua);
	GameScriptMirror::Register(m_lua);
	GameScriptInventoryObject::Register(m_lua);
	GameScriptSettings::Register(m_lua);
	GameScriptAnimations::Register(m_lua);
	GameScriptAudioTrack::Register(m_lua);
	GameScriptColor::Register(m_lua);
	GameScriptRotation::Register(m_lua);
/*** gameflow.lua.
These functions are called in gameflow.lua, a file loosely equivalent to winroomedit's SCRIPT.DAT.
They handle a game's 'metadata'; i.e., things such as level titles, loading screen paths, and default
ambient tracks.
@section gameflowlua
*/

/***
Add a level to the gameflow.
@function AddLevel
@tparam Level level a level object
*/
	m_lua->set_function("AddLevel", &GameFlow::AddLevel, this);

/*** Image to show when loading the game.
Must be a .jpg or .png image.
@function SetIntroImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	m_lua->set_function("SetIntroImagePath", &GameFlow::SetIntroImagePath, this);

/*** Image to show in the background of the title screen.
Must be a .jpg or .png image.
__(not yet implemented)__
@function SetTitleScreenImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	m_lua->set_function("SetTitleScreenImagePath", &GameFlow::SetTitleScreenImagePath, this);

/*** Maximum draw distance.
The maximum draw distance, in sectors (blocks), of any level in the game.
This is equivalent to TRNG's WorldFarView variable.
__(not yet implemented)__
@function SetGameFarView
@tparam byte farview Number of sectors. Must be in the range [1, 127].
*/
	m_lua->set_function("SetGameFarView", &GameFlow::SetGameFarView, this);

/*** settings.lua.
These functions are called in settings.lua, a file which holds your local settings.
settings.lua shouldn't be bundled with any finished levels/games.
@section settingslua
*/
/***
@function SetSettings
@tparam Settings settings a settings object 
*/
	m_lua->set_function("SetSettings", &GameFlow::SetSettings, this);
/***
@function SetSettings
@tparam Settings settings a settings object 
*/
	m_lua->set_function("SetAnimations", &GameFlow::SetAnimations, this);

/*** strings.lua. 
These functions used in strings.lua, which is generated by TombIDE.
You will not need to call them manually.
@section stringslua
*/
/*** Set string variable keys and their translations.
@function SetStrings
@tparam tab table array-style table with strings
*/
	m_lua->set_function("SetStrings", &GameFlow::SetStrings, this);

/*** Set language names for translations.
Specify which translations in the strings table correspond to which languages.
@function SetLanguageNames
@tparam tab table array-style table with language names
*/
	m_lua->set_function("SetLanguageNames", &GameFlow::SetLanguageNames, this);


	MakeReadOnlyTable("WeatherType", kWeatherTypes);
	MakeReadOnlyTable("LaraType", kLaraTypes);
	MakeReadOnlyTable("InvItem", kInventorySlots);
	MakeReadOnlyTable("RotationAxis", kRotAxes);
	MakeReadOnlyTable("ItemAction", kItemActions);
	MakeReadOnlyTable("ErrorMode", kErrorModes);
}

GameFlow::~GameFlow()
{
	for (auto& lev : Levels)
	{
		delete lev;
	}
}

void GameFlow::SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src)
{
	m_languageNames = std::move(src);
}

void GameFlow::SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src)
{
	m_translationsMap = std::move(src);
}

void GameFlow::SetSettings(GameScriptSettings const & src)
{
	m_settings = src;
}

void GameFlow::SetAnimations(GameScriptAnimations const& src)
{
	Animations = src;
}

void GameFlow::AddLevel(GameScriptLevel const& level)
{
	Levels.push_back(new GameScriptLevel{ level });
}

void GameFlow::SetIntroImagePath(std::string const& path)
{
	IntroImagePath = path;
}

void GameFlow::SetTitleScreenImagePath(std::string const& path)
{
	TitleScreenImagePath = path;
}

void GameFlow::SetGameFarView(byte val)
{
	bool cond = val <= 127 && val >= 1;
	std::string msg{ "Game far view value must be in the range [1, 127]." };
	if (!ScriptAssert(cond, msg))
	{
		ScriptWarn("Setting game far view to 32.");
		GameFarView = 32;
	}
	else
	{
		GameFarView = val;
	}
}

void GameFlow::LoadGameFlowScript()
{
	ExecuteScript("Scripts/Enums.lua");
	ExecuteScript("Scripts/Gameflow.lua");
	ExecuteScript("Scripts/Strings.lua");
	ExecuteScript("Scripts/Settings.lua");

	SetScriptErrorMode(GetSettings()->ErrorMode);
}

char const * GameFlow::GetString(const char* id) const
{
	if (!ScriptAssert(m_translationsMap.find(id) != m_translationsMap.end(), std::string{ "Couldn't find string " } + id))
	{
		return "String not found";
	}
	else
		return m_translationsMap.at(string(id)).at(0).c_str();
}

GameScriptSettings* GameFlow::GetSettings()
{
	return &m_settings;
}

GameScriptLevel* GameFlow::GetLevel(int id)
{
	return Levels[id];
}

int	GameFlow::GetNumLevels() const
{
	return Levels.size();
}

bool GameFlow::IsFlyCheatEnabled() const
{
	return FlyCheat;
}

bool GameFlow::DoGameflow()
{
	// We start with the title level
	CurrentLevel = 0;
	SelectedLevelForNewGame = 0;
	SelectedSaveGame = 0;
	SaveGameHeader header;

	// We loop indefinitely, looking for return values of DoTitle or DoLevel
	bool loadFromSavegame = false;

	while (true)
	{
		// First we need to fill some legacy variables in PCTomb5.exe
		GameScriptLevel* level = Levels[CurrentLevel];

		GAME_STATUS status;

		if (CurrentLevel == 0)
		{
			status = DoTitle(0, level->AmbientTrack);
		}
		else
		{
			// Prepare inventory objects table
			for (size_t i = 0; i < level->InventoryObjects.size(); i++)
			{
				GameScriptInventoryObject* obj = &level->InventoryObjects[i];
				if (obj->slot >= 0 && obj->slot < INVENTORY_TABLE_SIZE)
				{
					InventoryObject* invObj = &inventry_objects_list[obj->slot];

					invObj->objname = obj->name.c_str();
					invObj->scale1 = obj->scale;
					invObj->yoff = obj->yOffset;
					invObj->xrot = FROM_DEGREES(obj->rot.x);
					invObj->yrot = FROM_DEGREES(obj->rot.y);
					invObj->zrot = FROM_DEGREES(obj->rot.z);
					invObj->meshbits = obj->meshBits;
					invObj->opts = obj->action;
					invObj->rot_flags = obj->rotationFlags;
				}
			}

			status = DoLevel(CurrentLevel, level->AmbientTrack, loadFromSavegame);
			loadFromSavegame = false;
		}

		switch (status)
		{
		case GAME_STATUS::GAME_STATUS_EXIT_GAME:
			return true;
		case GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE:
			CurrentLevel = 0;
			break;
		case GAME_STATUS::GAME_STATUS_NEW_GAME:
			CurrentLevel = (SelectedLevelForNewGame != 0 ? SelectedLevelForNewGame : 1);
			SelectedLevelForNewGame = 0;
			InitialiseGame = true;
			break;
		case GAME_STATUS::GAME_STATUS_LOAD_GAME:
			// Load the header of the savegame for getting the level to load
			SaveGame::LoadHeader(SelectedSaveGame, &header);

			// Load level
			CurrentLevel = header.Level;
			loadFromSavegame = true;

			break;
		case GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED:
			if (LevelComplete == Levels.size())
			{
				// TODO: final credits
			}
			else
				CurrentLevel++;
			break;
		}
	}

	return true;
}

bool GameFlow::CanPlayAnyLevel() const
{
	return PlayAnyLevel;
}
