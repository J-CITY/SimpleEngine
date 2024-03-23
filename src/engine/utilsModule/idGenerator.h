#pragma once
#include <cstddef>
#include <functional>

namespace IKIGAI {
	template<typename T>
	class Id {
	public:
		explicit Id(unsigned int id) noexcept : mId(id) {}

		explicit operator unsigned int() const noexcept {
			return mId;
		}

		explicit operator int() const noexcept {
			return static_cast<int>(mId);
		}

		explicit operator size_t() const noexcept {
			return static_cast<size_t>(mId);
		}

		bool operator<(const Id& b) const noexcept {
			return mId < b.mId;
		}

		bool operator==(const Id& b) const noexcept {
			return mId == b.mId;
		}

		bool operator!=(const Id& b) const noexcept {
			return mId != b.mId;
		}

	private:
		int mId;
	};

	template<typename T>
	class IdGenerator {
	public:
		using id = Id<T>;

		IdGenerator() : mId(generateId()) {}

		id getUniqueId() const noexcept {
			return mId;
		}

		static id generateId() noexcept {
			static int current_id = 0;
			return id(current_id++);
		}

	protected:
		id mId;
	};

}

namespace std {
	template<typename T>
	struct hash<IKIGAI::Id<T>> {
		std::size_t operator()(const IKIGAI::Id<T>& k) const {
			using std::size_t;
			using std::hash;
			return (hash<int>()(static_cast<int>(k)));
		}
	};
}
