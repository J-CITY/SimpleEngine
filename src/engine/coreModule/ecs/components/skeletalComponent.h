#pragma once

#include "component.h"
#include "skeletalModule/skeleton.h"

namespace IKIGAI::ECS {
	class SkeletalComponent : public ComponentBase {
	public:
		SkeletalComponent(UTILS::Ref<ECS::Object> obj);
		~SkeletalComponent() override;

		void setSkeleton(const std::string& path);
		std::shared_ptr<SKELETON::Skeleton> getSkeleton() const { return mSkeleton; }

		template <class Context>
		constexpr static auto serde(Context& context, SkeletalComponent& value) {
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&SkeletalComponent::mSkeletonPath, "SkeletonPath", default_{std::string()});
		}
		virtual void onDeserialize(nlohmann::json& j) override;
		virtual void onSerialize(nlohmann::json& j) override;

	private:
		std::string mSkeletonPath;
		std::shared_ptr<SKELETON::Skeleton> mSkeleton;
	};
	
	template<>
	inline std::string GetType<SkeletalComponent>() { return "SkeletalComponent"; }
	template<>
	inline std::string GetComponentName<SkeletalComponent>() { return "SkeletalComponent"; }
}
