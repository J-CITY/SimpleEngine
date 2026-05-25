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
#include "utilsModule/reflection/reflection.h"
#include "commandHistory.h"

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


std::vector<std::string> genComponentsStringArray2() {
	std::vector<std::string> res;
	auto& manager = IKIGAI::UTILS::ReflectionManager::Instance();
	for (auto* typeInfo : manager.getTypesInGroup("Component")) {
		if (typeInfo->mFields.contains("ClassName")) {
			auto shortName = typeInfo->mFields.at("ClassName").get<std::string>(nullptr);
			if (shortName != "ComponentBase" && shortName != "TransformComponent") {
				res.push_back(shortName);
			}
		}
	}
	return res;
}

//----------------------------------

const std::string& drawSearchBox(const char* lable, const std::vector<std::string>& autocomplete) {
	static std::map<std::string, std::string> inputs;
	static std::map<std::string, bool> isOpens;

	std::string& input = inputs[lable];
	bool& isOpen = isOpens[lable];

	ImGui::InputText(lable, &input);
	ImGui::SameLine();
	
	bool isFocused = ImGui::IsItemFocused();
	isOpen |= ImGui::IsItemActive();
	if (isOpen) {
		ImGui::SetNextWindowPos({ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y});
		ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x, 0});
		const std::string popUpId = std::string("##popup") + lable;
		if (ImGui::Begin(popUpId.c_str(), &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Tooltip)) {
			ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
			isFocused |= ImGui::IsWindowFocused();
			for (int i = 0; i < autocomplete.size(); i++) {
				if (strstr(autocomplete[i].c_str(), input.c_str()) == NULL)
					continue;
				if (ImGui::Selectable(autocomplete[i].c_str()) || (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))) {
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

void addComponentForObject2(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName) {
	auto& manager = IKIGAI::UTILS::ReflectionManager::Instance();
	if (auto* typeInfo = manager.getType(compName)) {
		if (typeInfo->mMethods.contains("AddComponent")) {
			typeInfo->mMethods.at("AddComponent").invoke(nullptr, {obj});
		}
	}
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

void removeComponentFromObject2(std::shared_ptr<IKIGAI::ECS::Object> obj, std::string_view compName) {
	auto& manager = IKIGAI::UTILS::ReflectionManager::Instance();
	if (auto* typeInfo = manager.getType(compName)) {
		if (typeInfo->mMethods.contains("RemoveComponent")) {
			typeInfo->mMethods.at("RemoveComponent").invoke(nullptr, {obj});
		}
	}
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
void widgetFloat(CLASS* comp, IKIGAI::UTILS::MemberInfo<CLASS, PTR>& prop) {}
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

template<typename T>
void getPropsImpl(IKIGAI::UTILS::WeakPtr<IKIGAI::ECS::ComponentBase> comp) {
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
void getProps(IKIGAI::UTILS::WeakPtr<IKIGAI::ECS::ComponentBase> comp, Container<ComponentType...> opt) {
	(getPropsImpl<ComponentType>(comp), ...);
}

void getProps(IKIGAI::UTILS::WeakPtr<IKIGAI::ECS::ComponentBase> comp) {
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


	ImGui::Text(("Id: " + std::to_string(static_cast<int>(selectObject->getID().getUniqueId()))).c_str());

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
	const auto& componentToAdd = drawSearchBox("##input_search_component_manager_win", componentNames);
	if (ImGui::Button("Add")) {
		if (!componentToAdd.empty()) {
			addComponentForObject(selectObject, componentToAdd);
		}
	}

	//Object components
	//auto components = ECS::ComponentManager::GetInstance().getComponents(selectObject->getID());
	auto components = selectObject->getComponents();
	for (auto& [_, component] : components) {
		auto title = iconComp[component->getName()] + std::string(" ") + component->getName();
		bool needDelComponent = true;
		if (ImGui::CollapsingHeader(title.c_str(), &needDelComponent, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow)) {
			if (!needDelComponent) {
				removeComponentFromObject(selectObject, component->getTypeidName());
				break;
			}

			if (component->getName() == "MaterialRenderer") {
				ECS::MaterialRenderer::InitReflection();

				auto& manager = UTILS::ReflectionManager::Instance();
				auto* typeInfo = manager.getType<ECS::MaterialRenderer>();
				auto& field = typeInfo->mFields.at("materials");
				auto materials = field.get<ECS::MaterialRenderer::MaterialList>(component.get());

				for (auto& material : materials) {
					if (!material) {
						break;
					}
					auto shader = material->getShader();
					if (!shader) {
						continue;
					}
					const auto& refl = shader->getReflection();

					for (auto& data : refl.mUniforms) {
						auto& name = data.mName;
						if (material->isEngineUniform(name)) {
							continue;
						}
						if (data.mType == RENDER::ShaderReflection::UniformType::SAMPLER_2D) {
							auto val = std::get<std::shared_ptr<RENDER::TextureInterface>>(material->get(name));
							ImGui::PushID(("##" + name).c_str());

							//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
							if (ImGui::Selectable(("##" + name).c_str(), false, 0, ImVec2(100, 100))) {

							}
							auto pos = ImGui::GetCursorPos();
							ImGui::SetCursorPos(ImVec2(pos.x, pos.y - 100));

							if (val) {
								ImGui::Image(reinterpret_cast<ImTextureID>(val->getImguiId()), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
							}
							ImGui::PopID();

						} else if (data.mType == RENDER::ShaderReflection::UniformType::UNIFORM_BUFFER) {
							for (const auto& mamber : data.mMembers) {
								auto _name = mamber.mName;
								switch (mamber.mType) {
								case RENDER::ShaderReflection::UniformType::MAT4: break;
								case RENDER::ShaderReflection::UniformType::MAT3: break;
								case RENDER::ShaderReflection::UniformType::VEC4: {
									if (!IMGUI::CombineVecEdit::Data.count(_name)) {
										IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name, 4)});
									}
									auto val = std::get<MATH::Vector4f>(material->get(name + _name));
									if (IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
										material->set(name + _name, val);
									}
									break;
								}
								case RENDER::ShaderReflection::UniformType::VEC3: {
									if (!IMGUI::CombineVecEdit::Data.count(_name)) {
										IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name, 3)});
									}
									auto val = std::get<MATH::Vector3f>(material->get(name + _name));
									if (IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
										material->set(name + _name, val);
									}
									break;
								}
								case RENDER::ShaderReflection::UniformType::VEC2: {
									if (!IMGUI::CombineVecEdit::Data.count(_name)) {
										IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name,
											2)});
									}
									auto val =
										std::get<MATH::Vector2f>(material->get(name + _name)); if
										(IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
										material->set(name + _name, val);
									}
									break;
								}
								case RENDER::ShaderReflection::UniformType::INT: {
									if (!IMGUI::ScalarEdit::Data.count(_name)) {
										IMGUI::ScalarEdit::Data.insert({_name, IMGUI::ScalarEdit(_name, IMGUI::ScalarEdit::TYPE::INT)});
									}
									auto val = std::get<int>(material->get(name + _name));
									if (IMGUI::ScalarEdit::Data.at(_name).draw(val)) {
										material->set(name + _name, val);
									}
									break;
								}
								case RENDER::ShaderReflection::UniformType::FLOAT: {
									if (!IMGUI::ScalarEdit::Data.count(_name)) {
										IMGUI::ScalarEdit::Data.insert({_name, IMGUI::ScalarEdit(_name, IMGUI::ScalarEdit::TYPE::FLOAT)});
									}
									auto val = std::get<float>(material->get(name + _name));
									if (IMGUI::ScalarEdit::Data.at(_name).draw(val)) {
										material->set(name + _name, val);
									}
									break;
								}
								case RENDER::ShaderReflection::UniformType::BOOL: {
									if (!IMGUI::ScalarEdit::Data.count(_name)) {
										IMGUI::ScalarEdit::Data.insert({_name, IMGUI::ScalarEdit(_name, IMGUI::ScalarEdit::TYPE::BOOL)});
									}
									auto val = std::get<bool>(material->get(name + _name));
									if (IMGUI::ScalarEdit::Data.at(_name).draw(val)) {
										material->set(name + _name, val);
									}
									break;
								}
								case RENDER::ShaderReflection::UniformType::SAMPLER_2D: break;
								case RENDER::ShaderReflection::UniformType::SAMPLER_3D: break;
								case RENDER::ShaderReflection::UniformType::SAMPLER_CUBE: break;
								default: break;
								}
							}
						}
					}
				}
			} else {
				getProps(component);
			}
		}
	}
	ImGui::End();
}

