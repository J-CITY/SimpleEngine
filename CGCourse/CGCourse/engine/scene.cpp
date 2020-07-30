#include "scene.h"

using namespace SE;

Scene::Scene() {
	root = std::make_unique<Object>();
}

Scene::~Scene() {
}

void Scene::addToScene(std::unique_ptr<Object> obj) {
	root->addChild(std::move(obj));
}
