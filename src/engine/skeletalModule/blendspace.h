#pragma once

#include <memory>

#include "animation.h"
#include "animationBlend.h"
#include "animationInstance.h"

namespace IKIGAI::SKELETON {
	class Blendspace1D {
	public:
		struct Node {
			float value;
			Animation* anim;
			std::unique_ptr<AnimSample> sampler;

			Node(Skeleton* Skeleton, Animation* Anim, float Value) {
				anim = Anim;
				sampler = std::make_unique<AnimSample>(Skeleton, Anim);
				value = Value;
			}
		};

	public:
		Blendspace1D(Skeleton* Skeleton, std::vector<Node*> Nodes);
		~Blendspace1D();
		void setValue(float value);
		float max();
		float min();
		float value();
		Pose* evaluate(float dt);

	private:
		float mValue = 0.0f;
		float mMin = 0.0f;
		float mMax = 0.0f;
		std::vector<Node*> mNodes;
		std::unique_ptr<AnimBlend> mBlend;
	};

	class Blendspace2D {
	public:
		struct Node {
			float value;
			Animation* anim;
			std::unique_ptr<AnimSample> sampler;

			Node(Skeleton* skeleton, Animation* anim, float value) {
				anim = anim;
				sampler = std::make_unique<AnimSample>(skeleton, anim);
				value = value;
			}
		};

		struct Row {
			float value;
			std::vector<Node*> nodes;
		};

	public:
		Blendspace2D(Skeleton* skeleton, const std::vector<Row>& rows);
		~Blendspace2D();
		void setXValue(float value);
		float maxX();
		float minX();
		float valueX();
		void setYValue(float value);
		float maxY();
		float minY();
		float valueY();
		Pose* evaluate(float dt);

	private:
		Pose* blendedPoseFromRow(const Row& row, AnimBlend* blend, float dt);
		Pose* blendedPoseFromNodes(Node* low, Node* high, AnimBlend* blend, float dt);

		float mXMax;
		float mXMin;
		float mYMax;
		float mYMin;
		float mXValue;
		float mYValue;
		std::vector<Row> mRows;
		std::unique_ptr<AnimBlend> mBlend1;
		std::unique_ptr<AnimBlend> mBlend2;
		std::unique_ptr<AnimBlend> mBlend3;
	};
}
