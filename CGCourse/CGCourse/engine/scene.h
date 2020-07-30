#pragma once

#include <chrono>
#include <memory>

#include "object.h"
namespace SE {
	class Scene {
	public:
		Scene();
		virtual ~Scene();


		virtual void init() {};
		virtual void update(std::chrono::milliseconds delta) {};
		virtual void render() {};

		void addToScene(std::unique_ptr<Object> obj);
	private:
		std::unique_ptr<Object> root;
	};
};
