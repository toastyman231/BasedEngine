#pragma once

#include <cstdlib>

namespace based::math
{
	static int RandomRange(int min, int max)
	{
		int range = max - min + 1;
		return rand() % range + min;
	}

	static float RandomRange(float min, float max)
	{
		// From StackOverflow by John Dibling and Florin Mircea
		return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	}

	static float Random01()
	{
		return (float)rand() / RAND_MAX;
	}
}