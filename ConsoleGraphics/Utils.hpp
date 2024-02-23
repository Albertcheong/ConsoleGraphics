#pragma once

template <typename type>
type clamp(const type& value, const type& min, const type& max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}