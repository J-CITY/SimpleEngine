#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
namespace IKIGAI::UTILS {
using VarValue = std::variant<float, int, bool>;
class Environment {
	std::unordered_map<std::string, size_t> names;
	std::vector<VarValue> values;
	std::unordered_map<size_t, std::function<VarValue()>> subscriptions;
	std::unordered_set<size_t> triggers;

public:
	virtual ~Environment() = default;
	size_t slot(const std::string& name) const {
		auto it = names.find(name);
		if (it == names.end()) {
			throw std::runtime_error("Undeclared variable: " + name);
		}
		return it->second;
	}
	void declare(const std::string& name, VarValue value, bool trigger = false) {
		auto [it, added] = names.try_emplace(name, values.size());
		if (added) {
			values.push_back(value);
		}
		else if (values[it->second].index() != value.index()) {
			throw std::runtime_error("Variable type mismatch: " + name);
		}
		if (trigger) {
			triggers.insert(it->second);
		}
	}
	void set(size_t index, VarValue value) {
		if (values.at(index).index() != value.index()) {
			throw std::runtime_error("Variable type mismatch");
		}
		values[index] = value;
	}
	VarValue get(size_t index) const {
		auto it = subscriptions.find(index);
		return it == subscriptions.end() ? values.at(index) : it->second();
	}
	void setValue(const std::string& name, VarValue value) {
		set(slot(name), value);
	}
	VarValue getValue(const std::string& name) const {
		return get(slot(name));
	}
	bool hasVariable(const std::string& name) const {
		return names.contains(name);
	}
	void fire(const std::string& name) {
		auto index = slot(name);
		if (!triggers.contains(index)) {
			throw std::runtime_error("Not a trigger: " + name);
		}
		set(index, true);
	}
	void clearTriggers() {
		for (auto index : triggers) {
			values[index] = false;
		}
	}
	void addSubscription(const std::string& name, std::function<VarValue()> getter) {
		if (!hasVariable(name)) {
			declare(name, getter());
		}
		subscriptions[slot(name)] = std::move(getter);
	}
	void clearSubscriptions() {
		subscriptions.clear();
	}
	Environment snapshot() const {
		auto result = *this;
		for (const auto& [index, getter] : subscriptions) {
			result.values[index] = getter();
		}
		result.clearSubscriptions();
		return result;
	}
};
}
