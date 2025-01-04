#pragma once

// Taken from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/UUID.h

namespace based::core
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<based::core::UUID>
	{
		std::size_t operator()(const based::core::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};
}
