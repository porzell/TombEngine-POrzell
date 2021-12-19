#pragma once

#include "control/box.h"

#define CHK_ANY(var, flag) (var & flag) != 0
#define CHK_EXI(var, flag) var & flag
#define CHK_NOP(var, flag) !(var & flag)

enum LARA_MESH_MASK
{
	LARA_ONLY_LEGS = (0 << 1),
	LARA_ONLY_ARMS = (1 << 1),
	LARA_ONLY_TORSO = (2 << 1),
	LARA_ONLY_HEAD = (4 << 1),
	LARA_ONLY_LEFT_ARM = (8 << 1),
	LARA_ONLY_RIGHT_ARM = (16 << 1),
	LARA_LEGS_TORSO_HEAD = (32 << 1),
	LARA_LEGS_TORSO_HEAD_ARMS = (64 << 1)
};

CREATURE_INFO* GetCreatureInfo(ITEM_INFO* item);
void TargetNearestEntity(ITEM_INFO* item, CREATURE_INFO* creature);