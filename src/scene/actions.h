#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/circle.h>
#include <scene/scrollbox.h>
#include <scene/glass.h>
#include <common/actions.h>
#include <common/easing.h>

namespace Scene
{
	template <typename ObjectType, typename ValueType, typename GetterValueType, typename SetterValueType,
		GetterValueType(ObjectType::* Getter)() const, void (ObjectType::* Setter)(SetterValueType)>
	struct Property
	{
		using Type = ValueType;
		using Object = std::shared_ptr<ObjectType>;
		static Type GetValue(Object obj) { return static_cast<Type>((obj.get()->*Getter)()); }
		static void SetValue(Object obj, SetterValueType value) { (obj.get()->*Setter)(value); }
	};

	using ColorProperty = Property<Color, glm::vec3, glm::vec4, const glm::vec3&, &Color::getColor, &Color::setColor>;
	using AlphaProperty = Property<Color, float, float, float, &Color::getAlpha, &Color::setAlpha>;

	using AnchorProperty = Property<Transform, glm::vec2, glm::vec2, const glm::vec2&, &Transform::getAnchor, &Transform::setAnchor>;
	using HorizontalAnchorProperty = Property<Transform, float, float, float, &Transform::getHorizontalAnchor, &Transform::setHorizontalAnchor>;
	using VerticalAnchorProperty = Property<Transform, float, float, float, &Transform::getVerticalAnchor, &Transform::setVerticalAnchor>;

	using PivotProperty = Property<Transform, glm::vec2, glm::vec2, const glm::vec2&, &Transform::getPivot, &Transform::setPivot>;
	using HorizontalPivotProperty = Property<Transform, float, float, float, &Transform::getHorizontalPivot, &Transform::setHorizontalPivot>;
	using VerticalPivotProperty = Property<Transform, float, float, float, &Transform::getVerticalPivot, &Transform::setVerticalPivot>;

	using PositionProperty = Property<Transform, glm::vec2, glm::vec2, const glm::vec2&, &Transform::getPosition, &Transform::setPosition>;
	using HorizontalPositionProperty = Property<Transform, float, float, float, &Transform::getHorizontalPosition, &Transform::setHorizontalPosition>;
	using VerticalPositionProperty = Property<Transform, float, float, float, &Transform::getVerticalPosition, &Transform::setVerticalPosition>;

	using OriginProperty = Property<Transform, glm::vec2, glm::vec2, const glm::vec2&, &Transform::getOrigin, &Transform::setOrigin>;
	using HorizontalOriginProperty = Property<Transform, float, float, float, &Transform::getHorizontalOrigin, &Transform::setHorizontalOrigin>;
	using VerticalOriginProperty = Property<Transform, float, float, float, &Transform::getVerticalOrigin, &Transform::setVerticalOrigin>;

	using SizeProperty = Property<Transform, glm::vec2, glm::vec2, const glm::vec2&, &Transform::getSize, &Transform::setSize>;
	using HorizontalSizeProperty = Property<Transform, float, float, float, &Transform::getHorizontalSize, &Transform::setHorizontalSize>;
	using VerticalSizeProperty = Property<Transform, float, float, float, &Transform::getVerticalSize, &Transform::setVerticalSize>;

	using StretchProperty = Property<Transform, glm::vec2, glm::vec2, const glm::vec2&, &Transform::getStretch, &Transform::setStretch>;
	using HorizontalStretchProperty = Property<Transform, float, float, float, &Transform::getHorizontalStretch, &Transform::setHorizontalStretch>;
	using VerticalStretchProperty = Property<Transform, float, float, float, &Transform::getVerticalStretch, &Transform::setVerticalStretch>;

	using ScaleProperty = Property<Transform, glm::vec2, glm::vec2, const glm::vec2&, &Transform::getScale, &Transform::setScale>;
	using HorizontalScaleProperty = Property<Transform, float, float, float, &Transform::getHorizontalScale, &Transform::setHorizontalScale>;
	using VerticalScaleProperty = Property<Transform, float, float, float, &Transform::getVerticalScale, &Transform::setVerticalScale>;

	using RotationProperty = Property<Transform, float, float, float, &Transform::getRotation, &Transform::setRotation>;
	using RadialAnchorProperty = Property<Transform, float, float, float, &Transform::getRadialAnchor, &Transform::setRadialAnchor>;
	using RadialPivotProperty = Property<Transform, float, float, float, &Transform::getRadialPivot, &Transform::setRadialPivot>;

	using CirclePieProperty = Property<Circle, float, float, float, &Circle::getPie, &Circle::setPie>;
	using CirclePiePivotProperty = Property<Circle, float, float, float, &Circle::getPiePivot, &Circle::setPiePivot>;
	using CircleRadiusProperty = Property<Circle, float, float, float, &Circle::getRadius, &Circle::setRadius>;
	using CircleFillProperty = Property<Circle, float, float, float, &Circle::getFill, &Circle::setFill>;

	using ScrollPositionProperty = Property<Scrollbox, glm::vec2, glm::vec2, const glm::vec2&, &Scrollbox::getScrollPosition, &Scrollbox::setScrollPosition>;
	using HorizontalScrollPositionProperty = Property<Scrollbox, float, float, float, &Scrollbox::getHorizontalScrollPosition, &Scrollbox::setHorizontalScrollPosition>;
	using VerticalScrollPositionProperty = Property<Scrollbox, float, float, float, &Scrollbox::getVerticalScrollPosition, &Scrollbox::setVerticalScrollPosition>;

	using BlurIntensityProperty = Property<BlurredGlass, float, float, float, &BlurredGlass::getBlurIntensity, &BlurredGlass::setBlurIntensity>;
	using GrayscaleIntensityProperty = Property<GrayscaledGlass, float, float, float, &GrayscaledGlass::getGrayscaleIntensity, &GrayscaledGlass::setGrayscaleIntensity>;
}

namespace Actions::Collection
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

	std::unique_ptr<Action> ChangePositionByDirection(std::shared_ptr<Scene::Transform> node,
		const glm::vec2& direction, float speed);
	std::unique_ptr<Action> ChangePositionByDirection(std::shared_ptr<Scene::Transform> node,
		const glm::vec2& direction, float speed, float duration);

	std::unique_ptr<Action> ChangeColorRecursive(std::shared_ptr<Scene::Node> node, const glm::vec4& start,
		const glm::vec4& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> Hide(std::shared_ptr<Scene::Color> node, float duration,
		EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> Show(std::shared_ptr<Scene::Color> node, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> HideRecursive(std::shared_ptr<Scene::Node> node, float duration,
		EasingFunction easingFunction = Easing::Linear);

	template<Property T = Scene::OriginProperty>
		requires std::same_as<typename T::Type, glm::vec2>
	std::unique_ptr<Action> Shake(typename T::Object object, float radius, float duration, glm::vec2 base_value = { 0.0f, 0.0f })
	{
		return MakeSequence(
			Interpolate(1.0f, 0.0f, duration, Easing::Linear, [object, radius, base_value](float value) {
				auto power = radius * value;
				T::SetValue(object, base_value + (power != 0.0f ? glm::circularRand(power) : glm::vec2{ 0.0f, 0.0f }));
			}),
			Execute([object, base_value] {
				T::SetValue(object, base_value);
			})
		);
	}

	std::unique_ptr<Action> Kill(std::shared_ptr<Scene::Node> node);
}