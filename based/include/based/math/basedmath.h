#pragma once

#include <cmath>

namespace based::math
{
	static float Deg2Rad = 0.0174533f;

	static float Rad2Deg = 57.2958f;

	static float PI = 3.1415926f;

	static float Abs(float a)
	{
		return abs(a);
	}

	static float Acos(float a)
	{
		return acos(a);
	}

	static float Asin(float a)
	{
		return asin(a);
	}

	static float Atan(float a)
	{
		return atan(a);
	}

	static float Atan2(float y, float x)
	{
		return atan2(y, x);
	}

	static float Ceil(float v)
	{
		return ceil(v);
	}

	static float Clamp(float value, float min, float max)
	{
		return (value <= min) ? min : (value >= max) ? max : value;
	}

	static float Clamp01(float value, float min, float max)
	{
		return Clamp(value, 0, 1);
	}

	/**
	 * \brief Finds the next power of two for an integer.
	 * \param v The integer to find the next power of two for.
	 * \return The next power of two of v.
	 *
	 * This code is directly copy/pasted from:
	 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	*/
	static unsigned int NextPowerOfTwo(unsigned int v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}

	static float Cos(float a)
	{
		return cos(a);
	}

	static float Exp(float power)
	{
		return exp(power);
	}

	static float Floor(float v)
	{
		return floor(v);
	}

	static float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	static float InverseLerp(float value, float a, float b)
	{
		return (value - a) / (b - a);
	}

	static bool IsPowerOfTwo(unsigned int v)
	{
		return (v != 0) && ((v & (v - 1)) == 0);
	}

	static float Log(float base, float x)
	{
		return (log(x) / log(base));
	}

	static float Log10(float x)
	{
		return log(x);
	}

	static float Max(float a, float b)
	{
		return (a > b) ? a : b;
	}

	static float Min(float a, float b)
	{
		return (a < b) ? a : b;
	}

	static float Pow(float value, float power)
	{
		return pow(value, power);
	}

	static float Round(float v)
	{
		return round(v);
	}

	static float Sin(float a)
	{
		return sin(a);
	}

	/**
	* Implementation by AMD, taken from https://en.wikipedia.org/wiki/Smoothstep
	*/
	static float SmoothStep(float edge0, float edge1, float x)
	{
		if (x < edge0)
			return 0;

		if (x >= edge1)
			return 1;

		// Scale/bias into [0..1] range
		x = (x - edge0) / (edge1 - edge0);

		return x * x * (3 - 2 * x);
	}

	static float Sqrt(float v)
	{
		return sqrt(v);
	}

	static float Tan(float a)
	{
		return tan(a);
	}
}