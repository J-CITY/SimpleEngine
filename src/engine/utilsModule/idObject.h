#pragma once

namespace IKIGAI {
	template<typename T>
	class ObjectId {
	public:
		explicit ObjectId(unsigned int id) noexcept : m_id(id) {}

		explicit operator unsigned int() const noexcept {
			return m_id;
		}

		explicit operator int() const noexcept {
			return static_cast<int>(m_id);
		}

		explicit operator size_t() const noexcept {
			return static_cast<size_t>(m_id);
		}

		bool operator<(const ObjectId& b) const noexcept {
			return m_id < b.m_id;
		}

		bool operator==(const ObjectId& b) const noexcept {
			return m_id == b.m_id;
		}

		bool operator!=(const ObjectId& b) const noexcept {
			return m_id != b.m_id;
		}

	private:
		unsigned int m_id;
	};

	template<typename T>
	class ObjectIdGenerator {
	public:
		using id = ObjectId<T>;

		ObjectIdGenerator() : m_id(generateId()) {}

		id getUniqueId() const noexcept {
			return m_id;
		}

		static id generateId() noexcept {
			static int current_id = 0;
			return id(current_id++);
		}

	protected:
		id m_id;
	};

}

namespace std {
	template<typename T>
	struct hash<IKIGAI::ObjectId<T>> {
		std::size_t operator()(const IKIGAI::ObjectId<T>& k) const {
			using std::size_t;
			using std::hash;

			return (hash<int>()(static_cast<int>(k)));
		}
	};
}
