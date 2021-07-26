#pragma once

#include "GameScriptNamedBase.h"
#include "phd_global.h"

namespace sol {
	class state;
}
class GameScriptPosition;

class GameScriptSoundSourceInfo : public GameScriptNamedBase<GameScriptSoundSourceInfo, SOUND_SOURCE_INFO &>
{
public:
	GameScriptSoundSourceInfo(SOUND_SOURCE_INFO& ref, bool temp);
	~GameScriptSoundSourceInfo();
	static void Register(sol::state *);
	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

	int GetSoundID() const;
	void SetSoundID(int soundID);

	int GetFlags() const;
	void SetFlags(int flags);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	SOUND_SOURCE_INFO & m_soundSource;
	bool m_temporary;
};
