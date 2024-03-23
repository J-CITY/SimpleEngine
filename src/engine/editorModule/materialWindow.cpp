#include "materialWindow.h"
#ifdef USE_EDITOR
#include "editorRender.h"
#include "IconsFontAwesome5.h"
#include "imgui.h"
#include "renderModule/backends/gl/materialGl.h"
#include "resourceModule/materialManager.h"

#include "sceneModule/sceneManager.h"
#include "misc/cpp/imgui_stdlib.h"
#include "resourceModule/textureManager.h"
#include "utilsModule/imguiHelper/imguiWidgets.h"


bool isEngineUniform(const std::string& uniformName) {
	return uniformName.rfind("EngineUBO", 0) == 0 || uniformName.rfind("engine_", 0) == 0
		|| uniformName.rfind("Engine", 0) == 0;
}
#ifdef OPENGL_BACKEND
void IKIGAI::EDITOR::MaterialWindow::draw() {

	auto material = std::static_pointer_cast<RENDER::MaterialGl>(EditorRender::GlobalState.mSelectMaterial);
	ImGui::Begin("Material Editor");

	if (ImGui::Button("Save")) {
		nlohmann::json data;
		//TODO:
		//material->onSerialize(data);
		//std::ofstream f(UTILS::getRealPath(material->mPath));
		//f << data.dump(4) << std::endl;
		//f.close();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload")) {
		auto path = material->mPath;
		//editMaterial = RESOURCES::MaterialLoader::Create(path);
		EditorRender::GlobalState.mSelectMaterial = RESOURCES::ServiceManager::Get<RESOURCES::MaterialLoader>().loadResource(path);
	}
	ImGui::SameLine();
	if (ImGui::Button("Create new")) {
		//TODO:
		//popupStates["create_new_material"] = true;
	}

	ImGui::Text("Name:", material->mPath);

	static bool isMatSettingsOpen = true;
	if (ImGui::CollapsingHeader("Material settings", &isMatSettingsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Checkbox("Is Blendable", &material->mBlendable)) {
			//material->generateStateMask();
		}
		if (ImGui::Checkbox("Back face culling", &material->mBackfaceCulling)) {
			//material->generateStateMask();
		}
		if (ImGui::Checkbox("Front face culling", &material->mFrontfaceCulling)) {
			//material->generateStateMask();
		}
		if (ImGui::Checkbox("Depth test", &material->mDepthTest)) {
			//material->generateStateMask();
		}
		if (ImGui::Checkbox("Color writing", &material->mColorWriting)) {
			//material->generateStateMask();
		}
		if (ImGui::DragInt("GPU Instance", &material->mGpuInstances)) {
			//material->generateStateMask();
		}
	}

	static std::string shaderVPath = material->mShader->vertexPath.value();
	static std::string shaderFPath = material->mShader->fragmentPath.value();
	if (ImGui::InputText("Shader Fragment:", &shaderVPath, ImGuiInputTextFlags_EnterReturnsTrue)) {

	}
	if (ImGui::InputText("Shader Vertex:", &shaderFPath, ImGuiInputTextFlags_EnterReturnsTrue)) {

	}

	auto shader = material->getShader();
	if (!shader) {
		ImGui::End();
		return;
	}
	const auto& refl = shader->getUniformsInfo();

	for (auto& [name, data] : refl) {
		if (isEngineUniform(name)) continue;
		if (data.type == RENDER::UniformInform::TYPE::UNIFORM) {
			switch (data.members[0].type) {
			case RENDER::UNIFORM_TYPE::MAT4: break;
			case RENDER::UNIFORM_TYPE::MAT3: break;
			case RENDER::UNIFORM_TYPE::VEC4: {
				if (!IMGUI::CombineVecEdit::Data.count(name)) {
					IMGUI::CombineVecEdit::Data.insert({name, IMGUI::CombineVecEdit(name, 4)});
				}
				auto val = std::get<MATH::Vector4f>(material->get(name));
				if (IMGUI::CombineVecEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::VEC3: {
				if (!IMGUI::CombineVecEdit::Data.count(name)) {
					IMGUI::CombineVecEdit::Data.insert({name, IMGUI::CombineVecEdit(name, 3)});
				}
				auto val = std::get<MATH::Vector3f>(material->get(name));
				if (IMGUI::CombineVecEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::VEC2: {
				if (!IMGUI::CombineVecEdit::Data.count(name)) {
					IMGUI::CombineVecEdit::Data.insert({name, IMGUI::CombineVecEdit(name, 2)});
				}
				auto val = std::get<MATH::Vector2f>(material->get(name));
				if (IMGUI::CombineVecEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::INT: {
				if (!IMGUI::ScalarEdit::Data.count(name)) {
					IMGUI::ScalarEdit::Data.insert({name, IMGUI::ScalarEdit(name, IMGUI::ScalarEdit::TYPE::INT)});
				}
				auto val = std::get<int>(material->mUniformData.at(name));
				if (IMGUI::ScalarEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::FLOAT: {
				if (!IMGUI::ScalarEdit::Data.count(name)) {
					IMGUI::ScalarEdit::Data.insert({name, IMGUI::ScalarEdit(name, IMGUI::ScalarEdit::TYPE::FLOAT)});
				}
				auto val = std::get<float>(material->mUniformData.at(name));
				if (IMGUI::ScalarEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::BOOL: {
				if (!IMGUI::ScalarEdit::Data.count(name)) {
					IMGUI::ScalarEdit::Data.insert({name, IMGUI::ScalarEdit(name, IMGUI::ScalarEdit::TYPE::BOOL)});
				}
				auto val = std::get<bool>(material->mUniformData.at(name));
				if (IMGUI::ScalarEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::SAMPLER_2D: {
				auto val = std::get<std::shared_ptr<RENDER::TextureGl>>(material->mUniformData.at(name));
				ImGui::PushID(("##" + name).c_str());

				//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
				if (ImGui::Selectable(("##" + name).c_str(), false, 0, ImVec2(100, 100))) {

				}
				auto pos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(ImVec2(pos.x, pos.y - 100));

				if (ImGui::BeginDragDropTarget()) {
					ImGuiDragDropFlags target_flags = 0;
					//target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
					//target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_IMAGE_DATA", target_flags)) {
						auto path = EditorRender::GlobalState.mDndStringPayload;
						material->set(name, std::static_pointer_cast<RENDER::TextureGl>(RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createFromFile(path, true)));
						EditorRender::GlobalState.mDndStringPayload.clear();
					}
					ImGui::EndDragDropTarget();
				}
				if (val) {
					ImGui::Image(reinterpret_cast<ImTextureID>((uintptr_t)val->id), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
				} else {
					ImGui::Image(EditorRender::GlobalState.mTextureCache["default_texture"]->getImguiId(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_FILE)) {
					EditorRender::GlobalState.mPopupStates["file_chooser"] = true;
					EditorRender::GlobalState.mFileFormatsCb = [] {
						return ".png,.jpeg,.jpg";
					};
					EditorRender::GlobalState.mFileChooserCb = [material, name](std::string path) mutable {
						material->set(name, std::static_pointer_cast<RENDER::TextureGl>(RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createFromFile(path, true)));
					};
				}
				ImGui::PopID();
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
		} else if (data.type == RENDER::UniformInform::TYPE::UNIFORM_BUFFER) {
			for (const auto& mamber : data.members) {
				auto _name = mamber.name;
				switch (mamber.type) {
				case RENDER::UNIFORM_TYPE::MAT4: break;
				case RENDER::UNIFORM_TYPE::MAT3: break;
				case RENDER::UNIFORM_TYPE::VEC4: {
					if (!IMGUI::CombineVecEdit::Data.count(_name)) {
						IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name, 4)});
					}
					auto val = std::get<MATH::Vector4f>(material->get(name, mamber.name));
					if (IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::VEC3: {
					if (!IMGUI::CombineVecEdit::Data.count(_name)) {
						IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name, 3)});
					}
					auto val = std::get<MATH::Vector3f>(material->get(name, mamber.name));
					if (IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::VEC2: {
					if (!IMGUI::CombineVecEdit::Data.count(_name)) {
						IMGUI::CombineVecEdit::Data.insert({_name, IMGUI::CombineVecEdit(_name, 2)});
					}
					auto val = std::get<MATH::Vector2f>(material->get(name, mamber.name));
					if (IMGUI::CombineVecEdit::Data.at(_name).draw(val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::INT: {
					if (!IMGUI::ScalarEdit::Data.count(name)) {
						IMGUI::ScalarEdit::Data.insert({name, IMGUI::ScalarEdit(name, IMGUI::ScalarEdit::TYPE::INT)});
					}
					auto val = std::get<int>(material->mUniformData.at(name));
					if (IMGUI::ScalarEdit::Data.at(name).draw(val)) {
						material->set(name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::FLOAT: {
					if (!IMGUI::ScalarEdit::Data.count(name)) {
						IMGUI::ScalarEdit::Data.insert({name, IMGUI::ScalarEdit(name, IMGUI::ScalarEdit::TYPE::FLOAT)});
					}
					auto val = std::get<float>(material->mUniformData.at(name));
					if (IMGUI::ScalarEdit::Data.at(name).draw(val)) {
						material->set(name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::BOOL: {
					if (!IMGUI::ScalarEdit::Data.count(name)) {
						IMGUI::ScalarEdit::Data.insert({name, IMGUI::ScalarEdit(name, IMGUI::ScalarEdit::TYPE::BOOL)});
					}
					auto val = std::get<bool>(material->mUniformData.at(name));
					if (IMGUI::ScalarEdit::Data.at(name).draw(val)) {
						material->set(name, val);
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
#endif
