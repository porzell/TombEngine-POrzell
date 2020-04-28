#pragma once

#include "vodoo.h"

#define PI 3.14159265358979323846f
#define RADIAN 0.01745329252f

#define rcossin_tbl	ARRAY_(0x0050B46C, short, [8192])

#define SQUARE(x) ((x)*(x))
#define CLAMP(x, a, b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define SIGN(x) ((0 < (x)) - ((x) < 0))
#define CLAMPADD(x, a, b) ((x)<(a)?((x)+(a)):((x)>(b)?((x)-(b)):0))

#define ONE_DEGREE 182
#define CLICK(x) ((x) * STEP_SIZE)
#define SECTOR(x) ((x) * WALL_SIZE)

#define SQRT_ASM ((int(__cdecl*)(int)) 0x0048F980)
#define ATAN ((int(__cdecl*)(int, int)) 0x0048F8A0)
#define SIN(x) (4 * rcossin_tbl[(int(x) >> 3) & 8190])
#define COS(x) (4 * rcossin_tbl[((int(x) >> 3) & 8190) + 1])

short ANGLE(float angle);
float TR_ANGLE_TO_DEGREES(short angle);
float TR_ANGLE_TO_RAD(short angle);
// returns a float between 0-1
const float frand();

const float frandMinMax(float min, float max);

const float lerp(float v0, float v1, float t);