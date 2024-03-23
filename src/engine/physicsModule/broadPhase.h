#pragma once

#include <stack>
#include <vector>
#include <renderModule/frustum.h>

namespace IKIGAI
{
	namespace RENDER
	{
		struct BoundingSphere;
	}
}
namespace IKIGAI
{
    namespace PHYSICS
    {
	    class RigidBody;

		template <typename RigidBody>
		struct PotentialContact {
			RigidBody m_body[2];
		};

		template <typename BoundingVolumeClass, typename RigidBody>
		class BVHNode {
		public:
			bool IsLeaf() const {
				return m_body != nullptr;
			}

			bool Overlaps(const BVHNode<BoundingVolumeClass, RigidBody>* other) const;
			unsigned int GetPotentialContacts(PotentialContact<RigidBody>* contacts, unsigned int limit) const;
			unsigned int GetPotentialContactsWith(const BVHNode<BoundingVolumeClass, RigidBody>* other, PotentialContact<RigidBody>* contacts, unsigned int limit) const;
			void Insert(RigidBody body, const BoundingVolumeClass& volume);

			void GetNodeToDraw(std::vector<RigidBody>& toDraw, const RENDER::Frustum& frustum, bool useFrustum = true) const;

			RENDER::COLLISION::TYPE checkFrustum(const RENDER::Frustum& f);

			BVHNode() = default;
			BVHNode(BVHNode<BoundingVolumeClass, RigidBody>* parent, const BoundingVolumeClass& volume, RigidBody body);
			~BVHNode();
			BVHNode(const BVHNode&) = default;
			BVHNode& operator=(const BVHNode&) = default;
			BVHNode(BVHNode&&) = default;
			BVHNode& operator=(BVHNode&&) = default;

			BVHNode* m_parent = nullptr;
			BVHNode* m_children[2] = {nullptr, nullptr};
			BoundingVolumeClass m_volume;
			RigidBody m_body;

		private:
			void RecalculateBoundingVolume();
		};

		template <typename BoundingVolumeClass, typename RigidBody>
		bool BVHNode<BoundingVolumeClass, RigidBody>::Overlaps(const BVHNode<BoundingVolumeClass, RigidBody>* other) const {
			return (m_volume.intersect(other->m_volume) > static_cast<int>(RENDER::COLLISION::TYPE::OUTSIDE));
		}

		template <typename BoundingVolumeClass, typename RigidBody>
		unsigned int BVHNode<BoundingVolumeClass, RigidBody>::GetPotentialContacts(PotentialContact<RigidBody>* contacts, unsigned int limit) const {
			if (IsLeaf() || limit == 0) {
				return 0;
			}

			return m_children[0]->GetPotentialContactsWith(m_children[1], contacts, limit);
		}

