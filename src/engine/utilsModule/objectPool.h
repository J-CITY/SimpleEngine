#pragma once

#include <cstdint>

#include "memoryAlloc.h"

namespace IKIGAI::MEMORY {
	class IObjectPool {
	public:
		IObjectPool() = default;
		virtual ~IObjectPool() = default;
	};

	template <typename T>
	class ObjectPool : public IObjectPool {
		struct Chunk {
			void* mTarget = nullptr;
			Chunk* mNext = nullptr;
		};

	public:
		ObjectPool() = delete;

		explicit ObjectPool(size_t poolCapability) {
			mObjectSize = sizeof(T) + sizeof(Chunk);
			mPoolCapability = poolCapability;
			mHeapAddress = reinterpret_cast<unsigned char*>(Memory::Allocate(mPoolCapability * mObjectSize));
			mCurrentFreeChunk = reinterpret_cast<Chunk*>(mHeapAddress);

			ConstructPool();
		}

		~ObjectPool() override = default;

		T* Spawn() {
			if (!mCurrentFreeChunk) {
				return nullptr;
			}

			auto l_Object = mCurrentFreeChunk->mTarget;

			if (l_Object) {
				auto l_Next = mCurrentFreeChunk->mNext;
				if (l_Next) {
					mCurrentFreeChunk = l_Next;
				}
				else {
					mCurrentFreeChunk = nullptr;
				}
				return new(l_Object) T();
			}
			else {
				return nullptr;
			}
		}

		void Destroy(T* ptr) {
			auto l_NewFreeChunk = new(reinterpret_cast<unsigned char*>(ptr) - sizeof(Chunk)) Chunk();
			l_NewFreeChunk->mTarget = ptr;
			
			if (!mCurrentFreeChunk) {
				l_NewFreeChunk->mNext = nullptr;
				mCurrentFreeChunk = l_NewFreeChunk;
			}
			else {
				l_NewFreeChunk->mNext = mCurrentFreeChunk->mNext;
				mCurrentFreeChunk->mNext = l_NewFreeChunk;
			}
		}

	private:
		void ConstructPool() {
			auto l_ObjectUC = mHeapAddress;
			Chunk* l_PrevFreeChunk = nullptr;

			for (auto i = 0; i < mPoolCapability; i++) {
				auto l_NewFreeChunk = new(l_ObjectUC) Chunk();
				l_NewFreeChunk->mTarget = l_ObjectUC + sizeof(Chunk);
				
				if (l_PrevFreeChunk) {
					l_PrevFreeChunk->mNext = l_NewFreeChunk;
				}

				l_NewFreeChunk->mNext = nullptr;
				l_PrevFreeChunk = l_NewFreeChunk;
				l_ObjectUC += mObjectSize;
			}
		}

		std::size_t mObjectSize;
		std::size_t mPoolCapability;
		unsigned char* mHeapAddress;
		Chunk* mCurrentFreeChunk;

	public:
		static ObjectPool<T>* Create(uint32_t poolCapability) {
			auto l_TObjectPoolAddress = reinterpret_cast<ObjectPool<T>*>(UTILS::Memory::Allocate(sizeof(ObjectPool<T>)));
			auto l_TObjectPool = new(l_TObjectPoolAddress) ObjectPool<T>(poolCapability);
			return l_TObjectPool;
		}

		static void Clear(ObjectPool<T>* objectPool) {
			objectPool->ConstructPool();
		}

		static bool Destruct(ObjectPool<T>* objectPool) {
			Clear(objectPool);
			UTILS::Memory::Deallocate(objectPool);
			return true;
		}
	};
}
