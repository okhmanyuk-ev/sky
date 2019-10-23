#pragma once

namespace Common::Easing
{
	float Linear(float p);

	float QuadraticIn(float p);
	float QuadraticOut(float p);
	float QuadraticInOut(float p);

	float CubicIn(float p);
	float CubicOut(float p);
	float CubicInOut(float p);

	float QuarticIn(float p);
	float QuarticOut(float p);
	float QuarticInOut(float p);
	
	float QuinticIn(float p);
	float QuinticOut(float p);
	float QuinticInOut(float p);

	float SinusoidalIn(float p);
	float SinusoidalOut(float p);
	float SinusoidalInOut(float p);

	float ExponentialIn(float p);
	float ExponentialOut(float p);
	float ExponentialInOut(float p);
	
	float CircularIn(float p);
	float CircularOut(float p);
	float CircularInOut(float p);

	float ElasticIn(float p);
	float ElasticOut(float p);
	float ElasticInOut(float p);

	float BackIn(float p);
	float BackOut(float p);
	float BackInOut(float p);
	
	float BounceIn(float p);
	float BounceOut(float p);
	float BounceInOut(float p);
}