#include "frameworkandsol.h"
#include "ReservedScriptNames.h"
#include "Lara/lara.h"
#include "ObjectsHandler.h"
#include "ReservedScriptNames.h"
#include "ObjectIDs.h"

/***
Scripts that will be run on game startup.
@tentable Objects 
@pragma nostrip
*/

ObjectsHandler::ObjectsHandler(sol::state* lua) : LuaHandler{ lua }
{
	/***
	Get an ItemInfo by its name.
	@function GetItemByName
	@tparam string name the unique name of the item as set in, or generated by, Tomb Editor
	@treturn ItemInfo a non-owning ItemInfo referencing the item.
	*/
	m_lua->set_function(ScriptReserved_GetItemByName, &ObjectsHandler::GetByName<GameScriptItemInfo, ScriptReserved_ItemInfo>, this);

	/***
	Get a MeshInfo by its name.
	@function GetMeshByName
	@tparam string name the unique name of the mesh as set in, or generated by, Tomb Editor
	@treturn MeshInfo a non-owning MeshInfo referencing the mesh.
	*/
	m_lua->set_function(ScriptReserved_GetMeshByName, &ObjectsHandler::GetByName<GameScriptMeshInfo, ScriptReserved_MeshInfo>, this);

	/***
	Get a CameraInfo by its name.
	@function GetCameraByName
	@tparam string name the unique name of the camera as set in, or generated by, Tomb Editor
	@treturn CameraInfo a non-owning CameraInfo referencing the camera.
	*/
	m_lua->set_function(ScriptReserved_GetCameraByName, &ObjectsHandler::GetByName<GameScriptCameraInfo, ScriptReserved_CameraInfo>, this);

	/***
	Get a SinkInfo by its name.
	@function GetSinkByName
	@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
	@treturn SinkInfo a non-owning SinkInfo referencing the sink.
	*/
	m_lua->set_function(ScriptReserved_GetSinkByName, &ObjectsHandler::GetByName<GameScriptSinkInfo, ScriptReserved_SinkInfo>, this);

	/***
	Get a SoundSourceInfo by its name.
	@function GetSoundSourceByName
	@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
	@treturn SoundSourceInfo a non-owning SoundSourceInfo referencing the sink.
	*/
	m_lua->set_function(ScriptReserved_GetSoundSourceByName, &ObjectsHandler::GetByName<GameScriptSoundSourceInfo, ScriptReserved_SoundSourceInfo>, this);

	GameScriptItemInfo::Register(m_lua);
	GameScriptItemInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptMeshInfo::Register(m_lua);
	GameScriptMeshInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptCameraInfo::Register(m_lua);
	GameScriptCameraInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptSinkInfo::Register(m_lua);
	GameScriptSinkInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptAIObject::Register(m_lua);
	GameScriptAIObject::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptSoundSourceInfo::Register(m_lua);
	GameScriptSoundSourceInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	MakeReadOnlyTable(ScriptReserved_ObjID, kObjIDs);
}

void ObjectsHandler::AssignLara()
{
	m_lua->set("Lara", GameScriptItemInfo(Lara.itemNumber, false));
}

