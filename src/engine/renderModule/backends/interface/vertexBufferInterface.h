#pragma once

namespace IKIGAI::RENDER
{
	class ShaderInterface;

	class VertexBufferInterface {
	public:
		virtual ~VertexBufferInterface() = default;
		virtual void bind(const ShaderInterface& shader) = 0;
		virtual int getVertexCount() = 0;
	};
}
