#include "animation.h"

#include <algorithm>

using namespace IKIGAI::ANIMATION;
using namespace IKIGAI::MATH;

AnimationProperty::AnimationProperty(std::string name, std::function<void(PropType)> set, std::function<PropType()> get, InterpolationType type)
	: name(name)
	, _set(std::move(set))
	, _get(std::move(get))
	, interpolationType(type) { }

void AnimationProperty::set(PropType val) {
	_set(val);
};
PropType AnimationProperty::get() {
	return _get();
};
const std::string& AnimationProperty::getName() const {
	return name;
}
InterpolationType AnimationProperty::getInterpolationType() const {
	return interpolationType;
}

Animation::Builder::Builder(unsigned int framesCount, unsigned int fps, bool isLooped)
	: anim(new Animation(framesCount, fps, isLooped)) { }

Animation::Builder& Animation::Builder::addKeyFrame(int frame, std::map<std::string, PropType> prop) {
	anim->addKeyFrame(frame, prop);
	return *this;
}

Animation::Builder& Animation::Builder::addKeyFrameMerge(int frame, std::map<std::string, PropType> prop) {
	anim->addKeyFrameMerge(frame, prop);
	return *this;
}

Animation::Builder& Animation::Builder::addProperty(AnimationProperty p) {
	anim->addProperty(std::move(p));
	return *this;
}

std::unique_ptr<Animation> Animation::Builder::build() {
	return std::move(anim);
}

Animation::Animation(unsigned int frameCount, unsigned int fps, bool isLooped)
	: AnimationBase()
	, framesCount(frameCount)
	, FPS(fps) {
	time = static_cast<float>(frameCount * fps);
	this->isLooped = isLooped;
}

void Animation::setFrameCount(unsigned int _frameCount) {
	framesCount = _frameCount;
}

void Animation::setFPS(unsigned int fps) {
	FPS = fps;
}

void Animation::setLooped(bool b) {
	isLooped = b;
}

void Animation::init() {
	isInit = true;
	fastAccess.clear();
	for (auto& s : frames) {
		for (auto& p : s.second) {
			fastAccess[p.first].push_back(s.first);
		}
	}
	for (auto& v : fastAccess) {
		std::sort(v.second.begin(), v.second.end());
	}
}

float Animation::linearFunc(float x0, float x1, float progress) {
	if (progress > 1.f)
		progress = 1.0f;
	return static_cast<float>(x0 + (x1 - x0) * progress);
}

int Animation::linearFunc(int x0, int x1, float progress) {
	if (progress > 1.f)
		progress = 1.0f;
	return static_cast<int>(x0 + (x1 - x0) * progress);
}

bool Animation::linearFunc(bool x0, bool x1, float progress) {
	if (progress > 1.f)
		progress = 1.0f;
	return progress == 1.0f ? x1 : x0;
}

void Animation::moveFrameInTrack(const std::string& trackName, int oldFrame, int newFrame) {
	if (!frames.contains(oldFrame)) {
		return;
	}
	if (!frames[oldFrame].contains(trackName)) {
		return;
	}

	auto prop = frames[oldFrame][trackName];
	auto curve = curves[trackName][oldFrame];
	// Delete old
	frames[oldFrame].erase(trackName);
	curves[trackName].erase(oldFrame);

	// Add to new
	if (frames[newFrame].contains(trackName)) {
		if (newFrame > oldFrame) {
			if (newFrame < framesCount - 1) {
				newFrame++;
			}
			else {
				newFrame = oldFrame;
			}
		}
		else {
			if (newFrame > 0) {
				newFrame--;
			}
			else {
				newFrame = oldFrame;
			}
		}
	}
	curves[trackName][newFrame] = curve;
	frames[newFrame].insert_or_assign(trackName, prop);

	// frames.emplace(frame, prop);
	init();
}

void Animation::addProperty(AnimationProperty p) {
	auto n = p.getName();
	props.emplace(n, std::move(p));
	isInit = false;
}

