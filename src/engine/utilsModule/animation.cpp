#include "animation.h"

#include <algorithm>

using namespace IKIGAI::ANIMATION;

import glmath;

float FUNC::linearInterpolation(float a)
{
	return a;
}

float FUNC::quadraticEaseIn(float a)
{
	return a * a;
}

float FUNC::quadraticEaseOut(float a)
{
	return -(a * (a - static_cast<float>(2)));
}

float FUNC::quadraticEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
	{
		return static_cast<float>(2) * a * a;
	}
	else
	{
		return (-static_cast<float>(2) * a * a) + (4 * a) - 1.0f;
	}
}

float FUNC::cubicEaseIn(float a)
{
	return a * a * a;
}

float FUNC::cubicEaseOut(float a)
{
	float const f = a - 1.0f;
	return f * f * f + 1.0f;
}

float FUNC::cubicEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
	{
		return static_cast<float>(4) * a * a * a;
	}
	else
	{
		float const f = ((static_cast<float>(2) * a) - static_cast<float>(2));
		return static_cast<float>(0.5) * f * f * f + 1.0f;
	}
}

float FUNC::quarticEaseIn(float a)
{
	return a * a * a * a;
}

float FUNC::quarticEaseOut(float a)
{
	float const f = (a - 1.0f);
	return f * f * f * (1.0f - a) + 1.0f;
}

float FUNC::quarticEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
	{
		return static_cast<float>(8) * a * a * a * a;
	}
	else
	{
		float const f = (a - 1.0f);
		return -static_cast<float>(8) * f * f * f * f + 1.0f;
	}
}

float FUNC::quinticEaseIn(float a)
{
	return a * a * a * a * a;
}

float FUNC::quinticEaseOut(float a)
{
	float const f = (a - 1.0f);
	return f * f * f * f * f + 1.0f;
}

float FUNC::quinticEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
	{
		return static_cast<float>(16) * a * a * a * a * a;
	}
	else
	{
		float const f = ((static_cast<float>(2) * a) - static_cast<float>(2));
		return static_cast<float>(0.5) * f * f * f * f * f + 1.0f;
	}
}

float FUNC::sineEaseIn(float a)
{
	return sin((a - 1.0f) * (PI / 2.0f)) + 1.0f;
}

float FUNC::sineEaseOut(float a)
{
	return sin(a * (PI / 2.0f));
}

float FUNC::sineEaseInOut(float a)
{
	return static_cast<float>(0.5) * (1.0f - cos(a * PI));
}

float FUNC::circularEaseIn(float a)
{
	return 1.0f - sqrt(1.0f - (a * a));
}

float FUNC::circularEaseOut(float a)
{
	return sqrt((static_cast<float>(2) - a) * a);
}

float FUNC::circularEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
	{
		return static_cast<float>(0.5) * (1.0f - sqrtf(1.0f - static_cast<float>(4) * (a * a)));
	}
	else
	{
		return static_cast<float>(0.5) * (sqrtf(-((static_cast<float>(2) * a) - static_cast<float>(3)) * ((static_cast<float>(2) * a) - 1.0f)) + 1.0f);
	}
}

float FUNC::exponentialEaseIn(float a)
{
	if (a <= 0.0f)
		return a;
	else
	{
		float const Complementary = a - 1.0f;
		float const Two = static_cast<float>(2);

		return powf(Two, Complementary * static_cast<float>(10));
	}
}

float FUNC::exponentialEaseOut(float a)
{
	if (a >= 1.0f)
		return a;
	else
	{
		return 1.0f - powf(static_cast<float>(2), -static_cast<float>(10) * a);
	}
}

float FUNC::exponentialEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
		return static_cast<float>(0.5) * powf(static_cast<float>(2), (static_cast<float>(20) * a) - static_cast<float>(10));
	else
		return -static_cast<float>(0.5) * powf(static_cast<float>(2), (-static_cast<float>(20) * a) + static_cast<float>(10)) + 1.0f;
}

float FUNC::elasticEaseIn(float a)
{
	return sinf(static_cast<float>(13) * (PI / 2.0f) * a) * powf(static_cast<float>(2), static_cast<float>(10) * (a - 1.0f));
}

float FUNC::elasticEaseOut(float a)
{
	return sinf(-static_cast<float>(13) * (PI / 2.0f) * (a + 1.0f)) * powf(static_cast<float>(2), -static_cast<float>(10) * a) + 1.0f;
}

float FUNC::elasticEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
		return static_cast<float>(0.5) * sinf(static_cast<float>(13) * (PI / 2.0f) * (static_cast<float>(2) * a)) * powf(static_cast<float>(2), static_cast<float>(10) * ((static_cast<float>(2) * a) - 1.0f));
	else
		return static_cast<float>(0.5) * (sinf(-static_cast<float>(13) * (PI / 2.0f) * ((static_cast<float>(2) * a - 1.0f) + 1.0f)) * powf(static_cast<float>(2), -static_cast<float>(10) * (static_cast<float>(2) * a - 1.0f)) + static_cast<float>(2));
}

float FUNC::backEaseInInternal(float a, float o)
{
	float z = ((o + 1.0f) * a) - o;
	return (a * a * z);
}

float FUNC::backEaseOutInternal(float a, float o)
{
	float n = a - 1.0f;
	float z = ((o + 1.0f) * n) + o;
	return (n * n * z) + 1.0f;
}

