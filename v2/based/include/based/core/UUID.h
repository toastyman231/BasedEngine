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
