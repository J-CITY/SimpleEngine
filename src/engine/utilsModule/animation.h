#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "weakPtr.h"
#include "coreModule/ecs/object.h"
#include "sceneModule/sceneManager.h"


namespace IKIGAI::ANIMATION {
	namespace FUNC {
		float linearInterpolation(float a);
		float quadraticEaseIn(float a);
		float quadraticEaseOut(float a);
		float quadraticEaseInOut(float a);
		float cubicEaseIn(float a);
		float cubicEaseOut(float a);
		float cubicEaseInOut(float a);
		float quarticEaseIn(float a);
		float quarticEaseOut(float a);
		float quarticEaseInOut(float a);
		float quinticEaseIn(float a);
		float quinticEaseOut(float a);
		float quinticEaseInOut(float a);
		float sineEaseIn(float a);
		float sineEaseOut(float a);
		float sineEaseInOut(float a);
		float circularEaseIn(float a);
		float circularEaseOut(float a);
		float circularEaseInOut(float a);
		float exponentialEaseIn(float a);
		float exponentialEaseOut(float a);
		float exponentialEaseInOut(float a);
		float elasticEaseIn(float a);
		float elasticEaseOut(float a);
		float elasticEaseInOut(float a);
		float backEaseInInternal(float a, float o);
		float backEaseOutInternal(float a, float o);
		float backEaseInOutInternal(float a, float o);
		float backEaseIn(float a);
		float backEaseOut(float a);
		float backEaseInOut(float a);
		float bounceEaseOut(float a);
		float bounceEaseIn(float a);
		float bounceEaseInOut(float a);
	}

	using PropType = std::variant<float, int, bool, std::string, MATH::Vector4f, MATH::Vector3f, MATH::Vector2f>;

	enum class InterpolationType {
		LINEAR,
		QUADRATIC_IN,
		QUADRATIC_OUT,
		QUADRATIC_IN_OUT,
		CUBIC_IN,
		CUBIC_OUT,
		CUBIC_IN_OUT,
		QUADRIC_IN,
		QUADRIC_OUT,
		QUADRIC_IN_OUT,
		QUINTIC_IN,
		QUINTIC_OUT,
		QUINTIC_IN_OUT,
		SINE_IN,
		SINE_OUT,
		SINE_IN_OUT,
		CIRCULAR_IN,
		CIRCULAR_OUT,
		CIRCULAR_IN_OUT,
		EXPONENTIAL_IN,
		EXPONENTIAL_OUT,
		EXPONENTIAL_IN_OUT,
		ELASTIC_IN,
		ELASTIC_OUT,
		ELASTIC_IN_OUT,
		BACK_IN,
		BACK_OUT,
		BACK_IN_OUT,
		BOUNCE_IN,
		BOUNCE_OUT,
		BOUNCE_IN_OUT,