void IKIGAI::EDITOR::DrawMaterialWidget(ECS::MaterialRenderer::MaterialList& materials) {
	for (auto& material : materials) {
		if (!material) {
			break;
		}
		auto shader = material->getShader();
		if (!shader) {
			continue;
		}
		const auto& refl = shader->getReflection();

		for (auto& data : refl.mUniforms) {
			auto& name = data.mName;
			if (material->isEngineUniform(name)) {
				continue;
			}
			if (data.mType == RENDER::ShaderReflection::UniformType::SAMPLER_2D) {
				auto val = std::get<std::shared_ptr<RENDER::TextureInterface>>(material->get(name));
				ImGui::PushID(("##" + name).c_str());

				//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
				if (ImGui::Selectable(("##" + name).c_str(), false, 0, ImVec2(100, 100))) {

				}
				auto pos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(ImVec2(pos.x, pos.y - 100));

				if (val) {
					ImGui::Image(reinterpret_cast<ImTextureID>(val->getImguiId()), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
				}
				ImGui::PopID();

			} else if (data.mType == RENDER::ShaderReflection::UniformType::UNIFORM_BUFFER) {
				for (const auto& mamber : data.mMembers) {
					auto _name = mamber.mName;
					switch (mamber.mType) {
					case RENDER::ShaderReflection::UniformType::MAT4: break;
					case RENDER::ShaderReflection::UniformType::MAT3: break;
					case RENDER::ShaderReflection::UniformType::VEC4: {
						if (!IMGUI::CombineVecEdit::Data.count(_name)) {
							IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name, 4)});
						}
						auto val = std::get<MATH::Vector4f>(material->get(name + _name));
						if (IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
							material->set(name + _name, val);
						}
						break;
					}
					case RENDER::ShaderReflection::UniformType::VEC3: {
						if (!IMGUI::CombineVecEdit::Data.count(_name)) {
							IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name, 3)});
						}
						auto val = std::get<MATH::Vector3f>(material->get(name + _name));
						if (IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
							material->set(name + _name, val);
						}
						break;
					}
					case RENDER::ShaderReflection::UniformType::VEC2: {
						if (!IMGUI::CombineVecEdit::Data.count(_name)) {
							IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name,
								2)});
						}
						auto val =
							std::get<MATH::Vector2f>(material->get(name + _name)); if
							(IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
							material->set(name + _name, val);
						}
						break;
					}
					case RENDER::ShaderReflection::UniformType::INT: {
						if (!IMGUI::ScalarEdit::Data.count(_name)) {
							IMGUI::ScalarEdit::Data.insert({_name, IMGUI::ScalarEdit(_name, IMGUI::ScalarEdit::TYPE::INT)});
						}
						auto val = std::get<int>(material->get(name + _name));
						if (IMGUI::ScalarEdit::Data.at(_name).draw(val)) {
							material->set(name + _name, val);
						}
						break;
					}
					case RENDER::ShaderReflection::UniformType::FLOAT: {
						if (!IMGUI::ScalarEdit::Data.count(_name)) {
							IMGUI::ScalarEdit::Data.insert({_name, IMGUI::ScalarEdit(_name, IMGUI::ScalarEdit::TYPE::FLOAT)});
						}
						auto val = std::get<float>(material->get(name + _name));
						if (IMGUI::ScalarEdit::Data.at(_name).draw(val)) {
							material->set(name + _name, val);
						}
						break;
					}
					case RENDER::ShaderReflection::UniformType::BOOL: {
						if (!IMGUI::ScalarEdit::Data.count(_name)) {
							IMGUI::ScalarEdit::Data.insert({_name, IMGUI::ScalarEdit(_name, IMGUI::ScalarEdit::TYPE::BOOL)});
						}
						auto val = std::get<bool>(material->get(name + _name));
						if (IMGUI::ScalarEdit::Data.at(_name).draw(val)) {
							material->set(name + _name, val);
						}
						break;
					}
					case RENDER::ShaderReflection::UniformType::SAMPLER_2D: break;
					case RENDER::ShaderReflection::UniformType::SAMPLER_3D: break;
					case RENDER::ShaderReflection::UniformType::SAMPLER_CUBE: break;
					default: break;
					}
				}
			}
		}
	}
}

