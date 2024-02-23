#pragma once
#include <iostream>

class Vector2
{
	public:
	double x, y;

	public:
	Vector2(double _x, double _y)
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

	Vector2 operator*(double scalar) const
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

	double magnitude() const
	{
		return sqrt(x * x + y * y);
	}

	Vector2 normalize() const
	{
		double mag = magnitude();
		if (mag != 0)
			return (*this) * (1 / mag);

		else
			return *this;
	}

	~Vector2() {}
};