		CUSTOM
	};
	inline std::map<InterpolationType, std::function<float(float)>> functions = {
			{ InterpolationType::LINEAR, &FUNC::linearInterpolation },
			{ InterpolationType::QUADRATIC_IN, &FUNC::quadraticEaseIn },
			{ InterpolationType::QUADRATIC_OUT, &FUNC::quadraticEaseOut },
			{ InterpolationType::QUADRATIC_IN_OUT, &FUNC::quadraticEaseInOut },
			{ InterpolationType::CUBIC_IN, &FUNC::cubicEaseIn },
			{ InterpolationType::CUBIC_OUT, &FUNC::cubicEaseOut },
			{ InterpolationType::CUBIC_IN_OUT, &FUNC::cubicEaseInOut },
			{ InterpolationType::QUADRIC_IN, &FUNC::quarticEaseIn },
			{ InterpolationType::QUADRIC_OUT, &FUNC::quarticEaseOut },
			{ InterpolationType::QUADRIC_IN_OUT, &FUNC::quarticEaseInOut },
			{ InterpolationType::QUINTIC_IN, &FUNC::quinticEaseIn },
			{ InterpolationType::QUINTIC_OUT, &FUNC::quinticEaseOut },
			{ InterpolationType::QUINTIC_IN_OUT, &FUNC::quinticEaseInOut },
			{ InterpolationType::SINE_IN, &FUNC::sineEaseIn },
			{ InterpolationType::SINE_OUT, &FUNC::sineEaseOut },
			{ InterpolationType::SINE_IN_OUT, &FUNC::sineEaseInOut },
			{ InterpolationType::CIRCULAR_IN, &FUNC::circularEaseIn },
			{ InterpolationType::CIRCULAR_OUT, &FUNC::circularEaseOut },
			{ InterpolationType::CIRCULAR_IN_OUT, &FUNC::circularEaseInOut },
			{ InterpolationType::EXPONENTIAL_IN, &FUNC::exponentialEaseIn },
			{ InterpolationType::EXPONENTIAL_OUT, &FUNC::exponentialEaseOut },
			{ InterpolationType::EXPONENTIAL_IN_OUT, &FUNC::exponentialEaseInOut },
			{ InterpolationType::ELASTIC_IN, &FUNC::elasticEaseIn },
			{ InterpolationType::ELASTIC_OUT, &FUNC::elasticEaseOut },
			{ InterpolationType::ELASTIC_IN_OUT, &FUNC::elasticEaseInOut },
			{ InterpolationType::BACK_IN, &FUNC::backEaseIn },
			{ InterpolationType::BACK_OUT, &FUNC::backEaseOut },
			{ InterpolationType::BACK_IN_OUT, &FUNC::backEaseInOut },
			{ InterpolationType::BOUNCE_IN, &FUNC::bounceEaseOut },
			{ InterpolationType::BOUNCE_OUT, &FUNC::bounceEaseIn },
			{ InterpolationType::BOUNCE_IN_OUT, &FUNC::bounceEaseInOut }
	};

	//TODO: move to own file
	struct Curve {
		IKIGAI::MATH::Vector2f mLeft;
		IKIGAI::MATH::Vector2f mRight;
		IKIGAI::MATH::Vector2f mLeftTangent;
		IKIGAI::MATH::Vector2f mRightTangent;

		IKIGAI::MATH::Vector2f mLeftYOffset;

