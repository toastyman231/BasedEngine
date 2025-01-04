#include "pch.h"
#include "uuid.h"

// Taken from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/UUID.cpp

namespace based::core
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	UUID::UUID()
		: m_UUID(s_UniformDistribution(s_Engine))
	{
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
	}
}
