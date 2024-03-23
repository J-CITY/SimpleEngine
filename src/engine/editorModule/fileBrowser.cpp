#include "fileBrowser.h"

#ifdef USE_EDITOR
#include <functional>

#include "IconsFontAwesome5.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "utilsModule/stringUtils.h"
#include "misc/cpp/imgui_stdlib.h"
#include "renderModule/backends/interface/textureInterface.h"
#include "resourceModule/serviceManager.h"
#include "utilsModule/pathGetter.h"

#ifdef VULKAN_BACKEND
#include "renderModule/backends/vk/textureVk.h"
#endif
#ifdef OPENGL_BACKEND
#include "renderModule/backends/gl/textureGl.h"
#endif

#ifdef DX12_BACKEND
#include "renderModule/backends/dx12/textureDx12.h"
#endif


using namespace IKIGAI::EDITOR;

File::File(std::filesystem::path path): path(path) {
	type = File::GetFileType(path);
	ext = GetExtension(path);
	file = path.filename();

	//TODO: use gen id
	static int newid = 1;
	uid = newid++;
}

File::FileType File::GetFileType(const std::filesystem::path& path) {
	auto ext = std::filesystem::is_directory(path) ? "dir" :
		           path.has_extension() ? path.extension().string() : "";
	ext = UTILS::ToLower(ext);
	if (ext == "dir") {
		return FileType::DIR;
	}
	if (imageExt.contains(ext)) {
		return FileType::IMAGE;
	}
	if (fontExt.contains(ext)) {
		return FileType::TEXT;
	}
	if (materialExt.contains(ext)) {
		return FileType::MATERIAL;
	}
	return FileType::TEXT;
}

std::string File::GetExtension(const std::filesystem::path& path) {
	auto ext = std::filesystem::is_directory(path) ? "dir" : path.has_extension() ? path.extension().string() : "";
	ext = UTILS::ToLower(ext);
	return ext;
}

FileBrowserWindow::FileBrowserWindow(const std::string& path): mPath(path), mSelectedFolderPath(path), mRoot(path) {
#ifdef OPENGL_BACKEND
	mTextureCache["__image__"] = RENDER::TextureGl::Create(UTILS::GetRealPath("textures/debug/editor/image-white.png"), true);
	mTextureCache["__dir__"] = RENDER::TextureGl::Create(UTILS::GetRealPath("textures/debug/editor/folder-white.png"), true);
	mTextureCache["__file__"] = RENDER::TextureGl::Create(UTILS::GetRealPath("textures/debug/editor/file-white.png"), true);
	mTextureCache["__font__"] = RENDER::TextureGl::Create(UTILS::GetRealPath("textures/debug/editor/font-white.png"), true);
	mTextureCache["__object__"] = RENDER::TextureGl::Create(UTILS::GetRealPath("textures/debug/editor/cube-white.png"), true);
#endif
#ifdef VULKAN_BACKEND
	mTextureCache["__image__"] = RENDER::TextureVk::create(UTILS::GetRealPath("Textures/Debug/Editor/image-white.png"));
	mTextureCache["__dir__"] = RENDER::TextureVk::create(UTILS::GetRealPath("Textures/Debug/Editor/folder-white.png"));
	mTextureCache["__file__"] = RENDER::TextureVk::create(UTILS::GetRealPath("Textures/Debug/Editor/file-white.png"));
	mTextureCache["__font__"] = RENDER::TextureVk::create(UTILS::GetRealPath("Textures/Debug/Editor/font-white.png"));
	mTextureCache["__object__"] = RENDER::TextureVk::create(UTILS::GetRealPath("Textures/Debug/Editor/cube-white.png"));
#endif
#ifdef DX12_BACKEND
	mTextureCache["__image__"] = RENDER::TextureDx12::Create(UTILS::GetRealPath("Textures/Debug/Editor/image-white.png"));
	mTextureCache["__dir__"] = RENDER::TextureDx12::Create(UTILS::GetRealPath("Textures/Debug/Editor/folder-white.png"));
	mTextureCache["__file__"] = RENDER::TextureDx12::Create(UTILS::GetRealPath("Textures/Debug/Editor/file-white.png"));
	mTextureCache["__font__"] = RENDER::TextureDx12::Create(UTILS::GetRealPath("Textures/Debug/Editor/font-white.png"));
	mTextureCache["__object__"] = RENDER::TextureDx12::Create(UTILS::GetRealPath("Textures/Debug/Editor/cube-white.png"));
#endif
	initFileTree(mRoot);
}