void Animation::delProperty(const std::string& name) {
	props.erase(name);
	curves.erase(name);
	isInit = false;
}

void Animation::addKeyFrame(int frame, std::map<std::string, PropType> prop) {
	frames.emplace(frame, prop);

	for (auto& p : prop) {
		// int startFrame = -1;
		// for (auto& e : fastAccess[p.first]) {
		//	if (e < frame) {
		//		continue;
		//	}
		//	startFrame = e;
		//	break;
		// }
		// if (frame == 0 && !curves[p.first].empty() && curves[p.first][0].mLeft.x == 0.0f) {
		//	curves[p.first].erase(curves[p.first].begin());
		// }

		curves[p.first][frame] = Curve {
			{ 0.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.5f, 0.0f },
			{ -1.0f, 0.0f },
		};

		// std::sort(curves[p.first].begin(), curves[p.first].end(), [](const Curve& a, const Curve& b) {
		//	return a.mLeft.x < b.mLeft.x;
		// });
		//
		// auto& _curves = curves[p.first];
		// for (int i = 0; i < _curves.size()-1; ++i) {
		//	_curves[i].mRight = _curves[i + 1].mLeft;
		// }
		//
		// if (!_curves.empty() && _curves[0].mLeft.x != 0.0f) {
		//	_curves.emplace(_curves.begin(), Curve{
		//		{static_cast<float>(0.0f), 0.5f},
		//		_curves[0].mLeft,
		//		{0.5f, 0.0f},
		//		{-1.0f, 0.0f}
		//	});
		// }
	}

	init();
}

void Animation::delKeyFrameProp(int frame, std::map<std::string, PropType> prop) {
	if (!frames.contains(frame)) {
		return;
	}
	for (auto& e : prop) {
		frames[frame].erase(e.first);
		curves[e.first].erase(frame);
		// std::erase_if(curves[e.first], [frame](const Curve& x) { return static_cast<int>(x.mLeft.x) == frame; });
	}

	// frames.emplace(frame, prop);
	init();

	for (auto& e : prop) {
		if (fastAccess[e.first].empty()) {
			curves.erase(e.first);
		}
	}
}

void Animation::delKeyFrameProp(int frame, const std::string& propName) {
	if (!frames.contains(frame)) {
		return;
	}
	frames[frame].erase(propName);
	if (frames[frame].empty()) {
		frames.erase(frame);
	}

	curves[propName].erase(frame);
	// std::erase_if(curves[propName], [frame](const Curve& x) { return static_cast<int>(x.mLeft.x) == frame; });

	init();

	if (fastAccess[propName].empty()) {
		curves.erase(propName);
	}
}

void Animation::addKeyFrameMerge(int frame, std::map<std::string, PropType> prop) {
	if (frames.find(frame) == frames.end()) {
		frames.emplace(frame, prop);
	}
	else {
		for (auto& p : prop) {
			frames.at(frame).insert_or_assign(p.first, p.second);
		}
	}

	for (auto& p : prop) {
		// if (frame == 0 && !curves[p.first].empty() && curves[p.first][0].mLeft.x == 0.0f) {
		//	curves[p.first].erase(curves[p.first].begin());
		// }

		curves[p.first][frame] = Curve {
			{ 0.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.5f, 0.0f },
			{ -1.0f, 0.0f },
		};

		// std::sort(curves[p.first].begin(), curves[p.first].end(), [](const Curve& a, const Curve& b) {
		//	return a.mLeft.x < b.mLeft.x;
		// });
		//
		// auto& _curves = curves[p.first];
		// for (int i = 0; i < _curves.size() - 1; ++i) {
		//	_curves[i].mRight = _curves[i + 1].mLeft;
		// }
		//
		// if (!_curves.empty() && _curves[0].mLeft.x != 0.0f) {
		//	_curves.emplace(_curves.begin(), Curve{
		//		{static_cast<float>(0.0f), 0.5f},
		//		_curves[0].mLeft,
		//		{0.5f, 0.0f},
		//		{-1.0f, 0.0f}
		//	});
		// }
	}
	init();
}

