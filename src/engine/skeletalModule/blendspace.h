#pragma once

#include <memory>
#include <vector>

#include "animation.h"
#include "animationBlend.h"
#include "animationInstance.h"
#include "iAnimationPlayable.h"

namespace IKIGAI::SKELETON {
	
	// --- RESOURCES (Stateless) ---

	class Blendspace1D {
	public:
		struct Node {
			float value;
			Animation* anim;
		};

		Skeleton* mSkeleton;
		std::vector<Node> mNodes;
	};

	class Blendspace2D {
	public:
		struct Node {
			float value;
			Animation* anim;
		};
		struct Row {
			float value;
			std::vector<Node> nodes;
		};

		Skeleton* mSkeleton;
		std::vector<Row> mRows;
	};

	// --- INSTANCES (Stateful) ---

	class Blendspace1DInstance : public IAnimationPlayable {
	public:
		struct NodeState {
			float value;
			std::unique_ptr<AnimSample> sampler;
		};

	public:
		Blendspace1DInstance(std::shared_ptr<Blendspace1D> resource);
		~Blendspace1DInstance() override;
		
		void setValue(float value);
		float max() const;
		float min() const;
		float value() const;
		
		void update(float dt) override;
		Pose* getPose() override;
		std::unique_ptr<IAnimationPlayable> clone() const override;

	private:
		Pose* mLastPose = nullptr;
		float mValue = 0.0f;
		float mMin = 0.0f;
		float mMax = 0.0f;
		std::vector<NodeState> mNodes;
		std::unique_ptr<AnimBlend> mBlend;
		std::shared_ptr<Blendspace1D> mResource;
	};

	class Blendspace2DInstance : public IAnimationPlayable {
	public:
		struct NodeState {
			float value;
			std::unique_ptr<AnimSample> sampler;
		};
		struct RowState {
			float value;
			std::vector<NodeState> nodes;
		};

	public:
		Blendspace2DInstance(std::shared_ptr<Blendspace2D> resource);
		~Blendspace2DInstance() override;
		
		void setXValue(float value);
		float maxX() const;
		float minX() const;
		float valueX() const;
		
		void setYValue(float value);
		float maxY() const;
		float minY() const;
		float valueY() const;
		
		void update(float dt) override;
		Pose* getPose() override;
		std::unique_ptr<IAnimationPlayable> clone() const override;

	private:
		Pose* blendedPoseFromRow(const RowState& row, AnimBlend* blend, float dt);
		Pose* blendedPoseFromNodes(NodeState& low, NodeState& high, AnimBlend* blend, float dt);

		Pose* mLastPose = nullptr;
		float mXMax = 0.0f;
		float mXMin = 0.0f;
		float mYMax = 0.0f;
		float mYMin = 0.0f;
		float mXValue = 0.0f;
		float mYValue = 0.0f;
		std::vector<RowState> mRows;
		std::unique_ptr<AnimBlend> mBlend1;
		std::unique_ptr<AnimBlend> mBlend2;
		std::unique_ptr<AnimBlend> mBlend3;
		std::shared_ptr<Blendspace2D> mResource;
	};
}