void FileBrowserWindow::draw() {
	//ImGui::Columns(2);
	ImGui::Begin("File Browser");
	if (ImGui::BeginTable("File Browser Table", 2, ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button(ICON_FA_RETWEET)) {
			initFileTree(mRoot);
		}
		ImGui::SameLine();
		ImGui::Text("Search: ");
		ImGui::SameLine();
		static std::string searchInActors;
		if (ImGui::InputText("##search_in_file_tree", &searchInActors)) {
			mSearchedFileTreeIds.clear();
			if (!searchInActors.empty()) {
				std::function<void(File&)> closeAll;
				closeAll = [&closeAll, this](File& root) {
					for (auto& obj : root.files) {
						closeAll(obj);
						ImGui::TreeNodeSetOpen(obj.uid, false);
					}
					ImGui::TreeNodeSetOpen(root.uid, false);
				};
				closeAll(mRoot);

				std::function<bool(File&)> expandAll;
				expandAll = [&expandAll, this](File& root) -> bool {
					auto _res = false;
					for (auto& obj : root.files) {
						const auto foundInName = UTILS::ToLower(obj.file.string()).find(UTILS::ToLower(searchInActors)) != std::string::npos;
						const auto res = expandAll(obj);
						if (foundInName) {
							ImGui::TreeNodeSetOpen(obj.uid, true);
							mSearchedFileTreeIds.insert(obj.uid);
							_res = true;
						}
						if (res) {
							ImGui::TreeNodeSetOpen(obj.uid, true);
							_res = true;
						}
					}
					return _res;
				};
				if (expandAll(mRoot)) {
					ImGui::TreeNodeSetOpen(mRoot.uid, true);
				}

			}
		}

		ImGui::BeginChild("##fileTree_child_win");
		drawItem(mRoot);
		ImGui::EndChild();

		ImGui::TableNextColumn();
		drawFolder(mSelectedFolderPath);

		ImGui::EndTable();
	}
	ImGui::End();
	//ImGui::Begin("File Browser");
	//globalNodeId = 0;
	//drawItem(globalNodeId, mPath);
	//ImGui::End();
	//ImGui::Begin("Folder");
	//drawFolder(mSelectedFolderPath);
	//ImGui::End();
}

void FileBrowserWindow::initFileTree(File& fileTree) {
	fileTree.files.clear();
	if (std::filesystem::is_directory(fileTree.path)) {
		for (const auto& entry : std::filesystem::directory_iterator(fileTree.path)) {
			fileTree.files.emplace_back(entry);
			initFileTree(fileTree.files.back());
		}
	}
}

