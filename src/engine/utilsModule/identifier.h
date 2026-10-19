#pragma once
#include "idGenerator.h"

namespace IKIGAI::UTILS {
struct StringEntryTag;
template <class Hash> class BasicStringPool;
// Process-local symbol: identity includes the pool. Serialize its string, not this
// value. Does not own or prolong the lifetime of its pool.
class Identifier {
	template <class> friend class BasicStringPool;
	constexpr Identifier(std::uint32_t pool, Id<StringEntryTag> entry) noexcept
		: mPool(pool)
		, mEntry(entry) { }

public:
	constexpr Identifier() noexcept = default;
	constexpr bool valid() const noexcept {
		return mPool != 0 && mEntry.valid();
	}
	static constexpr Identifier invalid() noexcept {
		return { };
	}
	constexpr std::uint32_t poolToken() const noexcept {
		return mPool;
	}
	constexpr Id<StringEntryTag> entry() const noexcept {
		return mEntry;
	}
	constexpr auto operator<=>(const Identifier&) const noexcept = default;

private:
	std::uint32_t mPool = 0;
	Id<StringEntryTag> mEntry;
};
static_assert(sizeof(Identifier) == 8);
}
template <> struct std::hash<IKIGAI::UTILS::Identifier> {
	std::size_t operator()(IKIGAI::UTILS::Identifier id) const noexcept {
		return std::hash<std::uint64_t> { }((std::uint64_t { id.poolToken() } << 32) | id.entry().value());
	}
};
