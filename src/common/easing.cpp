#include "easing.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/easing.hpp>

namespace Easing
{
	float Linear(float p) { return glm::linearInterpolation(p); }

	float QuadraticIn(float p) { return glm::quadraticEaseIn(p); }
	float QuadraticOut(float p) { return glm::quadraticEaseOut(p); }
	float QuadraticInOut(float p) { return glm::quadraticEaseInOut(p); }

	float CubicIn(float p) { return glm::cubicEaseIn(p); }
	float CubicOut(float p) { return glm::cubicEaseOut(p); }
	float CubicInOut(float p) { return glm::cubicEaseInOut(p); }

	float QuarticIn(float p) { return glm::quarticEaseIn(p); }
	float QuarticOut(float p) { return glm::quarticEaseOut(p); }
	float QuarticInOut(float p) { return glm::quarticEaseInOut(p); }

	float QuinticIn(float p) { return glm::quinticEaseIn(p); }
	float QuinticOut(float p) { return glm::quinticEaseOut(p); }
	float QuinticInOut(float p) { return glm::quinticEaseInOut(p); }
	
	float SinusoidalIn(float p) { return glm::sineEaseIn(p); }
	float SinusoidalOut(float p) { return glm::sineEaseOut(p); }
	float SinusoidalInOut(float p) { return glm::sineEaseInOut(p); }

	float ExponentialIn(float p) { return glm::exponentialEaseIn(p); }
	float ExponentialOut(float p) { return glm::exponentialEaseOut(p); }
	float ExponentialInOut(float p) { return glm::exponentialEaseInOut(p); }
	
	float CircularIn(float p) { return glm::circularEaseIn(p); }
	float CircularOut(float p) { return glm::circularEaseOut(p); }
	float CircularInOut(float p) { return glm::circularEaseInOut(p); }

	float ElasticIn(float p) { return glm::elasticEaseIn(p); }
	float ElasticOut(float p) { return glm::elasticEaseOut(p); }
	float ElasticInOut(float p) { return glm::elasticEaseInOut(p); }

	float BackIn(float p) { return glm::backEaseIn(p); }
	float BackOut(float p) { return glm::backEaseOut(p); }
	float BackInOut(float p) { return glm::backEaseInOut(p); }

	float BounceIn(float p) { return glm::bounceEaseIn(p); }
	float BounceOut(float p) { return glm::bounceEaseOut(p); }
	float BounceInOut(float p) { return glm::bounceEaseInOut(p); }
}