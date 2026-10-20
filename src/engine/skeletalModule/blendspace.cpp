#include "blendspace.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace IKIGAI::SKELETON {

// --- Blendspace1DInstance ---

Blendspace1DInstance::Blendspace1DInstance(std::shared_ptr<Blendspace1D> resource)
	: mResource(resource) {
	mRevision = mResource->revision;
	mBlend = std::make_unique<AnimBlend>(mResource->mSkeleton);

	for (const auto& node : mResource->mNodes) {
		NodeState ns;
		ns.value = node.value;
		ns.sampler = std::make_unique<AnimSample>(mResource->mSkeleton, node.anim);
		mNodes.push_back(std::move(ns));
	}

	if (mNodes.empty()) {
		throw std::runtime_error("Empty 1D blendspace");
	}
	std::sort(mNodes.begin(), mNodes.end(), [](const auto& a, const auto& b) { return a.value < b.value; });
	for (size_t i = 1; i < mNodes.size(); ++i) {
		if (mNodes[i].value <= mNodes[i - 1].value) {
			throw std::runtime_error("Duplicate blendspace point");
		}
	}
	if (mNodes.size() > 0) {
		mMin = mNodes[0].value;
		mMax = mNodes[std::max(0.0f, static_cast<float>(mNodes.size()) - 1.0f)].value;
		mValue = mMin;
	}
	update(0);
}

Blendspace1DInstance::~Blendspace1DInstance() = default;

void Blendspace1DInstance::setValue(float value) {
	value = std::max(mMin, value);
	value = std::min(mMax, value);
	mValue = value;
}

float Blendspace1DInstance::max() const {
	return mMax;
}
float Blendspace1DInstance::min() const {
	return mMin;
}
float Blendspace1DInstance::value() const {
	return mValue;
}

Pose* Blendspace1DInstance::getPose() {
	return mLastPose;
}

void Blendspace1DInstance::update(float dt) {
	if (mRevision != mResource->revision) {
		Blendspace1DInstance candidate(mResource);
		mNodes = std::move(candidate.mNodes);
		mBlend = std::move(candidate.mBlend);
		mMin = candidate.mMin;
		mMax = candidate.mMax;
		setValue(mValue);
		mRevision = mResource->revision;
	}
	mPhase += dt / std::max(.001f, mNodes.front().sampler->duration());
	mPhase -= std::floor(mPhase);
	for (auto& node : mNodes) {
		node.sampler->seek(mPhase * node.sampler->duration());
	}
	for (uint32_t i = 0; i < mNodes.size(); i++) {
		if (mValue == mNodes[i].value) {
			mLastPose = mNodes[i].sampler->getPose();
			return;
		}
		else if (mValue < mNodes[i].value) {
			NodeState& low = mNodes[i - 1];
			NodeState& high = mNodes[i];

			float blendFactor = (mValue - low.value) / (high.value - low.value);
			Pose* lowPose = low.sampler->getPose();
			Pose* highPose = high.sampler->getPose();

			mLastPose = mBlend->blend(lowPose, highPose, blendFactor);
			return;
		}
	}
	mLastPose = nullptr;
}

std::unique_ptr<IAnimationPlayable> Blendspace1DInstance::clone() const {
	auto copy = std::make_unique<Blendspace1DInstance>(mResource);
	copy->mPhase = mPhase;
	copy->setValue(mValue);
	copy->update(0);
	return copy;
}

// --- Blendspace2DInstance ---

