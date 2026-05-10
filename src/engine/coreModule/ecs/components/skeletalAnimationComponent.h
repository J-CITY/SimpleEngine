#pragma once

#include "component.h"
#include "skeletalModule/iAnimationPlayable.h"
#include "utilsModule/environment.h"
#include <memory>

namespace IKIGAI::ECS {
	class SkeletalAnimationComponent : public ComponentBase {
	public:
		SkeletalAnimationComponent(UTILS::Ref<ECS::Object> obj);
		~SkeletalAnimationComponent() override;

		void onUpdate(std::chrono::duration<double> dt) override;

		void setPlayable(std::unique_ptr<SKELETON::IAnimationPlayable> playable);
		SKELETON::IAnimationPlayable* getPlayable() const { return mPlayable.get(); }

		std::shared_ptr<UTILS::Environment> getEnvironment() const { return mEnvironment; }

		template <class Context>
		constexpr static auto serde(Context& context, SkeletalAnimationComponent& value) {
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&SkeletalAnimationComponent::mGraphPath, "GraphPath", default_{std::string()});
		}
		
		void setStateGraph(const std::string& path);
		std::string getStateGraph() const { return mGraphPath; }

		virtual void onDeserialize(nlohmann::json& j) override;
		virtual void onSerialize(nlohmann::json& j) override;

	private:
		std::string mGraphPath;
		std::unique_ptr<SKELETON::IAnimationPlayable> mPlayable;
		std::shared_ptr<UTILS::Environment> mEnvironment;
	};
	
	template<>
	inline std::string GetType<SkeletalAnimationComponent>() { return "SkeletalAnimationComponent"; }
	template<>
	inline std::string GetComponentName<SkeletalAnimationComponent>() { return "SkeletalAnimationComponent"; }
}
