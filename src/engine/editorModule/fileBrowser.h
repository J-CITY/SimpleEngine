#pragma once
#include "editorWindow.h"
#ifdef USE_EDITOR
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <filesystem>


namespace IKIGAI
{
	namespace RENDER
	{
		class TextureInterface;
	}
}

namespace IKIGAI::EDITOR {

	struct File {
		enum class FileType {
			DIR,
			IMAGE,
			MODEL,
			TEXT,
			FONT,
			MATERIAL
		};
		FileType type;
		std::filesystem::path path;
		std::filesystem::path file;
		std::string ext;
		std::list<File> files;

		int uid = 0;
		File(std::filesystem::path path);

	private:
		inline static std::set<std::string> imageExt = {".png", ".jpg", ".jpeg", ".tga", ".dds"};
		inline static std::set<std::string> objExt = {".obj", ".fbx", ".dae"};
		inline static std::set<std::string> fontExt = {".ttf"};
		inline static std::set<std::string> materialExt = {".mat"};
	public:
		static FileType GetFileType(const std::filesystem::path& path);
		static std::string GetExtension(const std::filesystem::path& path);
	};

	class FileBrowserWindow: public EditorWindow{
		std::string mPath;
		std::string mSelectedFolderPath;
		std::stack<std::string> mHistory;
		int mElementSize = 100;

		File mRoot;
		std::set<int> mSearchedFileTreeIds;
		std::unordered_map<std::string, std::shared_ptr<RENDER::TextureInterface>> mTextureCache;
	public:
		FileBrowserWindow(const std::string& path);
		virtual ~FileBrowserWindow() override = default;
		void draw() override;
	private:
		void initFileTree(File& fileTree);
		void drawFolder(std::string_view path);
		void drawItem(File& file);
	};
}
#endif
