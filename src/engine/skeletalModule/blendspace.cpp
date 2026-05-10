#include "blendspace.h"
#include <algorithm>
#include "utilsModule/assertion.h"

namespace IKIGAI::SKELETON {
	
	// --- Blendspace1DInstance ---

	Blendspace1DInstance::Blendspace1DInstance(std::shared_ptr<Blendspace1D> resource) : mResource(resource) {
		mBlend = std::make_unique<AnimBlend>(mResource->mSkeleton);

		for (const auto& node : mResource->mNodes) {
			NodeState ns;
			ns.value = node.value;
			ns.sampler = std::make_unique<AnimSample>(mResource->mSkeleton, node.anim);
			mNodes.push_back(std::move(ns));
		}

		if (mNodes.size() > 0) {
			mMin = mNodes[0].value;
			mMax = mNodes[std::max(0.0f, static_cast<float>(mNodes.size()) - 1.0f)].value;
			mValue = mMin;
		}
	}

	Blendspace1DInstance::~Blendspace1DInstance() = default;

	void Blendspace1DInstance::setValue(float value) {
		value = std::max(mMin, value);
		value = std::min(mMax, value);
		mValue = value;
	}

	float Blendspace1DInstance::max() const { return mMax; }
	float Blendspace1DInstance::min() const { return mMin; }
	float Blendspace1DInstance::value() const { return mValue; }

	Pose* Blendspace1DInstance::getPose() { return mLastPose; }

	void Blendspace1DInstance::update(float dt) {
		for (uint32_t i = 0; i < mNodes.size(); i++) {
			if (mValue == mNodes[i].value) {
				mNodes[i].sampler->update(dt);
				mLastPose = mNodes[i].sampler->getPose();
				return;
			} else if (mValue < mNodes[i].value) {
				NodeState& low = mNodes[i - 1];
				NodeState& high = mNodes[i];

				float blendFactor = (mValue - low.value) / (high.value - low.value);

				low.sampler->update(dt);
				high.sampler->update(dt);
				Pose* lowPose = low.sampler->getPose();
				Pose* highPose = high.sampler->getPose();

				mLastPose = mBlend->blend(lowPose, highPose, blendFactor);
				return;
			}
		}
		mLastPose = nullptr;
	}

	std::unique_ptr<IAnimationPlayable> Blendspace1DInstance::clone() const {
		return std::make_unique<Blendspace1DInstance>(mResource);
	}

	// --- Blendspace2DInstance ---

	Blendspace2DInstance::Blendspace2DInstance(std::shared_ptr<Blendspace2D> resource) : mResource(resource) {
		mBlend1 = std::make_unique<AnimBlend>(mResource->mSkeleton);
		mBlend2 = std::make_unique<AnimBlend>(mResource->mSkeleton);
		mBlend3 = std::make_unique<AnimBlend>(mResource->mSkeleton);

		for (const auto& row : mResource->mRows) {
			RowState rs;
			rs.value = row.value;
			for (const auto& node : row.nodes) {
				NodeState ns;
				ns.value = node.value;
				ns.sampler = std::make_unique<AnimSample>(mResource->mSkeleton, node.anim);
				rs.nodes.push_back(std::move(ns));
			}
			mRows.push_back(std::move(rs));
		}

		if (mRows.size() > 0) {
			mYMax = mRows[0].value;
			mYMin = mRows[0].value;
			mXMax = mRows[0].nodes[0].value;
			mXMin = mRows[0].nodes[0].value;

			for (const auto& row : mRows) {
				mYMax = std::max(mYMax, row.value);
				mYMin = std::min(mYMin, row.value);
				ASSERT_IF(row.nodes.size() > 0, "");
				for (const auto& node : row.nodes) {
					mXMax = std::max(mXMax, node.value);
					mXMin = std::min(mXMin, node.value);
				}
			}
			mXValue = mXMin;
			mYValue = mYMin;
		}
	}

	Blendspace2DInstance::~Blendspace2DInstance() = default;

	void Blendspace2DInstance::setXValue(float value) {
		value = std::max(mXMin, value);
		value = std::min(mXMax, value);
		mXValue = value;
	}

	float Blendspace2DInstance::maxX() const { return mXMax; }
	float Blendspace2DInstance::minX() const { return mXMin; }
	float Blendspace2DInstance::valueX() const { return mXValue; }

	void Blendspace2DInstance::setYValue(float value) {
		value = std::max(mYMin, value);
		value = std::min(mYMax, value);
		mYValue = value;
	}

	float Blendspace2DInstance::maxY() const { return mYMax; }
	float Blendspace2DInstance::minY() const { return mYMin; }
	float Blendspace2DInstance::valueY() const { return mYValue; }

	Pose* Blendspace2DInstance::getPose() { return mLastPose; }

	void Blendspace2DInstance::update(float dt) {
		for (uint32_t i = 0; i < mRows.size(); i++) {
			if (mYValue == mRows[i].value) {
				mLastPose = blendedPoseFromRow(mRows[i], mBlend1.get(), dt);
				return;
			} else if (mYValue < mRows[i].value) {
				const RowState& low = mRows[i - 1];
				const RowState& high = mRows[i];

				float blendFactor = (mYValue - low.value) / (high.value - low.value);

				Pose* lowPose = blendedPoseFromRow(low, mBlend1.get(), dt);
				Pose* highPose = blendedPoseFromRow(high, mBlend2.get(), dt);

				mLastPose = mBlend3->blend(lowPose, highPose, blendFactor);
				return;
			}
		}
		mLastPose = nullptr;
	}

	Pose* Blendspace2DInstance::blendedPoseFromRow(const RowState& row, AnimBlend* blend, float dt) {
		if (row.nodes.size() == 1) {
			// Const cast is fine here as we're just updating the sampler which is mutable effectively, 
			// but we didn't mark RowState as const.
			// Actually we pass const RowState& row, so we cannot call update on unique_ptr.
			// Wait, the method signature is `const RowState& row` but `sampler->update` is non-const.
			auto& mutableRow = const_cast<RowState&>(row);
			mutableRow.nodes[0].sampler->update(dt);
			return mutableRow.nodes[0].sampler->getPose();
		}

		for (uint32_t j = 0; j < row.nodes.size(); j++) {
			if (mXValue == row.nodes[j].value) {
				auto& mutableRow = const_cast<RowState&>(row);
				mutableRow.nodes[j].sampler->update(dt);
				return mutableRow.nodes[j].sampler->getPose();
			} else if (mXValue < row.nodes[j].value) {
				auto& mutableRow = const_cast<RowState&>(row);
				NodeState& low = mutableRow.nodes[j - 1];
				NodeState& high = mutableRow.nodes[j];

				return blendedPoseFromNodes(low, high, blend, dt);
			}
		}
		return nullptr;
	}

	Pose* Blendspace2DInstance::blendedPoseFromNodes(NodeState& low, NodeState& high, AnimBlend* blend, float dt) {
		float blendFactor = (mXValue - low.value) / (high.value - low.value);

		low.sampler->update(dt);
		high.sampler->update(dt);
		Pose* lowPose = low.sampler->getPose();
		Pose* highPose = high.sampler->getPose();

		return blend->blend(lowPose, highPose, blendFactor);
	}

	std::unique_ptr<IAnimationPlayable> Blendspace2DInstance::clone() const {
		return std::make_unique<Blendspace2DInstance>(mResource);
	}
}