		template <typename BoundingVolumeClass, typename RigidBody>
		unsigned int BVHNode<BoundingVolumeClass, RigidBody>::GetPotentialContactsWith(const BVHNode<BoundingVolumeClass, RigidBody>* other, PotentialContact<RigidBody>* contacts, unsigned int limit) const {
			if (Overlaps(other) == false || limit == 0) {
				return 0;
			}

			if (IsLeaf() && other->IsLeaf()) {
				contacts->m_body[0] = m_body;
				contacts->m_body[1] = other->m_body;
				return 1;
			}

			if (other->IsLeaf() || ((IsLeaf() == false) && m_volume.getSize() >= other->m_volume.getSize())) {
				unsigned int count = m_children[0]->GetPotentialContactsWith(m_children[1], contacts, limit);

				if (limit > count) {
					count += m_children[0]->GetPotentialContactsWith(other, contacts + count, limit - count);
				}

				if (limit > count) {
					count += m_children[1]->GetPotentialContactsWith(other, contacts + count, limit - count);
				}

				return count;
			}
			else {
				unsigned int count = other->m_children[0]->GetPotentialContactsWith(other->m_children[1], contacts, limit);

				if (limit > count) {
					count += GetPotentialContactsWith(other->m_children[0], contacts + count, limit - count);
				}

				if (limit > count) {
					count += GetPotentialContactsWith(other->m_children[1], contacts + count, limit - count);
				}

				return count;
			}
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		void BVHNode<BoundingVolumeClass, RigidBody>::Insert(RigidBody body, const BoundingVolumeClass& volume) {
			if (IsLeaf()) {
				m_children[0] = new BVHNode<BoundingVolumeClass, RigidBody>(this, m_volume, m_body);

				m_children[1] = new BVHNode<BoundingVolumeClass, RigidBody>(this, volume, body);

				this->m_body = nullptr;

				RecalculateBoundingVolume();
			}
			else {
				float growth1 = m_children[0]->m_volume.calcGrowth(volume);
				float growth2 = m_children[1]->m_volume.calcGrowth(volume);

				if (growth1 == 0.f) {
					growth1 = std::numeric_limits<float>::max();
				}

				if (growth2 == 0.f) {
					growth2 = std::numeric_limits<float>::max();
				}

				if (growth1 < growth2) {
					m_children[0]->Insert(body, volume);
				}
				else {
					m_children[1]->Insert(body, volume);
				}
			}
		}

		template <typename BoundingVolumeClass, typename RigidBody>
		void BVHNode<BoundingVolumeClass, RigidBody>::GetNodeToDraw(std::vector<RigidBody>& toDraw, const RENDER::Frustum& frustum, bool useFrustum) const {
			if (IsLeaf()) {
				if (m_body)
					toDraw.push_back(m_body);
				return;
			}
			if (!useFrustum) {
				if (m_children[0]) {
					m_children[0]->GetNodeToDraw(toDraw, frustum, false);
				}
				if (m_children[1]) {
					m_children[1]->GetNodeToDraw(toDraw, frustum, false);
				}
			}
			else {
				for (int i = 0; i < 2; i++) {
					if (!m_children[i]) {
						continue;
					}
					auto state = m_children[i]->checkFrustum(frustum);
					switch (state)
					{
					case RENDER::COLLISION::TYPE::INSIDE:
						m_children[i]->GetNodeToDraw(toDraw, frustum, false);
						break;
					case RENDER::COLLISION::TYPE::INTERSECTION:
						m_children[i]->GetNodeToDraw(toDraw, frustum);
						break;
					case RENDER::COLLISION::TYPE::OUTSIDE:
						break;
					}
				}
			}
		}

		template <typename BoundingVolumeClass, typename RigidBody>
		RENDER::COLLISION::TYPE BVHNode<BoundingVolumeClass, RigidBody>::checkFrustum(const RENDER::Frustum& frustum) {
			return frustum.boundingSphereInFrustumCollide(m_volume);
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		BVHNode<BoundingVolumeClass, RigidBody>::BVHNode(BVHNode<BoundingVolumeClass, RigidBody>* parent, const BoundingVolumeClass& volume, RigidBody body) :
			m_parent(parent), m_volume(volume), m_body(body) {
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		inline BVHNode<BoundingVolumeClass, RigidBody>::~BVHNode() {
			if (m_parent) {
				BVHNode<BoundingVolumeClass, RigidBody>* sibling;
				if (m_parent->m_children[0] == this) {
					sibling = m_parent->m_children[1];
				}
				else {
					sibling = m_parent->m_children[0];
				}

				m_parent->m_volume = sibling->m_volume;
				m_parent->m_body = sibling->m_body;
				m_parent->m_children[0] = sibling->m_children[0];
				m_parent->m_children[1] = sibling->m_children[1];

				if (sibling->m_children[0]) {
					sibling->m_children[0]->m_parent = m_parent;
				}

				if (sibling->m_children[1]) {
					sibling->m_children[1]->m_parent = m_parent;
				}

				sibling->m_parent = nullptr;
				sibling->m_body = nullptr;
				sibling->m_children[0] = nullptr;
				sibling->m_children[1] = nullptr;
				delete sibling;

				m_parent->RecalculateBoundingVolume();
			}

			if (m_children[0]) {
				m_children[0]->m_parent = nullptr;
				delete m_children[0];
			}

			if (m_children[1]) {
				m_children[1]->m_parent = nullptr;
				delete m_children[1];
			}
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		void BVHNode<BoundingVolumeClass, RigidBody>::RecalculateBoundingVolume() {
			if (IsLeaf() || m_parent == nullptr) {
				return;
			}

			m_volume = BoundingVolumeClass();
			m_volume.createFromTwo(m_children[0]->m_volume, m_children[1]->m_volume);
			if (m_parent) {
				m_parent->RecalculateBoundingVolume();
			}
		}

		template <typename T>
		class BVHTreeIterater {
		public:
			using value_type = T;
			using reference = T&;
			using pointer = T*;
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;

			using Node = typename T::NodeType;

			explicit BVHTreeIterater(Node* pNode) {
				m_cur = pNode;
			}
			~BVHTreeIterater() = default;
			BVHTreeIterater(const BVHTreeIterater<T>&) = default;
			BVHTreeIterater<T>& operator=(const BVHTreeIterater<T>&) = default;
			BVHTreeIterater(BVHTreeIterater<T>&&) = default;
			BVHTreeIterater<T>& operator=(BVHTreeIterater<T>&&) = default;

			BVHTreeIterater<T>& operator++(int) {
				BVHTreeIterater<T> prev = *this;
				++(*this);
				return prev;
			}

			BVHTreeIterater<T>& operator++() {
				if (m_cur == nullptr) {
					// DoNothing
				}
				else if (m_cur->m_children[1]) {
					m_cur = m_cur->m_children[1];
					while (m_cur->m_children[0]) {
						m_cur = m_cur->m_children[0];
					}
				}
				else {
					Node* pNode = m_cur->m_parent;
					while (pNode != nullptr && pNode->m_children[1] == m_cur) {
						m_cur = pNode;
						pNode = m_cur->m_parent;
					}
					m_cur = pNode;
				}

				return *this;
			}

			bool operator==(const BVHTreeIterater<T>& other) const {
				return m_cur == other.m_cur;
			}

			bool operator!=(const BVHTreeIterater<T>& other) const {
				return !(*this == other);
			}

			Node* operator->() const {
				return m_cur;
			}

			Node& operator*() const {
				return *m_cur;
			}

		private:
			Node* m_cur = nullptr;
		};

		template <typename BoundingVolumeClass, typename RigidBody>
		class BVHTree {
		public:
			using NodeType = BVHNode<BoundingVolumeClass, RigidBody>;

			void Insert(RigidBody body, const BoundingVolumeClass& volume);
			void Remove(RigidBody body);
			BVHNode<BoundingVolumeClass, RigidBody>* Find(RigidBody body);

			BVHTreeIterater<BVHTree<BoundingVolumeClass, RigidBody>> begin();
			BVHTreeIterater<BVHTree<BoundingVolumeClass, RigidBody>> end();

			size_t LeafSize() const { return m_leafSize; }

			unsigned int GetPotentialContacts(PotentialContact<RigidBody>* contacts, unsigned int limit) const;
			void GetNodeToDraw(std::vector<RigidBody>& toDraw, const RENDER::Frustum& frustum) const;
		private:
			BVHNode<BoundingVolumeClass, RigidBody> m_root;
			size_t m_leafSize = 0;
		};

		template<typename BoundingVolumeClass, typename RigidBody>
		void BVHTree<BoundingVolumeClass, RigidBody>::Insert(RigidBody body, const BoundingVolumeClass& volume) {
			if (m_root.m_children[0] == nullptr) {
				m_root.m_children[0] = new BVHNode<BoundingVolumeClass, RigidBody>(&m_root, volume, body);
			}
			else {
				m_root.m_children[0]->Insert(body, volume);
			}

			++m_leafSize;
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		void BVHTree<BoundingVolumeClass, RigidBody>::Remove(RigidBody body) {
			if (auto found = Find(body)) {
				delete found;
				--m_leafSize;
			}
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		BVHNode<BoundingVolumeClass, RigidBody>* BVHTree<BoundingVolumeClass, RigidBody>::Find(RigidBody body) {
			if (m_root.m_children[0] == nullptr) {
				return nullptr;
			}

			BVHNode<BoundingVolumeClass, RigidBody>* found = nullptr;
			std::stack<BVHNode<BoundingVolumeClass, RigidBody>*> visit;

			visit.push(m_root.m_children[0]);

			while (visit.empty() == false) {
				BVHNode<BoundingVolumeClass, RigidBody>* cur = visit.top();
				visit.pop();

				if (cur->IsLeaf() && cur->m_body == body) {
					found = cur;
					break;
				}
				else {
					if (cur->m_children[0]) {
						visit.push(cur->m_children[0]);
					}

					if (cur->m_children[1]) {
						visit.push(cur->m_children[1]);
					}
				}
			}

			return found;
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		BVHTreeIterater<BVHTree<BoundingVolumeClass, RigidBody>> BVHTree<BoundingVolumeClass, RigidBody>::begin() {
			BVHNode<BoundingVolumeClass, RigidBody>* pNode = &m_root;
			while (pNode->m_children[0]) {
				pNode = pNode->m_children[0];
			}

			return BVHTreeIterater<BVHTree<BoundingVolumeClass, RigidBody>>(pNode);
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		BVHTreeIterater<BVHTree<BoundingVolumeClass, RigidBody>> BVHTree<BoundingVolumeClass, RigidBody>::end() {
			return BVHTreeIterater<BVHTree<BoundingVolumeClass, RigidBody>>(&m_root);
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		inline unsigned int BVHTree<BoundingVolumeClass, RigidBody>::GetPotentialContacts(PotentialContact<RigidBody>* contacts, unsigned int limit) const {
			if (m_root.m_children[0] == nullptr) {
				return 0;
			}

			return m_root.m_children[0]->GetPotentialContacts(contacts, limit);
		}

		template<typename BoundingVolumeClass, typename RigidBody>
		inline void BVHTree<BoundingVolumeClass, RigidBody>::GetNodeToDraw(std::vector<RigidBody>& toDraw, const RENDER::Frustum& frustum) const {
			if (m_root.m_children[0] == nullptr) {
				return;
			}
			m_root.m_children[0]->GetNodeToDraw(toDraw, frustum);
		}


    } // namespace PHYSICS
    
} // namespace IKIGAI