float FUNC::backEaseInOutInternal(float a, float o)
{
	float s = o * static_cast<float>(1.525);
	float x = static_cast<float>(0.5);
	float n = a / static_cast<float>(0.5);

	if (n < static_cast<float>(1))
	{
		float z = ((s + static_cast<float>(1)) * n) - s;
		float m = n * n * z;
		return x * m;
	}
	else
	{
		n -= static_cast<float>(2);
		float z = ((s + static_cast<float>(1)) * n) + s;
		float m = (n * n * z) + static_cast<float>(2);
		return x * m;
	}
}

float FUNC::backEaseIn(float a)
{
	return backEaseInInternal(a, static_cast<float>(1.70158));
}

float FUNC::backEaseOut(float a)
{
	return FUNC::backEaseOutInternal(a, static_cast<float>(1.70158));
}

float FUNC::backEaseInOut(float a)
{
	return FUNC::backEaseInOutInternal(a, static_cast<float>(1.70158));
}

float FUNC::bounceEaseOut(float a)
{
	if (a < static_cast<float>(4.0 / 11.0))
	{
		return (static_cast<float>(121) * a * a) / static_cast<float>(16);
	}
	else if (a < static_cast<float>(8.0 / 11.0))
	{
		return (static_cast<float>(363.0 / 40.0) * a * a) - (static_cast<float>(99.0 / 10.0) * a) + static_cast<float>(17.0 / 5.0);
	}
	else if (a < static_cast<float>(9.0 / 10.0))
	{
		return (static_cast<float>(4356.0 / 361.0) * a * a) - (static_cast<float>(35442.0 / 1805.0) * a) + static_cast<float>(16061.0 / 1805.0);
	}
	else
	{
		return (static_cast<float>(54.0 / 5.0) * a * a) - (static_cast<float>(513.0 / 25.0) * a) + static_cast<float>(268.0 / 25.0);
	}
}

float FUNC::bounceEaseIn(float a)
{
	return 1.0f - bounceEaseOut(1.0f - a);
}

float FUNC::bounceEaseInOut(float a)
{
	if (a < static_cast<float>(0.5))
	{
		return static_cast<float>(0.5) * (1.0f - bounceEaseOut(a * static_cast<float>(2)));
	}
	else
	{
		return static_cast<float>(0.5) * bounceEaseOut(a * static_cast<float>(2) - 1.0f) + static_cast<float>(0.5);
	}
}


AnimationProperty::AnimationProperty(std::string name,
	std::function<void(PropType)> set,
	std::function<PropType()> get,
	std::function<float(float)> interpolation) :
	name(name), _set(std::move(set)), _get(std::move(get)),
	interpolation(interpolation) {

}

void AnimationProperty::set(PropType val) {
	_set(val);
};
PropType AnimationProperty::get() {
	return  _get();
};
const std::string& AnimationProperty::getName() const {
	return name;
}
std::function<float(float)> AnimationProperty::getInterpolation() const {
	return interpolation;
}

Animation::Builder::Builder(unsigned int framesCount, unsigned int fps, bool isLooped) :
	anim(new Animation(framesCount, fps, isLooped)) {
}

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

Animation::Animation(unsigned int frameCount, unsigned int fps, bool isLooped) : AnimationBase(),
	framesCount(frameCount), FPS(fps)
{
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
	return x0 + (x1 - x0) * progress;
}

void Animation::addProperty(AnimationProperty p) {
	auto n = p.getName();
	props.emplace(n, std::move(p));
}

void Animation::addKeyFrame(int frame, std::map<std::string, PropType> prop) {
	frames.emplace(frame, prop);
}

void Animation::delKeyFrameProp(int frame, std::map<std::string, PropType> prop) {
	if (!frames.contains(frame)) {
		return;
	}
	for (auto& e : prop) {
		frames[frame].erase(e.first);
	}
	frames.emplace(frame, prop);
}

void Animation::addKeyFrameMerge(int frame, std::map<std::string, PropType> prop) {
	if (frames.find(frame) == frames.end()) {
		frames.emplace(frame, prop);
		return;
	}
	for (auto& p : prop) {
		frames.at(frame).insert_or_assign(p.first, p.second);
	}
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


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
				std::visit(overloaded {
					[this, &p, a1, a2, progress](const float& val) {
						auto val1 = std::get_if<float>(&frames[a1][p.first]);
						auto val2 = std::get_if<float>(&frames[a2][p.first]);
						if (val1 && val2) {
							curState[p.first] = linearFunc(*val1, *val2, p.second.getInterpolation()(progress));
						}
					},
					[this, &p, a1, a2, progress](const int& val) {
						auto val1 = std::get_if<int>(&frames[a1][p.first]);
						auto val2 = std::get_if<int>(&frames[a2][p.first]);
						if (val1 && val2) {
							curState[p.first] = linearFunc(*val1, *val2, p.second.getInterpolation()(progress));
						}
					},
					[this, &p, a1, a2, progress](const bool& val) {
						auto val1 = std::get_if<bool>(&frames[a1][p.first]);
						auto val2 = std::get_if<bool>(&frames[a2][p.first]);
						if (val1 && val2) {
							curState[p.first] = linearFunc(*val1, *val2, p.second.getInterpolation()(progress));
						}
					},
					[this, &p, a1, a2, progress](const auto& val) {
						
					}
				}, frames[a1][p.first]);
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

//State Machine

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

StateMachine::Builder& StateMachine::Builder::addEdge(StateType from, StateType to,
	std::function<bool()> act) {
	if (stateGraph.find(from) == stateGraph.end()) {
		stateGraph[from] = Edges();
	}
	stateGraph[from].push_back(Edge{ to, act });
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

StateMachine::StateType StateMachine::getCurState()  const {
	return currentState;
}

StateMachine::StateType StateMachine::getStartState()  const {
	return startState;
}

StateMachine::Status StateMachine::getStatus() const {
	return status;
}