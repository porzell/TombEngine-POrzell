#pragma once

#include "Specific/RGBAColor8Byte.h"

enum class WeatherType
{
	None,
	Rain,
	Snow
};

// todo make this a scoped enum
enum LaraType
{
	Normal = 1,
	Young = 2,
	Bunhead = 3,
	Catsuit = 4,
	Divesuit = 5,
	Invisible = 7
};

class ScriptInterfaceLevel {
public:
	virtual ~ScriptInterfaceLevel() = default;

	virtual bool GetSkyLayerEnabled(int index) const = 0;
	virtual short GetSkyLayerSpeed(int index) const = 0;
	virtual LaraType GetLaraType() const = 0;
	virtual bool HasStorm() const = 0;
	virtual float GetWeatherStrength() const = 0;
	virtual WeatherType GetWeatherType() const = 0;
	virtual RGBAColor8Byte GetSkyLayerColor(int index) const = 0;
};
