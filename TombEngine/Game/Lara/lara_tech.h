#pragma once
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"

/*struct TrackedItem {
	ItemInfo* item;
	short lifetime;
};*/

extern std::vector<ItemInfo*> g_trackedItems;

void lara_tech_update();

void lara_tech_remove_tracker(ItemInfo* item);
void lara_tech_place_tracker(ItemInfo* item);