void ComponentInspectorWindow::drawObjectInfo(std::shared_ptr<ECS::Object> selectObject) {
	ImGui::Text(("Id: " + std::to_string(static_cast<int>(selectObject->getID().getUniqueId()))).c_str());

	auto name = selectObject->getName();
	if (IMGUI::InputText("Name:", "##object_name_v2", name)) {
		selectObject->setName(name);
	}

	auto tag = selectObject->getTag();
	if (IMGUI::InputText("Tag:", "##object_tag_v2", tag)) {
		selectObject->setTag(tag);
	}
}

void ComponentInspectorWindow::drawAddComponent(std::shared_ptr<ECS::Object> selectObject) {
	static auto componentNames = genComponentsStringArray2();
	const auto& componentToAdd = drawSearchBox("##input_search_component_manager_win_id", componentNames);
	if (ImGui::Button("Add")) {
		if (!componentToAdd.empty()) {
			addComponentForObject2(selectObject, componentToAdd);
		}
	}
}

void ComponentInspectorWindow::drawComponent(std::shared_ptr<ECS::Object> selectObject, UTILS::WeakPtr<ECS::ComponentBase> component, const std::string& compName, const std::string& typeIdName, const std::string& iconStr) {
	auto title = iconStr + std::string(" ") + compName;

	bool needDelComponent = true;
	if (ImGui::CollapsingHeader(title.c_str(), &needDelComponent, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow)) {
		if (!needDelComponent) {
			removeComponentFromObject2(selectObject, typeIdName);
			return;
		}

		auto* typeInfo = UTILS::ReflectionManager::Instance().getType(typeIdName);
		if (typeInfo) {
			for (auto& [fieldName, field] : typeInfo->mFields) {
				auto wType = field.metadata.get<UTILS::WidgetType>("EditWidget", UTILS::WidgetType::NONE);
				auto wFlags = field.metadata.get<std::vector<std::string>>("Flags", {});

				if (std::find(wFlags.begin(), wFlags.end(), "USE_IN_EDITOR_COMPONENT_INSPECTOR") == wFlags.end()) {
					continue;
				}
				
				if (wType == UTILS::WidgetType::NONE) {
					if (field.typeId == UTILS::ReflectionManager::GetTypeId<float>()) wType = UTILS::WidgetType::DRAG_FLOAT;
					else if (field.typeId == UTILS::ReflectionManager::GetTypeId<int>()) wType = UTILS::WidgetType::DRAG_INT;
					else if (field.typeId == UTILS::ReflectionManager::GetTypeId<bool>()) wType = UTILS::WidgetType::BOOL;
					else if (field.typeId == UTILS::ReflectionManager::GetTypeId<std::string>()) wType = UTILS::WidgetType::STRING;
					else if (field.typeId == UTILS::ReflectionManager::GetTypeId<MATH::Vector2f>()) wType = UTILS::WidgetType::DRAG_FLOAT_3;
					else if (field.typeId == UTILS::ReflectionManager::GetTypeId<MATH::Vector3f>()) wType = UTILS::WidgetType::DRAG_FLOAT_3;
					else if (field.typeId == UTILS::ReflectionManager::GetTypeId<MATH::Vector4f>()) wType = UTILS::WidgetType::DRAG_FLOAT_4;
				}

				float min = 0.0f, max = 0.0f, step = 0.1f;
				if (field.metadata.contains("EditRange")) {
					auto range = field.metadata.get<MATH::Vector2f>("EditRange", MATH::Vector2f(0.0f, 0.0f));
					min = range.x;
					max = range.y;
				}
				if (field.metadata.contains("EditStep")) {
					step = field.metadata.get<float>("EditStep", 0.1f);
				}

				ImGuiID fieldId = ImGui::GetID(fieldName.c_str());
				ImGui::PushID(fieldName.c_str());

				bool changed = false;
				bool activated = false;
				bool deactivated = false;

				switch (wType) {
				case UTILS::WidgetType::DRAG_FLOAT: {
					float val = field.get<float>(component.get());
					changed = ImGui::DragFloat(fieldName.c_str(), &val, step, min, max);
					activated = ImGui::IsItemActivated();
					deactivated = ImGui::IsItemDeactivatedAfterEdit();
					if (changed) field.set<float>(component.get(), val);
					break;
				}
				case UTILS::WidgetType::DRAG_INT: {
					int val = field.get<int>(component.get());
					changed = ImGui::DragInt(fieldName.c_str(), &val, step, min, max);
					activated = ImGui::IsItemActivated();
					deactivated = ImGui::IsItemDeactivatedAfterEdit();
					if (changed) field.set<int>(component.get(), val);
					break;
				}
				case UTILS::WidgetType::BOOL: {
					bool val = field.get<bool>(component.get());
					changed = ImGui::Checkbox(fieldName.c_str(), &val);
					activated = ImGui::IsItemActivated();
					deactivated = ImGui::IsItemDeactivatedAfterEdit();
					if (changed) field.set<bool>(component.get(), val);
					break;
				}
				case UTILS::WidgetType::STRING: {
					std::string val = field.get<std::string>(component.get());
					changed = IMGUI::InputText(fieldName, "##str_edit", val);
					activated = ImGui::IsItemActivated();
					deactivated = ImGui::IsItemDeactivatedAfterEdit();
					if (changed) field.set<std::string>(component.get(), val);
					break;
				}
				case UTILS::WidgetType::DRAG_FLOAT_3: {
					if (field.typeId == UTILS::ReflectionManager::GetTypeId<MATH::Vector2f>()) {
						MATH::Vector2f val = field.get<MATH::Vector2f>(component.get());
						int state = IMGUI::DrawCombineVecEdit(fieldName, 2, reinterpret_cast<float*>(&val), min, max, step, false);
						changed = state & 1; activated = state & 2; deactivated = state & 4;
						if (changed) field.set<MATH::Vector2f>(component.get(), val);
					} else {
						MATH::Vector3f val = field.get<MATH::Vector3f>(component.get());
						int state = IMGUI::DrawCombineVecEdit(fieldName, 3, reinterpret_cast<float*>(&val), min, max, step, false);
						changed = state & 1; activated = state & 2; deactivated = state & 4;
						if (changed) field.set<MATH::Vector3f>(component.get(), val);
					}
					break;
				}
				case UTILS::WidgetType::DRAG_FLOAT_4: {
					MATH::Vector4f val = field.get<MATH::Vector4f>(component.get());
					int state = IMGUI::DrawCombineVecEdit(fieldName, 4, reinterpret_cast<float*>(&val), min, max, step, false);
					changed = state & 1; activated = state & 2; deactivated = state & 4;
					if (changed) field.set<MATH::Vector4f>(component.get(), val);
					break;
				}
				case UTILS::WidgetType::DRAG_COLOR_3: {
					MATH::Vector3f val = field.get<MATH::Vector3f>(component.get());
					int state = IMGUI::DrawCombineVecEdit(fieldName, 3, reinterpret_cast<float*>(&val), min, max, step, true);
					changed = state & 1; activated = state & 2; deactivated = state & 4;
					if (changed) field.set<MATH::Vector3f>(component.get(), val);
					break;
				}
				case UTILS::WidgetType::DRAG_COLOR_4: {
					MATH::Vector4f val = field.get<MATH::Vector4f>(component.get());
					int state = IMGUI::DrawCombineVecEdit(fieldName, 4, reinterpret_cast<float*>(&val), min, max, step, true);
					changed = state & 1; activated = state & 2; deactivated = state & 4;
					if (changed) field.set<MATH::Vector4f>(component.get(), val);
					break;
				}
				case UTILS::WidgetType::COMBO: {
					auto* enumInfo = UTILS::ReflectionManager::Instance().getEnum(field.typeId);
					if (enumInfo) {
						int val = field.get<int>(component.get());
						std::string currentName = enumInfo->valueToName.count(val) ? enumInfo->valueToName[val] : "Unknown";
						if (ImGui::BeginCombo(fieldName.c_str(), currentName.c_str())) {
							for (const auto& [name, v] : enumInfo->nameToValue) {
								bool isSelected = (v == val);
								if (ImGui::Selectable(name.c_str(), isSelected)) {
									try {
										field.set<int>(component.get(), v);
										EditorRender::GlobalState.mCommandHistory.addCommand(std::make_unique<ReflectionChangePropertyCommand>(selectObject, typeIdName, fieldName, val, v));
									} catch (...) {}
								}
								if (isSelected) ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
					} else {
						ImGui::Text("Enum info missing for %s", fieldName.c_str());
					}
					break;
				}
				case UTILS::WidgetType::MATERIAL: {
					auto materials = field.get<ECS::MaterialRenderer::MaterialList>(component.get());
					DrawMaterialWidget(materials);
					break;
				}
				default:
					ImGui::Text("Unsupported widget for %s", fieldName.c_str());
					break;
				}

				static std::map<ImGuiID, UTILS::Any> activeEdits;
				if (activated && field.getter) {
					activeEdits[fieldId] = field.getter(component.get());
				}
				if (deactivated && field.getter) {
					if (activeEdits.contains(fieldId)) {
						UTILS::Any oldVal = activeEdits[fieldId];
						UTILS::Any newVal = field.getter(component.get());
						EditorRender::GlobalState.mCommandHistory.addCommand(std::make_unique<ReflectionChangePropertyCommand>(selectObject, typeIdName, fieldName, std::move(oldVal), std::move(newVal)));
						activeEdits.erase(fieldId);
					}
				}

				ImGui::PopID();
			}
		} else {
			ImGui::Text("No reflection data for %s", compName.c_str());
		}
	}
}

void ComponentInspectorWindow::draw() {
	const auto selectObject = EditorRender::GlobalState.mSelectObject;

	ImGui::Begin((ICON_FA_DATABASE + std::string(" Component Inspector V2")).c_str(), &mIsOpen);

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

	drawObjectInfo(selectObject);
	drawAddComponent(selectObject);

	auto components = selectObject->getComponents();
	for (auto& [_, component] : components) {
		auto compName = component->getName();
		auto typeIdName = component->getTypeidName();
		auto iconIt = iconComp.find(compName);
		auto iconStr = (iconIt != iconComp.end()) ? iconIt->second : ICON_FA_GLOBE;

		drawComponent(selectObject, component, compName, typeIdName, iconStr);
	}
	ImGui::End();
}
#endif