Blendspace2DInstance::Blendspace2DInstance(std::shared_ptr<Blendspace2D> resource)
	: mResource(resource) {
	mRevision = mResource->revision;
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
		if (rs.nodes.empty()) {
			throw std::runtime_error("Empty blendspace row");
		}
		std::sort(rs.nodes.begin(), rs.nodes.end(), [](const auto& a, const auto& b) { return a.value < b.value; });
		for (size_t i = 1; i < rs.nodes.size(); ++i) {
			if (rs.nodes[i].value <= rs.nodes[i - 1].value) {
				throw std::runtime_error("Duplicate blendspace point");
			}
		}
		mRows.push_back(std::move(rs));
	}

	if (mRows.empty()) {
		throw std::runtime_error("Empty 2D blendspace");
	}
	std::sort(mRows.begin(), mRows.end(), [](const auto& a, const auto& b) { return a.value < b.value; });
	for (size_t i = 1; i < mRows.size(); ++i) {
		if (mRows[i].value <= mRows[i - 1].value) {
			throw std::runtime_error("Duplicate blendspace row");
		}
	}
	if (mRows.size() > 0) {
		mYMax = mRows[0].value;
		mYMin = mRows[0].value;
		mXMax = mRows[0].nodes[0].value;
		mXMin = mRows[0].nodes[0].value;

		for (const auto& row : mRows) {
			mYMax = std::max(mYMax, row.value);
			mYMin = std::min(mYMin, row.value);
			for (const auto& node : row.nodes) {
				mXMax = std::max(mXMax, node.value);
				mXMin = std::min(mXMin, node.value);
			}
		}
		mXValue = mXMin;
		mYValue = mYMin;
	}
	update(0);
}

Blendspace2DInstance::~Blendspace2DInstance() = default;

void Blendspace2DInstance::setXValue(float value) {
	value = std::max(mXMin, value);
	value = std::min(mXMax, value);
	mXValue = value;
}

float Blendspace2DInstance::maxX() const {
	return mXMax;
}
float Blendspace2DInstance::minX() const {
	return mXMin;
}
float Blendspace2DInstance::valueX() const {
	return mXValue;
}

void Blendspace2DInstance::setYValue(float value) {
	value = std::max(mYMin, value);
	value = std::min(mYMax, value);
	mYValue = value;
}

float Blendspace2DInstance::maxY() const {
	return mYMax;
}
float Blendspace2DInstance::minY() const {
	return mYMin;
}
float Blendspace2DInstance::valueY() const {
	return mYValue;
}

Pose* Blendspace2DInstance::getPose() {
	return mLastPose;
}

void Blendspace2DInstance::update(float dt) {
	if (mRevision != mResource->revision) {
		Blendspace2DInstance candidate(mResource);
		mRows = std::move(candidate.mRows);
		mBlend1 = std::move(candidate.mBlend1);
		mBlend2 = std::move(candidate.mBlend2);
		mBlend3 = std::move(candidate.mBlend3);
		mXMin = candidate.mXMin;
		mXMax = candidate.mXMax;
		mYMin = candidate.mYMin;
		mYMax = candidate.mYMax;
		setXValue(mXValue);
		setYValue(mYValue);
		mRevision = mResource->revision;
	}
	mPhase += dt / std::max(.001f, mRows.front().nodes.front().sampler->duration());
	mPhase -= std::floor(mPhase);
	for (auto& row : mRows) {
		for (auto& node : row.nodes) {
			node.sampler->seek(mPhase * node.sampler->duration());
		}
	}
	for (uint32_t i = 0; i < mRows.size(); i++) {
		if (mYValue == mRows[i].value) {
			mLastPose = blendedPoseFromRow(mRows[i], mBlend1.get(), dt);
			return;
		}
		else if (mYValue < mRows[i].value) {
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
	if (row.nodes.size() == 1 || mXValue <= row.nodes.front().value) {
		return row.nodes.front().sampler->getPose();
	}
	if (mXValue >= row.nodes.back().value) {
		return row.nodes.back().sampler->getPose();
	}

	for (uint32_t j = 0; j < row.nodes.size(); j++) {
		if (mXValue == row.nodes[j].value) {
			auto& mutableRow = const_cast<RowState&>(row);
			return mutableRow.nodes[j].sampler->getPose();
		}
		else if (mXValue < row.nodes[j].value) {
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
	Pose* lowPose = low.sampler->getPose();
	Pose* highPose = high.sampler->getPose();

	return blend->blend(lowPose, highPose, blendFactor);
}

std::unique_ptr<IAnimationPlayable> Blendspace2DInstance::clone() const {
	auto copy = std::make_unique<Blendspace2DInstance>(mResource);
	copy->mPhase = mPhase;
	copy->setXValue(mXValue);
	copy->setYValue(mYValue);
	copy->update(0);
	return copy;
}
}
