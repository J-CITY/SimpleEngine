#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <functional>

namespace IKIGAI::UTILS {
	using VarValue = std::variant<float, int, bool>;

	class Environment {
	public:
		Environment() = default;
		virtual ~Environment() = default;

		void addSubscription(const std::string& name, std::function<VarValue()> getter) {
			mVariables[name] = std::move(getter);
		}

		VarValue getValue(const std::string& name) const {
			if (auto it = mVariables.find(name); it != mVariables.end()) {
				return it->second();
			}
			return 0.0f; // Default value if not found
		}

		bool hasVariable(const std::string& name) const {
			return mVariables.contains(name);
		}

		void clearSubscriptions() {
			mVariables.clear();
		}

	private:
		std::unordered_map<std::string, std::function<VarValue()>> mVariables;
	};
}
