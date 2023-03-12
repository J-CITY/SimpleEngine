#include "debugRender.h"

#include <memory>
#include <serdepp/include/serdepp/adaptor/reflection.hpp>


#include "../render/backends/interface/driverInterface.h"

#ifdef VULKAN_BACKEND
#include <vulkan/vulkan_core.h>
#include "imgui/imgui_impl_vulkan.h"
#include "../render/backends/vk/driverVk.h"
#include "../render/backends/vk/frameBufferVk.h"
#endif

#include "../render/gameRendererGl.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"

#ifdef OPENGL_BACKEND
#include "imgui/imgui_impl_opengl3.h"
#include "../render/backends/gl/materialGl.h"
#endif
#include "../window/window.h"
#include "../inputManager/inputManager.h"

using namespace KUMA;
using namespace KUMA::DEBUG;

#include "imgui/imgui_internal.h"
#include "../ecs/object.h"
#include "../gui/guiObject.h"
#include "../scene/sceneManager.h"
#include "../core/core.h"


int uniqueNodeId = 0;
std::shared_ptr<KUMA::ECS::Object> selectObj;
std::shared_ptr<KUMA::GUI::GuiObject> selectObjGui;

std::shared_ptr<KUMA::ECS::Object> recursiveDraw(KUMA::SCENE_SYSTEM::Scene& activeScene, std::shared_ptr<KUMA::ECS::Object> parentEntity) {
	std::shared_ptr<KUMA::ECS::Object> selectedNode;

	std::span<std::shared_ptr<KUMA::ECS::Object>> nodeList;

	if (parentEntity) {
		nodeList = parentEntity->getChildren();
	}
	else {
		nodeList = activeScene.getObjects();
	}
	auto i = 0u;
	for (auto node : nodeList) {
		ImGui::PushID(("node_" + std::to_string(i)).c_str());
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Selected;

		bool isParent = node->getChildren().size();

		if (!isParent) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		else {
			nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}
		const auto name = node->getName();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		bool nodeIsOpen = ImGui::TreeNodeBehavior(ImGui::GetCurrentWindow()->GetID(node->getName().c_str()), nodeFlags, name.c_str());
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
		if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
			if (ImGui::MenuItem("Create new")) {

			}
			if (ImGui::MenuItem("Delete")) {

			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("__SCENE_NODE_DRAG__", &node, sizeof(KUMA::ECS::Object*));
			ImGui::TextUnformatted(name.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::ECS::Object**>(payload->Data);

				//change node parent
				auto parent = other->getParent();
				if (parent) {
					//parent->removeChild(other);
					//other->setParent(nullptr);
				}
				//node.second->addChild(other);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked()) {
			selectedNode = node;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		ImGui::InvisibleButton("__NODE_ORDER_SET__", { -1, 5 });
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::ECS::Object**>(payload->Data);
				if (node.get() != other && node->getParent()) {
					auto parent = other->getParent();
					if (parent) {
						//parent->removeChild(other);
						//other->setParent(nullptr);
					}
					//node.second->getParent()->addChild(other->getID(), other, i + 1);
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
void drawNodeTree(KUMA::CORE_SYSTEM::Core& core) {
	static bool isobjTreeOpen = true;
	if (core.sceneManager->hasCurrentScene()) {
		auto& scene = core.sceneManager->getCurrentScene();
		if (ImGui::Begin("Scene Hierarchy", &isobjTreeOpen)) {
			uniqueNodeId = 0;
			auto _selectNode = recursiveDraw(scene, nullptr);
			if (_selectNode) {
				selectObj = _selectNode;
			}
		}
		ImGui::End();
	}
}

/*
std::shared_ptr<KUMA::GUI::GuiObject> recursiveDrawGui(KUMA::SCENE_SYSTEM::Scene& activeScene, std::shared_ptr<KUMA::GUI::GuiObject> parentEntity) {
	std::shared_ptr<KUMA::GUI::GuiObject> selectedNode;

	std::vector<std::shared_ptr<KUMA::GUI::GuiObject>> nodeList;

	if (parentEntity) {
		nodeList = parentEntity->childs;
	}
	else {
		nodeList = activeScene.guiObjs;
	}
	auto i = 0u;
	for (auto node : nodeList) {
		ImGui::PushID(("node_" + std::to_string(i)).c_str());
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Selected;

		bool isParent = node->childs.size();

		if (!isParent) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		else {
			nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}
		const auto name = node->name;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		bool nodeIsOpen = ImGui::TreeNodeBehavior(ImGui::GetCurrentWindow()->GetID(node->name.c_str()), nodeFlags, name.c_str());
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
		if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
			if (ImGui::MenuItem("Create new")) {

			}
			if (ImGui::MenuItem("Delete")) {

			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("__SCENE_NODE_DRAG__", &node, sizeof(KUMA::ECS::Object*));
			ImGui::TextUnformatted(name.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::GUI::GuiObject**>(payload->Data);

				//change node parent
				auto parent = other->parent;
				if (parent) {
					//parent->removeChild(other);
					//other->setParent(nullptr);
				}
				//node.second->addChild(other);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked()) {
			selectedNode = node;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		ImGui::InvisibleButton("__NODE_ORDER_SET__", { -1, 5 });
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::GUI::GuiObject**>(payload->Data);
				if (node.get() != other && node->parent) {
					auto parent = other->parent;
					if (parent) {
						//parent->removeChild(other);
						//other->setParent(nullptr);
					}
					//node.second->getParent()->addChild(other->getID(), other, i + 1);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isParent && nodeIsOpen) {
			auto childClickedEntity = recursiveDrawGui(activeScene, node);
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
*/
//void drawNodeTreeGui(KUMA::CORE_SYSTEM::Core& core) {
//	static bool isobjTreeOpen = true;
//	if (core.sceneManager->hasCurrentScene()) {
//		auto& scene = core.sceneManager->getCurrentScene();
//		if (ImGui::Begin("Scene Hierarchy", &isobjTreeOpen)) {
//			uniqueNodeId = 0;
//			auto _selectNode = recursiveDrawGui(scene, nullptr);
//			if (_selectNode) {
//				selectObjGui = _selectNode;
//			}
//		}
//		ImGui::End();
//	}
//}


template<class T>
bool drawWidget(std::string name, T& elem) {
	using type = std::remove_reference_t<decltype(elem)>;
	int a=1;
	return false;
}

template<>
bool drawWidget(std::string name, float& elem) {
	return ImGui::DragFloat(name.c_str(), &elem, 0.1f, 0.1f, 500.0f);
}
template<>
bool drawWidget(std::string name, int& elem) {
	return ImGui::DragInt(name.c_str(), &elem, 1.0f, 0, 100);
}
template<>
bool drawWidget(std::string name, bool& elem) {
	return ImGui::Checkbox(name.c_str(), &elem);
}
template<>
bool drawWidget(std::string name, std::string& elem) {
	return false;
}
template<>
bool drawWidget(std::string name, MATHGL::Vector2u& elem) {
	int arr[2];
	arr[0] = elem.x;
	arr[1] = elem.y;
	auto b = ImGui::DragInt2(name.c_str(), arr, 1, 0, 4000);
	elem.x = arr[0];
	elem.y = arr[1];
	return b;
}

template<std::size_t I = 0, typename T>
inline std::enable_if < I == serde::tuple_size_v<T>, bool>::type go(const serde::serde_struct_info<T>& t, T& val) {
	return true;
}

template<std::size_t I = 0, typename T>
inline std::enable_if < I < serde::tuple_size_v<T>, bool>::type go(const serde::serde_struct_info<T>& t, T& val) {
	auto b = drawWidget(std::string(t.member_info<I>(val).name()), t.member_info<I>(val).value());
	return b || go<I + 1, T>(t, val);
}

template<typename T>
bool buildWidget(T& data) {
	constexpr auto info = serde::type_info<T>;
	return go(info, data);
}

struct DebugConfig {
	enum class WidgetType {
		WINDOW = 0
	};

	static bool check(WidgetType t) { return conf[static_cast<int>(t)]; }
	inline static std::bitset<10> conf;
};

DebugRender::DebugRender() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		initForVk();
		auto win = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getGLFWWin();
		ImGui_ImplGlfw_InitForVulkan(win, true);
	}
#endif


	// Setup Platform/Renderer backends
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		auto& window = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getContext();
		ImGui_ImplGlfw_InitForOpenGL(&window, true);
		const char* glsl_version = "#version 330";
		ImGui_ImplOpenGL3_Init(glsl_version);
	}
#endif

#ifdef OPENGL_BACKEND || VULKAN_BACKEND
	RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().keyEvent.add([](GLFWwindow* window, int key, int scancode, int action, int mods){
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	});
	RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().mouseButtonEvent.add([](GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	});
#endif
}
#ifdef VULKAN_BACKEND
#include "../render/gameRendererVk.h"
void DebugRender::initForVk() {
	auto render = reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).getDriver();

	auto rd = render->GetRenderData();

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = render->m_VulkanInstance;
	init_info.PhysicalDevice = render->m_MainDevice.PhysicalDevice;
	init_info.Device = render->m_MainDevice.LogicalDevice;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.Allocator = nullptr;
	init_info.QueueFamily = rd.graphic_queue_index;
	init_info.Queue = rd.graphic_queue;
	init_info.DescriptorPool = rd.imgui_descriptor_pool;
	init_info.MinImageCount = rd.min_image_count;
	init_info.ImageCount = rd.image_count;
	init_info.CheckVkResultFn = nullptr;


	ImGui_ImplVulkan_Init(&init_info, render->defaultFb->m_RenderPass);

	VkCommandPool command_pool = render->m_CommandHandler.GetCommandPool();

	VkCommandBuffer command_buffer = render->m_CommandHandler.GetCommandBuffer(0);

	vkResetCommandPool(rd.device, command_pool, 0);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &command_buffer;
	vkEndCommandBuffer(command_buffer);
	vkQueueSubmit(rd.graphic_queue, 1, &end_info, VK_NULL_HANDLE);
	vkDeviceWaitIdle(rd.device);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}
#endif

DebugRender::~DebugRender() {
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
#endif
}

void DebugRender::drawWindowWidget(CORE_SYSTEM::Core& core) {
	if (DebugConfig::check(DebugConfig::WidgetType::WINDOW)) {
		ImGui::Begin("Window Config");
		auto b = buildWidget(core.window->getSetting());
		if (b) {
			core.window->updateWindow();
		}
		ImGui::End();
	}
}

class CombineVecEdit {
	size_t mVecSize = 3;
	std::string mName;
	std::array<float, 4> mVec = { 0.0f, 0.0f, 0.0f, 0.0f };
	bool mIsColorMode = false;
	std::string mButtonLblColor;
	std::string mButtonLblVec;
public:
	CombineVecEdit(std::string_view name, size_t vecSize, std::array<float, 4> vec = { 0.0f, 0.0f, 0.0f, 0.0f }):
		mVecSize(vecSize), mName(name), mVec(vec) {
		switch (mVecSize) {
		case 3: mButtonLblVec = "XYZ"; mButtonLblColor = "RGB"; break;
		case 4: mButtonLblVec = "XYZW"; mButtonLblColor = "RGBA"; break;
		}
	}
	inline static std::unordered_map<std::string, CombineVecEdit> Data;
	bool draw() {
		ImGui::Columns(2);
		ImGui::SetColumnWidth(1, 200.0f);

		ImGui::Text(mName.c_str());
		ImGui::NextColumn();

		auto _nameId = "##" + mName;
		if (mIsColorMode) {
			ImGui::SetNextItemWidth(200);
			if (mVecSize == 3 && ImGui::ColorEdit3(_nameId.c_str(), mVec.data())) {
				return true;
			}
			else if (mVecSize == 4 && ImGui::ColorEdit4(_nameId.c_str(), mVec.data())) {
				return true;
			}
		}
		else {
			if (mVecSize == 2) {
				ImGui::SetNextItemWidth(200);
			}
			else {
				ImGui::SetNextItemWidth(150);
			}
			if (mVecSize == 2 && ImGui::DragFloat2(_nameId.c_str(), mVec.data(), 0.1f)) {
				return true;
			}
			else if (mVecSize == 3 && ImGui::DragFloat3(_nameId.c_str(), mVec.data(), 0.1f)) {
				return true;
			}
			else if (mVecSize == 4 && ImGui::DragFloat4(_nameId.c_str(), mVec.data(), 0.1f)) {
				return true;
			}
		}
		if (mVecSize > 2) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			if (ImGui::SmallButton(mIsColorMode ? mButtonLblColor.c_str() : mButtonLblVec.c_str())) {
				mIsColorMode = !mIsColorMode;
			}
		}

		ImGui::NextColumn();
		ImGui::Columns(1);
		return false;
	}

	const std::array<float, 4>& getVectorData() {
		return mVec;
	}
};

