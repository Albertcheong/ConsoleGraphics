#pragma once

template <typename type>
type clamp(const type& value, const type& min, const type& max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

bool compare_y_axis(const POINT& a, const POINT& b)
{
	return a.y < b.y;
}

