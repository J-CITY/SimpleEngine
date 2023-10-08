#pragma once

#include "memory.h"

namespace IKIGAI::MEMORY {
	class IObjectPool {
	public:
		virtual ~IObjectPool() = default;
	};

	template <typename T>
	class ObjectPool : public IObjectPool {
		struct Chunk {
			void* m_Target = nullptr;
			Chunk* m_Next = nullptr;
		};

	public:
		ObjectPool() = delete;

		explicit ObjectPool(size_t poolCapability) {
			m_ObjectSize = sizeof(T) + sizeof(Chunk);
			m_PoolCapability = poolCapability;
			m_HeapAddress = reinterpret_cast<unsigned char*>(Memory::Allocate(m_PoolCapability * m_ObjectSize));
			m_CurrentFreeChunk = reinterpret_cast<Chunk*>(m_HeapAddress);

			ConstructPool();
		}

		~ObjectPool() = default;

		T* Spawn() {
			if (!m_CurrentFreeChunk) {
				return nullptr;
			}

			auto l_Object = m_CurrentFreeChunk->m_Target;

			if (l_Object) {
				auto l_Next = m_CurrentFreeChunk->m_Next;
				if (l_Next) {
					m_CurrentFreeChunk = l_Next;
				}
				else {
					m_CurrentFreeChunk = nullptr;
				}
				return new(l_Object) T();
			}
			else {
				return nullptr;
			}
		}

		void Destroy(T* ptr) {
			auto l_NewFreeChunk = new(reinterpret_cast<unsigned char*>(ptr) - sizeof(Chunk)) Chunk();
			l_NewFreeChunk->m_Target = ptr;
			
			if (!m_CurrentFreeChunk) {
				l_NewFreeChunk->m_Next = nullptr;
				m_CurrentFreeChunk = l_NewFreeChunk;
			}
			else {
				l_NewFreeChunk->m_Next = m_CurrentFreeChunk->m_Next;
				m_CurrentFreeChunk->m_Next = l_NewFreeChunk;
			}
		}

	private:
		void ConstructPool() {
			auto l_ObjectUC = m_HeapAddress;
			Chunk* l_PrevFreeChunk = nullptr;

			for (auto i = 0; i < m_PoolCapability; i++) {
				auto l_NewFreeChunk = new(l_ObjectUC) Chunk();
				l_NewFreeChunk->m_Target = l_ObjectUC + sizeof(Chunk);
				
				if (l_PrevFreeChunk) {
					l_PrevFreeChunk->m_Next = l_NewFreeChunk;
				}

				l_NewFreeChunk->m_Next = nullptr;
				l_PrevFreeChunk = l_NewFreeChunk;
				l_ObjectUC += m_ObjectSize;
			}
		}

		std::size_t m_ObjectSize;
		std::size_t m_PoolCapability;
		unsigned char* m_HeapAddress;
		Chunk* m_CurrentFreeChunk;

	public:
		static ObjectPool<T>* Create(uint32_t poolCapability) {
			auto l_TObjectPoolAddress = reinterpret_cast<ObjectPool<T>*>(Memory::Allocate(sizeof(ObjectPool<T>)));
			auto l_TObjectPool = new(l_TObjectPoolAddress) ObjectPool<T>(poolCapability);
			return l_TObjectPool;
		}

		static void Clear(ObjectPool<T>* objectPool) {
			objectPool->ConstructPool();
		}

		static bool Destruct(ObjectPool<T>* objectPool) {
			Clear(objectPool);
			Memory::Deallocate(objectPool);
			return true;
		}
	};
}
