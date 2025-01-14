#include "framework.h"
#include "Math/Objects/Vector2i.h"

namespace TEN::Math
{
	const Vector2i Vector2i::Zero = Vector2i(0, 0);

	Vector2i::Vector2i(const Vector2& vector)
	{
		x = (int)round(vector.x);
		y = (int)round(vector.y);
	}

	float Vector2i::Distance(const Vector2i& origin, const Vector2i& target)
	{
		return Vector2::Distance(origin.ToVector2(), target.ToVector2());
	}
	
	float Vector2i::DistanceSquared(const Vector2i& origin, const Vector2i& target)
	{
		return Vector2::DistanceSquared(origin.ToVector2(), target.ToVector2());
	}

	Vector2 Vector2i::ToVector2() const
	{
		return Vector2(x, y);
	}

	bool Vector2i::operator ==(const Vector2i& vector) const
	{
		return ((x == vector.x) && (y == vector.y));
	}

	bool Vector2i::operator !=(const Vector2i& vector) const
	{
		return !(*this == vector);
	}

	Vector2i& Vector2i::operator =(const Vector2i& vector)
	{
		x = vector.x;
		y = vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator +=(const Vector2i& vector)
	{
		x += vector.x;
		y += vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator -=(const Vector2i& vector)
	{
		x -= vector.x;
		y -= vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator *=(const Vector2i& vector)
	{
		x *= vector.x;
		y *= vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator *=(float scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	Vector2i& Vector2i::operator /=(float scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}

	Vector2i Vector2i::operator +(const Vector2i& vector) const
	{
		return Vector2i(x + vector.x, y + vector.y);
	}

	Vector2i Vector2i::operator -(const Vector2i& vector) const
	{
		return Vector2i(x - vector.x, y - vector.y);
	}

	Vector2i Vector2i::operator *(const Vector2i& vector) const
	{
		return Vector2i(x * vector.x, y * vector.y);
	}

	Vector2i Vector2i::operator *(float scale) const
	{
		return Vector2i((int)round(x * scale), (int)round(y * scale));
	}

	Vector2i Vector2i::operator /(float scale) const
	{
		return Vector2i((int)round(x / scale), (int)round(y / scale));
	}
}
