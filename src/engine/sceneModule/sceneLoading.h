#pragma once
#include "resourceModule/resourceRequest.h"

namespace IKIGAI::SCENE_SYSTEM {
enum class SceneTransitionState { Idle, ShowingLoadingScene, Preparing, Unloading, Loading, Activating, HidingLoadingScene, CleaningUp, Failed };
enum class SceneLoadStatus { Queued, Running, Completed, Cancelled, Failed };
struct SceneLoadState {
	std::string path;
	std::string phase = "Queued";
	std::string error;
	double progress = 0.0;
	int resultRootId = 0;
	SceneLoadStatus status = SceneLoadStatus::Queued;
	std::shared_ptr<RESOURCES::ResourceLoadControl> control = std::make_shared<RESOURCES::ResourceLoadControl>();
	std::function<void()> cancelResource;
	bool done() const {
		return status == SceneLoadStatus::Completed || status == SceneLoadStatus::Cancelled || status == SceneLoadStatus::Failed;
	}
	void cancel() {
		if (!done()) {
			control->cancelled = true;
			if (cancelResource)
				cancelResource();
		}
	}
};
class SceneLoadHandle {
	friend class SceneManager;
	std::shared_ptr<SceneLoadState> mState;
	explicit SceneLoadHandle(std::shared_ptr<SceneLoadState> state)
		: mState(std::move(state)) { }

public:
	SceneLoadHandle() = default;
	explicit operator bool() const {
		return static_cast<bool>(mState);
	}
	bool done() const {
		return !mState || mState->done();
	}
	SceneLoadStatus status() const {
		return mState ? mState->status : SceneLoadStatus::Cancelled;
	}
	double progress() const {
		return mState ? mState->progress : 0.0;
	}
	std::string phase() const {
		return mState ? mState->phase : std::string { };
	}
	std::string error() const {
		return mState ? mState->error : std::string { };
	}
	int resultRootId() const {
		return mState ? mState->resultRootId : 0;
	}
	void cancel() const {
		if (mState)
			mState->cancel();
	}
};
}
