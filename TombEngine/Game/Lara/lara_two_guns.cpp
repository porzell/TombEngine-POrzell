#include "framework.h"
#include "Game/Lara/lara_two_guns.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/savegame.h"
#include "Math/Random.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;
using namespace TEN::Math::Random;

struct PistolDef
{
	short ObjectNumber;
	int Draw1Anim2;
	int Draw1Anim;
	int Draw2Anim;
	int RecoilAnim;
	int ReloadAnim;
};

enum LaraTwoGunsStates {
	LARA_TWO_GUNS_IDLE,
	LARA_TWO_GUNS_DRAWING,
	LARA_TWO_GUNS_AIMING,
	LARA_TWO_GUNS_FIRING,
	LARA_TWO_GUNS_RELOADING,
	LARA_TWO_GUNS_UNAIMING,
	LARA_TWO_GUNS_UNDRAWING
};

PistolDef PistolsTable[4] =
{
	{ ID_LARA, 0, 0, 0, 0, 0 },
	{ ID_PISTOLS_ANIM, 4, 5, 13, 24, 33 },
	{ ID_REVOLVER_ANIM , 7, 8, 15, 29, 7 },
	{ ID_UZI_ANIM, 4, 5, 13, 24, 4 }
};

constexpr Vector3i GunsmokeOffsets[4] = {
	{0,0,0},
	{4, 128, 40},
	{16, 160, 56},
	{-16, 140, 48}
};

short constexpr DefaultMagazineCounts[4] = {
	0,
	12,
	6,
	32
};

short MagazineCounts[4][2] = {
	{ DefaultMagazineCounts[(int)LaraWeaponType::None], DefaultMagazineCounts[(int)LaraWeaponType::None]},
	{ DefaultMagazineCounts[(int)LaraWeaponType::Pistol], DefaultMagazineCounts[(int)LaraWeaponType::Pistol]},
	{ DefaultMagazineCounts[(int)LaraWeaponType::Revolver], DefaultMagazineCounts[(int)LaraWeaponType::Revolver]},
	{ DefaultMagazineCounts[(int)LaraWeaponType::Uzi], DefaultMagazineCounts[(int)LaraWeaponType::Uzi]}
};


bool doReloadRight = false;
bool doReloadLeft = false;

