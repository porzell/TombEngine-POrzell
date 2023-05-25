#include "framework.h"
#include "Objects/TR4/Entity/tr4_dog.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::TR4
{
	template<typename ... Args>
	std::string string_format(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	constexpr auto DOG_BITE_ATTACK_DAMAGE = 10;
	constexpr auto DOG_JUMP_ATTACK_DAMAGE = 20;

	constexpr auto DOG_BITE_ATTACK_RANGE = SQUARE(BLOCK(0.55));
	constexpr auto DOG_JUMP_ATTACK_RANGE = SQUARE(BLOCK(1));

	// Peter: Dog will follow these objects if spawned nearby.
	constexpr GAME_OBJECT_ID DOG_MASTER_ITEMIDS[] = { ID_GUARD1, ID_GUARD2, ID_GUARD3 };
	// Peter: Dog must be placed this close to master for it to start following them.
	constexpr auto DOG_MASTER_ASSIGN_DISTANCE = BLOCK(4);
	// Peter: Have dog follow at most 2 squares behind master.
	constexpr auto DOG_FOLLOW_DISTANCE = SQUARE(BLOCK(2));
	// Peter: Use item flags to store index of dog master item.
	constexpr auto DOGMASTER_FLAG = 1;
	// Peter: Item flag for no master.
	constexpr auto DOG_NO_MASTER = 0;
	
	const auto DogBite = BiteInfo(Vector3(0.0f, 0.0f, 100.0f), 3.0f);
	const auto DogJumpAttackJoints = std::vector<unsigned int>{ 3, 6, 9, 10, 13, 14 };
	const auto DogBiteAttackJoints = std::vector<unsigned int>{ 3, 6 };

	enum DogState
	{
		DOG_STATE_NONE = 0, // TODO: Check what this is actually used for an rename accordingling.
		DOG_STATE_IDLE = 1,
		DOG_STATE_WALK_FORWARD = 2,
		DOG_STATE_RUN_FORWARD = 3,
		DOG_STATE_STALK = 5,
		DOG_STATE_JUMP_ATTACK = 6,
		DOG_STATE_HOWL = 7,
		DOG_STATE_SLEEP = 8,
		DOG_STATE_STALK_IDLE = 9,
		DOG_STATE_RUN_FORWARD_RIGHT = 10, // Unused.
		DOG_STATE_DEATH = 11,
		DOG_STATE_BITE_ATTACK = 12
	};

	const char* state_strings[]
	{
		"DOG_STATE_NONE",
		"DOG_STATE_IDLE",
		"DOG_STATE_WALK_FORWARD",
		"DOG_STATE_RUN_FORWARD",
		"DOG_STATE_STALK",
		"DOG_STATE_JUMP_ATTACK",
		"DOG_STATE_HOWL",
		"DOG_STATE_SLEEP",
		"DOG_STATE_STALK_IDLE",
		"DOG_STATE_RUN_FORWARD_RIGHT",
		"DOG_STATE_DEATH",
		"DOG_STATE_BITE_ATTACK"
	};

	enum DogAnim
	{
		DOG_ANIM_SLEEP = 0,
		DOG_ANIM_AWAKEN = 1,
		DOG_ANIM_IDLE_TO_WALK_FORWARD = 2,
		DOG_ANIM_WALK_FORWARD = 3,
		DOG_ANIM_WALK_FORWARD_TO_STALK = 4,
		DOG_ANIM_STALK_FORWARD = 5,
		DOG_ANIM_STALK_FORWARD_TO_RUN_FORWARD = 6,
		DOG_ANIM_RUN_FORWARD = 7,
		DOG_ANIM_IDLE = 8,
		DOG_ANIM_JUMP_ATTACK = 9,
		DOG_ANIM_STALK_IDLE_TO_STALK_FORWARD = 10,
		DOG_ANIM_STALK_FORWARD_TO_STALK_IDLE_START = 11,
		DOG_ANIM_STALK_FORWARD_TO_STALK_IDLE_END = 12,
		DOG_ANIM_STALK_IDLE_TO_RUN_FORWARD = 13,
		DOG_ANIM_STALK_IDLE = 14,
		DOG_ANIM_RUN_FORWARD_TO_STALK_IDLE = 15,
		DOG_ANIM_HOWL = 16,
		DOG_ANIM_IDLE_TO_STALK_IDLE = 17,
		DOG_ANIM_RUN_FORWARD_RIGHT = 18, // Unused.
		DOG_ANIM_WALK_FORWARD_TO_IDLE = 19,
		DOG_ANIM_DEATH_1 = 20,
		DOG_ANIM_DEATH_2 = 21,
		DOG_ANIM_DEATH_3 = 22,
		DOG_ANIM_BITE_ATTACK = 23,
		DOG_ANIM_STALK_IDLE_TO_IDLE = 24
	};

	const std::array DogDeathAnims = { DOG_ANIM_DEATH_1, DOG_ANIM_DEATH_2, DOG_ANIM_DEATH_3 };


	// Peter: Find closest dog treat (flare item for now)
	ItemInfo* FindClosestTreat(ItemInfo* item) {

		float shortestDistance = INFINITY;

		ItemInfo* foundTreat = nullptr;

		for (auto& potentialTreat : g_Level.Items)
		{
			if (!potentialTreat.Active || potentialTreat.ObjectNumber == NO_ITEM || potentialTreat.RoomNumber == NO_ROOM)
				continue;

			// Make sure the object is a valid dog treat.
			if (potentialTreat.ObjectNumber != ID_FLARE_ITEM) {
				continue;
			}
			
			float distance = Vector3i::Distance(item->Pose.Position, potentialTreat.Pose.Position);
			if (distance < shortestDistance) {
				shortestDistance = distance;
				foundTreat = &potentialTreat;
			}
		}

		// If it isn't close enough, return no treat.
		//if (shortestDistance < SQUARE(BLOCK(2)))
			//foundTreat = nullptr;

		return foundTreat;
	}

	short FindClosestMaster(ItemInfo* item) {
		float shortestDistance = INFINITY;

		short masterIndex = DOG_NO_MASTER;

		TENLog("Beinning master search...\n");
		for (auto& potentialMaster : g_Level.Items)
		{
			if (potentialMaster.ObjectNumber == NO_ITEM || potentialMaster.RoomNumber == NO_ROOM)
				continue;

			// Make sure the object is a valid dog master.
			bool is_valid_master = false;
			for (auto& id : DOG_MASTER_ITEMIDS) {
				if (potentialMaster.ObjectNumber == id) {
					is_valid_master = true;
					break;
				}
			}
			if (!is_valid_master) {
				continue;
			}
			else {
				TENLog(string_format("Found potential master %i of type %i!\n", (int)(potentialMaster.Index), (int)(potentialMaster.ObjectNumber)));
			}

			//if (SameZone(GetCreatureInfo(item), &potentialMaster))
			//{
			TENLog(string_format("Checiking index %i\n", (int)potentialMaster.Index));
			float distance = Vector3i::Distance(item->Pose.Position, potentialMaster.Pose.Position);
			//if (potentialMaster.IsCreature())//&& distance < DOG_MASTER_ASSIGN_DISTANCE)
			//{
			if (distance < shortestDistance) {
				//TENLog("Found master\n");
				shortestDistance = distance;
				masterIndex = potentialMaster.Index;
			}
			//}
		//}
		}

		if (masterIndex == DOG_NO_MASTER)
			TENLog("[Guard Dog] Could not find master!\n");
		else
			TENLog(string_format("[Guard Dog] Found master index %i\n", (int)masterIndex));
		return masterIndex;
	}

	void InitialiseTr4Dog(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		// OCB 1 makes the dog sitting down until fired
		if (item->TriggerFlags)
		{
			SetAnimation(item, DOG_ANIM_AWAKEN);
			item->Status -= ITEM_INVISIBLE;
		}
		else
			SetAnimation(item, DOG_ANIM_IDLE);
	}

	void InitialiseGuardDog(short itemNumber) {
		auto* item = &g_Level.Items[itemNumber];

		// OCB 1 makes the dog sitting down until fired
		if (item->TriggerFlags)
		{
			SetAnimation(item, DOG_ANIM_AWAKEN);
			item->Status -= ITEM_INVISIBLE;
		}
		else
			SetAnimation(item, DOG_ANIM_IDLE);

		// Peter: set dog's master (if possible)
		TENLog("[Guard Dog]: Trying to find master at init...\n");
		item->ItemFlags[DOGMASTER_FLAG] = FindClosestMaster(item);
	}

	void GuardDogControl(short itemNumber) {
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short joint2 = 0;
		short joint1 = 0;
		short joint0 = 0;

		// Peter: Easy bool flag for if the dog has a master.
		bool has_master = (item->ItemFlags[DOGMASTER_FLAG] != DOG_NO_MASTER);

		// Peter: Make the dog yelp when it takes damage sometimes and occasionally run back to master.
		if (item->HitStatus) {
			if (Random::TestProbability(1 / 3.0f))
			{
				SoundEffect(SFX_TR4_DOG_DEATH, &item->Pose);
				//creature->Mood = MoodType::Escape;
				//creature->Enemy = &g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]];
			}
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.AnimNumber == object->animIndex + 1)
				item->HitPoints = object->HitPoints;
			else if (item->Animation.ActiveState != DOG_STATE_DEATH)
				SetAnimation(item, DogDeathAnims[Random::GenerateInt(0, DogDeathAnims.size() - 1)]);
		}
		else
		{
			// Peter: If we have a master, follow them.
			/*if (item->ItemFlags[DOGMASTER_FLAG]) {
				creature->Enemy = &g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]];
			}*/

			auto* oldEnemy = creature->Enemy;

			if (item->AIBits)
				GetAITarget(creature);
			else {
				// Peter: If we have a master, follow them.
				auto* treat = FindClosestTreat(item);

				if (treat) {
					creature->Enemy = treat;
				}
				else if (item->ItemFlags[DOGMASTER_FLAG] && g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]].Active) {
					creature->Enemy = &g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]];
					if ((creature->Enemy->HitPoints < 0 || GetCreatureInfo(creature->Enemy)->HurtByLara) && LaraItem->HitPoints > 0) {
						creature->Enemy = LaraItem;
					}
				}
				else {
					creature->Enemy = LaraItem;
				}
			}

			// DEBUG: Print enemy target changes.
			if (oldEnemy != creature->Enemy) {
				std::string old_name = "None";
				std::string new_name = "None";

				if (oldEnemy)
					old_name = oldEnemy->Name;

				if (creature->Enemy)
					new_name = creature->Enemy->Name;

				TENLog("[Guard Dog]" + item->Name + " is switching target from: " + old_name + " to " + new_name + "\n");
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance;
			if (creature->Enemy->IsLara())
			{
				distance = AI.distance;
			}
			else {
				int dx = creature->Enemy->Pose.Position.x - item->Pose.Position.x;
				int dz = creature->Enemy->Pose.Position.z - item->Pose.Position.z;
				AI.distance = SQUARE(dx) + SQUARE(dz);
				distance = AI.distance;
				AI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				AI.ahead = (AI.angle > -FRONT_ARC && AI.angle < FRONT_ARC);
			}
			/*else if (has_master) {
				auto* master_item = &g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]];
				
				int dx = master_item->Pose.Position.x - item->Pose.Position.x;
				int dz = master_item->Pose.Position.z - item->Pose.Position.z;
				AI.distance = SQUARE(dx) + SQUARE(dz);
				distance = AI.distance;
				AI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				AI.ahead = (AI.angle > -FRONT_ARC && AI.angle < FRONT_ARC);
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				phd_atan(dz, dx);
				distance = pow(dx, 2) + pow(dz, 2);
			}*/

			if (AI.ahead)
			{
				joint2 = AI.xAngle; // TODO: Maybe swapped
				joint1 = AI.angle;
			}

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			if (creature->Mood == MoodType::Bored)
				creature->MaxTurn /= 2;

			angle = CreatureTurn(item, creature->MaxTurn);
			joint0 = angle * 4;

			if (creature->HurtByLara || distance < pow(SECTOR(3), 2) && !(item->AIBits & MODIFY))
			{
				AlertAllGuards(itemNumber);
				item->AIBits &= ~MODIFY;
			}

			int frame = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

			auto state = item->Animation.TargetState;

			switch (item->Animation.ActiveState)
			{
			case DOG_STATE_NONE:
			case DOG_STATE_SLEEP:
				joint1 = 0;
				joint2 = 0;

				if (creature->Mood != MoodType::Bored && item->AIBits != MODIFY)
					item->Animation.TargetState = DOG_STATE_IDLE;
				else
				{
					creature->Flags++;
					creature->MaxTurn = 0;

					if (creature->Flags > 300 && Random::TestProbability(1.0f / 256))
						item->Animation.TargetState = DOG_STATE_IDLE;
				}

				break;

			case DOG_STATE_IDLE:
			case DOG_STATE_STALK_IDLE:
				creature->MaxTurn = 0;

				if (item->Animation.ActiveState == DOG_STATE_STALK_IDLE &&
					item->Animation.RequiredState != NO_STATE)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					break;
				}

				if (item->AIBits & GUARD)
				{
					joint1 = AIGuard(creature);

					if (Random::TestProbability(0.996f))
						break;

					if (item->Animation.ActiveState == DOG_STATE_IDLE)
					{
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						break;
					}
				}
				else
				{
					if (item->Animation.ActiveState == DOG_STATE_STALK_IDLE &&
						Random::TestProbability(1.0f / 256))
					{
						item->Animation.TargetState = DOG_STATE_IDLE;
						break;
					}

					if (item->AIBits & PATROL1)
					{
						if (item->Animation.ActiveState == DOG_STATE_IDLE)
							item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
						else
							item->Animation.TargetState = DOG_STATE_IDLE;

						break;
					}

					if (creature->Mood == MoodType::Escape)
					{
						if (Lara.TargetEntity == item || !AI.ahead || item->HitStatus)
						{
							item->Animation.TargetState = DOG_STATE_STALK_IDLE;
							item->Animation.RequiredState = DOG_STATE_RUN_FORWARD;
						}
						else
							item->Animation.TargetState = DOG_STATE_IDLE;

						break;
					}

					if (creature->Mood != MoodType::Bored)
					{
						if (item->Animation.ActiveState == DOG_STATE_IDLE)
							item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						
						//Peter: Why?
						item->Animation.RequiredState = DOG_STATE_RUN_FORWARD;
						//item->Animation.RequiredState = DOG_STATE_WALK_FORWARD;
						break;
					}

					creature->MaxTurn = ANGLE(1.0f);
					creature->Flags = 0;

					if (Random::TestProbability(1 / 128.0f))
					{
						if (item->AIBits & MODIFY)
						{
							if (item->Animation.ActiveState == DOG_STATE_IDLE)
							{
								item->Animation.TargetState = DOG_STATE_SLEEP;
								creature->Flags = 0;
								break;
							}
						}
					}

					if (Random::TestProbability(0.875f))
					{
						if (Random::TestProbability(1 / 30.0f))
							item->Animation.TargetState = DOG_STATE_HOWL;

						break;
					}

					if (item->Animation.ActiveState == DOG_STATE_IDLE)
					{
						item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
						break;
					}
				}

				item->Animation.TargetState = DOG_STATE_IDLE;
				break;

			case DOG_STATE_WALK_FORWARD:
				creature->MaxTurn = ANGLE(3.0f);

				if (item->AIBits & PATROL1)
					item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Bored && Random::TestProbability(1 / 128.0f))
					item->Animation.TargetState = DOG_STATE_IDLE;
				else
					item->Animation.TargetState = DOG_STATE_STALK;

				break;

			case DOG_STATE_RUN_FORWARD:
				creature->MaxTurn = ANGLE(6.0f);

				if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
				}
				else if (creature->Mood != MoodType::Bored)
				{
					// Peter: Don't attack master.
					if (creature->Enemy == &g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]]) {
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						break;
					}

					if (AI.bite && AI.distance < DOG_JUMP_ATTACK_RANGE)
						item->Animation.TargetState = DOG_STATE_JUMP_ATTACK;
					else if (AI.distance < pow(SECTOR(1.5f), 2))
					{
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						item->Animation.RequiredState = DOG_STATE_STALK;
					}
				}
				else
					item->Animation.TargetState = DOG_STATE_STALK_IDLE;

				break;

			case DOG_STATE_STALK:
				creature->MaxTurn = ANGLE(3.0f);

				if (creature->Mood != MoodType::Bored)
				{
					if (creature->Mood == MoodType::Escape)
						item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
					else if (AI.bite && AI.distance < DOG_BITE_ATTACK_RANGE)
					{
						// Peter: Don't attack master.
						if (creature->Enemy == &g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]]) {
							item->Animation.TargetState = DOG_STATE_IDLE;
							break;
						}

						item->Animation.TargetState = DOG_STATE_BITE_ATTACK;
						item->Animation.RequiredState = DOG_STATE_STALK;
					}
					else if (AI.distance > pow(SECTOR(1.5f), 2) || item->HitStatus) {
						if(creature->Enemy == &g_Level.Items[item->ItemFlags[DOGMASTER_FLAG]])
							item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
						else
							item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
					}
				}
				else
					item->Animation.TargetState = DOG_STATE_STALK_IDLE;

				break;

			case DOG_STATE_JUMP_ATTACK:
				if (AI.bite && item->TouchBits.Test(DogJumpAttackJoints) &&
					frame >= 4 && frame <= 14)
				{
					DoDamage(creature->Enemy, DOG_JUMP_ATTACK_DAMAGE);
					CreatureEffect2(item, DogBite, 2, -1, DoBloodSplat);
				}

				if (creature->Enemy->ObjectNumber == ID_FLARE_ITEM) {
					SoundEffect(SFX_TR4_DOG_HOWL, &item->Pose);
					KillItem(creature->Enemy->Index);
					creature->Enemy = nullptr;
				}

				item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
				break;

			case DOG_STATE_HOWL:
				joint1 = 0;
				joint2 = 0;
				break;

			case DOG_STATE_BITE_ATTACK:
				if (AI.bite && item->TouchBits.Test(DogBiteAttackJoints) &&
					((frame >= 9 && frame <= 12) || (frame >= 22 && frame <= 25)))
				{
					DoDamage(creature->Enemy, DOG_BITE_ATTACK_DAMAGE);
					CreatureEffect2(item, DogBite, 2, -1, DoBloodSplat);
				}

				if (creature->Enemy->ObjectNumber == ID_FLARE_ITEM) {
					SoundEffect(SFX_TR4_DOG_HOWL, &item->Pose);
					KillItem(creature->Enemy->Index);
					creature->Enemy = nullptr;
				}

				break;

			default:
				break;
			}
			
			auto new_state = item->Animation.TargetState;

			/*if (state != new_state) {
				TENLog(string_format("[Guard Dog] Went from state %s to %s.\n", state_strings[state], state_strings[new_state]));
			}*/
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}

	void Tr4DogControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short joint2 = 0;
		short joint1 = 0;
		short joint0 = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.AnimNumber == object->animIndex + 1)
				item->HitPoints = object->HitPoints;
			else if (item->Animation.ActiveState != DOG_STATE_DEATH)
				SetAnimation(item, DogDeathAnims[Random::GenerateInt(0, DogDeathAnims.size() - 1)]);
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance;
			if (creature->Enemy->IsLara())
			{
				distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				phd_atan(dz, dx);
				distance = pow(dx, 2) + pow(dz, 2);
			}

			if (AI.ahead)
			{
				joint2 = AI.xAngle; // TODO: Maybe swapped
				joint1 = AI.angle;
			}

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			if (creature->Mood == MoodType::Bored)
				creature->MaxTurn /= 2;

			angle = CreatureTurn(item, creature->MaxTurn);
			joint0 = angle * 4;

			if (creature->HurtByLara || distance < pow(SECTOR(3), 2) && !(item->AIBits & MODIFY))
			{
				AlertAllGuards(itemNumber);
				item->AIBits &= ~MODIFY;
			}

			int frame = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

			switch (item->Animation.ActiveState)
			{
			case DOG_STATE_NONE:
			case DOG_STATE_SLEEP:
				joint1 = 0;
				joint2 = 0;

				if (creature->Mood != MoodType::Bored && item->AIBits != MODIFY)
					item->Animation.TargetState = DOG_STATE_IDLE;
				else
				{
					creature->Flags++;
					creature->MaxTurn = 0;

					if (creature->Flags > 300 && Random::TestProbability(1.0f / 256))
						item->Animation.TargetState = DOG_STATE_IDLE;
				}

				break;

			case DOG_STATE_IDLE:
			case DOG_STATE_STALK_IDLE:
				creature->MaxTurn = 0;

				if (item->Animation.ActiveState == DOG_STATE_STALK_IDLE &&
					item->Animation.RequiredState != NO_STATE)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					break;
				}

				if (item->AIBits & GUARD)
				{
					joint1 = AIGuard(creature);

					if (Random::TestProbability(0.996f))
						break;

					if (item->Animation.ActiveState == DOG_STATE_IDLE)
					{
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						break;
					}
				}
				else
				{
					if (item->Animation.ActiveState == DOG_STATE_STALK_IDLE &&
						Random::TestProbability(1.0f / 256))
					{
						item->Animation.TargetState = DOG_STATE_IDLE;
						break;
					}

					if (item->AIBits & PATROL1)
					{
						if (item->Animation.ActiveState == DOG_STATE_IDLE)
							item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
						else
							item->Animation.TargetState = DOG_STATE_IDLE;

						break;
					}

					if (creature->Mood == MoodType::Escape)
					{
						if (Lara.TargetEntity == item || !AI.ahead || item->HitStatus)
						{
							item->Animation.TargetState = DOG_STATE_STALK_IDLE;
							item->Animation.RequiredState = DOG_STATE_RUN_FORWARD;
						}
						else
							item->Animation.TargetState = DOG_STATE_IDLE;

						break;
					}

					if (creature->Mood != MoodType::Bored)
					{
						if (item->Animation.ActiveState == DOG_STATE_IDLE)
							item->Animation.TargetState = DOG_STATE_STALK_IDLE;

						item->Animation.RequiredState = DOG_STATE_RUN_FORWARD;
						break;
					}

					creature->MaxTurn = ANGLE(1.0f);
					creature->Flags = 0;

					if (Random::TestProbability(1 / 128.0f))
					{
						if (item->AIBits & MODIFY)
						{
							if (item->Animation.ActiveState == DOG_STATE_IDLE)
							{
								item->Animation.TargetState = DOG_STATE_SLEEP;
								creature->Flags = 0;
								break;
							}
						}
					}

					if (Random::TestProbability(0.875f))
					{
						if (Random::TestProbability(1 / 30.0f))
							item->Animation.TargetState = DOG_STATE_HOWL;

						break;
					}

					if (item->Animation.ActiveState == DOG_STATE_IDLE)
					{
						item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
						break;
					}
				}

				item->Animation.TargetState = DOG_STATE_IDLE;
				break;

			case DOG_STATE_WALK_FORWARD:
				creature->MaxTurn = ANGLE(3.0f);

				if (item->AIBits & PATROL1)
					item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Bored && Random::TestProbability(1 / 128.0f))
					item->Animation.TargetState = DOG_STATE_IDLE;
				else
					item->Animation.TargetState = DOG_STATE_STALK;

				break;

			case DOG_STATE_RUN_FORWARD:
				item->Animation.ActiveState = DOG_STATE_WALK_FORWARD;
				break;
				creature->MaxTurn = ANGLE(6.0f);

				if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
				}
				else if (creature->Mood != MoodType::Bored)
				{
					if (AI.bite && AI.distance < DOG_JUMP_ATTACK_RANGE)
						item->Animation.TargetState = DOG_STATE_JUMP_ATTACK;
					else if (AI.distance < pow(SECTOR(1.5f), 2))
					{
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						item->Animation.RequiredState = DOG_STATE_STALK;
					}
				}
				else
					item->Animation.TargetState = DOG_STATE_STALK_IDLE;

				break;

			case DOG_STATE_STALK:
				creature->MaxTurn = ANGLE(3.0f);

				if (creature->Mood != MoodType::Bored)
				{
					if (creature->Mood == MoodType::Escape)
						item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
					else if (AI.bite && AI.distance < DOG_BITE_ATTACK_RANGE)
					{
						item->Animation.TargetState = DOG_STATE_BITE_ATTACK;
						item->Animation.RequiredState = DOG_STATE_STALK;
					}
					else if (AI.distance > pow(SECTOR(1.5f), 2) || item->HitStatus)
						item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
				}
				else
					item->Animation.TargetState = DOG_STATE_STALK_IDLE;

				break;

			case DOG_STATE_JUMP_ATTACK:
				if (AI.bite && item->TouchBits.Test(DogJumpAttackJoints) &&
					frame >= 4 && frame <= 14)
				{
					DoDamage(creature->Enemy, DOG_JUMP_ATTACK_DAMAGE);
					CreatureEffect2(item, DogBite, 2, -1, DoBloodSplat);
				}

				item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
				break;

			case DOG_STATE_HOWL:
				joint1 = 0;
				joint2 = 0;
				break;

			case DOG_STATE_BITE_ATTACK:
				if (AI.bite && item->TouchBits.Test(DogBiteAttackJoints) &&
					((frame >= 9 && frame <= 12) || (frame >= 22 && frame <= 25)))
				{
					DoDamage(creature->Enemy, DOG_BITE_ATTACK_DAMAGE);
					CreatureEffect2(item, DogBite, 2, -1, DoBloodSplat);
				}

				break;

			default:
				break;
			}
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
