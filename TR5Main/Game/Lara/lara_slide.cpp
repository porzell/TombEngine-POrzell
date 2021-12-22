#include "framework.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/input.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/camera.h"
#include "Specific/level.h"
#include "Game/items.h"

// -----------------------------
// SLIDE
// Control & Collision Functions
// -----------------------------

// State:		LS_SLIDE_FORWARD (24)
// Collision:	lara_col_slide_forward()
void lara_as_slide_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	Camera.targetElevation = -ANGLE(45.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH; //
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		// Not yet.
		//if (TrInput & IN_LEFT/* &&
		//	g_GameFlow->Animations.SlideExtended*/)
		//{
		//	info->turnRate -= LARA_TURN_RATE;
		//	if (info->turnRate < -LARA_SLIDE_TURN_MAX)
		//		info->turnRate = -LARA_SLIDE_TURN_MAX;

		//	DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
		//}
		//else if (TrInput & IN_RIGHT/* &&
		//	g_GameFlow->Animations.SlideExtended*/)
		//{
		//	info->turnRate += LARA_TURN_RATE;
		//	if (info->turnRate > LARA_SLIDE_TURN_MAX)
		//		info->turnRate = LARA_SLIDE_TURN_MAX;

		//	DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
		//}

		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = LS_JUMP_FORWARD;
			StopSoundEffect(SFX_TR4_LARA_SLIPPING);
			return;
		}

		item->goalAnimState = LS_SLIDE_FORWARD;
		return;
	}

	if (TrInput & IN_FORWARD)
		item->goalAnimState = LS_RUN_FORWARD;
	else
		item->goalAnimState = LS_IDLE;

	StopSoundEffect(SFX_TR4_LARA_SLIPPING);
	return;
}

// State:		LS_SLIDE_FORWARD (24)
// Control:		lara_as_slide_forward()
void lara_col_slide_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -CLICK(2);
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor >= CLICK(1))
	{
		SetLaraFallState(item);
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
		return;
	}

	if (TestLaraSlide(item, coll))
		SetLaraSlideState(item, coll);

	LaraDeflectEdge(item, coll);

	if (TestLaraStep(coll))
	{
		LaraSnapToHeight(item, coll);
		//DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SLIDE_BACK (32)
// Collision:	lara_col_slide_back()
void lara_as_slide_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	Camera.targetElevation = -ANGLE(45.0f);
	Camera.targetAngle = ANGLE(135.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH; //
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		// Not yet.
		//if (TrInput & IN_LEFT/* &&
		//	g_GameFlow->Animations.SlideExtended*/)
		//{
		//	info->turnRate -= LARA_TURN_RATE;
		//	if (info->turnRate < -LARA_SLIDE_TURN_MAX)
		//		info->turnRate = -LARA_SLIDE_TURN_MAX;

		//	DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
		//}
		//else if (TrInput & IN_RIGHT/* &&
		//	g_GameFlow->Animations.SlideExtended*/)
		//{
		//	info->turnRate += LARA_TURN_RATE;
		//	if (info->turnRate > LARA_SLIDE_TURN_MAX)
		//		info->turnRate = LARA_SLIDE_TURN_MAX;

		//	DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
		//}

		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = LS_JUMP_BACK;
			StopSoundEffect(SFX_TR4_LARA_SLIPPING);
			return;
		}

		item->goalAnimState = LS_SLIDE_BACK;
		return;
	}

	item->goalAnimState = LS_IDLE;
	StopSoundEffect(SFX_TR4_LARA_SLIPPING);
	return;
}

// State:		LS_SLIDE_BACK (32)
// Control:		lara_as_slide_back()
void lara_col_slide_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -CLICK(2);
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor >= CLICK(1))
	{
		SetLaraFallBackState(item);
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
		return;
	}

	if (TestLaraSlide(item, coll))
		SetLaraSlideState(item, coll);

	LaraDeflectEdge(item, coll);

	if (TestLaraStep(coll))
	{
		LaraSnapToHeight(item, coll);
		//DoLaraStep(item, coll);
		return;
	}
}