void AnimatePistols(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* weapon = &Weapons[(int)weaponType];
	auto* p = &PistolsTable[(int)lara->Control.Weapon.GunType];

	int fired = false;

	if (lara->LeftArm.FrameNumber == p->ReloadAnim + 5) {
		TriggerGunShell(0, ID_PISTOLS_AMMO_ITEM, weaponType); // Left hand
		SoundEffect(SFX_TR4_LARA_RELOAD, &LaraItem->Pose);
	}
	if (lara->RightArm.FrameNumber == p->ReloadAnim + 5) {
		TriggerGunShell(1, ID_PISTOLS_AMMO_ITEM, weaponType); // Left hand
		SoundEffect(SFX_TR4_LARA_RELOAD, &LaraItem->Pose);
	}

	if (laraItem->MeshBits.TestAny())
	{
		if (lara->LeftArm.GunSmoke)
		{
			auto offset = GunsmokeOffsets[(int)weaponType];

			auto pos = GetJointPosition(laraItem, LM_LHAND, offset);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weaponType, lara->LeftArm.GunSmoke);
		}

		if (lara->RightArm.GunSmoke)
		{
			auto offset = GunsmokeOffsets[(int)weaponType];

			auto pos = GetJointPosition(laraItem, LM_RHAND, offset);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weaponType, lara->RightArm.GunSmoke);
		}
	}

	// Shooting action for right arm.
	int frameRight = lara->RightArm.FrameNumber;  // frame number of DRAW_END?
	if ((TrInput & IN_ACTION && !lara->TargetEntity) || lara->RightArm.Locked)
	{
		// POINT ARMS FORWARD
		// at or beyond (2) DRAW_END start frame AND before (0) SHOOT_START end frame...
		if ((frameRight >= 0) && (frameRight < p->Draw1Anim2))
		{
			// ...increment toward (0) SHOOT_START end frame
			frameRight++;
		}
		// at (0) SHOOT_START end frame
		else if (frameRight == p->Draw1Anim2)
		{
			// actually shoot, bang bang
			if (TrInput & IN_ACTION)
			{
				if (weaponType != LaraWeaponType::Revolver)
				{
					auto rightArmOrient = EulerAngles(
						lara->RightArm.Orientation.x,
						lara->RightArm.Orientation.y + laraItem->Pose.Orientation.y,
						0
					);

					if (FireWeapon(weaponType, lara->TargetEntity, laraItem, rightArmOrient) != FireWeaponType::NoAmmo)
					{
						lara->RightArm.GunSmoke = 28;

						TriggerGunShell(1, ID_GUNSHELL, weaponType); // Right Hand

						lara->RightArm.GunFlash = weapon->FlashTime;

						SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 0.9f, 0.3f);
						SoundEffect(weapon->SampleNum, &laraItem->Pose);
						fired = true;

						if (weaponType == LaraWeaponType::Uzi)
							lara->Control.Weapon.UziRight = true;

						Statistics.Game.AmmoUsed++;
						MagazineCounts[(int)weaponType][1]--;

						if (MagazineCounts[(int)weaponType][1] <= 0) {
							MagazineCounts[(int)weaponType][1] = DefaultMagazineCounts[(int)weaponType];
							//GetCreatureInfo(lara)
							//SetAnimation(lara, )
							doReloadRight = true;
						}
					}
				}

				// go to (3) SHOOT_CONTINUE start frame
				frameRight = p->RecoilAnim;

				if (doReloadRight) {
					frameRight = p->ReloadAnim;
					doReloadRight = false;
				}
			}
			else if (lara->Control.Weapon.UziRight)
			{
				SoundEffect(weapon->SampleNum + 1, &laraItem->Pose);
				lara->Control.Weapon.UziRight = false;
			}
		}
		// at or beyond (3) SHOOT_CONTINUE start frame
		else if (frameRight >= p->RecoilAnim && frameRight < p->ReloadAnim)
		{
			if (weaponType == LaraWeaponType::Uzi)
			{
				SoundEffect(weapon->SampleNum, &laraItem->Pose);
				lara->Control.Weapon.UziRight = true;
			}

			// increment toward (3) SHOOT_CONTINUE end frame (finish recoil before allowing to shoot again)
			frameRight++;

			// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
			if (frameRight == (p->RecoilAnim + weapon->RecoilFrame))
				frameRight = p->Draw1Anim2;
		}
		else if (frameRight >= p->ReloadAnim) {
			// increment toward (3) SHOOT_CONTINUE end frame (finish recoil before allowing to shoot again)
			frameRight++;

			// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
			if (frameRight == (p->RecoilAnim + 23))
				frameRight = p->Draw1Anim2;
		}
	}
	// HAS LET GO OF ACTION
	else
	{
		// let (3) SHOOT_CONTINUE finish
		if ((frameRight >= p->RecoilAnim) && (frameRight < p->RecoilAnim + weapon->RecoilFrame))
			frameRight++;
		// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
		if (frameRight == (p->RecoilAnim + weapon->RecoilFrame))
			frameRight = p->Draw1Anim2;
		// go back to ready stance
		else if ((frameRight > 0) && (frameRight <= p->Draw1Anim2))
			frameRight--;
		else if (frameRight >= p->ReloadAnim && frameRight <= p->ReloadAnim + 23)
			frameRight++;
		else
			frameRight = p->Draw1Anim2;

		if (lara->Control.Weapon.UziRight)
		{
			SoundEffect(weapon->SampleNum + 1, &laraItem->Pose);
			lara->Control.Weapon.UziRight = false;
		}
	}

	SetArmInfo(laraItem, lara->RightArm, frameRight);

	// Shooting for left arm.
	int frameLeft = lara->LeftArm.FrameNumber;
	if ((TrInput & IN_ACTION && !lara->TargetEntity) || lara->LeftArm.Locked)
	{
		if ((frameLeft >= 0) && (frameLeft < p->Draw1Anim2))
			frameLeft++;
		else if (frameLeft == p->Draw1Anim2)
		{
			if (TrInput & IN_ACTION)
			{
				auto leftArmOrient = EulerAngles(
					lara->LeftArm.Orientation.x,
					lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
					0
				);

				if (FireWeapon(weaponType, lara->TargetEntity, laraItem, leftArmOrient) != FireWeaponType::NoAmmo)
				{
					if (weaponType == LaraWeaponType::Revolver)
					{
						lara->RightArm.GunSmoke = 28;
						lara->RightArm.GunFlash = weapon->FlashTime;
					}
					else
					{
						lara->LeftArm.GunSmoke = 28;
						TriggerGunShell(0, ID_GUNSHELL, weaponType); // Left hand
						lara->LeftArm.GunFlash = weapon->FlashTime;
					}

					if (!fired)
					{
						SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 0.9f, 0.3f);
						SoundEffect(weapon->SampleNum, &laraItem->Pose);
						fired = true;
					}

					if (weaponType == LaraWeaponType::Uzi)
						lara->Control.Weapon.UziLeft = true;

					Statistics.Game.AmmoUsed++;

					MagazineCounts[(int)weaponType][0]--;

					if (MagazineCounts[(int)weaponType][0] <= 0) {
						MagazineCounts[(int)weaponType][0] = DefaultMagazineCounts[(int)weaponType];
						//GetCreatureInfo(lara)
						//SetAnimation(lara, )
						doReloadLeft = true;
					}
				}

				frameLeft = p->RecoilAnim;

				if (doReloadLeft) {
					frameLeft = p->ReloadAnim;
					doReloadLeft = false;
				}
			}
			else if (lara->Control.Weapon.UziLeft)
			{
				SoundEffect(weapon->SampleNum + 1, &laraItem->Pose);
				lara->Control.Weapon.UziLeft = false;
			}
		}
		else if (frameLeft >= p->RecoilAnim && frameLeft <= p->RecoilAnim)
		{
			if (weaponType == LaraWeaponType::Uzi)
			{
				SoundEffect(weapon->SampleNum, &laraItem->Pose);
				lara->Control.Weapon.UziLeft = true;
			}

			frameLeft++;

			if (frameLeft == (p->RecoilAnim + weapon->RecoilFrame))
				frameLeft = p->Draw1Anim2;
		}
		else if (frameRight >= p->ReloadAnim) {
			// increment toward (3) SHOOT_CONTINUE end frame (finish recoil before allowing to shoot again)
			frameRight++;

			// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
			if (frameRight == (p->RecoilAnim + 23))
				frameRight = p->Draw1Anim2;
		}
	}
	else       																// Havent GOT a LOCK ON..
	{
		if ((frameLeft >= p->RecoilAnim) && (frameLeft < p->RecoilAnim + weapon->RecoilFrame))
			frameLeft++;
		if (frameLeft == (p->RecoilAnim + weapon->RecoilFrame))
			frameLeft = p->Draw1Anim2;
		else if ((frameLeft > 0) && (frameLeft <= p->Draw1Anim2))
			frameLeft--;
		else if (frameLeft >= p->ReloadAnim && frameLeft <= p->ReloadAnim + 23)
			frameLeft++;
		else
			frameLeft = p->Draw1Anim2;

		if (lara->Control.Weapon.UziLeft)
		{
			SoundEffect(weapon->SampleNum + 1, &laraItem->Pose);
			lara->Control.Weapon.UziLeft = false;
		}
	}

	if (fired) // Rumble gamepad only once if any of the hands fired.
	{
		float power = weaponType == LaraWeaponType::Uzi ? GenerateFloat(0.1f, 0.3f) : 1.0f;
		Rumble(power, 0.1f);
	}

	SetArmInfo(laraItem, lara->LeftArm, frameLeft);
}

