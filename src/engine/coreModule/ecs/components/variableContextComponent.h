#pragma once
#include "component.h"
#include "coreModule/ecs/componentReference.h"
#include "scriptModule/contextPredicate.h"
#include "utilsModule/expressions/expression.h"
#include "variableDescriptor.h"
#include <optional>

namespace IKIGAI::ECS {
class VariableContextComponent : public ComponentBase {
public:
	struct Descriptor : ComponentBase::Descriptor {
		std::string Type;
		std::map<std::string, VariableDescriptor> Variables;
		template <class C> static auto serde(C& c, Descriptor& v) {
			serde::serde_struct(c, v).field(&Descriptor::Type, "VariableContextComponentType").field(&Descriptor::Variables, "Variables");
		}
	};
	explicit VariableContextComponent(UTILS::Ref<Object>);
	VariableContextComponent(UTILS::Ref<Object>, const ComponentBase::Descriptor&);
	Descriptor getDescriptor() const {
		return settings;
	}
	static auto GetMembers() {
		return std::tuple {};
	}
	size_t slot(const std::string&) const;
	DATA::ValueType type(size_t i) const {
		return DATA::typeOf(snapshot.at(i));
	}
	const DATA::Value& read(size_t i) const {
		return snapshot.at(i);
	}
	void write(size_t, const DATA::Value&);
	void fire(size_t);
	uint64_t tick() const {
		return sequence;
	}
	void update(SCENE_SYSTEM::Scene&);
	Descriptor settings;

private:
	struct Entry {
		enum class Source { Owned, Property, Expression, Script } source = Source::Owned;
		bool event = false;
		std::optional<PropertyBinding> property;
		std::optional<DATA::Expression> expression;
		std::shared_ptr<SCRIPTING::ContextPredicate> script;
		std::vector<size_t> arguments;
	};
	bool prepared = false;
	uint64_t sequence = 0;
	DATA::Symbols symbols;
	std::vector<Entry> entries;
	std::vector<DATA::Value> owned, snapshot;
	std::vector<size_t> order;
	void prepare(SCENE_SYSTEM::Scene&);
};
template <> inline std::string GetType<VariableContextComponent>() {
	return "class IKIGAI::ECS::VariableContextComponent";
}
template <> inline std::string GetComponentName<VariableContextComponent>() {
	return "VariableContextComponent";
}

}
