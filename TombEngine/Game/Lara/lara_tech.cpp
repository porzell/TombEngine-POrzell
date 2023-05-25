#include "framework.h"
#include "lara_tech.h"
#include "Specific/clock.h"
#include "Renderer/Renderer11.h"
#include "lara.h"
#include "Game/misc.h"

using TEN::Renderer::g_Renderer;

auto constexpr TRACKED_ITEM_LIFETIME = 15 * FPS;

std::vector<ItemInfo*> g_trackedItems;

void lara_tech_update() {
}

void lara_tech_remove_tracker(ItemInfo* item) {
	bool removed = false;
	for (auto it = g_trackedItems.begin(); it != g_trackedItems.end(); ) {
		if ((*it) == item) {
			it = g_trackedItems.erase(it);
			removed = true;
		} else
			++it;
	}

	if (removed)
		SoundEffect(SFX_TR5_KEYPAD_ENTRY_NO, &LaraItem->Pose);
}

void lara_tech_place_tracker(ItemInfo* item) {
	
	if (item->IsCreature()) {
		auto *creature = GetCreatureInfo(item);
		
		bool dont_add = false;
		if (creature->Tracked)
			dont_add = true;
		
		creature->Tracked = TRACKED_ITEM_LIFETIME;
		
		if (dont_add)
			return;
	}
	//g_trackedItems.push_back(TrackedItem{ LaraItem, 300 });
	TENLog("[Tracker Darts] Added item " + item->Name + " to tracked items.");
	g_trackedItems.push_back(item);
	SoundEffect(SFX_TR5_KEYPAD_ENTRY_YES, &LaraItem->Pose);
}