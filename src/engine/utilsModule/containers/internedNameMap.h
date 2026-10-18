#pragma once
#include "flatMap.h"
#include "utilsModule/stringPool.h"

namespace IKIGAI::UTILS {
// Small symbol lookup table. Copies share the string domain, but own their maps.
// The pool is scoped to the consumer (e.g. shader reflection), not process-global.
template <class Value> class InternedNameMap {
public:
	InternedNameMap()
		: InternedNameMap(std::make_shared<SynchronizedStringPool>()) { }
	explicit InternedNameMap(std::shared_ptr<SynchronizedStringPool> pool,
		std::shared_ptr<MEMORY::IAllocator> domain = MEMORY::BufferAllocator<MEMORY::Category::General>())
		: mPool(std::move(pool))
		, mValues(typename decltype(mValues)::allocator_type(std::move(domain))) {
		if (!mPool)
			throw std::invalid_argument("Null name pool");
	}
	// Copy the pool handle even on move: moved-from maps remain reusable.
	InternedNameMap(const InternedNameMap&) = default;
	InternedNameMap& operator=(const InternedNameMap& other) {
		if (this != &other) {
			InternedNameMap copy(other);
			mValues.swap(copy.mValues);
			mPool.swap(copy.mPool);
		}
		return *this;
	}
	InternedNameMap(InternedNameMap&& other)
		: mPool(other.mPool)
		, mValues(std::move(other.mValues)) { }
	InternedNameMap& operator=(InternedNameMap&& other) {
		mValues = std::move(other.mValues);
		mPool = other.mPool;
		return *this;
	}
	Identifier identifier(std::string_view name) const {
		return mPool->find(name);
	}
	std::string_view name(Identifier id) const {
		return mPool->view(id);
	}
	Value& operator[](std::string_view name) {
		return mValues[mPool->intern(name)];
	}
	const Value& at(std::string_view name) const {
		return at(identifier(name));
	}
	Value& at(std::string_view name) {
		return at(identifier(name));
	}
	const Value& at(Identifier id) const {
		return mValues.at(id);
	}
	Value& at(Identifier id) {
		return mValues.at(id);
	}
	bool contains(std::string_view name) const {
		return contains(identifier(name));
	}
	bool contains(Identifier id) const {
		return id.valid() && mValues.contains(id);
	}
	std::size_t size() const noexcept {
		return mValues.size();
	}
	bool empty() const noexcept {
		return mValues.empty();
	}
	void clear() noexcept {
		mValues.clear();
	}
	void reserve(std::size_t n) {
		mValues.reserve(n);
	}

private:
	std::shared_ptr<SynchronizedStringPool> mPool;
	FlatMap<Identifier, Value> mValues;
};
}