void PistolHandler(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto& weapon = Weapons[(int)weaponType];

	FindNewTarget(laraItem, weapon);
	if (TrInput & IN_ACTION)
		LaraTargetInfo(laraItem, weapon);

	AimWeapon(laraItem, lara->LeftArm, weapon);
	AimWeapon(laraItem, lara->RightArm, weapon);

	if (lara->LeftArm.Locked && !lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.x = lara->LeftArm.Orientation.x / 2;
		lara->ExtraTorsoRot.y = lara->LeftArm.Orientation.y / 2;

		if (Camera.oldType != CameraType::Look)
			lara->ExtraHeadRot = lara->ExtraTorsoRot;
	}
	else if (!lara->LeftArm.Locked && lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.x = lara->RightArm.Orientation.x / 2;
		lara->ExtraTorsoRot.y = lara->RightArm.Orientation.y / 2;

		if (Camera.oldType != CameraType::Look)
			lara->ExtraHeadRot = lara->ExtraTorsoRot;
	}
	else if (lara->LeftArm.Locked && lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.x = (lara->LeftArm.Orientation.x + lara->RightArm.Orientation.x) / 4;
		lara->ExtraTorsoRot.y = (lara->LeftArm.Orientation.y + lara->RightArm.Orientation.y) / 4;

		if (Camera.oldType != CameraType::Look)
			lara->ExtraHeadRot = lara->ExtraTorsoRot;
	}

	AnimatePistols(laraItem, weaponType);
	
	if (lara->LeftArm.GunFlash || lara->RightArm.GunFlash)
	{
		auto pos = GetJointPosition(laraItem, 
			(lara->LeftArm.GunFlash != 0) ? LM_LHAND : LM_RHAND,
			Vector3i(
				(byte)GetRandomControl() - 128,
				(GetRandomControl() & 0x7F) - 63,
				(byte)GetRandomControl() - 128
			));
		TriggerDynamicLight(pos.x+GenerateFloat(-128,128), pos.y + GenerateFloat(-128, 128), pos.z + GenerateFloat(-128, 128), GenerateFloat(8,11), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, GetRandomControl() & 0x3F);
	}
}

