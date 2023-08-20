#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Physics.h"

#include "Game/animation.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/effects.h"
#include "Game/effects/Ripple.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Objects/Generic/Object/Pushables/PushableObject_BridgeCol.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Scans.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Stack.h"
#include "Math/Math.h"
#include "Math/Random.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Effects::Ripple;
using namespace TEN::Input;
using namespace TEN::Math::Random;

namespace TEN::Entities::Generic
{
	std::unordered_map<PushablePhysicState, std::function<void(int)>> PUSHABLES_STATES_MAP;

	constexpr auto PUSHABLE_FALL_VELOCITY_MAX = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);
	constexpr auto GRAVITY_AIR = 8.0f;
	constexpr auto GRAVITY_ACCEL = 0.5f;
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;
	constexpr auto WATER_SURFACE_DISTANCE = CLICK(0.5f);
	constexpr auto FRAMES_BETWEEN_RIPPLES = 8.0f;

	void InitializePushablesStatesMap()
	{
		if (PUSHABLES_STATES_MAP.empty() )
		{
			PUSHABLES_STATES_MAP.clear();

			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Idle, &HandleIdleState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Moving, &HandleMovingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Falling, &HandleFallingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Sinking, &HandleSinkingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Floating, &HandleFloatingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::UnderwaterIdle, &HandleUnderwaterState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::WatersurfaceIdle, &HandleWatersurfaceState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Sliding, &HandleSlidingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::StackHorizontalMove, &HandleStackHorizontalMoveState);
		}
	}

	void HandleIdleState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. Check if Lara is interacting with it
		if (Lara.Context.InteractedItem == itemNumber)
		{
			//If Lara is grabbing, check the push pull actions.
			if (LaraItem->Animation.ActiveState == LS_PUSHABLE_GRAB || TestLastFrame(LaraItem, LA_PUSHABLE_GRAB))
			{
				//First checks conditions.
				bool hasPushAction = IsHeld(In::Forward);
				bool hasPullAction = IsHeld(In::Back);

				//Cond 1: Is pressing Forward or Back?
				if (!hasPushAction && !hasPullAction)
					return;

				//Cond 2: Can do the interaction with that side of the pushable?
				int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
				auto& pushableSidesAttributes = pushable.SidesMap[quadrant]; //0 North, 1 East, 2 South or 3 West.

				if ((hasPushAction && !pushableSidesAttributes.Pushable) ||
					(hasPullAction && !pushableSidesAttributes.Pullable))
					return;

				//Cond 3: Is its stacked pushables under the limit?
				if (!IsUnderStackLimit(itemNumber))
					return;

				//Cond 4: Does it comply with the room collision conditions?.
				if (!PushableMovementConditions(itemNumber, hasPushAction, hasPullAction))
					return;

				//Then Do the interaction.
				if (hasPushAction)
				{
					int pushAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;
					SetAnimation(LaraItem, pushAnimNumber);
				}
				else if (hasPullAction)
				{
					int pullAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
					SetAnimation(LaraItem, pullAnimNumber);
				}

				pushable.StartPos = pushableItem.Pose.Position;
				pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
				pushable.BehaviourState = PushablePhysicState::Moving;

				StartMovePushableStack(itemNumber);		//Prepare the upper pushables in the stack for the move.
				UnpilePushable(itemNumber);				//Cut the link with the lower pushables in the stack.

				ResetPlayerFlex(LaraItem);

				DeactivateClimbablePushableCollider(itemNumber);
			}
			else if (	LaraItem->Animation.ActiveState != LS_PUSHABLE_GRAB &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PULL &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PUSH)
			{
				Lara.Context.InteractedItem = NO_ITEM;
			}

			return;
		}

		//2. Check if it's in a water room
		if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			//check if it's deep water. (Otherwise, is shallow water, keep this idle state).
			auto pointColl = GetCollision(&pushableItem);
			int waterHeight = GetWaterHeight(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
			int distanceToSurface = abs(waterHeight - pointColl.Position.Floor);
			if (distanceToSurface > CLICK(2))
			{
				//Is deep water, transition to water states.

				DeactivateClimbablePushableCollider(itemNumber);
				SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);

				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushable.BehaviourState = PushablePhysicState::Floating;
					pushable.Gravity = 0.0f;
				}
				else
				{
					pushable.BehaviourState = PushablePhysicState::Sinking;
					pushable.Gravity = 0.0f;
				}
				return;
			}
			else
			{
				// Effects: Spawn ripples.
				//TODO: Enhace the effect to make the ripples increase their size through the time.
				if (pushable.StartPos.y != NO_HEIGHT && std::fmod(GameTimer, FRAMES_BETWEEN_RIPPLES) <= 0.0f)
				{
					SpawnRipple(Vector3(pushableItem.Pose.Position.x, waterHeight, pushableItem.Pose.Position.z), pushableItem.RoomNumber, GameBoundingBox(&pushableItem).GetWidth() + (GetRandomControl() & 15), (int)RippleFlags::SlowFade | (int)RippleFlags::LowOpacity);
				}
			}
		}

		//3. Check if floor has changed
		DeactivateClimbablePushableCollider(itemNumber);
		int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;
		ActivateClimbablePushableCollider(itemNumber);


		if (floorHeight > pushableItem.Pose.Position.y)			//The floor has decresed. (Flip map, trapdoor, etc...)
		{
			//If the diffence is not very big, just teleport it.
			if (abs(pushableItem.Pose.Position.y - floorHeight) >= CLICK(1))
			{
				pushable.BehaviourState = PushablePhysicState::Falling;
				SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);
				DeactivateClimbablePushableCollider(itemNumber);
			}
			else
			{
				int heightdifference = floorHeight - pushableItem.Pose.Position.y;
				pushableItem.Pose.Position.y += heightdifference;
				VerticalPosAddition(itemNumber, heightdifference); //WIP: function to elevate the stack.
			}
			
			return;
		}
		else if (floorHeight < pushableItem.Pose.Position.y)	//The floor has risen. (Elevator, raising block, etc...)
		{
			int heightdifference = floorHeight - pushableItem.Pose.Position.y;
			pushableItem.Pose.Position.y += heightdifference;
			VerticalPosAddition(itemNumber, heightdifference); //WIP: function to elevate the stack.
			return;
		}

		return;
	}

	void HandleMovingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		bool isPlayerPulling = LaraItem->Animation.ActiveState == LS_PUSHABLE_PULL;

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushable.StartPos.x;
		int newPosZ = pushable.StartPos.z;

		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		if (pushable.CurrentSoundState == PushableSoundState::Moving)
			pushable.CurrentSoundState = PushableSoundState::Stopping;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->BoundingBox.Z2;
		displaceBox -= isPlayerPulling ? (BLOCK(1) + displaceDepth) : (displaceDepth - BLOCK(1));

		//Lara is doing the pushing / pulling animation
		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			//1. Decide the goal position
			switch (quadrantDir)
			{
				case NORTH:
					newPosZ += displaceBox;
					break;

				case EAST:
					newPosX += displaceBox;
					break;

				case SOUTH:
					newPosZ -= displaceBox;
					break;

				case WEST:
					newPosX -= displaceBox;
					break;

				default:
					break;
			}

			pushable.CurrentSoundState = PushableSoundState::Stopping;

			//2. Move pushable
			
			//Don't move the pushable if the distance is too big (it may happens because the animation bounds changes in the animation loop of push pull process).
			if (abs(pushableItem.Pose.Position.z - newPosZ) > BLOCK(0.75f))
				return;
			if (abs(pushableItem.Pose.Position.x - newPosX) > BLOCK(0.75f))
				return;

			// move only if the move direction is oriented to the action
			// So pushing only moves pushable forward, and pulling only moves backwards

			//Z axis
			if (isPlayerPulling)
			{
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z > newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z < newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					pushable.CurrentSoundState = PushableSoundState::Moving;
				}
			}
			else
			{
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z > newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					pushable.CurrentSoundState = PushableSoundState::Moving;
				}
			}

			//X axis
			if (isPlayerPulling)
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x > newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x < newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					pushable.CurrentSoundState = PushableSoundState::Moving;
				}
			}
			else
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x < newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x > newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					pushable.CurrentSoundState = PushableSoundState::Moving;
				}
			}
		}
		else
		{
			//Pushing Pulling animation ended
			
			//1. Realign with sector center
			pushableItem.Pose.Position = GetNearestSectorCenter(pushableItem.Pose.Position);

			//TODO: It only should do it if there is not any other pushable remaining there
			SetPushableStopperFlag(false, pushable.StartPos.ToVector3i(), pushable.StartPos.RoomNumber);

			pushable.StartPos = pushableItem.Pose.Position;
			pushable.StartPos.RoomNumber = pushableItem.RoomNumber;

			//2. Activate trigger
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

			//3. Check floor height
			// Check if pushing pushable over edge. Then can't keep pushing/pulling and pushable start to fall.
			int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;
			if (floorHeight > pushableItem.Pose.Position.y)
			{
				LaraItem->Animation.TargetState = LS_IDLE;
				Lara.Context.InteractedItem = NO_ITEM;
				pushable.BehaviourState = PushablePhysicState::Falling;
				pushable.CurrentSoundState = PushableSoundState::None;

				return;
			}

			//5. Check input too see if it can keep the movement

			// Check the pushable animation system in use, if is using block animation which can't loop, go back to idle state.
			if (!PushableAnimInfos[pushable.AnimationSystemIndex].EnableAnimLoop ||
				!IsHeld(In::Action) ||
				!PushableMovementConditions(itemNumber, !isPlayerPulling, isPlayerPulling))
			{
				LaraItem->Animation.TargetState = LS_IDLE;
				pushable.BehaviourState = PushablePhysicState::Idle;
				StopMovePushableStack(itemNumber); //Set the upper pushables back to normal.
				ActivateClimbablePushableCollider(itemNumber);

				//The pushable is going to stop here, do the checks to conect it with another Stack.
				int FoundStack = SearchNearPushablesStack(itemNumber);
				StackPushable(itemNumber, FoundStack);

				return;
			}
			//Otherwise, continue the movement	
		}

		return;
	}

	void HandleFallingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. Prepare data (floor height and velocities).
		auto pointColl = GetCollision(&pushableItem);

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;
		 
		//2. Move the pushable downwards
		if (currentY < (pointColl.Position.Floor - velocityY))
		{
			float newVelocityY = velocityY + pushable.Gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_FALL_VELOCITY_MAX);

			pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
			
			//3. Check if it's in a water room
			if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushable.BehaviourState = PushablePhysicState::Sinking;

				// Effect: Water splash.
				int waterHeight = GetWaterHeight(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
				SplashSetup.y = waterHeight - 1;
				SplashSetup.x = pushableItem.Pose.Position.x;
				SplashSetup.z = pushableItem.Pose.Position.z;
				SplashSetup.splashPower = pushableItem.Animation.Velocity.y * 2;
				SplashSetup.innerRadius = 250;
				SetupSplash(&SplashSetup, pushableItem.RoomNumber);
			}
			return;
		}

		//Reached the ground
		//3. Check room collision
		
		//If it's a flat ground?
		if (pointColl.FloorTilt.x == 0 && pointColl.FloorTilt.y == 0)
		{
			//The pushable is going to stop here, do the checks to conect it with another Stack.
			int FoundStack = SearchNearPushablesStack(itemNumber);
			StackPushable(itemNumber, FoundStack);

			//Set Stopper Flag
			if (pushable.StackLowerItem == NO_ITEM)
				SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

			//Activate trigger
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

			//place on ground
			pushable.BehaviourState = PushablePhysicState::Idle;
			pushableItem.Pose.Position.y = pointColl.Position.Floor;

			ActivateClimbablePushableCollider(itemNumber);

			pushableItem.Animation.Velocity.y = 0;

			// Shake floor if pushable landed at high enough velocity.
			if (velocityY >= PUSHABLE_FALL_RUMBLE_VELOCITY)
				FloorShake(&pushableItem);
		
		}
		else
		{
			//TODO: if it's a slope ground?...
			//Then proceed to the sliding state.
		}
	}

	void HandleSinkingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. Ensure it's in water room
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			pushable.BehaviourState = PushablePhysicState::Falling;
			pushable.Gravity = GRAVITY_AIR;
			return;
		}

		//2. Prepare data, (floor height and velocities).
		auto pointColl = GetCollision(&pushableItem);

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;

		// TODO: [Effects Requirement] Add bubbles during this phase.

		//3. Manage gravity force
		if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
		{
			// Slowly reverses gravity direction. If gravity is 0, then it floats.
			pushable.Gravity = pushable.Gravity - GRAVITY_ACCEL;
			if (pushable.Gravity <= 0.0f)
			{
				pushable.BehaviourState = PushablePhysicState::Floating;
				return;
			}
		}
		else
		{
			// Decreases gravity, continues to fall until hits ground.
			pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, 4.0f);
		}

		//4. Move Object
		if (currentY < pointColl.Position.Floor - velocityY)
		{
			// Sinking down.
			float newVelocityY = velocityY + pushable.Gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

			pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;

			return;
		}

		// 5. Hit ground
		int waterHeight = GetWaterHeight(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
		int distanceToSurface = abs(waterHeight - pointColl.Position.Floor);
		if (distanceToSurface <= CLICK(2))
		{
			//shallow water
			pushable.BehaviourState = PushablePhysicState::Idle;
			pushableItem.Pose.Position.y = pointColl.Position.Floor;
			ActivateClimbablePushableCollider(itemNumber);

			//Activate trigger
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
		}
		else
		{
			//deep water
			if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
			{
				pushable.Gravity = 0.0f;
				pushable.BehaviourState = PushablePhysicState::Floating;
			}
			else
			{
				pushable.BehaviourState = PushablePhysicState::UnderwaterIdle;
				pushableItem.Pose.Position.y = pointColl.Position.Floor;
				ActivateClimbablePushableCollider(itemNumber);

				//Activate trigger
				TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
			}
			pushableItem.Animation.Velocity.y = 0.0f;
		}
	}

	void HandleFloatingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. Ensure it's in water room
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			//Set Stopper Flag
			if (pushable.StackLowerItem == NO_ITEM)
				SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

			pushable.BehaviourState = PushablePhysicState::Falling;
			pushable.Gravity = GRAVITY_AIR;
			return;
		}

		//2. Prepare data (goal height and velocities).
		auto pointColl = GetCollision(&pushableItem);

		int goalHeight = 0;

		int waterDepth = GetWaterSurface(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
		if (waterDepth != NO_HEIGHT)
		{
			goalHeight = waterDepth - WATER_SURFACE_DISTANCE + pushable.Height;

			//height position where the ripples will spawn
			pushable.StartPos.y = GetWaterHeight(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber) - 1;
		}
		else
		{
			goalHeight = pointColl.Position.Ceiling + WATER_SURFACE_DISTANCE + pushable.Height;

			//With no_height, ripples won't spawn
			pushable.StartPos.y = NO_HEIGHT;
		}

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;

		pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, -4.0f);

		//3. Move pushable upwards
		if (currentY > goalHeight)
		{
			// Floating up.
			float newVelocityY = velocityY + pushable.Gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

			// Update pushable's position and move its stack.
			pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
			return;
		}

		// Reached water surface.
		pushable.BehaviourState = PushablePhysicState::WatersurfaceIdle;
		pushableItem.Pose.Position.y = goalHeight;

		ActivateClimbablePushableCollider(itemNumber);

		pushableItem.Animation.Velocity.y = GenerateFloat(0.0f, 125.0f);
	}

	void HandleUnderwaterState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. Ensure it's in water room
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			pushable.BehaviourState = PushablePhysicState::Idle;
			pushable.Gravity = GRAVITY_AIR;

			pushableItem.Animation.Velocity.y = 0;
			return;
		}

		//2. If it's buoyant, it needs to float. (case for sudden flipmaps).
		if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
		{
			pushable.Gravity = 0.0f;
			pushable.BehaviourState = PushablePhysicState::Floating;
		}

	}

	void HandleWatersurfaceState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. Ensure it's in water room
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			pushable.BehaviourState = PushablePhysicState::Falling;
			pushable.Gravity = GRAVITY_AIR;
			pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);

			DeactivateClimbablePushableCollider(itemNumber);

			pushableItem.Animation.Velocity.y = 0;
			pushable.StartPos.y = NO_HEIGHT;
			return;
		}

		//2. Do water ondulation effect.
		if (!pushable.UsesRoomCollision)
			FloatingItem(pushableItem, pushable.FloatingForce);
		else
			FloatingBridge(pushableItem, pushable.FloatingForce);

		// Effects: Spawn ripples.
		//TODO: Enhace the effect to make the ripples increase their size through the time.
		if (pushable.StartPos.y != NO_HEIGHT && std::fmod(GameTimer, FRAMES_BETWEEN_RIPPLES) <= 0.0f)
		{
			SpawnRipple(Vector3(pushableItem.Pose.Position.x, pushable.StartPos.y, pushableItem.Pose.Position.z), pushableItem.RoomNumber, GameBoundingBox(&pushableItem).GetWidth() + (GetRandomControl() & 15), (int)RippleFlags::SlowFade | (int)RippleFlags::LowOpacity);
		}

	}

	void HandleSlidingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//TODO:
		//1. DETECT FLOOR, (Slope orientation)
		//2. CALCULATE DIRECTION AND SPEED
		//3. MOVE OBJECT
		//4. DETECT END CONDITIONS OF NEXT SECTOR 
			//	Is a slope-> keep sliding
			//	Is a flat floor -> so ends slide
			//	Is a fall -> so passes to falling
			//	Is a forbiden Sector -> freezes the slide
		//5. Incorporate effects? Smoke or sparkles?

	}

	void HandleStackHorizontalMoveState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto& MovingPushableItem = g_Level.Items[Lara.Context.InteractedItem];
		pushableItem.Pose.Position.x = MovingPushableItem.Pose.Position.x;
		pushableItem.Pose.Position.z = MovingPushableItem.Pose.Position.z;
	}

	/*TODO: 
	void HandleStackFallingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto& MovingPushableItem = g_Level.Items[Lara.Context.InteractedItem];
		pushableItem.Pose.Position.y = MovingPushableItem.Pose.Position.y;
	}*/
}
