#include "variableContextComponent.h"
#include "coreModule/ecs/componentReference.inl"
#include <serdepp/adaptor/nlohmann_json.hpp>

namespace IKIGAI::ECS {
VariableContextComponent::VariableContextComponent(UTILS::Ref<Object> o)
	: ComponentBase(o) {
	__NAME__ = "VariableContextComponent";
	settings.Type = GetType<VariableContextComponent>();
}
VariableContextComponent::VariableContextComponent(UTILS::Ref<Object> o, const ComponentBase::Descriptor& d)
	: VariableContextComponent(o) {
	settings = static_cast<const Descriptor&>(d);
}
size_t VariableContextComponent::slot(const std::string& name) const {
	auto i = symbols.find(name);
	if (i == symbols.end()) {
		throw std::runtime_error("Unknown context variable: " + name);
	}
	return i->second.slot;
}
void VariableContextComponent::write(size_t i, const DATA::Value& value) {
	if (entries.at(i).source != Entry::Source::Owned || entries[i].event) {
		throw std::runtime_error("Context variable is read-only or event");
	}
	if (DATA::typeOf(value) != DATA::typeOf(owned.at(i))) {
		throw std::runtime_error("Context variable type mismatch");
	}
	owned[i] = value;
}
void VariableContextComponent::fire(size_t i) {
	if (!entries.at(i).event || entries[i].source != Entry::Source::Owned) {
		throw std::runtime_error("Not an owned event");
	}
	owned[i] = true;
}
void VariableContextComponent::prepare(SCENE_SYSTEM::Scene& scene) {
	// Build locally, then publish a complete schema. Failed references can be retried after loading.
	DATA::Symbols schema;
	std::vector<DATA::Value> values;
	std::vector<Entry> candidate;
	for (const auto& [name, v] : settings.Variables) {
		auto value = v.initialValue();
		schema.emplace(name, DATA::Symbol { values.size(), DATA::typeOf(value) });
		values.push_back(value);
		candidate.emplace_back();
	}
	for (const auto& [name, v] : settings.Variables) {
		auto& entry = candidate[schema.at(name).slot];
		entry.event = v.Type == VariableType::Event;
		if (int(v.Property.has_value()) + int(v.Expression.has_value()) + int(v.Script.has_value()) > 1) {
			throw std::runtime_error("Context variable has multiple sources: " + name);
		}
		if (v.Property.has_value()) {
			const auto& ref = *v.Property;
			entry.property = PropertyRegistry::bind(scene, ref, schema.at(name).type);
			entry.source = Entry::Source::Property;
		}
		else if (v.Script.has_value()) {
			if (schema.at(name).type != DATA::ValueType::Bool) {
				throw std::runtime_error("Script predicate output must be Bool");
			}
			const auto& script = *v.Script;
			entry.script = std::make_shared<SCRIPTING::ContextPredicate>(script.Path, script.Function);
			for (const auto& arg : script.Arguments) {
				const auto symbol = schema.at(arg);
				if (symbol.type == DATA::ValueType::Vector3) {
					throw std::runtime_error("Predicate argument must be scalar");
				}
				entry.arguments.push_back(symbol.slot);
			}
			entry.source = Entry::Source::Script;
		}
		else if (v.Expression.has_value()) {
			entry.expression = DATA::Expression::compile(*v.Expression, schema);
			entry.source = Entry::Source::Expression;
			if (entry.expression->type() != schema.at(name).type) {
				throw std::runtime_error("Context expression type mismatch: " + name);
			}
		}
	}
	std::vector<int> marks(candidate.size());
	std::vector<size_t> sorted;
	std::function<void(size_t)> visit = [&](size_t i) {
		if (marks[i] == 1) {
			throw std::runtime_error("Cyclic variable context");
		}
		if (marks[i] == 2) {
			return;
		}
		marks[i] = 1;
		if (candidate[i].expression) {
			for (auto dep : candidate[i].expression->dependencies()) {
				visit(dep);
			}
		}
		for (auto dep : candidate[i].arguments) {
			visit(dep);
		}
		marks[i] = 2;
		sorted.push_back(i);
	};
	for (size_t i = 0; i < candidate.size(); ++i) {
		visit(i);
	}
	symbols = std::move(schema);
	owned = values;
	snapshot = std::move(values);
	entries = std::move(candidate);
	order = std::move(sorted);
	prepared = true;
}
void VariableContextComponent::update(SCENE_SYSTEM::Scene& scene) {
	if (!prepared) {
		prepare(scene);
	}
	auto next = owned;
	for (auto i : order) {
		const auto& e = entries[i];
		if (e.property) {
			next[i] = e.property->read(scene);
		}
		else if (e.expression) {
			next[i] = e.expression->evaluate([&](size_t slot) { return next.at(slot); });
		}
		else if (e.script) {
			std::vector<DATA::Value> args;
			for (auto slot : e.arguments) {
				args.push_back(next.at(slot));
			}
			next[i] = e.script->evaluate(args);
		}
	}
	snapshot = std::move(next);
	++sequence;
	for (size_t i = 0; i < entries.size(); ++i) {
		if (entries[i].event && entries[i].source == Entry::Source::Owned) {
			owned[i] = false;
		}
	}
}
}