void ReadyPistols(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->Control.HandStatus = HandStatus::WeaponReady;
	lara->LeftArm.Orientation = EulerAngles::Zero;
	lara->RightArm.Orientation = EulerAngles::Zero;
	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->TargetEntity = nullptr;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
	lara->LeftArm.FrameBase = Objects[GetWeaponObjectID(weaponType)].frameBase;
	lara->RightArm.FrameBase = Objects[GetWeaponObjectID(weaponType)].frameBase;
}

void DrawPistols(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* p = &PistolsTable[(int)lara->Control.Weapon.GunType];

	int frame = lara->LeftArm.FrameNumber + 1;

	if (frame < p->Draw1Anim || frame > p->RecoilAnim - 1)
		frame = p->Draw1Anim;
	else if (frame == p->Draw2Anim)
	{
		DrawPistolMeshes(laraItem, weaponType);
		SoundEffect(SFX_TR4_LARA_DRAW, &laraItem->Pose);
	}
	else if (frame == p->RecoilAnim - 1)
	{
		ReadyPistols(laraItem, weaponType);
		frame = 0;
	}

	SetArmInfo(laraItem, lara->RightArm, frame);
	SetArmInfo(laraItem, lara->LeftArm, frame);
}

void UndrawPistols(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* weapon = &Weapons[(int)weaponType];
	auto* p = &PistolsTable[(int)lara->Control.Weapon.GunType];

	int frameLeft = lara->LeftArm.FrameNumber;

	// Finish recoil anim before reholstering weapon.
	if ((frameLeft >= p->RecoilAnim) && (frameLeft < p->RecoilAnim + weapon->RecoilFrame))
		frameLeft++;

	if (frameLeft == (p->RecoilAnim + weapon->RecoilFrame))
		frameLeft = p->Draw1Anim2;
	else if (frameLeft > 0 && frameLeft < p->Draw1Anim)
	{
		lara->LeftArm.Orientation.x -= lara->LeftArm.Orientation.x / frameLeft;
		lara->LeftArm.Orientation.y -= lara->LeftArm.Orientation.y / frameLeft;
		frameLeft--;
	}
	else if (frameLeft == 0)
	{
		lara->LeftArm.Orientation = EulerAngles::Zero;
		frameLeft = p->RecoilAnim - 1;
	}
	else if (frameLeft > p->Draw1Anim && (frameLeft < p->RecoilAnim))
	{
		frameLeft--;

		if (frameLeft == p->Draw2Anim - 1)
		{
			UndrawPistolMeshLeft(laraItem, weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &laraItem->Pose);
		}
	}

	SetArmInfo(laraItem, lara->LeftArm, frameLeft);

	int frameRight = lara->RightArm.FrameNumber;

	if ((frameRight >= p->RecoilAnim) && (frameRight < p->RecoilAnim + weapon->RecoilFrame))
		frameRight++;

	if (frameRight == (p->RecoilAnim + weapon->RecoilFrame))
		frameRight = p->Draw1Anim2;
	else if (frameRight > 0 && frameRight < p->Draw1Anim)
	{
		lara->RightArm.Orientation.x -= lara->RightArm.Orientation.x / frameRight;
		lara->RightArm.Orientation.y -= lara->RightArm.Orientation.y / frameRight;
		frameRight--;
	}
	else if (frameRight == 0)
	{
		lara->RightArm.Orientation = EulerAngles::Zero;
		frameRight = p->RecoilAnim - 1;
	}
	else if (frameRight > p->Draw1Anim && (frameRight < p->RecoilAnim))
	{
		frameRight--;

		if (frameRight == p->Draw2Anim - 1)
		{
			UndrawPistolMeshRight(laraItem, weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &laraItem->Pose);
		}
	}

	SetArmInfo(laraItem, lara->RightArm, frameRight);

	if (frameLeft == p->Draw1Anim && frameRight == p->Draw1Anim)
	{
		lara->Control.HandStatus = HandStatus::Free;
		lara->LeftArm.FrameNumber = 0;
		lara->RightArm.FrameNumber = 0;
		lara->TargetEntity = nullptr;
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
	}

	if (!(TrInput & IN_LOOK))
	{
		lara->ExtraHeadRot = (lara->LeftArm.Orientation + lara->RightArm.Orientation) / 4;
		lara->ExtraTorsoRot = (lara->LeftArm.Orientation + lara->RightArm.Orientation) / 4;
	}
}