void FileBrowserWindow::drawFolder(std::string_view path)
{
	const int sliderSize = 200;

	std::optional<std::string> newPath;

	ImGui::BeginChild("##FolderBar", ImVec2(0, 30));
	ImGui::Text(std::string(path).c_str());
	ImGui::SameLine();
	ImGui::Text("Search: ");
	ImGui::SameLine();
	static std::string searchInFolder;
	ImGui::PushItemWidth(sliderSize);
	if (ImGui::InputText("##search_in_file_tree", &searchInFolder)) {

	}
	ImGui::PopItemWidth();

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_ARROW_LEFT)) {//back
		if (!mHistory.empty()) {
			newPath = mHistory.top();
			mHistory.pop();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_ARROW_UP)) { //level up
		std::filesystem::path p = mSelectedFolderPath;
		if (mSelectedFolderPath != mPath) {
			mHistory.push(mSelectedFolderPath);
			newPath = p.parent_path().string();
		}
	}

	ImGui::SameLine(std::max(1.0f, ImGui::GetWindowWidth() - sliderSize));
	ImGui::PushItemWidth(sliderSize);
	ImGui::SliderInt("##FSize", &mElementSize, 20, 150);
	ImGui::PopItemWidth();
	ImGui::EndChild();

	ImGui::BeginChild("##FolderFiles", ImVec2(0, 0));
	const auto winSize = ImGui::GetWindowSize();
	//const float col0Size = ImGui::GetColumnWidth(0);
	//const float col1Size = winSize.x - col0Size;
	//ImGui::SetColumnWidth(1, col1Size);
	auto cols = static_cast<int>(winSize.x / (mElementSize + 15));
	if (cols <= 0) {
		cols = 1;
	}
	//ImGui::Columns(static_cast<int>(col1Size / elementSize));
	int i = 0;
	int uid = 0;
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		uid++;
		if (!searchInFolder.empty()) {
			if (UTILS::ToLower(entry.path().filename().string())
				.find(UTILS::ToLower(searchInFolder)) == std::string::npos) {
				continue;
			}
		}
		ImGui::PushID(("folder_item_" + std::to_string(i)).c_str());
		bool isDirectory = std::filesystem::is_directory(entry);
		auto extType = File::GetFileType(entry);
		ImGui::BeginGroup();
		std::string imPath;
		switch (extType) {
		case File::FileType::DIR: imPath = "__dir__";  break;
		case File::FileType::IMAGE: {
			imPath = entry.path().string();
			if (!mTextureCache.contains(imPath)) {
#ifdef OPENGL_BACKEND
				//TODO: check backend
				mTextureCache[imPath] = RENDER::TextureGl::Create(imPath, true);
#endif
#ifdef VULKAN_BACKEND
				mTextureCache[imPath] = RENDER::TextureVk::create(imPath);
#endif
#ifdef DX12_BACKEND
				mTextureCache[imPath] = RENDER::TextureDx12::Create(imPath);
#endif
			}
			//imPath = "__image__";
			break;
		};
		case File::FileType::MODEL: imPath = "__object__"; break;
		case File::FileType::TEXT: imPath = "__file__"; break;
		case File::FileType::FONT: imPath = "__font__"; break;
		default: imPath = "__file__"; break;;
		}

		//TODO: create withget for it
		static int selected = -1;
		if (ImGui::Selectable(("##" + std::to_string(uid)).c_str(), uid == selected, 0, ImVec2(mElementSize, mElementSize))) {
			selected = uid;
		}

		ImGuiDragDropFlags src_flags = 0;
		src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
		//src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we disable the feature of opening foreign treenodes/tabs while dragging
		//src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
		if (ImGui::BeginDragDropSource(src_flags)) {
			//if (!(src_flags))
			//	ImGui::Text("Moving \"%s\"", names[n]);
			int a = 0;
			if (extType == File::FileType::IMAGE) {
				ImGui::SetDragDropPayload("DND_IMAGE_DATA", &a, sizeof(int));
				//TODO: set global state
				//dndStringPayload = imPath;
			} else if (extType == File::FileType::MATERIAL) {
				ImGui::SetDragDropPayload("DND_MATERIAL_DATA", &a, sizeof(int));
				//TODO: set global state
				//dndStringPayload = entry.path().string();
			}
			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
			if (isDirectory) {
				mHistory.push(mSelectedFolderPath);
				newPath = entry.path().string();
			} else if (extType == File::FileType::MATERIAL) {
				//TODO: set to global state
				//editMaterial = RESOURCES::ServiceManager::Get<RESOURCES::MaterialLoader>().loadResource(entry.path().string());
			}
		}
		auto pos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(pos.x, pos.y - mElementSize));
		ImGui::Image(mTextureCache.at(imPath)->getImguiId(), {static_cast<float>(mElementSize), static_cast<float>(mElementSize)}, ImVec2(0, 1), ImVec2(1, 0));