bool isEngineUniform(const std::string& uniformName) {
	return uniformName.rfind("EngineUBO", 0) == 0 || uniformName.rfind("engine_", 0) == 0
		|| uniformName.rfind("Engine", 0) == 0;
}
#ifdef OPENGL_BACKEND
void DebugRender::drawMaterialWidget(RENDER::MaterialGl* material) {
	ImGui::Begin("Material Editor");

	auto shader = material->getShader();
	const auto& refl = shader->getUniformsInfo();

	for (auto& [name, data]: refl) {
		if (isEngineUniform(name)) continue;
		if (data.type == RENDER::UniformInform::TYPE::UNIFORM) {
			switch (data.members[0].type) {
			case RENDER::UNIFORM_TYPE::MAT4: break;
			case RENDER::UNIFORM_TYPE::MAT3: break;
			case RENDER::UNIFORM_TYPE::VEC4: {
				if (!CombineVecEdit::Data.count(name)) {
					auto val = std::get<MATHGL::Vector4>(material->get(name));
					CombineVecEdit::Data.insert({ name, CombineVecEdit(name, 4, { val.x, val.y, val.z, val.w }) });
				}
				if (CombineVecEdit::Data.at(name).draw()) {
					const auto& data = CombineVecEdit::Data.at(name).getVectorData();
					material->set(name, MATHGL::Vector4(data[0], data[1], data[2], data[3]));
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::VEC3: {
				if (!CombineVecEdit::Data.count(name)) {
					auto val = std::get<MATHGL::Vector3>(material->get(name));
					CombineVecEdit::Data.insert({ name, CombineVecEdit(name, 3, { val.x, val.y, val.z, 0.0f }) });
				}
				if (CombineVecEdit::Data.at(name).draw()) {
					const auto& data = CombineVecEdit::Data.at(name).getVectorData();
					material->set(name, MATHGL::Vector3(data[0], data[1], data[2]));
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::VEC2: {
				if (!CombineVecEdit::Data.count(name)) {
					auto val = std::get<MATHGL::Vector2f>(material->get(name));
					CombineVecEdit::Data.insert({ name, CombineVecEdit(name, 2, { val.x, val.y, 0.0f, 0.0f }) });
				}
				if (CombineVecEdit::Data.at(name).draw()) {
					const auto& data = CombineVecEdit::Data.at(name).getVectorData();
					material->set(name, MATHGL::Vector2f(data[0], data[1]));
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::INT: {
				auto val = std::get<int>(material->mUniformData.at(name));
				if (ImGui::DragInt(name.c_str(), &val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::FLOAT: {
				auto val = std::get<float>(material->mUniformData.at(name));
				if (ImGui::DragFloat(name.c_str(), &val, 0.1f)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::BOOL: {
				auto val = std::get<bool>(material->mUniformData.at(name));
				if (ImGui::Checkbox(name.c_str(), &val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::SAMPLER_2D: {
				auto val = std::get<std::shared_ptr<RENDER::TextureGl>>(material->mUniformData.at(name));
				if (val) {
					//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
					ImGui::Image((void*)val->id, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
				}
				//ImGui::GetWindowDrawList()->AddImage(
				//	(void*)val->getId(),
				//	ImVec2(ImGui::GetCursorScreenPos()),
				//	ImVec2(ImGui::GetCursorScreenPos().x + size.x / 2,
				//		ImGui::GetCursorScreenPos().y + size.y / 2), ImVec2(0, 1), ImVec2(1, 0));
				break;
			}
			case RENDER::UNIFORM_TYPE::SAMPLER_3D: break;
			case RENDER::UNIFORM_TYPE::SAMPLER_CUBE: break;
			default: break;
			}
		}
		else if (data.type == RENDER::UniformInform::TYPE::UNIFORM_BUFFER) {
			for (const auto& mamber : data.members) {
				auto _name = mamber.name;
				switch (mamber.type) {
				case RENDER::UNIFORM_TYPE::MAT4: break;
				case RENDER::UNIFORM_TYPE::MAT3: break;
				case RENDER::UNIFORM_TYPE::VEC4: {
					if (!CombineVecEdit::Data.count(_name)) {
						auto val = std::get<MATHGL::Vector4>(material->get(name, mamber.name));
						CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 4, { val.x, val.y, val.z, val.w }) });
					}
					if (CombineVecEdit::Data.at(_name).draw()) {
						const auto& data = CombineVecEdit::Data.at(_name).getVectorData();
						material->set(name, mamber.name, MATHGL::Vector4(data[0], data[1], data[2], data[3]));
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::VEC3: {
					if (!CombineVecEdit::Data.count(_name)) {
						auto val = std::get<MATHGL::Vector3>(material->get(name, mamber.name));
						CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 3, { val.x, val.y, val.z, 0.0f }) });
					}
					if (CombineVecEdit::Data.at(_name).draw()) {
						const auto& data = CombineVecEdit::Data.at(_name).getVectorData();
						material->set(name, mamber.name, MATHGL::Vector3(data[0], data[1], data[2]));
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::VEC2: {
					if (!CombineVecEdit::Data.count(_name)) {
						auto val = std::get<MATHGL::Vector2f>(material->get(name, mamber.name));
						CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 2, { val.x, val.y, 0.0f, 0.0f }) });
					}
					if (CombineVecEdit::Data.at(_name).draw()) {
						const auto& data = CombineVecEdit::Data.at(_name).getVectorData();
						material->set(name, mamber.name, MATHGL::Vector2f(data[0], data[1]));
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::INT: {
					auto val = std::get<int>(material->get(name, mamber.name));
					if (ImGui::DragInt(name.c_str(), &val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::FLOAT: {
					auto val = std::get<float>(material->get(name, mamber.name));
					if (ImGui::DragFloat(mamber.name.c_str(), &val, 0.1f)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::BOOL: {
					auto val = std::get<bool>(material->get(name, mamber.name));
					if (ImGui::Checkbox(mamber.name.c_str(), &val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::SAMPLER_2D: break;
				case RENDER::UNIFORM_TYPE::SAMPLER_3D: break;
				case RENDER::UNIFORM_TYPE::SAMPLER_CUBE: break;
				default: break;
				}
			}
		}
	}
	ImGui::End();
}
#endif

void drawComponent(ECS::TransformComponent* component) {
	if (ImGui::CollapsingHeader(component->getName().c_str(), ImGuiTreeNodeFlags_None)) {
		std::array<float, 3> vecPos = { component->getTransform().getLocalPosition().x,
			component->getTransform().getLocalPosition().y, component->getTransform().getLocalPosition().z };
		if (ImGui::DragFloat3("##transformPos", vecPos.data(), 0.1f)) {
			component->getTransform().setLocalPosition({ vecPos[0], vecPos[1], vecPos[2] });
		}
		std::array<float, 3> vecScale = { component->getTransform().getLocalScale().x,
			component->getTransform().getLocalScale().y, component->getTransform().getLocalScale().z };
		if (ImGui::DragFloat3("##transformScale", vecScale.data(), 0.1f)) {
			component->getTransform().setLocalScale({ vecScale[0], vecScale[1], vecScale[2] });
		}
		auto rotMat = MATHGL::Quaternion::ToEulerAngles(component->getTransform().getLocalRotation());
		std::array<float, 3> vecRotate = { rotMat.x, rotMat.y, rotMat.z };
		if (ImGui::DragFloat3("##transformRotate", vecRotate.data(), 0.1f)) {
			component->getTransform().setLocalRotation({ MATHGL::Vector3{vecRotate[0], vecRotate[1], vecRotate[2]} });
		}

		if (component->getTransform().isAnchorPivotMode()) {
			std::array<float, 2> vecAnchor = { component->getTransform().getLocalAnchor().x, component->getTransform().getLocalAnchor().y };
			if (ImGui::DragFloat2("##transformAnchor", vecAnchor.data(), 0.1f)) {
				component->getTransform().setLocalAnchor({ vecAnchor[0], vecAnchor[1] });
			}

			std::array<float, 2> vecPivot = { component->getTransform().getLocalPivot().x, component->getTransform().getLocalPivot().y };
			if (ImGui::DragFloat2("##transformPivot", vecPivot.data(), 0.1f)) {
				component->getTransform().setLocalPivot({ vecPivot[0], vecPivot[1] });
			}

			std::array<float, 2> vecSize = { component->getTransform().getLocalSize().x, component->getTransform().getLocalSize().y };
			if (ImGui::DragFloat2("##transformSize", vecSize.data(), 0.1f)) {
				component->getTransform().setLocalSize({ vecSize[0], vecSize[1] });
			}
		}
	}
}

void DebugRender::drawComponentInspector() {
	ImGui::Begin("Component Inspector");

	
	if (selectObj) {
		auto components = ECS::ComponentManager::getInstance()->getComponents(selectObj->getID());
		for (auto component : components) {
			if (component->getName() == "Transform") {
				drawComponent(reinterpret_cast<ECS::TransformComponent*>(component.getPtr().get()));
			}
		}
	}

	ImGui::End();

}

void DebugRender::drawTextureWatcher() {
#ifdef OPENGL_BACKEND
	ImGui::Begin("Texture Watcher");

	auto& renderer = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
	auto _renderer = reinterpret_cast<RENDER::GameRendererGl*>(&renderer);


	const char* items[] = { "Before Post Processing", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
	static int item_current = 0;
	ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items));
	{
		if (item_current == 0) {
			auto val = _renderer->mDeferredTexture;
			if (val) {
				auto winSize = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
				ImVec2 imWinSize = ImGui::GetWindowSize();

				float w = 0.0f;
				float h = 0.0f;
				
				if (imWinSize.x < imWinSize.y) {
					w = imWinSize.x;
					h = (winSize.y * imWinSize.x) / winSize.x;
				}
				else
				{
					w = (imWinSize.y * winSize.x) / winSize.y;
					h = imWinSize.y;
				}

				ImGui::Image((void*)val->id, ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
			}
		}
	}

	ImGui::End();
#endif
}

void drawMainWindow()
{

	static bool isOpen = true;
	//static const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
	//ImGuiID dockSpace = ImGui::GetID("MainWindowDockspace");
	//ImGui::DockSpace(dockSpace, ImVec2(0.0f, 0.0f), dockspaceFlags);
	{//menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Examples"))
			{
				ImGui::MenuItem("Main menu bar", NULL, &[](){});
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}

#ifdef IMGUI_HAS_VIEWPORT
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	auto h = ImGui::GetFrameHeight();
	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + h), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - h), ImGuiCond_Always);
	ImGui::SetNextWindowViewport(viewport->ID);
#else 
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif



	ImGui::Begin("##MainWin", &isOpen,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
	
	ImGui::End();
}

void DebugRender::draw(CORE_SYSTEM::Core& core) {

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplOpenGL3_NewFrame();
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		ImGui_ImplVulkan_NewFrame();
	}
#endif
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	drawMainWindow();

	{//Debug
		ImGui::Begin("Render pipeline");

		/*static bool isBloom = true;
		ImGui::Checkbox("Bloom", &isBloom);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::BLOOM, isBloom);

		static bool isGodRay = true;
		ImGui::Checkbox("God ray", &isGodRay);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::GOOD_RAYS, isGodRay);

		static bool isMotionBlur = true;
		ImGui::Checkbox("Motion blur", &isMotionBlur);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::MOTION_BLUR, isMotionBlur);

		static bool isFXAA = true;
		ImGui::Checkbox("FXAA", &isFXAA);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::FXAA, isFXAA);

		static bool isHDR = true;
		ImGui::Checkbox("HDR", &isHDR);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::HDR, isHDR);

		static bool isVIGNETTE = true;
		ImGui::Checkbox("VIGNETTE", &isVIGNETTE);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::VIGNETTE, isVIGNETTE);

		static bool isCOLOR_GRADING = true;
		ImGui::Checkbox("COLOR_GRADING", &isCOLOR_GRADING);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::COLOR_GRADING, isCOLOR_GRADING);

		static bool isCHROMATIC_ABBERATION = true;
		ImGui::Checkbox("CHROMATIC_ABBERATION", &isCHROMATIC_ABBERATION);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::CHROMATIC_ABBERATION, isCHROMATIC_ABBERATION);

		static bool isVOLUMETRIC_LIGHT = true;
		ImGui::Checkbox("VOLUMETRIC_LIGHT", &isVOLUMETRIC_LIGHT);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::VOLUMETRIC_LIGHT, isVOLUMETRIC_LIGHT);

		//func(core.renderer->pipeline.vignette);
		//constexpr auto info = serde::type_info<RENDER::Vignette>;
		buildWidget(core.renderer->pipeline.vignette);

		if (selectObjGui) {
			{
				ImGui::DragFloat("PivotX", &selectObjGui->transform->pivot.x, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("PivotY", &selectObjGui->transform->pivot.y, 0.1f, 0.0f, 1.0f);
			}
			{
				ImGui::DragFloat("AnchorX", &selectObjGui->transform->anchor.x, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("AnchorY", &selectObjGui->transform->anchor.y, 0.1f, 0.0f, 1.0f);
			}
			{
				ImGui::DragFloat("PoxX", &selectObjGui->transform->position.x, 1.0f);;
				ImGui::DragFloat("PosY", &selectObjGui->transform->position.y, 1.0f);
			}
			{
				ImGui::DragFloat("ScaleX", &selectObjGui->transform->scale.x, 1.0f);
				ImGui::DragFloat("ScaleY", &selectObjGui->transform->scale.y, 1.0f);
			}
			{
				ImGui::DragFloat("RotX", &selectObjGui->transform->rotation.x, 1.0f);
				ImGui::DragFloat("RotY", &selectObjGui->transform->rotation.z, 1.0f);
			}
		}*/

		/* {
			static float a = 0.00f;
			ImGui::DragFloat("PivotX", &a, 0.1f, 0.0f, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("PivotY", &b, 0.1f, 0.0f, 1.0f);
			core.renderer->f.pivot = MATHGL::Vector2f(a, b);
		}
		{
			static float a = 0.00f;
			ImGui::DragFloat("AnchorX", &a, 0.1f, 0.0f, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("AnchorY", &b, 0.1f, 0.0f, 1.0f);
			core.renderer->f.anchor = MATHGL::Vector2f(a, b);
		}
		{
			static float a = 0.00f;
			ImGui::DragFloat("PoxX", &a, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("PosY", &b, 1.0f);
			core.renderer->f.position = MATHGL::Vector3(a, b, 0.0f);
		}
		{
			static float a = 1.00f;
			ImGui::DragFloat("ScaleX", &a, 1.0f);
			static float b = 1.00f;
			ImGui::DragFloat("ScaleY", &b, 1.0f);
			core.renderer->f.scale = MATHGL::Vector3(a, b, 1.0f);
		}
		{
			static float a = 0.00f;
			ImGui::DragFloat("RotX", &a, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("RotY", &b, 1.0f);
			core.renderer->f.rotation = MATHGL::Vector3(a, 0.0f, b);
		}*/

		auto& im = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>();
		if (im.isGamepadExist(0)) {
			for (auto e : im.getGamepad(0).buttons) {
				ImGui::LabelText(("Btn" + std::to_string(static_cast<int>(e.first))).c_str(), std::to_string(e.second).c_str());
			}
			ImGui::LabelText("LStick", (std::to_string(im.getGamepad(0).leftSticX) + " " + std::to_string(im.getGamepad(0).leftSticY)).c_str());
			ImGui::LabelText("RStick", (std::to_string(im.getGamepad(0).rightSticX) + " " + std::to_string(im.getGamepad(0).rightSticX)).c_str());
			ImGui::LabelText("Triggers", (std::to_string(im.getGamepad(0).leftTrigger) + " " + std::to_string(im.getGamepad(0).rightTrigger)).c_str());
		}

		auto boxMaterial = ECS::ComponentManager::getInstance()->getComponent<ECS::MaterialRenderer>(ECS::Entity(3));
#ifdef OPENGL_BACKEND
		if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
			drawMaterialWidget(reinterpret_cast<RENDER::MaterialGl*>(boxMaterial.getPtr()->GetMaterialAtIndex(0).get()));
		}
#endif

		drawComponentInspector();
		drawTextureWatcher();
		ImGui::End();
	}
	drawWindowWidget(core);
	{
		drawNodeTree(core);
		//drawNodeTreeGui(core);
	}
	{
		//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		//ImGui::Begin("Scene Window");
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//ImGui::GetWindowDrawList()->AddImage(
		//	(void*)core.renderer->getResultTexture().getId(),
		//	ImVec2(ImGui::GetCursorScreenPos()),
		//	ImVec2(ImGui::GetCursorScreenPos().x + size.x / 2,
		//		ImGui::GetCursorScreenPos().y + size.y / 2), ImVec2(0, 1), ImVec2(1, 0));
		//ImGui::End();
	}
	{
		//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		//ImGui::Begin("Scene Window Light");
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//ImGui::GetWindowDrawList()->AddImage(
		//	(void*)core.renderer->getResultTexture2().getId(),
		//	ImVec2(ImGui::GetCursorScreenPos()),
		//	ImVec2(ImGui::GetCursorScreenPos().x + size.x / 2,
		//		ImGui::GetCursorScreenPos().y + size.y / 2), ImVec2(0, 1), ImVec2(1, 0));

		{
			//static float x = -20.0f;
			//static float y = 40.0f;
			//static float z = 10.0f;
			//if (ImGui::DragFloat("LightX", &x, 1.0f, -100.0f, 100.0f)) {
			//	core.renderer->setLightPos(x, y, z);
			//}
			//if (ImGui::DragFloat("LightY", &y, 1.0f, -100.0f, 100.0f)) {
			//	core.renderer->setLightPos(x, y, z);
			//}
			//if (ImGui::DragFloat("LightZ", &z, 1.0f, -100.0f, 100.0f)) {
			//	core.renderer->setLightPos(x, y, z);
			//}
		}

		//ImGui::End();
	}
	//ImGui::SFML::Render(*window);

	ImGui::Render();

	//

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
#endif
	
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
	//window->popGLStates();
}