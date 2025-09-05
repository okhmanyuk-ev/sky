#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/circle.h>
#include <scene/scrollbox.h>
#include <scene/glass.h>
#include <sky/actions.h>
#include <common/easing.h>

namespace Scene
{
	template <typename ObjectType, auto Getter, auto Setter>
	struct Property
	{
		using Type = std::invoke_result_t<decltype(Getter), ObjectType*>;
		using Object = std::shared_ptr<ObjectType>;
		static auto GetValue(Object obj) { return std::invoke(Getter, obj.get()); }
		static void SetValue(Object obj, auto value) { std::invoke(Setter, obj.get(), std::move(value)); }
	};

	using ColorProperty = Property<Color, &Color::getColor, static_cast<void (Color::*)(const glm::vec3&)>(&Color::setColor)>;
	using ColorRgbProperty = Property<Color, &Color::getRGB, &Color::setRGB>;
	using AlphaProperty = Property<Color, &Color::getAlpha, &Color::setAlpha>;

	using AnchorProperty = Property<Transform, &Transform::getAnchor, static_cast<void (Transform::*)(const glm::vec2&)>(&Transform::setAnchor)>;
	using HorizontalAnchorProperty = Property<Transform, &Transform::getHorizontalAnchor, &Transform::setHorizontalAnchor>;
	using VerticalAnchorProperty = Property<Transform, &Transform::getVerticalAnchor, &Transform::setVerticalAnchor>;

	using PivotProperty = Property<Transform, &Transform::getPivot, static_cast<void (Transform::*)(const glm::vec2&)>(&Transform::setPivot)>;
	using HorizontalPivotProperty = Property<Transform, &Transform::getHorizontalPivot, &Transform::setHorizontalPivot>;
	using VerticalPivotProperty = Property<Transform, &Transform::getVerticalPivot, &Transform::setVerticalPivot>;

	using PositionProperty = Property<Transform, &Transform::getPosition, &Transform::setPosition>;
	using HorizontalPositionProperty = Property<Transform, &Transform::getHorizontalPosition, &Transform::setHorizontalPosition>;
	using VerticalPositionProperty = Property<Transform, &Transform::getVerticalPosition, &Transform::setVerticalPosition>;

	using OriginProperty = Property<Transform, &Transform::getOrigin, static_cast<void (Transform::*)(const glm::vec2&)>(&Transform::setOrigin)>;
	using HorizontalOriginProperty = Property<Transform, &Transform::getHorizontalOrigin, &Transform::setHorizontalOrigin>;
	using VerticalOriginProperty = Property<Transform, &Transform::getVerticalOrigin, &Transform::setVerticalOrigin>;

	using SizeProperty = Property<Transform, &Transform::getSize, static_cast<void (Transform::*)(const glm::vec2&)>(&Transform::setSize)>;
	using HorizontalSizeProperty = Property<Transform, &Transform::getHorizontalSize, &Transform::setHorizontalSize>;
	using VerticalSizeProperty = Property<Transform, &Transform::getVerticalSize, &Transform::setVerticalSize>;

	using StretchProperty = Property<Transform, &Transform::getStretch, static_cast<void (Transform::*)(const glm::vec2&)>(&Transform::setStretch)>;
	using HorizontalStretchProperty = Property<Transform, &Transform::getHorizontalStretch, &Transform::setHorizontalStretch>;
	using VerticalStretchProperty = Property<Transform, &Transform::getVerticalStretch, &Transform::setVerticalStretch>;

	using ScaleProperty = Property<Transform, &Transform::getScale, static_cast<void (Transform::*)(const glm::vec2&)>(&Transform::setScale)>;
	using HorizontalScaleProperty = Property<Transform, &Transform::getHorizontalScale, &Transform::setHorizontalScale>;
	using VerticalScaleProperty = Property<Transform, &Transform::getVerticalScale, &Transform::setVerticalScale>;

	using RotationProperty = Property<Transform, &Transform::getRotation, &Transform::setRotation>;
	using RadialAnchorProperty = Property<Transform, &Transform::getRadialAnchor, &Transform::setRadialAnchor>;
	using RadialPivotProperty = Property<Transform, &Transform::getRadialPivot, &Transform::setRadialPivot>;

	using CirclePieProperty = Property<Circle, &Circle::getPie, &Circle::setPie>;
	using CirclePiePivotProperty = Property<Circle, &Circle::getPiePivot, &Circle::setPiePivot>;
	using CircleRadiusProperty = Property<Circle, &Circle::getRadius, &Circle::setRadius>;
	using CircleFillProperty = Property<Circle, &Circle::getFill, &Circle::setFill>;

	using ScrollPositionProperty = Property<Scrollbox, &Scrollbox::getScrollPosition, static_cast<void (Scrollbox::*)(const glm::vec2&)>(&Scrollbox::setScrollPosition)>;
	using HorizontalScrollPositionProperty = Property<Scrollbox, &Scrollbox::getHorizontalScrollPosition, &Scrollbox::setHorizontalScrollPosition>;
	using VerticalScrollPositionProperty = Property<Scrollbox, &Scrollbox::getVerticalScrollPosition, &Scrollbox::setVerticalScrollPosition>;

	using BlurIntensityProperty = Property<BlurredGlass, &BlurredGlass::getBlurIntensity, &BlurredGlass::setBlurIntensity>;
	using GrayscaleIntensityProperty = Property<GrayscaledGlass, &GrayscaledGlass::getGrayscaleIntensity, &GrayscaledGlass::setGrayscaleIntensity>;
}

