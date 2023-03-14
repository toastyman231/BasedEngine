#pragma once

#include <cstdlib>

namespace based::math
{
	static int RandomRange(int min, int max)
	{
		int range = max - min + 1;
		return rand() % range + min;
	}

	static float Random01()
	{
		return (float)rand() / RAND_MAX;
	}
}