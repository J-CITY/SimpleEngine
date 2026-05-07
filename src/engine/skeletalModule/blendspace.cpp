#include "blendspace.h"
#include <algorithm>

#include "utilsModule/assertion.h"

namespace IKIGAI::SKELETON {
	Blendspace1D::Blendspace1D(Skeleton* skeleton, std::vector<Node*> nodes) : mNodes(nodes) {
		mBlend = std::make_unique<AnimBlend>(skeleton);

		if (mNodes.size() > 0) {
			mMin = mNodes[0]->value;
			mMax = mNodes[std::max(0.0f, float(mNodes.size() - 1.0f))]->value;
			mValue = mMin;
		}
	}

	Blendspace1D::~Blendspace1D() {
		for (auto& node : mNodes) {
			if (node) {
				delete node;
			}
		}
	}

	void Blendspace1D::setValue(float value) {
		value = std::max(mMin, value);
		value = std::min(mMax, value);
		mValue = value;
	}

	float Blendspace1D::max() {
		return mMax;
	}

	float Blendspace1D::min() {
		return mMin;
	}

	float Blendspace1D::value() {
		return mValue;
	}

	Pose* Blendspace1D::evaluate(float dt) {
		for (uint32_t i = 0; i < mNodes.size(); i++) {
			if (mValue == mNodes[i]->value) {
				return mNodes[i]->sampler->sample(dt);
			} else if (mValue < mNodes[i]->value) {
				Node* low = mNodes[i - 1];
				Node* high = mNodes[i];

				float blendFactor = (mValue - low->value) / (high->value - low->value);

				Pose* lowPose = low->sampler->sample(dt);
				Pose* highPose = high->sampler->sample(dt);

				return mBlend->blend(lowPose, highPose, blendFactor);
			}
		}

		return nullptr;
	}

	Blendspace2D::Blendspace2D(Skeleton* skeleton, const std::vector<Row>& rows) : mRows(rows) {
		mBlend1 = std::make_unique<AnimBlend>(skeleton);
		mBlend2 = std::make_unique<AnimBlend>(skeleton);
		mBlend3 = std::make_unique<AnimBlend>(skeleton);

		mYMax = mRows[0].value;
		mYMin = mRows[0].value;

		mXMax = mRows[0].nodes[0]->value;
		mXMin = mRows[0].nodes[0]->value;

		for (const auto& row : mRows) {
			mYMax = std::max(mYMax, row.value);
			mYMin = std::min(mYMin, row.value);

			ASSERT_IF(row.nodes.size() > 0, "");

			for (const auto& node : row.nodes) {
				ASSERT_IF(node != nullptr, "");

				mXMax = std::max(mXMax, node->value);
				mXMin = std::min(mXMin, node->value);
			}
		}

		mXValue = mXMin;
		mYValue = mYMin;
	}

	Blendspace2D::~Blendspace2D() {
		for (const auto& row : mRows) {
			for (const auto& node : row.nodes) {
				if (node) {
					delete node;
				}
			}
		}
	}

	void Blendspace2D::setXValue(float value) {
		value = std::max(mXMin, value);
		value = std::min(mXMax, value);
		mXValue = value;
	}

	float Blendspace2D::maxX() {
		return mXMax;
	}

	float Blendspace2D::minX() {
		return mXMin;
	}

	float Blendspace2D::valueX() {
		return mXValue;
	}

	void Blendspace2D::setYValue(float value) {
		value = std::max(mYMin, value);
		value = std::min(mYMax, value);
		mYValue = value;
	}

	float Blendspace2D::maxY() {
		return mYMax;
	}

	float Blendspace2D::minY() {
		return mYMin;
	}

	float Blendspace2D::valueY() {
		return mYValue;
	}

	Pose* Blendspace2D::evaluate(float dt) {
		for (uint32_t i = 0; i < mRows.size(); i++) {
			if (mYValue == mRows[i].value) {
				return blendedPoseFromRow(mRows[i], mBlend1.get(), dt);
			} else if (mYValue < mRows[i].value) {
				const Row& low = mRows[i - 1];
				const Row& high = mRows[i];

				float blendFactor = (mYValue - low.value) / (high.value - low.value);

				Pose* lowPose = blendedPoseFromRow(low, mBlend1.get(), dt);
				Pose* highPose = blendedPoseFromRow(high, mBlend2.get(), dt);

				return mBlend3->blend(lowPose, highPose, blendFactor);
			}
		}

		return nullptr;
	}

	Pose* Blendspace2D::blendedPoseFromRow(const Row& row, AnimBlend* blend, float dt) {
		if (row.nodes.size() == 1)
			return row.nodes[0]->sampler->sample(dt);

		for (uint32_t j = 0; j < row.nodes.size(); j++) {
			if (mXValue == row.nodes[j]->value) {
				return row.nodes[j]->sampler->sample(dt);
			} else if (mXValue < row.nodes[j]->value) {
				Node* low = row.nodes[j - 1];
				Node* high = row.nodes[j];

				return blendedPoseFromNodes(low, high, blend, dt);
			}
		}

		return nullptr;
	}

	Pose* Blendspace2D::blendedPoseFromNodes(Node* low, Node* high, AnimBlend* blend, float dt) {
		float blendFactor = (mXValue - low->value) / (high->value - low->value);

		Pose* lowPose = low->sampler->sample(dt);
		Pose* highPose = high->sampler->sample(dt);

		return blend->blend(lowPose, highPose, blendFactor);
	}
}
