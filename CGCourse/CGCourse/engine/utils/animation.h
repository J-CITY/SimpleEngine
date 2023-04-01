#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>


namespace KUMA::ANIMATION {
	namespace animation {
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
			{ InterpolationType::LINEAR, &animation::linearInterpolation },
			{ InterpolationType::QUADRATIC_IN, &animation::quadraticEaseIn },
			{ InterpolationType::QUADRATIC_OUT, &animation::quadraticEaseOut },
			{ InterpolationType::QUADRATIC_IN_OUT, &animation::quadraticEaseInOut },
			{ InterpolationType::CUBIC_IN, &animation::cubicEaseIn },
			{ InterpolationType::CUBIC_OUT, &animation::cubicEaseOut },
			{ InterpolationType::CUBIC_IN_OUT, &animation::cubicEaseInOut },
			{ InterpolationType::QUADRIC_IN, &animation::quarticEaseIn },
			{ InterpolationType::QUADRIC_OUT, &animation::quarticEaseOut },
			{ InterpolationType::QUADRIC_IN_OUT, &animation::quarticEaseInOut },
			{ InterpolationType::QUINTIC_IN, &animation::quinticEaseIn },
			{ InterpolationType::QUINTIC_OUT, &animation::quinticEaseOut },
			{ InterpolationType::QUINTIC_IN_OUT, &animation::quinticEaseInOut },
			{ InterpolationType::SINE_IN, &animation::sineEaseIn },
			{ InterpolationType::SINE_OUT, &animation::sineEaseOut },
			{ InterpolationType::SINE_IN_OUT, &animation::sineEaseInOut },
			{ InterpolationType::CIRCULAR_IN, &animation::circularEaseIn },
			{ InterpolationType::CIRCULAR_OUT, &animation::circularEaseOut },
			{ InterpolationType::CIRCULAR_IN_OUT, &animation::circularEaseInOut },
			{ InterpolationType::EXPONENTIAL_IN, &animation::exponentialEaseIn },
			{ InterpolationType::EXPONENTIAL_OUT, &animation::exponentialEaseOut },
			{ InterpolationType::EXPONENTIAL_IN_OUT, &animation::exponentialEaseInOut },
			{ InterpolationType::ELASTIC_IN, &animation::elasticEaseIn },
			{ InterpolationType::ELASTIC_OUT, &animation::elasticEaseOut },
			{ InterpolationType::ELASTIC_IN_OUT, &animation::elasticEaseInOut },
			{ InterpolationType::BACK_IN, &animation::backEaseIn },
			{ InterpolationType::BACK_OUT, &animation::backEaseOut },
			{ InterpolationType::BACK_IN_OUT, &animation::backEaseInOut },
			{ InterpolationType::BOUNCE_IN, &animation::bounceEaseOut },
			{ InterpolationType::BOUNCE_OUT, &animation::bounceEaseIn },
			{ InterpolationType::BOUNCE_IN_OUT, &animation::bounceEaseInOut }
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
			std::function<float(float)> interpolation = &animation::linearInterpolation);
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
