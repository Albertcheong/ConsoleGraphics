#pragma once
#include <iostream>

template <typename type>
class Vector2
{
	public:
	Vector2(type _x, type _y)
		: x(_x), y(_y) {}

	Vector2()
		: x(0.0), y(0.0) {}

	Vector2(const Vector2& other)
		: x(other.x), y(other.y) {}

	//Vector2* operator=(const Vector2& other)
	//{
	//	if (this != &other)
	//	{
	//		x = other.x;
	//		y = other.y;
	//	}
	//	return this; // Returning a pointer
	//}              // technically ini bisa tp hrus didereference by itself

	Vector2& operator=(const Vector2& other)
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
		}
		return *this; // returning the object itself as a reference
	}

	Vector2 operator+(const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator-(const Vector2& other) const
	{
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 operator*(type scalar) const
	{
		return Vector2(x * scalar, y * scalar);
	}

	Vector2& operator+=(const Vector2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	Vector2& operator-=(const Vector2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	type magnitude() const
	{
		return sqrt(x * x + y * y);
	}

	Vector2 normalize() const
	{
		type mag = magnitude();
		if (mag != 0)
			return (*this) * (1 / mag);

		else
			return *this;
	}

	~Vector2() {}

	public:
	type x, y;
};
