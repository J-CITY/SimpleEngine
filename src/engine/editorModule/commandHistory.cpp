#include "commandHistory.h"
#include <iostream>
#include "sceneModule/sceneManager.h"

namespace IKIGAI::EDITOR {

	void CommandHistory::addCommand(std::unique_ptr<ICommand> cmd) {
        cmd->execute();
		if (mCurrentIndex < static_cast<int>(mCommands.size()) - 1) {
			mCommands.erase(mCommands.begin() + mCurrentIndex + 1, mCommands.end());
		}
		mCommands.push_back(std::move(cmd));
		mCurrentIndex++;
	}

	void CommandHistory::undo() {
		if (mCurrentIndex >= 0) {
			mCommands[mCurrentIndex]->undo();
			mCurrentIndex--;
		}
	}

	void CommandHistory::redo() {
		if (mCurrentIndex < static_cast<int>(mCommands.size()) - 1) {
            mCommands[mCurrentIndex + 1]->execute();
            mCurrentIndex++;
		}
	}

	ReflectionChangePropertyCommand::ReflectionChangePropertyCommand(std::shared_ptr<ECS::Object> obj, const std::string& compType, const std::string& fieldName, UTILS::Any oldVal, UTILS::Any newVal)
		: mObject(obj), mComponentTypeName(compType), mFieldName(fieldName), mOldValue(std::move(oldVal)), mNewValue(std::move(newVal)) {
        auto& scene = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
        mSceneLifetime = scene.lifetime;
        mObjectId = obj->getIDInt();
	}

	void ReflectionChangePropertyCommand::execute() {
		applyValue(mNewValue);
	}

	void ReflectionChangePropertyCommand::undo() {
		applyValue(mOldValue);
	}

	std::string ReflectionChangePropertyCommand::getName() const {
		return "Change " + mComponentTypeName + "::" + mFieldName;
	}

	void ReflectionChangePropertyCommand::applyValue(const UTILS::Any& val) {
        if (mSceneLifetime.expired()) return;
        auto& scene = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
        mObject = scene.findObjectByID(ECS2::Entity(ECS2::Entity::ID(mObjectId)));
        if (!mObject) return;
		auto components = mObject->getComponents();
		for (auto& [typeIdx, comp] : components) {
			if (comp && comp->getTypeidName() == mComponentTypeName) {
				auto* typeInfo = UTILS::ReflectionManager::Instance().getType(mComponentTypeName);
				if (typeInfo && typeInfo->mFields.contains(mFieldName)) {
					auto& field = typeInfo->mFields.at(mFieldName);
					if (field.setter) {
						try {
							field.setter(comp.get(), val);
						}
						catch (...) {
							std::cerr << "Failed to apply value in ReflectionChangePropertyCommand" << std::endl;
						}
					}
				}
				break;
			}
		}
	}

}
