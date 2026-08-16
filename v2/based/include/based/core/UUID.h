#pragma once

namespace based
{
    class UUID final : public ExplicitlyCopyable
    {
    public:
        UUID();
        UUID(uint64 nUUID) : m_nUUID(nUUID) {}
        UUID(const UUID&) = default;

        operator uint64() const { return m_nUUID; }
    private:
        uint64 m_nUUID;
    };
}

namespace std
{
    template <>
    struct hash<based::UUID>
    {
        size_t operator()(const based::UUID& uuid) const noexcept
        {
            return std::hash<uint64>{}(uuid);
        }
    };
}