void SetArmInfo(ItemInfo* laraItem, ArmInfo& arm, int frame)
{
	auto* lara = GetLaraInfo(laraItem);
	const auto& pistols = PistolsTable[(int)lara->Control.Weapon.GunType];

	int animBase = Objects[(int)pistols.ObjectNumber].animIndex;

	if (frame < pistols.Draw1Anim)
		arm.AnimNumber = animBase;
	else if (frame < pistols.Draw2Anim)
		arm.AnimNumber = animBase + 1;
	else if (frame < pistols.RecoilAnim)
		arm.AnimNumber = animBase + 2;
	else
		arm.AnimNumber = animBase + 3;

	arm.FrameNumber = frame;
	arm.FrameBase = g_Level.Anims[arm.AnimNumber].FramePtr;
}

void DrawPistolMeshes(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	if (weaponType != LaraWeaponType::Revolver)
		lara->Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Empty;

	lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;

	laraItem->Model.MeshIndex[LM_RHAND] = Objects[GetWeaponObjectMeshID(laraItem, weaponType)].meshIndex + LM_RHAND;
	if (weaponType != LaraWeaponType::Revolver)
		laraItem->Model.MeshIndex[LM_LHAND] = Objects[GetWeaponObjectMeshID(laraItem, weaponType)].meshIndex + LM_LHAND;
}

void UndrawPistolMeshRight(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	laraItem->Model.MeshIndex[LM_RHAND] = laraItem->Model.BaseMesh + LM_RHAND;
	if (lara->Weapons[(int)weaponType].Present)
		lara->Control.Weapon.HolsterInfo.RightHolster = GetWeaponHolsterSlot(weaponType);
	else
		lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;
}

void UndrawPistolMeshLeft(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	if (weaponType != LaraWeaponType::Revolver)
	{
		laraItem->Model.MeshIndex[LM_LHAND] = laraItem->Model.BaseMesh + LM_LHAND;

		if (lara->Weapons[(int)weaponType].Present)
			lara->Control.Weapon.HolsterInfo.LeftHolster = GetWeaponHolsterSlot(weaponType);
		else
			lara->Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Empty;
	}
}