namespace sky::Actions
{
	template<typename T>
	struct InterpolateWrapper
	{
		auto operator()(typename T::Object object, const typename T::Type& start, const typename T::Type& dest,
			float duration, EasingFunction easing = Easing::Linear) const
		{
			return Interpolate<T>(object, start, dest, duration, easing);
		}

		auto operator()(typename T::Object object, const typename T::Type& dest,
			float duration, EasingFunction easing = Easing::Linear) const
		{
			return Interpolate<T>(object, dest, duration, easing);
		}
	};

	constexpr InterpolateWrapper<Scene::ColorProperty> ChangeColor;
	constexpr InterpolateWrapper<Scene::ColorRgbProperty> ChangeColorRgb;
	constexpr InterpolateWrapper<Scene::AlphaProperty> ChangeAlpha;

	constexpr InterpolateWrapper<Scene::HorizontalAnchorProperty> ChangeHorizontalAnchor;
	constexpr InterpolateWrapper<Scene::VerticalAnchorProperty> ChangeVerticalAnchor;
	constexpr InterpolateWrapper<Scene::AnchorProperty> ChangeAnchor;

	constexpr InterpolateWrapper<Scene::HorizontalPivotProperty> ChangeHorizontalPivot;
	constexpr InterpolateWrapper<Scene::VerticalPivotProperty> ChangeVerticalPivot;
	constexpr InterpolateWrapper<Scene::PivotProperty> ChangePivot;

	constexpr InterpolateWrapper<Scene::HorizontalPositionProperty> ChangeHorizontalPosition;
	constexpr InterpolateWrapper<Scene::VerticalPositionProperty> ChangeVerticalPosition;
	constexpr InterpolateWrapper<Scene::PositionProperty> ChangePosition;

	constexpr InterpolateWrapper<Scene::HorizontalOriginProperty> ChangeHorizontalOrigin;
	constexpr InterpolateWrapper<Scene::VerticalOriginProperty> ChangeVerticalOrigin;
	constexpr InterpolateWrapper<Scene::OriginProperty> ChangeOrigin;

	constexpr InterpolateWrapper<Scene::HorizontalSizeProperty> ChangeHorizontalSize;
	constexpr InterpolateWrapper<Scene::VerticalSizeProperty> ChangeVerticalSize;
	constexpr InterpolateWrapper<Scene::SizeProperty> ChangeSize;

	constexpr InterpolateWrapper<Scene::HorizontalStretchProperty> ChangeHorizontalStretch;
	constexpr InterpolateWrapper<Scene::VerticalStretchProperty> ChangeVerticalStretch;
	constexpr InterpolateWrapper<Scene::StretchProperty> ChangeStretch;

	constexpr InterpolateWrapper<Scene::HorizontalScaleProperty> ChangeHorizontalScale;
	constexpr InterpolateWrapper<Scene::VerticalScaleProperty> ChangeVerticalScale;
	constexpr InterpolateWrapper<Scene::ScaleProperty> ChangeScale;

	constexpr InterpolateWrapper<Scene::RotationProperty> ChangeRotation;
	constexpr InterpolateWrapper<Scene::RadialAnchorProperty> ChangeRadialAnchor;
	constexpr InterpolateWrapper<Scene::RadialPivotProperty> ChangeRadialPivot;

	constexpr InterpolateWrapper<Scene::CirclePieProperty> ChangeCirclePie;
	constexpr InterpolateWrapper<Scene::CirclePiePivotProperty> ChangeCirclePiePivot;
	constexpr InterpolateWrapper<Scene::CircleRadiusProperty> ChangeCircleRadius;
	constexpr InterpolateWrapper<Scene::CircleFillProperty> ChangeCircleFill;

	constexpr InterpolateWrapper<Scene::HorizontalScrollPositionProperty> ChangeHorizontalScrollPosition;
	constexpr InterpolateWrapper<Scene::VerticalScrollPositionProperty> ChangeVerticalScrollPosition;
	constexpr InterpolateWrapper<Scene::ScrollPositionProperty> ChangeScrollPosition;

	constexpr InterpolateWrapper<Scene::BlurIntensityProperty> ChangeBlurIntensity;
	constexpr InterpolateWrapper<Scene::GrayscaleIntensityProperty> ChangeGrayscaleIntensity;

	Action ChangePositionByDirection(std::shared_ptr<Scene::Transform> node,
		const glm::vec2& direction, float speed);
	Action ChangePositionByDirection(std::shared_ptr<Scene::Transform> node,
		const glm::vec2& direction, float speed, float duration);

	Action ChangeColorRecursive(std::shared_ptr<Scene::Node> node, const glm::vec4& start,
		const glm::vec4& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	Action Hide(std::shared_ptr<Scene::Color> node, float duration,
		EasingFunction easingFunction = Easing::Linear);
	Action Show(std::shared_ptr<Scene::Color> node, float duration,
		EasingFunction easingFunction = Easing::Linear);

	Action HideRecursive(std::shared_ptr<Scene::Node> node, float duration,
		EasingFunction easingFunction = Easing::Linear);

	template<Property T = Scene::OriginProperty>
		requires std::same_as<typename T::Type, glm::vec2>
	Action Shake(typename T::Object object, float radius, float duration, glm::vec2 base_value = { 0.0f, 0.0f })
	{
		return Sequence(
			Interpolate(1.0f, 0.0f, duration, Easing::Linear, [object, radius, base_value](float value) {
				auto power = radius * value;
				T::SetValue(object, base_value + (power != 0.0f ? glm::circularRand(power) : glm::vec2{ 0.0f, 0.0f }));
			}),
			Execute([object, base_value] {
				T::SetValue(object, base_value);
			})
		);
	}

	Action Kill(std::shared_ptr<Scene::Node> node);
}