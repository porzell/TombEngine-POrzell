#pragma once
#include <string>
#include <functional>
#include "Specific/level.h"

typedef DWORD D3DCOLOR;
using VarMapVal = std::variant< short,
	std::reference_wrapper<MESH_INFO>,
	std::reference_wrapper<LEVEL_CAMERA_INFO>,
	std::reference_wrapper<SINK_INFO>,
	std::reference_wrapper<SOUND_SOURCE_INFO>,
	std::reference_wrapper<AI_OBJECT>>;

using CallbackDrawString = std::function<void(std::string const&, D3DCOLOR, int, int, int)>;

class ScriptInterfaceEntity {
public:
	virtual ~ScriptInterfaceEntity() = default;

	virtual bool AddName(std::string const& key, VarMapVal val) = 0;
	virtual void FreeEntities() = 0;
	virtual void AssignLara() = 0;
};

extern ScriptInterfaceEntity* g_GameScriptEntities;
