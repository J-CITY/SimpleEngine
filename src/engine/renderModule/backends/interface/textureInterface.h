#pragma once

namespace IKIGAI::RENDER
{
	enum class TextureType {
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,
		TEXTURE_2D_ARRAY,
	};

	class TextureInterface
	{
	public:
		TextureType type = TextureType::TEXTURE_2D;
		virtual ~TextureInterface() = default;


		virtual void* getImguiId() = 0;
	};
}