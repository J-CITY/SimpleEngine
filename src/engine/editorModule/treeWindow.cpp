#include "treeWindow.h"
#ifdef USE_EDITOR
#include <string>

#include "editorRender.h"
#include "IconsFontAwesome5.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "coreModule/core/core.h"
#include "coreModule/ecs/object.h"
#include "sceneModule/scene.h"
#include "sceneModule/sceneManager.h"
#include "utilsModule/stringUtils.h"
#include "misc/cpp/imgui_stdlib.h"

using namespace IKIGAI::EDITOR;

std::set<IKIGAI::ECS::Object::Id_> searchedObjectsIds;

std::shared_ptr<IKIGAI::ECS::Object> recursiveDraw(IKIGAI::SCENE_SYSTEM::Scene& activeScene, std::shared_ptr<IKIGAI::ECS::Object> parentEntity) {
	std::shared_ptr<IKIGAI::ECS::Object> selectedNode;

	std::span<std::shared_ptr<IKIGAI::ECS::Object>> nodeList;
	if (parentEntity) {
		nodeList = parentEntity->getChildren();
	} else {
		nodeList = activeScene.getObjects();
	}

	unsigned i = 0u;
	for (auto node : nodeList) {
		ImGui::PushID(("node_" + std::to_string(i)).c_str());
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow;

		bool isParent = node->getChildren().size();

		if (!isParent) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		} else {
			nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}
		const auto name = node->getName();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});

		//Visable button
		if (ImGui::Button(node->getIsActive() ? ICON_FA_EYE : ICON_FA_EYE_SLASH)) {
			node->setActive(!node->getIsActive());
		}
		ImGui::SameLine();
		bool inSearch = searchedObjectsIds.contains(node->getID());
		if (inSearch) {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		}
		const bool nodeIsOpen = ImGui::TreeNodeBehavior(static_cast<int>(node->getID()), nodeFlags, name.c_str());
		if (inSearch) {
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{5.0f, 5.0f});
		if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
			if (ImGui::MenuItem("Create new child")) {
				if (IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().hasCurrentScene()) {
					auto& scene = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().getCurrentScene();
					const auto newObject = scene.createObject("new_object");
					newObject->setParent(node);
				}
			}
			if (ImGui::MenuItem("Create new before")) {
				if (IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().hasCurrentScene()) {
					auto& scene = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().getCurrentScene();
					const auto newObject = scene.createObjectBefore(node->getID(), "new_object");
					newObject->setParent(node->getParent());
				}
			}
			if (ImGui::MenuItem("Create new after")) {
				if (IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().hasCurrentScene()) {
					auto& scene = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().getCurrentScene();
					const auto newObject = scene.createObjectAfter(node->getID(), "new_object");
					newObject->setParent(node->getParent());
				}
			}
			if (ImGui::MenuItem("Delete")) {
				if (IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().hasCurrentScene()) {
					auto& scene = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>().getCurrentScene();
					scene.destroyObject(node);
				}
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("__SCENE_NODE_DRAG_TREE_WIN__", &node, sizeof(IKIGAI::ECS::Object*));
			ImGui::TextUnformatted(name.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG_TREE_WIN__")) {
				auto other = *static_cast<IKIGAI::ECS::Object**>(payload->Data);

				//change node parent
				other->setParent(node);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked()) {
			selectedNode = node;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
		ImGui::InvisibleButton("__NODE_ORDER_SE_TREE_WINT__", {-1, 5});
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__NODE_ORDER_SET_TREE_WIN__")) {
				auto other = *static_cast<IKIGAI::ECS::Object**>(payload->Data);
				if (node.get() != other && node->getParent()) {
					other->setParentInPos(node, i + 1);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isParent && nodeIsOpen) {
			auto childClickedEntity = recursiveDraw(activeScene, node);
			if (!selectedNode) {
				selectedNode = childClickedEntity;
			}
			ImGui::TreePop();
		}
		i++;
		ImGui::PopID();
	}
	return selectedNode;
}
void TreeWindow::drawNodeTree() {

	auto& sceneManager = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::SCENE_SYSTEM::SceneManager>();
	if (!sceneManager.hasCurrentScene()) {
		return;
	}
	auto& scene = sceneManager.getCurrentScene();

	if (ImGui::Begin("Scene Hierarchy", &mIsOpen)) {
		ImGui::Text("Search: ");
		ImGui::SameLine();
		static std::string searchInActors;
		if (ImGui::InputText("##search_in_actors", &searchInActors)) {
			searchedObjectsIds.clear();
			if (!searchInActors.empty()) {
				for (auto obj : scene.getObjects()) {
					ImGui::TreeNodeSetOpen(static_cast<int>(obj->getID()), false);
				}
				for (auto obj : scene.getObjects()) {
					const auto foundInName = IKIGAI::UTILS::ToLower(obj->getName()).find(IKIGAI::UTILS::ToLower(searchInActors)) != std::string::npos;
					const auto foundInTag = IKIGAI::UTILS::ToLower(obj->getTag()).find(IKIGAI::UTILS::ToLower(searchInActors)) != std::string::npos;
					if (foundInName || foundInTag) {
						searchedObjectsIds.insert(obj->getID());
						std::function<void(std::shared_ptr<IKIGAI::ECS::Object>)> expandAll;
						expandAll = [&expandAll](std::shared_ptr<IKIGAI::ECS::Object> obj) {
							ImGui::TreeNodeSetOpen(static_cast<int>(obj->getID()), true);
							if (obj->getParent()) {
								expandAll(obj->getParent());
							}
						};
						if (obj->getParent()) {
							expandAll(obj->getParent());
						}
					}
				}
			}
		}


		//uniqueNodeId = 0;
		auto _selectNode = recursiveDraw(scene, nullptr);
		if (_selectNode) {
			EditorRender::GlobalState.mSelectObject = _selectNode;
		}
			//	selectObj = _selectNode;
		//	if (selectObj) {
		//		auto components = IKIGAI::ECS::ComponentManager::GetInstance().getComponents(selectObj->getID());
		//		for (auto& e : components) {
		//			if (!convertComp.count(e->getName())) continue; //TODO:: write info to log
		//			auto c = convertComp[e->getName()](*e.get());
		//			rttr::type t = std::visit(
		//				[](auto& arg) {
		//					return rttr::type::get<std::remove_reference_t<decltype(*arg)>>();
		//				},
		//				c);
		//			for (auto& prop : t.get_properties()) {
		//				auto flags = prop.get_metadata(IKIGAI::UTILS::MetaInfo::FLAGS).get_value<MetaInfo::Flags>();
		//				if (flags & (MetaInfo::USE_IN_EDITOR_ANIMATION)) {
		//					SequencerItemTypeNames.push_back(AnimationLineInfo{
		//						selectObj->getID(), selectObj->getName(),
		//						e->getName(), std::string(prop.get_name()), 0.0f, c});
		//				}
		//			}
		//		}
		//	}
		//}
	}
	ImGui::End();
	
}


void TreeWindow::draw() {
	drawNodeTree();
}
#endif
