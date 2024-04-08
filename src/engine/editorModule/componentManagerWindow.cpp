#include "componentManagerWindow.h"

#ifdef USE_EDITOR
#include <functional>
#include <map>
#include <string>

#include "editorRender.h"
#include "IconsFontAwesome5.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "utilsModule/weakPtr.h"
#include "coreModule/ecs/object.h"
#include "utilsModule/imguiHelper/imguiWidgets.h"
#include "misc/cpp/imgui_stdlib.h"
#include "utilsModule/assertion.h"

using namespace IKIGAI::EDITOR;


template<typename T>
void genComponentsStringArrayImpl(std::vector<std::string>& arr) {
	arr.push_back(IKIGAI::ECS::GetType<T>());
}

template<template<typename...> class Container, typename...ComponentType>
void genComponentsStringArray(std::vector<std::string>& arr, Container<ComponentType...> opt) {
	(genComponentsStringArrayImpl<ComponentType>(arr), ...);
}

std::vector<std::string> genComponentsStringArray() {
	std::vector<std::string> res;
	genComponentsStringArray(res, IKIGAI::ECS::ComponentsTypeProviderType{});
	return res;
}

//----------------------------------

const std::string& drawSearchBox(const std::vector<std::string>& autocomplete) {
	static std::string input;
	ImGui::InputText("##input_search_component_manager_win", &input);
	ImGui::SameLine();
	static bool isOpen = false;
	bool isFocused = ImGui::IsItemFocused();
	isOpen |= ImGui::IsItemActive();
	if (isOpen) {
		ImGui::SetNextWindowPos({ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y});
		ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x, 0});
		if (ImGui::Begin("##popup", &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Tooltip)) {
			ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
			isFocused |= ImGui::IsWindowFocused();
			for (int i = 0; i < autocomplete.size(); i++) {
				if (strstr(autocomplete[i].c_str(), input.c_str()) == NULL)
					continue;
				if (ImGui::Selectable(autocomplete[i].c_str()) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter))) {
					input = autocomplete[i];
					isOpen = false;
				}
			}
		}
		ImGui::End();
		isOpen &= isFocused;
	}
	return input;
}

//----------------------------------

template<typename T>
void addComponentForObjectImpl(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName) {
	auto t = IKIGAI::ECS::GetType<T>();
	if (compName == t) {
		obj->addComponent<T>();
	}
}

template<template<typename...> class Container, typename...ComponentType>
void addComponentForObject(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName, Container<ComponentType...> opt) {
	(addComponentForObjectImpl<ComponentType>(obj, compName), ...);
}

void addComponentForObject(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName) {
	addComponentForObject(obj, compName, IKIGAI::ECS::ComponentsTypeProviderType{});
}

//----------------------------------

template<typename T>
void removeComponentFromObjectImpl(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName) {
	if (compName == typeid(T).name()) {
		obj->removeComponent<T>();
	}
}

template<>
void removeComponentFromObjectImpl<IKIGAI::ECS::TransformComponent>(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName) {}

template<template<typename...> class Container, typename...ComponentType>
void removeComponentFromObject(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName, Container<ComponentType...> opt) {
	(removeComponentFromObjectImpl<ComponentType>(obj, compName), ...);
}

void removeComponentFromObject(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName) {
	removeComponentFromObject(obj, compName, IKIGAI::ECS::ComponentsTypeProviderType{});
}

//----------------------------------


