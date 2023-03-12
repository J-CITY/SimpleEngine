#pragma once

namespace KUMA::RENDER {
	class ShaderInterface;

	class IndexBufferInterface {
	public:
		virtual ~IndexBufferInterface() = default;
		virtual void bind(const ShaderInterface& shader) = 0;
		virtual int getIndexCount() = 0;
	};
}
