#pragma once
namespace KUMA::RENDER
{
	class UniformBufferInterface {
	public:
		virtual ~UniformBufferInterface() = default;
		virtual int getSetId() const = 0;
	};

	class PushConstantInterface {
	public:
		virtual ~PushConstantInterface() = default;
	};
}