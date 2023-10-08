#include "system.h"

using namespace IKIGAI::ECS;


void System::delayed(std::function<void()> task) {
	m_delayedTask.push(task);
}

void System::onUpdateDelayed(std::chrono::duration<double> dt) {
	while (auto task = m_delayedTask.pop()) {
		(*task)();
	}
}

std::set<Entity>& System::getEntities() {
	return m_entities;
}

const std::set<Entity>& System::getEntities() const {
	return m_entities;
}