//#ifdef VULKAN_BACKEND
//				ImGui::Image((ImTextureID)textureCache.at(imPath)->descriptor_set, {static_cast<float>(elementSize), static_cast<float>(elementSize)});
//#endif
//#ifdef DX12_BACKEND
//				ImGui::Image((ImTextureID)textureCache.at(imPath)->mGpuSrv.ptr, {static_cast<float>(elementSize), static_cast<float>(elementSize)});
//#endif
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 100);
		ImGui::Text(entry.path().filename().string().c_str());
		ImGui::PopTextWrapPos();

		ImGui::EndGroup();
		i = (i + 1) % cols;
		if (i) {
			ImGui::SameLine();
		}
		//ImGui::NextColumn();
		ImGui::PopID();
	}
	ImGui::EndChild();
	//ImGui::Columns(2);
	if (newPath) {
		mSelectedFolderPath = newPath.value();
	}
}

void FileBrowserWindow::drawItem(File& file) {
	ImGui::PushID(("fileTree_" + std::to_string(file.uid)).c_str());
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Selected;
	auto path = file.path.string();
	bool isDirectory = std::filesystem::is_directory(path);
	if (!isDirectory) {
		nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	} else {
		nodeFlags |= ImGuiTreeNodeFlags_None;
	}
	std::string _path = std::string(path.begin(), path.end());
	if (path.back() == '\\' || path.back() == '/') {
		_path = std::filesystem::path(path).parent_path().string();
	}
	const auto name = std::filesystem::path(_path).filename().string();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
	bool inSearch = mSearchedFileTreeIds.contains(file.uid);
	if (inSearch) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
	}
	bool nodeIsOpen = ImGui::TreeNodeBehavior(file.uid, nodeFlags, name.c_str());
	if (inSearch) {
		ImGui::PopStyleColor();
	}
	ImGui::PopStyleVar();


	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
	//if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
	//	if (ImGui::MenuItem("Create new")) {
	//
	//	}
	//	if (ImGui::MenuItem("Delete")) {
	//
	//	}
	//
	//	ImGui::EndPopup();
	//}
	//ImGui::PopStyleVar();

	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
	//if (ImGui::BeginDragDropSource()) {
	//	ImGui::SetDragDropPayload("__SCENE_NODE_DRAG__", &node, sizeof(IKIGAI::ECS::Object*));
	//	ImGui::TextUnformatted(name.c_str());
	//	ImGui::EndDragDropSource();
	//}
	//ImGui::PopStyleVar();

	//if (ImGui::BeginDragDropTarget()) {
	//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
	//		auto other = *static_cast<IKIGAI::ECS::Object**>(payload->Data);
	//
	//		//change node parent
	//		auto parent = other->getParent();
	//		if (parent) {
	//			//parent->removeChild(other);
	//			//other->setParent(nullptr);
	//		}
	//		//node.second->addChild(other);
	//	}
	//	ImGui::EndDragDropTarget();
	//}

	if (isDirectory && ImGui::IsItemClicked()) {
		mHistory.push(mSelectedFolderPath);
		mSelectedFolderPath = _path;
	}
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
	ImGui::InvisibleButton("__NODE_ORDER_SET__", {-1, 5});
	ImGui::PopStyleVar();

	//if (ImGui::BeginDragDropTarget()) {
	//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
	//		auto other = *static_cast<IKIGAI::ECS::Object**>(payload->Data);
	//		if (node.get() != other && node->getParent()) {
	//			auto parent = other->getParent();
	//			if (parent) {
	//				//parent->removeChild(other);
	//				//other->setParent(nullptr);
	//			}
	//			//node.second->getParent()->addChild(other->getID(), other, i + 1);
	//		}
	//	}
	//	ImGui::EndDragDropTarget();
	//}

	if (isDirectory && nodeIsOpen) {
		for (auto& entry : file.files) {
			drawItem(entry);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}
#endif
