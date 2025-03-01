#pragma once

#include <glm/glm.hpp>

namespace sky
{
	enum class Color
	{
		AliceBlue,
		AntiqueWhite,
		Aquamarine,
		Azure,
		Beige,
		Bisque,
		Black,
		BlanchedAlmond,
		Blue,
		BlueViolet,
		Brown,
		BurlyWood,
		CadetBlue,
		Chartreuse,
		Chocolate,
		Coral,
		CornflowerBlue,
		Cornsilk,
		Crimson,
		Cyan,
		DarkBlue,
		DarkCyan,
		DarkYellow,
		DarkGoldenRod,
		DarkGray,
		DarkGreen,
		DarkKhaki,
		DarkMagenta,
		DarkOliveGreen,
		DarkOrange,
		DarkOrchid,
		DarkRed,
		DarkSalmon,
		DarkSeaGreen,
		DarkSlateBlue,
		DarkSlateGray,
		DarkTurquoise,
		DarkViolet,
		DeepPink,
		DeepSkyBlue,
		DimGray,
		DodgerBlue,
		FireBrick,
		FloralWhite,
		ForestGreen,
		Gainsboro,
		GhostWhite,
		Gold,
		GoldenRod,
		Gray,
		Green,
		GreenYellow,
		HoneyDew,
		HotPink,
		IndianRed,
		Indigo,
		Ivory,
		Khaki,
		Lavender,
		LavenderBlush,
		LawnGreen,
		LemonChiffon,
		LightBlue,
		LightCoral,
		LightCyan,
		LightGoldenYellow,
		LightGray,
		LightGreen,
		LightPink,
		LightSalmon,
		LightSeaGreen,
		LightSkyBlue,
		LightSlateGray,
		LightSteelBlue,
		LightYellow,
		Lime,
		LimeGreen,
		Linen,
		Magenta,
		Maroon,
		MediumAquaMarine,
		MediumBlue,
		MediumOrchid,
		MediumPurple,
		MediumSeaGreen,
		MediumSlateBlue,
		MediumSpringGreen,
		MediumTurquoise,
		MediumVioletRed,
		MidnightBlue,
		MintCream,
		MistyRose,
		Moccasin,
		NavajoWhite,
		Navy,
		OldLace,
		Olive,
		OliveDrab,
		Orange,
		OrangeRed,
		Orchid,
		PaleGoldenRod,
		PaleGreen,
		PaleTurquoise,
		PaleVioletRed,
		PapayaWhip,
		PeachPuff,
		Peru,
		Pink,
		Plum,
		PowderBlue,
		Purple,
		RebeccaPurple,
		Red,
		RosyBrown,
		RoyalBlue,
		SaddleBrown,
		Salmon,
		SandyBrown,
		SeaGreen,
		SeaShell,
		Sienna,
		Silver,
		SkyBlue,
		SlateBlue,
		SlateGray,
		Snow,
		SpringGreen,
		SteelBlue,
		Tan,
		Teal,
		Thistle,
		Tomato,
		Turquoise,
		Violet,
		Wheat,
		White,
		WhiteSmoke,
		Yellow,
		YellowGreen,
	};

	template<typename T = glm::vec3>
	T GetColor(Color color);

	template<>
	glm::vec3 GetColor<glm::vec3>(Color color);

	template<>
	glm::vec4 GetColor<glm::vec4>(Color color);

	glm::vec4 ColorToNormalized(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
	glm::vec4 ColorToNormalized(uint32_t value);
	uint32_t ColorToUInt32(const glm::vec4& color);
	uint32_t ColorToUInt32(const glm::vec3& color);

	namespace HsvColors
	{
		const float HueRed = 0.0f;
		const float HueYellow = 60.0f;
		const float HueGreen = 120.0f;
		const float HueCyan = 180.0f;
		const float HueBlue = 240.0f;
		const float HuePink = 300.0f;
	}
}