		template<class Context>
		constexpr static auto serde(Context& context, Curve& value) {
			using Self = Curve;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::mLeft, "Left")
				.field(&Self::mRight, "Right")
				.field(&Self::mLeftTangent, "LeftTangent")
				.field(&Self::mRightTangent, "RightTangent")
				.field(&Self::mLeftYOffset, "LeftYOffset");
		}
	};

	class AnimationProperty {
		std::string name;
		InterpolationType interpolationType;
		//std::function<float(float)> interpolation;
		std::function<void(PropType)> _set;
		std::function<PropType()> _get;
	public:
		AnimationProperty() = default;
		AnimationProperty(std::string name,
			std::function<void(PropType)> set,
			std::function<PropType()> get,
			InterpolationType type = InterpolationType::LINEAR);
		void set(PropType val);
		PropType get();
		const std::string& getName() const;
		InterpolationType getInterpolationType() const;
	};

	struct TimelineAnimationFrameDescriptor {
		int frame = 0;
		PropType value;

		InterpolationType curveType = InterpolationType::LINEAR;
		Curve curve;

		template<class Context>
		constexpr static auto serde(Context& context, TimelineAnimationFrameDescriptor& value) {
			using Self = TimelineAnimationFrameDescriptor;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::frame, "Frame")
				.field(&Self::value, "Value")
				.field(&Self::curveType, "CurveType")
				.field(&Self::curve, "Curve");
		}
	};

	struct TimelineAnimationTrackDescriptor {
		std::string objectName;
		int objectId = -1;
		std::string componentName;
		std::string propertyName;

		template<class Context>
		constexpr static auto serde(Context& context, TimelineAnimationTrackDescriptor& value) {
			using Self = TimelineAnimationTrackDescriptor;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::objectName, "ObjectName")
				.field(&Self::objectId, "ObjectId")
				.field(&Self::componentName, "ComponentName")
				.field(&Self::propertyName, "PropertyName");
		}
	};

	struct TimelineAnimationDescriptor {
		bool isLooped = false;
		float framesCount = 100;
		float FPS = 60;
		//track name -> track info
		std::map<std::string, TimelineAnimationTrackDescriptor> tracks;

		//track name -> array of frames
		std::map<std::string, std::vector<TimelineAnimationFrameDescriptor>> trackFrames;

		std::string pathResource;

		template<class Context>
		constexpr static auto serde(Context& context, TimelineAnimationDescriptor& value) {
			using Self = TimelineAnimationDescriptor;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::isLooped, "IsLooped")
				.field(&Self::framesCount, "FramesCount")
				.field(&Self::FPS, "FPS")
				.field(&Self::tracks, "Tracks")
				.field(&Self::trackFrames, "TrackFrames");
		}
	};

	class AnimationBase
		//:public UTILS::ControlBlockHandler TODO: add component
	{
	public:
		AnimationBase() = default;
		virtual ~AnimationBase() = default;

		enum class Status {
			PLAY,
			PAUSE,
			STOP
		};
		virtual void update(float dt) = 0;
		virtual void stop() = 0;
		virtual void play() = 0;
		virtual void pause() = 0;
		const Status& getStatus();
	protected:
		bool isLooped = false;
		Status animStatus = Status::STOP;
	};


	class Animation : public AnimationBase {
		friend class Builder;
	public:
		int FPS = 30;
		int framesCount = 50;
		float time = 0.f;
		float curTime = 0.f;

		std::map<int, std::map<std::string, PropType>> frames;
		std::map<std::string, AnimationProperty> props;
		std::map<std::string, std::map<int, Curve>> curves;
		bool isInit = false;
		std::map<std::string, std::vector<int>> fastAccess;
		std::map<std::string, PropType> curState;
	public:
		struct Builder {
		private:
			std::unique_ptr<Animation> anim;
		public:
			Builder(unsigned int framesCount, unsigned int fps, bool isLooped);
			Builder& addKeyFrame(int frame, std::map<std::string, PropType> prop);
			Builder& addKeyFrameMerge(int frame, std::map<std::string, PropType> prop);
			Builder& addProperty(AnimationProperty p);
			std::unique_ptr<Animation> build();
		};
	//private:
		Animation(unsigned int frameCount, unsigned int fps, bool isLooped);
		void setFrameCount(unsigned _frameCount);
		void setFPS(unsigned fps);
		void setLooped(bool b);
		unsigned getFrameCount() const {
			return framesCount;
		};
		unsigned getFPS() const {
			return FPS;
		};
		bool getLooped() const {
			return isLooped;
		};
		void init();

		float linearFunc(float x0, float x1, float progress);
		int linearFunc(int x0, int x1, float progress);
		bool linearFunc(bool x0, bool x1, float progress);

		void moveFrameInTrack(const std::string& trackName, int oldFrame, int newFrame);

		void addProperty(AnimationProperty p);
		void delProperty(const std::string& name);
		void addKeyFrame(int frame, std::map<std::string, PropType> prop);
		void delKeyFrameProp(int frame, std::map<std::string, PropType> prop);
		void delKeyFrameProp(int frame, const std::string& propName);
		void addKeyFrameMerge(int frame, std::map<std::string, PropType> prop);

		bool hasFrameForProperty(int frame, const std::string& name);
	public:
		void update(float dt) override;
		void play() override;
		void stop() override;
		void pause() override;
	};


	class StateMachine {
		friend class Builder;
	public:
		enum class Status {
			PLAY,
			PAUSE,
			STOP,
		};

		using StateType = unsigned int;
		using Condition = std::function<bool()>;
		struct Edge {
			StateType type;
			Condition condition;
		};
		using Edges = std::vector<Edge>;
		using StateGraphMap = std::map<StateType, Edges>;

		struct Builder {
		private:
			StateGraphMap stateGraph;
			std::map<StateType, std::vector<std::unique_ptr<AnimationBase>>> states;
			StateType start = 0;
		public:
			Builder();
			Builder& addEdge(StateType from, StateType to, std::function<bool()> act);
			Builder& addState(StateType newState, std::unique_ptr<AnimationBase> anim);
			Builder& setStartState(StateType state);
			StateMachine build();
		};

		void setStartState(StateType state);
		void setState(StateType state);
		void update(float dt);
		void play();
		void stop();
		void pause();
		StateType getCurState()  const;
		StateType getStartState()  const;
		Status getStatus()  const;
	private:
		StateMachine();

		StateGraphMap stateGraph;
		std::map<StateType, std::vector<std::unique_ptr<AnimationBase>>> states;
		StateType currentState;
		StateType startState;
		Status status = Status::STOP;
	};

	class AnimationSystem
	{
	public:
		void update() {}
	};


	//class NodableAnimation : public AnimationBase {
	//	std::unique_ptr<NodableAnimationNode> root;
	//public:
	//	void update(float dt) override {
	//		root->update(dt);
	//	}
	//	void stop() override;
	//	void play() override;
	//	void pause() override;
	//};

	class NodableAnimationNode {
	public:
		enum class Status {
			NOT_START,
			PROGRESS,
			FINISH,
		};
		NodableAnimationNode() = default;
		virtual ~NodableAnimationNode() = default;
		Status status = Status::NOT_START;

		virtual void update(float dt) = 0;
	};

	class NodableAnimationNodeSequence: public NodableAnimationNode {
		std::vector<std::unique_ptr<NodableAnimationNode>> childs;
		size_t currentId = 0;
	public:
		virtual void update(float dt) override {
			if (status == Status::FINISH || childs.empty()) {
				return;
			}
			if (status == Status::NOT_START) {
				status = Status::PROGRESS;
			}

			if (childs[currentId]->status == Status::FINISH) {
				currentId++;
				if (currentId >= childs.size()) {
					status = Status::FINISH;
					return;
				}
			}
			childs[currentId]->update(dt);
		}

		void clear() {
			childs.clear();
		}

		void add(std::unique_ptr<NodableAnimationNode> node) {
			childs.push_back(std::move(node));
		}
	};

	class NodableAnimationNodeSimultaneous : public NodableAnimationNode {
	public:
		std::vector<std::unique_ptr<NodableAnimationNode>> childs;
		size_t currentId = 0;
	public:
		virtual void update(float dt) override {
			if (status == Status::FINISH || childs.empty()) {
				return;
			}
			if (status == Status::NOT_START) {
				status = Status::PROGRESS;
			}

			bool allFinish = false;
			for (const auto& child : childs) {
				child->update(dt);
				if (child->status != Status::FINISH) {
					allFinish = true;
				}
			}

			if (allFinish) {
				status = Status::FINISH;
			}
		}

		void clear() {
			childs.clear();
		}

		void add(std::unique_ptr<NodableAnimationNode> node) {
			childs.push_back(std::move(node));
		}
	};


	class NodableAnimationNodePos : public NodableAnimationNode {
	public:
		ECS::Object::Id id;
		MATH::Vector3f toPos{};

		MATH::Vector3f fromPos{};
		float time = 0.0f;
		float curTime = 0.0f;
		
		Curve curve;
		AnimationProperty prop;
	public:
		NodableAnimationNodePos(ECS::Object::Id id, MATH::Vector3f toPos, float time, InterpolationType interpolation = InterpolationType::LINEAR):
			id(id), toPos(toPos), time(time) {

			auto& scene = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
			auto obj = scene.findObjectByID(id);
			auto component = obj->getComponent<ECS::TransformComponent>();
			fromPos = component->getLocalPosition();
			
			prop = AnimationProperty(
				"propPos",
				std::function<void(PropType)>([component](PropType prop) {
					if (auto val = std::get_if<MATH::Vector3f>(&prop)) {
						if (component) {
							component->setLocalPosition(*val);
						}
					}
				}),
				[component]() {
					if (component) {
						return component->getLocalPosition();
					}
					throw;
				},
				interpolation
			);
		}

		virtual void update(float dt) override {
			if (status == Status::FINISH ) {
				return;
			}
			if (status == Status::NOT_START) {
				status = Status::PROGRESS;
			}

			curTime += dt;

			float progress = 0.0f;
			if (prop.getInterpolationType() == InterpolationType::CUSTOM) {
				//progress = cubicBezierCurve(, std::min(1.0f, curTime / time));
			} else {
				progress = functions[prop.getInterpolationType()](std::min(1.0f, curTime / time));
			}

			const auto newPos = MATH::Vector3(
				fromPos.x + progress * (toPos.x - fromPos.x),
				fromPos.y + progress * (toPos.y - fromPos.y),
				fromPos.z + progress * (toPos.z - fromPos.z));
			prop.set(newPos);
			if (progress >= 1.0f) {
				status = Status::FINISH;
			}
		}
	};

	class NodableAnimationNodeScale : public NodableAnimationNode {
	public:
		ECS::Object::Id id;
		MATH::Vector3f to{};

		MATH::Vector3f from{};
		float time = 0.0f;
		float curTime = 0.0f;

		Curve curve;
		AnimationProperty prop;
	public:
		NodableAnimationNodeScale(ECS::Object::Id id, MATH::Vector3f to, float time, InterpolationType interpolation = InterpolationType::LINEAR) :
			NodableAnimationNode(), id(id), to(to), time(time) {

			auto& scene = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
			auto obj = scene.findObjectByID(id);
			auto component = obj->getComponent<ECS::TransformComponent>();
			from = component->getLocalScale();

			prop = AnimationProperty(
				"propScale",
				std::function<void(PropType)>([component](PropType prop) {
					if (auto val = std::get_if<MATH::Vector3f>(&prop)) {
						if (component) {
							component->setLocalScale(*val);
						}
					}
					}),
				[component]() {
						if (component) {
							return component->getLocalScale();
						}
						throw;
					},
					interpolation
				);
		}

		virtual void update(float dt) override {
			if (status == Status::FINISH) {
				return;
			}
			if (status == Status::NOT_START) {
				status = Status::PROGRESS;
			}

			curTime += dt;
			//const auto progress = prop.getInterpolation()(std::min(1.0f, curTime / time));

			float progress = 0.0f;
			if (prop.getInterpolationType() == InterpolationType::CUSTOM) {
				//progress = cubicBezierCurve(, std::min(1.0f, curTime / time));
			} else {
				progress = functions[prop.getInterpolationType()](std::min(1.0f, curTime / time));
			}

			const auto newPos = MATH::Vector3(
				from.x + progress * (to.x - from.x),
				from.y + progress * (to.y - from.y),
				from.z + progress * (to.z - from.z));
			prop.set(newPos);
			if (progress >= 1.0f) {
				status = Status::FINISH;
			}
		}
	};

	class NodableAnimationNodeRotate : public NodableAnimationNode {
	public:
		ECS::Object::Id id;
		MATH::Vector3f to{};

		MATH::Vector3f from{};
		float time = 0.0f;
		float curTime = 0.0f;

		Curve curve;
		AnimationProperty prop;
	public:
		NodableAnimationNodeRotate(ECS::Object::Id id, MATH::Vector3f to, float time, InterpolationType interpolation = InterpolationType::LINEAR) :
			NodableAnimationNode(), id(id), to(to), time(time) {

			auto& scene = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
			auto obj = scene.findObjectByID(id);
			auto component = obj->getComponent<ECS::TransformComponent>();
			from = component->getLocalRotationDeg();

			prop = AnimationProperty(
				"propRotate",
				std::function<void(PropType)>([component](PropType prop) {
					if (auto val = std::get_if<MATH::Vector3f>(&prop)) {
						if (component) {
							component->setLocalRotationDeg(*val);
						}
					}
					}),
				[component]() {
						if (component) {
							return component->getLocalRotationDeg();
						}
						throw;
					},
					interpolation
				);
		}

		virtual void update(float dt) override {
			if (status == Status::FINISH) {
				return;
			}
			if (status == Status::NOT_START) {
				status = Status::PROGRESS;
			}

			curTime += dt;
			//const auto progress = prop.getInterpolation()(std::min(1.0f, curTime / time));
			float progress = 0.0f;
			if (prop.getInterpolationType() == InterpolationType::CUSTOM) {
				//progress = cubicBezierCurve(, std::min(1.0f, curTime / time));
			} else {
				progress = functions[prop.getInterpolationType()](std::min(1.0f, curTime / time));
			}
			const auto newPos = MATH::Vector3(
				from.x + progress * (to.x - from.x),
				from.y + progress * (to.y - from.y),
				from.z + progress * (to.z - from.z));
			prop.set(newPos);
			if (progress >= 1.0f) {
				status = Status::FINISH;
			}
		}
	};

	class NodableAnimationNodeWait : public NodableAnimationNode {
	public:
		float time = 0.0f;
		float curTime = 0.0f;

	public:
		NodableAnimationNodeWait(float time) :
			NodableAnimationNode(), time(time) {
			
		}

		virtual void update(float dt) override {
			if (status == Status::FINISH) {
				return;
			}
			if (status == Status::NOT_START) {
				status = Status::PROGRESS;
			}

			curTime += dt;
			const auto progress = std::min(1.0f, curTime / time);

			if (progress >= 1.0f) {
				status = Status::FINISH;
			}
		}
	};

	class NodableAnimationNodeEvent : public NodableAnimationNode {
	public:
		float time = 0.0f;
		float curTime = 0.0f;
		std::string eventId;

	public:
		NodableAnimationNodeEvent(const std::string& eventId, float time) :
			NodableAnimationNode(), time(time), eventId(eventId){

		}

		virtual void update(float dt) override {
			if (status == Status::FINISH) {
				return;
			}
			if (status == Status::NOT_START) {
				status = Status::PROGRESS;
			}

			curTime += dt;
			const auto progress = std::min(1.0f, curTime / time);

			if (progress >= 1.0f) {
				RESOURCES::ServiceManager::Get<EVENT::EventBroadcaster>().run(eventId, EVENT::EventBroadcaster::Payload{});
				status = Status::FINISH;
			}
		}
	};

	class NodableAnimation : public AnimationBase {
	public:
		NodableAnimation() = default;
		virtual ~NodableAnimation() = default;
		Status status = Status::STOP;
		std::vector<std::unique_ptr<NodableAnimationNode>> animations;
		int currentAnim = 0;
		void update(float dt) {
			if (status == Status::PAUSE || status == Status::STOP) {
				return;
			}
			if (currentAnim >= animations.size()) {
				status = Status::STOP;
				return;
			}
			animations.at(currentAnim)->update(dt);
			if (animations.at(currentAnim)->status == NodableAnimationNode::Status::FINISH) {
				currentAnim++;
			}
		};

		void stop() override {
			status = Status::STOP;
			if (currentAnim < animations.size()) {
				status = Status::STOP;
				animations.at(currentAnim)->status = NodableAnimationNode::Status::NOT_START;
			}
			currentAnim = 0;
		}
		void play() override {
			status = Status::PLAY;
		}
		void pause() override {
			status = Status::PAUSE;
		}
	};
	
}
