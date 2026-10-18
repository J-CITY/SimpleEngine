#pragma once
#include "containers/chunkedVector.h"

namespace IKIGAI::UTILS {
// Compatibility facade; CONT remains only for existing declarations.
// Broken middle insertion is intentionally not part of this API.
template<class T, template<class, class> class CONT = std::vector, std::size_t ChunkSize = 100>
class ChunkList : public ChunkedVector<T, ChunkSize, MEMORY::StlAllocator<T, MEMORY::Category::Component>> {
    using Base = ChunkedVector<T, ChunkSize, MEMORY::StlAllocator<T, MEMORY::Category::Component>>;
public:
    using Base::Base;
    using Iterator = typename Base::iterator;
    T&& moveFrom(std::size_t index) { return std::move(this->at(index)); }
    void moveTo(std::size_t index, T&& value) { this->at(index) = std::move(value); }
};
}
