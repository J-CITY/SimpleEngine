#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <functional>

namespace IKIGAI::ECS2 {

	class Signature {
	public:
		using Word = uint64_t;
		static constexpr size_t WORD_SIZE = 64;

		Signature() = default;

		void set(size_t pos, bool val = true) {
			size_t wordIdx = pos / WORD_SIZE;
			size_t bitIdx = pos % WORD_SIZE;

			if (wordIdx >= mBits.size()) {
				if (val) {
					mBits.resize(wordIdx + 1, 0);
				}
				else {
					return; // Unsetting a bit outside range is no-op
				}
			}

			if (val) {
				mBits[wordIdx] |= (1ULL << bitIdx);
			}
			else {
				mBits[wordIdx] &= ~(1ULL << bitIdx);
			}

			// Trim trailing zeros if we unset the last bit
			if (!val && wordIdx == mBits.size() - 1 && mBits[wordIdx] == 0) {
				trim();
			}
		}

		[[nodiscard]] bool test(size_t pos) const {
			size_t wordIdx = pos / WORD_SIZE;
			size_t bitIdx = pos % WORD_SIZE;

			if (wordIdx >= mBits.size()) {
				return false;
			}

			return (mBits[wordIdx] & (1ULL << bitIdx)) != 0;
		}

		// Alias for test for bitset compatibility
		bool operator[](size_t pos) const {
			return test(pos);
		}

		void reset() {
			mBits.clear();
		}

		bool operator==(const Signature& other) const {
			return mBits == other.mBits;
		}

		bool operator!=(const Signature& other) const {
			return !(*this == other);
		}

		Signature& operator&=(const Signature& other) {
			size_t commonSize = std::min(mBits.size(), other.mBits.size());
			for (size_t i = 0; i < commonSize; ++i) {
				mBits[i] &= other.mBits[i];
			}
			if (mBits.size() > commonSize) {
				mBits.resize(commonSize); // Trailing bits become 0 effectively
				trim();
			}
			return *this;
		}

		Signature& operator|=(const Signature& other) {
			if (other.mBits.size() > mBits.size()) {
				mBits.resize(other.mBits.size(), 0);
			}
			for (size_t i = 0; i < other.mBits.size(); ++i) {
				mBits[i] |= other.mBits[i];
			}
			return *this;
		}

		Signature operator&(const Signature& other) const {
			Signature res = *this;
			res &= other;
			return res;
		}

		Signature operator|(const Signature& other) const {
			Signature res = *this;
			res |= other;
			return res;
		}

		// Check if this signature contains all bits set in other (subset check)
		// equivalent to (this & other) == other
		bool contains(const Signature& other) const {
			if (other.mBits.size() > mBits.size()) {
				// If other has more words, check if those extra words are non-zero.
				// If they are, then this cannot contain other.
				for (size_t i = mBits.size(); i < other.mBits.size(); ++i) {
					if (other.mBits[i] != 0) return false;
				}
			}

			size_t checkSize = std::min(mBits.size(), other.mBits.size());
			for (size_t i = 0; i < checkSize; ++i) {
				if ((mBits[i] & other.mBits[i]) != other.mBits[i]) {
					return false;
				}
			}
			return true;
		}

		bool empty() const {
			return mBits.empty();
		}

		size_t hash() const {
			size_t seed = 0;
			for (auto word : mBits) {
				// Combine hash using a common pattern (e.g. from boost::hash_combine)
				// seed ^= hash(word) + 0x9e3779b9 + (seed<<6) + (seed>>2)
				seed ^= std::hash<Word>{}(word)+0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}

	private:
		void trim() {
			while (!mBits.empty() && mBits.back() == 0) {
				mBits.pop_back();
			}
		}

		std::vector<Word> mBits;
	};
}

namespace std {
	template<>
	struct hash<IKIGAI::ECS2::Signature> {
		size_t operator()(const IKIGAI::ECS2::Signature& s) const {
			return s.hash();
		}
	};
}
