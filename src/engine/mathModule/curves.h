#pragma once
#include "math.h"
#include <functional>
#include <map>
#include <vector>

namespace IKIGAI::MATH::CURVES {
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
inline std::map<InterpolationType, std::function<float(float)>> functions = { { InterpolationType::LINEAR, &FUNC::linearInterpolation },
	{ InterpolationType::QUADRATIC_IN, &FUNC::quadraticEaseIn }, { InterpolationType::QUADRATIC_OUT, &FUNC::quadraticEaseOut },
	{ InterpolationType::QUADRATIC_IN_OUT, &FUNC::quadraticEaseInOut }, { InterpolationType::CUBIC_IN, &FUNC::cubicEaseIn },
	{ InterpolationType::CUBIC_OUT, &FUNC::cubicEaseOut }, { InterpolationType::CUBIC_IN_OUT, &FUNC::cubicEaseInOut },
	{ InterpolationType::QUADRIC_IN, &FUNC::quarticEaseIn }, { InterpolationType::QUADRIC_OUT, &FUNC::quarticEaseOut },
	{ InterpolationType::QUADRIC_IN_OUT, &FUNC::quarticEaseInOut }, { InterpolationType::QUINTIC_IN, &FUNC::quinticEaseIn },
	{ InterpolationType::QUINTIC_OUT, &FUNC::quinticEaseOut }, { InterpolationType::QUINTIC_IN_OUT, &FUNC::quinticEaseInOut },
	{ InterpolationType::SINE_IN, &FUNC::sineEaseIn }, { InterpolationType::SINE_OUT, &FUNC::sineEaseOut },
	{ InterpolationType::SINE_IN_OUT, &FUNC::sineEaseInOut }, { InterpolationType::CIRCULAR_IN, &FUNC::circularEaseIn },
	{ InterpolationType::CIRCULAR_OUT, &FUNC::circularEaseOut }, { InterpolationType::CIRCULAR_IN_OUT, &FUNC::circularEaseInOut },
	{ InterpolationType::EXPONENTIAL_IN, &FUNC::exponentialEaseIn }, { InterpolationType::EXPONENTIAL_OUT, &FUNC::exponentialEaseOut },
	{ InterpolationType::EXPONENTIAL_IN_OUT, &FUNC::exponentialEaseInOut }, { InterpolationType::ELASTIC_IN, &FUNC::elasticEaseIn },
	{ InterpolationType::ELASTIC_OUT, &FUNC::elasticEaseOut }, { InterpolationType::ELASTIC_IN_OUT, &FUNC::elasticEaseInOut },
	{ InterpolationType::BACK_IN, &FUNC::backEaseIn }, { InterpolationType::BACK_OUT, &FUNC::backEaseOut },
	{ InterpolationType::BACK_IN_OUT, &FUNC::backEaseInOut }, { InterpolationType::BOUNCE_IN, &FUNC::bounceEaseOut },
	{ InterpolationType::BOUNCE_OUT, &FUNC::bounceEaseIn }, { InterpolationType::BOUNCE_IN_OUT, &FUNC::bounceEaseInOut } };

// TODO: move to own file
struct Curve {
	IKIGAI::MATH::Vector2f mLeft;
	IKIGAI::MATH::Vector2f mRight;
	IKIGAI::MATH::Vector2f mLeftTangent;
	IKIGAI::MATH::Vector2f mRightTangent;

	IKIGAI::MATH::Vector2f mLeftYOffset;

	template <class Context> constexpr static auto serde(Context& context, Curve& value) {
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

// Parametric evaluation preserves the historical animation easing semantics.
Vector2f evaluateParametric(const Curve& curve, float u);
// The X control points must be ordered between the endpoints.
float evaluateAtTime(const Curve& curve, float time);

enum class SegmentInterpolation { Step, Linear, Bezier, Easing };
struct FloatKey {
	float time = 0, value = 0;
	SegmentInterpolation interpolation = SegmentInterpolation::Linear;
	// Normalized control points for the outgoing segment.
	Vector2f outControl { 1.f / 3, 1.f / 3 }, inControl { 2.f / 3, 2.f / 3 };
	InterpolationType easing = InterpolationType::LINEAR;
};
class FloatCurve {
public:
	std::vector<FloatKey> keys;
	void validate() const;
	float evaluate(float time) const;
};
}
