#include "framework.h"
#include "GameScriptAnimations.h"

/***
New custom animations which Lara can perform.
@pregameclass Animations
@pragma nostrip
*/

void GameScriptAnimations::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptAnimations>("Animations",
		"crawlExtended", &GameScriptAnimations::CrawlExtended,
		"crouchRoll", &GameScriptAnimations::CrouchRoll,
		"crawlspaceSwandive", &GameScriptAnimations::CrawlspaceSwandive,
		"monkeyReverse", &GameScriptAnimations::MonkeyReverse,
		"monkeyAutoJump", &GameScriptAnimations::MonkeyAutoJump,
		"oscillateHang", &GameScriptAnimations::OscillateHang,
		"pose", &GameScriptAnimations::Pose
		);
}