bool Animation::hasFrameForProperty(int frame, const std::string& name) {
	auto& vec = fastAccess[name];
	return std::find(vec.begin(), vec.end(), frame) != vec.end();
}

template <class... Ts> struct overloaded : Ts... {
	using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

float cubicBezierCurve(const Curve& curve, float t) {
	return IKIGAI::MATH::CURVES::evaluateParametric(curve, t).y;
}

void Animation::update(float dt) {
	if (animStatus == AnimationBase::Status::PAUSE || animStatus == AnimationBase::Status::STOP) {
		return;
	}
	if (!isInit) {
		init();
	}
	if (dt < std::numeric_limits<float>::epsilon()) {
		dt = 16.66f;
	}
	curTime += dt;

	auto curFrame = static_cast<int>(curTime / FPS);
	if (curFrame > framesCount) {
		if (isLooped) {
			curTime = 0.0f;
		}
		else {
			animStatus = AnimationBase::Status::STOP;
		}
	}
	curState.clear();
	for (auto& p : props) {
		for (auto i = 0u; i < fastAccess[p.first].size() - 1; i++) {
			if (curFrame >= fastAccess[p.first][i]) {
				auto a1 = fastAccess[p.first][i];
				auto a2 = fastAccess[p.first][i + 1];
				if (curTime < a1 * FPS || curTime > a2 * FPS) {
					continue;
				}
				auto progress = (curTime - a1 * FPS) / ((a2 - a1) * FPS);
				if (progress > 1.0f) {
					progress = 1.0f;
				}
				if (progress < 0.0f) {
					progress = 0.0f;
				}

				if (p.second.getInterpolationType() == InterpolationType::CUSTOM) {
					progress = cubicBezierCurve(curves[p.first].at(i), progress);
				}
				else {
					progress = functions[p.second.getInterpolationType()](progress);
				}

				std::visit(overloaded { [this, &p, a1, a2, progress](const float& val) {
										   auto val1 = std::get_if<float>(&frames[a1][p.first]);
										   auto val2 = std::get_if<float>(&frames[a2][p.first]);
										   if (val1 && val2) {
											   curState[p.first] = linearFunc(*val1, *val2, progress);
										   }
									   },
						   [this, &p, a1, a2, progress](const int& val) {
							   auto val1 = std::get_if<int>(&frames[a1][p.first]);
							   auto val2 = std::get_if<int>(&frames[a2][p.first]);
							   if (val1 && val2) {
								   curState[p.first] = linearFunc(*val1, *val2, progress);
							   }
						   },
						   [this, &p, a1, a2, progress](const bool& val) {
							   auto val1 = std::get_if<bool>(&frames[a1][p.first]);
							   auto val2 = std::get_if<bool>(&frames[a2][p.first]);
							   if (val1 && val2) {
								   curState[p.first] = linearFunc(*val1, *val2, progress);
							   }
						   },
						   [this, &p, a1, a2, progress](const MATH::Vector2f& val) {
							   auto val1 = std::get_if<MATH::Vector2f>(&frames[a1][p.first]);
							   auto val2 = std::get_if<MATH::Vector2f>(&frames[a2][p.first]);
							   if (val1 && val2) {
								   curState[p.first] = MATH::Vector2f(linearFunc(val1->x, val2->x, progress), linearFunc(val1->y, val2->y, progress));
							   }
						   },
						   [this, &p, a1, a2, progress](const MATH::Vector3f& val) {
							   auto val1 = std::get_if<MATH::Vector3f>(&frames[a1][p.first]);
							   auto val2 = std::get_if<MATH::Vector3f>(&frames[a2][p.first]);
							   if (val1 && val2) {
								   curState[p.first] = MATH::Vector3f(linearFunc(val1->x, val2->x, progress), linearFunc(val1->y, val2->y, progress),
									   linearFunc(val1->z, val2->z, progress));
							   }
						   },
						   [this, &p, a1, a2, progress](const MATH::Vector4f& val) {
							   auto val1 = std::get_if<MATH::Vector4f>(&frames[a1][p.first]);
							   auto val2 = std::get_if<MATH::Vector4f>(&frames[a2][p.first]);
							   if (val1 && val2) {
								   curState[p.first] = MATH::Vector4f(linearFunc(val1->x, val2->x, progress), linearFunc(val1->y, val2->y, progress),
									   linearFunc(val1->z, val2->z, progress), linearFunc(val1->w, val2->w, progress));
							   }
						   },
						   [this, &p, a1, a2, progress, curFrame](const std::string& val) {
							   // auto val1 = std::get_if<std::string>(&frames[a1][p.first]);
							   // auto val2 = std::get_if<std::string>(&frames[a2][p.first]);
							   // if (val1 && val2) {
							   if (curFrame == a1) {
								   curState[p.first] = val;
							   }
							   // else if (curFrame == a2) {
							   //	curState[p.first] = &val2;
							   // }
							   //}
						   },
						   [this, &p, a1, a2, progress](const auto& val) {

						   } },
					frames[a1][p.first]);
			}
		}
	}

	for (auto& v : curState) {
		props.at(v.first).set(v.second);
	}
}

void Animation::play() {
	animStatus = AnimationBase::Status::PLAY;
}

void Animation::stop() {
	curTime = 0.0f;
	animStatus = AnimationBase::Status::STOP;
}

void Animation::pause() {
	animStatus = AnimationBase::Status::PAUSE;
}

// State Machine

StateMachine::Builder::Builder() = default;

StateMachine::Builder& StateMachine::Builder::addState(StateType newState, std::unique_ptr<AnimationBase> anim) {
	if (states.find(newState) == states.end()) {
		states[newState] = std::vector<std::unique_ptr<AnimationBase>>();
	}
	states.at(newState).push_back(std::move(anim));
	return *this;
}
StateMachine::Builder& StateMachine::Builder::setStartState(StateType state) {
	start = state;
	return *this;
}

StateMachine::Builder& StateMachine::Builder::addEdge(StateType from, StateType to, std::function<bool()> act) {
	if (stateGraph.find(from) == stateGraph.end()) {
		stateGraph[from] = Edges();
	}
	stateGraph[from].push_back(Edge { to, act });
	return *this;
}

StateMachine StateMachine::Builder::build() {
	StateMachine stateMachine;
	stateMachine.states = std::move(states);
	stateMachine.stateGraph = std::move(stateGraph);
	stateMachine.setStartState(start);
	return std::move(stateMachine);
}

StateMachine::StateMachine() = default;

void StateMachine::setStartState(StateType state) {
	startState = state;
}

void StateMachine::setState(StateType state) {
	currentState = state;
	for (auto& anim : states.at(currentState)) {
		anim->play();
	}
}

void StateMachine::update(float dt) {
	if (status != Status::PLAY) {
		return;
	}
	for (auto& anim : states.at(currentState)) {
		anim->update(dt);
	}
	if (stateGraph.find(currentState) == stateGraph.end()) {
		return;
	}
	for (auto& s : stateGraph.at(currentState)) {
		if (s.condition()) {
			for (auto& anim : states.at(currentState)) {
				anim->stop();
			}
			currentState = s.type;
			for (auto& anim : states.at(currentState)) {
				anim->play();
			}
			break;
		}
	}
}

void StateMachine::play() {
	currentState = startState;
	status = Status::PLAY;
}

void StateMachine::stop() {
	status = Status::STOP;
	currentState = startState;
}

void StateMachine::pause() {
	status = Status::PAUSE;
}

StateMachine::StateType StateMachine::getCurState() const {
	return currentState;
}

StateMachine::StateType StateMachine::getStartState() const {
	return startState;
}

StateMachine::Status StateMachine::getStatus() const {
	return status;
}
