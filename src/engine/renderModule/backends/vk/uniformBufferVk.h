#pragma once

#ifdef VULKAN_BACKEND
#include "driverVk.h"
#include "shaderVk.h"
#include <cstdint>
#include <vector>
namespace IKIGAI::RENDER {
	class UniformBufferVk : public UniformBufferInterface {
        struct Snapshot {
            VmaBuffer buffer;
            std::vector<uint8_t> bytes;
            mutable uint64_t usedFrame = 0;
        };
        std::vector<Snapshot> mSnapshots;
        std::vector<uint8_t> mBytes;
        size_t mSnapshot = 0;
        VmaBuffer mEmpty;
        inline static uint64_t sFrame = 1;
        void snapshot();
	public:
        // DriverVk serializes submissions with a fence before begin(). A slot
        // may be overwritten next frame, but never after a draw used it in the
        // current command buffer. Identical payloads can share an immutable slot.
        static void BeginFrame() { ++sFrame; }
		const VmaBuffer& getBuffer() const {
            if (mSnapshots.empty()) return mEmpty;
            mSnapshots[mSnapshot].usedFrame = sFrame;
            return mSnapshots[mSnapshot].buffer;
        }

		UniformBufferVk(void* data, size_t size);
		~UniformBufferVk() override;

		template<class T>
		UniformBufferVk(const T& data) : UniformBufferVk((void*)&data, sizeof(T)) {

		}

		void setData(const void* data, size_t sz, size_t offset = 0) override;
		
	};



}
#endif
