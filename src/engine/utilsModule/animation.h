#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>


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

	using PropType = std::variant<float, int, bool, std::string>;
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
		BOUNCE_IN_OUT
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

	class AnimationProperty {
		std::string name;
		std::function<float(float)> interpolation;
		std::function<void(PropType)> _set;
		std::function<PropType()> _get;
	public:
		AnimationProperty(std::string name,
			std::function<void(PropType)> set,
			std::function<PropType()> get,
			std::function<float(float)> interpolation = &FUNC::linearInterpolation);
		void set(PropType val);
		PropType get();
		const std::string& getName() const;
		std::function<float(float)> getInterpolation() const;
	};

	class AnimationBase {
	public:
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
		int FPS = 30;
		int framesCount = 50;
		float time = 0.f;
		float curTime = 0.f;

		std::map<int, std::map<std::string, PropType>> frames;
		std::map<std::string, AnimationProperty> props;
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
		void init();
		float linearFunc(float x0, float x1, float progress);
		int linearFunc(int x0, int x1, float progress);
		bool linearFunc(bool x0, bool x1, float progress);
		void addProperty(AnimationProperty p);
		void addKeyFrame(int frame, std::map<std::string, PropType> prop);
		void delKeyFrameProp(int frame, std::map<std::string, PropType> prop);
		void addKeyFrameMerge(int frame, std::map<std::string, PropType> prop);
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
}