template<class CLASS, class PTR>
void widgetFloat4(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {
	const std::string& propName = prop.getName();
	if (!IKIGAI::IMGUI::CombineVecEdit::Data.contains(propName)) {
		IKIGAI::IMGUI::CombineVecEdit::Data.insert({propName, IKIGAI::IMGUI::CombineVecEdit(propName, 4, IKIGAI::IMGUI::CombineVecEdit::MODE::POS)});
	}
	auto val = prop.get(*comp);
	if (IKIGAI::IMGUI::CombineVecEdit::Data.at(propName).draw(val)) {
		prop.set(*comp, val);
	}
}

template<class CLASS, class PTR>
void widgetColor4(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {
	const std::string& propName = prop.getName();
	if (!IKIGAI::IMGUI::CombineVecEdit::Data.contains(propName)) {
		IKIGAI::IMGUI::CombineVecEdit::Data.insert({propName, IKIGAI::IMGUI::CombineVecEdit(propName, 4, IKIGAI::IMGUI::CombineVecEdit::MODE::COLOR)});
	}
	auto val = prop.get(*comp);
	if (IKIGAI::IMGUI::CombineVecEdit::Data.at(propName).draw(val)) {
		prop.set(*comp, val);
	}
}
template<class CLASS, class PTR>
void widgetFloat3(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {
	const std::string& propName = prop.getName();
	if (!IKIGAI::IMGUI::CombineVecEdit::Data.contains(propName)) {
		IKIGAI::IMGUI::CombineVecEdit::Data.insert({propName, IKIGAI::IMGUI::CombineVecEdit(propName, 3, IKIGAI::IMGUI::CombineVecEdit::MODE::POS)});
	}
	auto val = prop.get(*comp);
	if (IKIGAI::IMGUI::CombineVecEdit::Data.at(propName).draw(val)) {
		prop.set(*comp, val);
	}
}

template<class CLASS, class PTR>
void widgetColor3(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {
	const std::string& propName = prop.getName();
	if (!IKIGAI::IMGUI::CombineVecEdit::Data.contains(propName)) {
		IKIGAI::IMGUI::CombineVecEdit::Data.insert({propName, IKIGAI::IMGUI::CombineVecEdit(propName, 3, IKIGAI::IMGUI::CombineVecEdit::MODE::COLOR)});
	}
	auto val = prop.get(*comp);
	if (IKIGAI::IMGUI::CombineVecEdit::Data.at(propName).draw(val)) {
		prop.set(*comp, val);
	}
}

template<class CLASS, class PTR>
void widgetFloat(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) { }
template<class CLASS>
void widgetFloat(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, float>& prop) {
	const std::string& propName = prop.getName();
	float val = prop.get(*comp);

	//const auto flagsRageData = prop.getMetadata().at();
	//const auto flagsStepData = prop.get_metadata();
	auto _speed = 1;
	auto _min = 0.0f;
	auto _max = 0.0f;
	if (prop.getMetadata().contains(IKIGAI::UTILS::MetaParam::EDIT_RANGE)) {
		auto v = std::get<IKIGAI::MATH::Vector2f>(prop.getMetadata().at(IKIGAI::UTILS::MetaParam::EDIT_RANGE));
		_min = v.x;
		_max = v.y;
	}
	if (prop.getMetadata().contains(IKIGAI::UTILS::MetaParam::EDIT_STEP)) {
		_speed = std::get<float>(prop.getMetadata().at(IKIGAI::UTILS::MetaParam::EDIT_STEP));
	}
	if (ImGui::DragFloat(propName.c_str(), &val, _speed, _min, _max)) {
		prop.set(*comp, val);
	}
}

template<class CLASS, class PTR>
void widgetInt(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {}
template<class CLASS>
void widgetInt(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, int>& prop) {
	const std::string& propName = prop.getName();
	auto val = prop.get(*comp);

	//const auto flagsRageData = prop.getMetadata().at();
	//const auto flagsStepData = prop.get_metadata();
	auto _speed = 1;
	auto _min = 0;
	auto _max = 0;
	if (prop.getMetadata().contains(IKIGAI::UTILS::MetaParam::EDIT_RANGE)) {
		auto v = std::get<IKIGAI::MATH::Vector2f>(prop.getMetadata().at(IKIGAI::UTILS::MetaParam::EDIT_RANGE));
		_min = v.x;
		_max = v.y;
	}
	if (prop.getMetadata().contains(IKIGAI::UTILS::MetaParam::EDIT_STEP)) {
		_speed = std::get<int>(prop.getMetadata().at(IKIGAI::UTILS::MetaParam::EDIT_STEP));
	}
	if (ImGui::DragInt(propName.c_str(), &val, _speed, _min, _max)) {
		prop.set(*comp, val);
	}
}

template<class CLASS, class PTR>
void widgetBool(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {}
template<class CLASS>
void widgetBool(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, bool>& prop) {
	const std::string& propName = prop.getName();
	auto val = prop.get(*comp);
	if (ImGui::Checkbox(propName.c_str(), &val)) {
		prop.set(*comp, val);
	}
}

//TODO: add dnd
//void widgetStringFixFileChoose(UTILS::WeakPtr<ECS::Component> component, const rttr::property& prop) {
//	const std::string propName = prop.get_name().to_string();
//	std::unique_ptr<rttr::instance> inst;
//	getInstance(component, inst);
//
//	auto fileExt = prop.get_metadata(EditorMetaInfo::FILE_EXTENSION).to_string();
//	auto val = prop.get_value(*inst).get_value<std::string>();
//	if (ImGui::InputText(propName.c_str(), &val)) {
//		std::ignore = prop.set_value(*inst, val);
//	}
//	ImGui::SameLine();
//	if (ImGui::Button(ICON_FA_FILE)) {
//		popupStates["file_chooser"] = true;
//		fileFormatsCb = [fileExt] {
//			return fileExt;
//		};
//		fileChooserCb = [prop, component](std::string path) mutable {
//			std::unique_ptr<rttr::instance> inst;
//			getInstance(component, inst);
//			std::ignore = prop.set_value(*inst, path);
//		};
//	}
//}

template<class CLASS, class PTR>
void widgetString(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {}
template<class CLASS>
void widgetString(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, const std::string&>& prop) {
	const std::string& propName = prop.getName();
	auto val = prop.get(*comp);
	if (ImGui::InputText(propName.c_str(), &val)) {
		prop.set(*comp, val);
	}
}
template<class CLASS>
void widgetString(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, std::string>& prop) {
	const std::string& propName = prop.getName();
	auto val = prop.get(*comp);
	if (ImGui::InputText(propName.c_str(), &val)) {
		prop.set(*comp, val);
	}
}

//void widgetStringArray(UTILS::WeakPtr<ECS::Component> component, const rttr::property& prop) {
//	const std::string propName = prop.get_name().to_string();
//	std::unique_ptr<rttr::instance> inst;
//	getInstance(component, inst);
//	ImGui::Text(propName.c_str());
//	auto val = prop.get_value(*inst).get_value<std::vector<std::string>>();
//	for (auto& e : val) {
//		if (ImGui::InputText(propName.c_str(), &e)) {
//			std::ignore = prop.set_value(*inst, val);
//		}
//	}
//}
//
//void widgetModelLod(UTILS::WeakPtr<ECS::Component> component, const rttr::property& prop) {
//	const std::string propName = prop.get_name().to_string();
//	std::unique_ptr<rttr::instance> inst;
//	getInstance(component, inst);
//	ImGui::Text(propName.c_str());
//	auto val = prop.get_value(*inst).get_value<std::vector<ECS::ModelLODRenderer::ModelLodRefl>>();
//	int i = 0;
//	for (auto& e : val) {
//		ImGui::PushID(i);
//		if (ImGui::DragFloat("Distance", &e.m_distance)) {
//			std::ignore = prop.set_value(*inst, val);
//		}
//		ImGui::SameLine();
//		if (ImGui::InputText("Model", &e.m_path, ImGuiInputTextFlags_EnterReturnsTrue)) {
//			std::ignore = prop.set_value(*inst, val);
//		}
//		ImGui::SameLine();
//		if (ImGui::Button("X")) {
//			val.erase(val.begin() + i);
//			std::ignore = prop.set_value(*inst, val);
//		}
//		ImGui::PopID();
//		i++;
//	}
//	if (ImGui::Button("Add")) {
//		val.push_back({});
//		std::ignore = prop.set_value(*inst, val);
//	}
//}

//void widgetMaterial(UTILS::WeakPtr<ECS::Component> component, const rttr::property& prop) {
//	const std::string propName = prop.get_name().to_string();
//	std::unique_ptr<rttr::instance> inst;
//	getInstance(component, inst);
//	ImGui::Text(propName.c_str());
//	auto val = prop.get_value(*inst).get_value<ECS::MaterialRenderer*>();
//	int i = 0;
//	const auto& names = val->getMaterialNames();
//	for (auto& e : val->getMaterials()) {
//		if (!e) {
//			break;
//		}
//		ImGui::PushID(i);
//		if (ImGui::InputText(names[i].c_str(), &e->mPath, ImGuiInputTextFlags_EnterReturnsTrue)) {
//			//TODO: check file exist
//			val->setMaterial(i, RESOURCES::ServiceManager::Get<RESOURCES::MaterialLoader>().loadResource(e->mPath));
//			//val->setMaterial(i, RESOURCES::MaterialLoader::Create(e->mPath));
//		}
//		ImGui::SameLine();
//		if (ImGui::Button(ICON_FA_FILE)) {
//			popupStates["file_chooser"] = true;
//			fileFormatsCb = [] {
//				return ".mat";
//			};
//			fileChooserCb = [val, i](std::string path) mutable {
//				if (val) {
//					auto mat = RESOURCES::ServiceManager::Get<RESOURCES::MaterialLoader>().loadResource(path);
//					//RESOURCES::MaterialLoader::Create(path);
//					val->setMaterial(i, mat);
//				}
//			};
//		}
//		++i;
//		ImGui::PopID();
//	}
//}
template<class CLASS, class PTR, typename std::enable_if<!std::is_enum<PTR>::value>::type* = nullptr>
void widgetCombo(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {}

template<class CLASS, class PTR, typename std::enable_if<std::is_enum<PTR>::value>::type* = nullptr>
void widgetCombo(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {
	//TODO: support my enums
	const std::string propName = prop.getName();
	
	auto val = prop.get(*comp);
	constexpr auto enumNames = magic_enum::enum_names<PTR>();
	constexpr auto enumValues = magic_enum::enum_values<PTR>();

	int itemCurrentIndex = [&enumValues, &val]() {
		int i = 0;
		for (auto& e : enumValues) {
			if (e == val) {
				break;
			}
			i++;
		}
		return i;
	}();
	const auto comboLabel = enumNames[itemCurrentIndex];
	if (ImGui::BeginCombo(propName.c_str(), comboLabel.data())) {
		for (int n = 0; n < enumNames.size(); n++) {
			const bool isSelected = (itemCurrentIndex == n);
			if (ImGui::Selectable(enumNames[n].data(), isSelected)) {
				itemCurrentIndex = n;
				prop.set(*comp, enumValues[itemCurrentIndex]);
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

//template<class T>
//auto GetMembers() {
//	return std::tuple{};
//}
//
//template<>
//auto GetMembers<IKIGAI::ECS::TransformComponent>() {
//	return IKIGAI::ECS::TransformComponent::GetMembers();
//}

//static std::map<IKIGAI::UTILS::WidgetType, std::function<void(IKIGAI::UTILS::MemberInfoBase&)>> widgetDrawer = {
//	{IKIGAI::UTILS::WidgetType::DRAG_FLOAT_3, &widgetFloat3},
//	//	{EditorMetaInfo::DRAG_COLOR_3, &widgetColor3},
//	//	{EditorMetaInfo::DRAG_FLOAT, &widgetFloat},
//	//	{EditorMetaInfo::BOOL, &widgetBool},
//	//	{EditorMetaInfo::STRING, &widgetString},
//	//	{EditorMetaInfo::STRING_WITH_FILE_CHOOSE, &widgetStringFixFileChoose},
//	//	{EditorMetaInfo::COMBO, &widgetCombo},
//	//	{EditorMetaInfo::STRINGS_ARRAY, &widgetStringArray},
//	//	{EditorMetaInfo::MATERIAL, &widgetMaterial},
//	//	{EditorMetaInfo::MODEL_LOD, &widgetModelLod},
//};

template<typename T>
void getPropsImpl(IKIGAI::UTILS::WeakPtr<IKIGAI::ECS::Component> comp) {
	if (comp->getName() == IKIGAI::ECS::GetComponentName<T>()) {
		auto props = T::GetMembers();
		std::apply([&comp]<typename... Args> (Args&... tpl) {
			auto drawElem = [&comp](auto& prop) {
				const auto propName = prop.getName();
				const auto wType = std::get<IKIGAI::UTILS::WidgetType>(prop.getMetadata().at(IKIGAI::UTILS::MetaParam::EDIT_WIDGET));
				switch (wType) {
				case IKIGAI::UTILS::WidgetType::DRAG_FLOAT_4: {
					widgetFloat4(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::DRAG_COLOR_4: {
					widgetColor4(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::DRAG_FLOAT_3: {
					widgetFloat3(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::DRAG_COLOR_3: {
					widgetColor3(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::DRAG_FLOAT: {
					widgetFloat(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::DRAG_INT: {
					widgetInt(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::BOOL: {
					widgetBool(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::STRING: {
					widgetString(static_cast<T*>(comp.get()), prop);
					break;
				}
				case IKIGAI::UTILS::WidgetType::COMBO: {
					widgetCombo(static_cast<T*>(comp.get()), prop);
					break;
				}
				}
			};
			(drawElem(tpl), ...);
		}, props);
	}
}

template<template<typename...> class Container, typename...ComponentType>
void getProps(IKIGAI::UTILS::WeakPtr<IKIGAI::ECS::Component> comp, Container<ComponentType...> opt) {
	(getPropsImpl<ComponentType>(comp), ...);
}

void getProps(IKIGAI::UTILS::WeakPtr<IKIGAI::ECS::Component> comp) {
	getProps(comp, IKIGAI::ECS::ComponentsTypeProviderType{});
}

void ComponentManagerWindow::draw() {
	const auto selectObject = EditorRender::GlobalState.mSelectObject;

	ImGui::Begin((ICON_FA_DATABASE + std::string(" Component Inspector")).c_str());
	if (!selectObject) {
		ImGui::Text("Object not selected");
		ImGui::End();
		return;
	}

	static std::map<std::string, std::string> iconComp = {
		{"TransformComponent", ICON_FA_GLOBE},
		{"SpotLight", ICON_FA_LIGHTBULB},
		{"DirectionalLight", ICON_FA_SUN},
		{"PointLight", ICON_FA_LIGHTBULB},
		{"AmbientSphereLight", ICON_FA_CIRCLE},
		{"AmbientLight", ICON_FA_SQUARE},
		{"CameraComponent", ICON_FA_CAMERA},
		{"AudioListenerComponent", ICON_FA_MUSIC},
		{"AudioComponent", ICON_FA_MUSIC},
		{"InputComponent", ICON_FA_HAND_POINT_UP},
		{"LogicComponent", ICON_FA_CODE},
		{"MaterialRenderer", ICON_FA_IMAGE},
		{"ModelRenderer", ICON_FA_CUBE},
		{"PhysicsComponent", ICON_FA_WEIGHT},
		{"ScriptComponent", ICON_FA_CODE},
		{"Skeletal", ICON_FA_SKULL},
	};


	ImGui::Text(("Id: " + std::to_string(static_cast<int>(selectObject->getID()))).c_str());

	auto name = selectObject->getName();
	if (IMGUI::InputText("Name:", "##object_name", name)) {
		selectObject->setName(name);
	}

	auto tag = selectObject->getTag();
	if (IMGUI::InputText("Tag:", "##object_tag", tag)) {
		selectObject->setTag(tag);
	}

	//Add new component to object
	static auto componentNames = genComponentsStringArray();
	const auto& componentToAdd = drawSearchBox(componentNames);
	if (ImGui::Button("Add")) {
		if (!componentToAdd.empty()) {
			addComponentForObject(selectObject, componentToAdd);
		}
	}

	//Object components
	auto components = ECS::ComponentManager::GetInstance().getComponents(selectObject->getID());
	for (auto& component : components) {
		auto title = iconComp[component->getName()] + std::string(" ") + component->getName();
		bool needDelComponent = true;
		if (ImGui::CollapsingHeader(title.c_str(), &needDelComponent, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow)) {
			if (!needDelComponent) {
				removeComponentFromObject(selectObject, component->getTypeidName());
				break;
			}
			getProps(component);
		}
	}
	ImGui::End();
}
#endif
