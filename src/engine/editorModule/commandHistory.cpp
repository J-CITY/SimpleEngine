#include "commandHistory.h"
#include <iostream>

namespace IKIGAI::EDITOR {

	void CommandHistory::addCommand(std::unique_ptr<ICommand> cmd) {
		if (mCurrentIndex < static_cast<int>(mCommands.size()) - 1) {
			mCommands.erase(mCommands.begin() + mCurrentIndex + 1, mCommands.end());
		}
		cmd->execute();
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
			mCurrentIndex++;
			mCommands[mCurrentIndex]->execute();
		}
	}

	ReflectionChangePropertyCommand::ReflectionChangePropertyCommand(std::shared_ptr<ECS::Object> obj, const std::string& compType, const std::string& fieldName, UTILS::Any oldVal, UTILS::Any newVal)
		: mObject(obj), mComponentTypeName(compType), mFieldName(fieldName), mOldValue(std::move(oldVal)), mNewValue(std::move(newVal)) {
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
