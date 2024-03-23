#pragma once
#ifdef USE_EDITOR
#include <functional>
#include <map>

#include "editorWindow.h"
#include <memory>
#include <string>

namespace IKIGAI
{
	namespace RENDER
	{
		class TextureInterface;
		class MaterialInterface;
	}
}

namespace IKIGAI
{
	namespace ECS
	{
		class Object;
	}
}

namespace IKIGAI::EDITOR {
	class EditorRender {
	public:
		struct GlobalState {
			std::shared_ptr<ECS::Object> mSelectObject;
			std::shared_ptr<RENDER::MaterialInterface> mSelectMaterial;

			std::string mDndStringPayload;

			std::map<std::string, bool> mPopupStates;
			std::function<void(std::string)> mFileChooserCb;
			std::function<std::string()> mFileFormatsCb;


			std::unordered_map<std::string, std::shared_ptr<RENDER::TextureInterface>> mTextureCache;
		};
		inline static GlobalState GlobalState;

		struct Internal;

		EditorRender();

		std::unique_ptr<Internal> mData;


		virtual ~EditorRender() = default;
		void draw();
		void drawPopup();
	};
}
#endif
