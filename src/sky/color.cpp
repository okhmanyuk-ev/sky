#include "color.h"
#include <unordered_map>

template<>
glm::vec3 sky::GetColor<glm::vec3>(Color color)
{
	static const std::unordered_map<Color, glm::vec3> ColorMap = {
		{ Color::AliceBlue, { 0.941176f, 0.972549f, 1.000000f } },
		{ Color::AntiqueWhite, { 0.980392f, 0.921569f, 0.843137f } },
		{ Color::Aquamarine, { 0.498039f, 1.000000f, 0.831373f } },
		{ Color::Azure, { 0.941176f, 1.000000f, 1.000000f } },
		{ Color::Beige, { 0.960784f, 0.960784f, 0.862745f } },
		{ Color::Bisque, { 1.000000f, 0.894118f, 0.768627f } },
		{ Color::Black, { 0.000000f, 0.000000f, 0.000000f } },
		{ Color::BlanchedAlmond, { 1.000000f, 0.921569f, 0.803922f } },
		{ Color::Blue, { 0.000000f, 0.000000f, 1.000000f } },
		{ Color::BlueViolet, { 0.541176f, 0.168627f, 0.886275f } },
		{ Color::Brown, { 0.647059f, 0.164706f, 0.164706f } },
		{ Color::BurlyWood, { 0.870588f, 0.721569f, 0.529412f } },
		{ Color::CadetBlue, { 0.372549f, 0.619608f, 0.627451f } },
		{ Color::Chartreuse, { 0.498039f, 1.000000f, 0.000000f } },
		{ Color::Chocolate, { 0.823529f, 0.411765f, 0.117647f } },
		{ Color::Coral, { 1.000000f, 0.498039f, 0.313726f } },
		{ Color::CornflowerBlue, { 0.392157f, 0.584314f, 0.929412f } },
		{ Color::Cornsilk, { 1.000000f, 0.972549f, 0.862745f } },
		{ Color::Crimson, { 0.862745f, 0.078431f, 0.235294f } },
		{ Color::Cyan, { 0.000000f, 1.000000f, 1.000000f } },
		{ Color::DarkBlue, { 0.000000f, 0.000000f, 0.545098f } },
		{ Color::DarkCyan, { 0.000000f, 0.545098f, 0.545098f } },
		{ Color::DarkYellow, { 0.545098f, 0.545098f, 0.000000f } },
		{ Color::DarkGoldenRod, { 0.721569f, 0.525490f, 0.043137f } },
		{ Color::DarkGray, { 0.662745f, 0.662745f, 0.662745f } },
		{ Color::DarkGreen, { 0.000000f, 0.392157f, 0.000000f } },
		{ Color::DarkKhaki, { 0.741176f, 0.717647f, 0.419608f } },
		{ Color::DarkMagenta, { 0.545098f, 0.000000f, 0.545098f } },
		{ Color::DarkOliveGreen, { 0.333333f, 0.419608f, 0.184314f } },
		{ Color::DarkOrange, { 1.000000f, 0.549020f, 0.000000f } },
		{ Color::DarkOrchid, { 0.600000f, 0.196078f, 0.800000f } },
		{ Color::DarkRed, { 0.545098f, 0.000000f, 0.000000f } },
		{ Color::DarkSalmon, { 0.913725f, 0.588235f, 0.478431f } },
		{ Color::DarkSeaGreen, { 0.560784f, 0.737255f, 0.560784f } },
		{ Color::DarkSlateBlue, { 0.282353f, 0.239216f, 0.545098f } },
		{ Color::DarkSlateGray, { 0.184314f, 0.309804f, 0.309804f } },
		{ Color::DarkTurquoise, { 0.000000f, 0.807843f, 0.819608f } },
		{ Color::DarkViolet, { 0.580392f, 0.000000f, 0.827451f } },
		{ Color::DeepPink, { 1.000000f, 0.078431f, 0.576471f } },
		{ Color::DeepSkyBlue, { 0.000000f, 0.749020f, 1.000000f } },
		{ Color::DimGray, { 0.411765f, 0.411765f, 0.411765f } },
		{ Color::DodgerBlue, { 0.117647f, 0.564706f, 1.000000f } },
		{ Color::FireBrick, { 0.698039f, 0.133333f, 0.133333f } },
		{ Color::FloralWhite, { 1.000000f, 0.980392f, 0.941176f } },
		{ Color::ForestGreen, { 0.133333f, 0.545098f, 0.133333f } },
		{ Color::Gainsboro, { 0.862745f, 0.862745f, 0.862745f } },
		{ Color::GhostWhite, { 0.972549f, 0.972549f, 1.000000f } },
		{ Color::Gold, { 1.000000f, 0.843137f, 0.000000f } },
		{ Color::GoldenRod, { 0.854902f, 0.647059f, 0.125490f } },
		{ Color::Gray, { 0.501961f, 0.501961f, 0.501961f } },
		{ Color::Green, { 0.000000f, 0.501961f, 0.000000f } },
		{ Color::GreenYellow, { 0.678431f, 1.000000f, 0.184314f } },
		{ Color::HoneyDew, { 0.941176f, 1.000000f, 0.941176f } },
		{ Color::HotPink, { 1.000000f, 0.411765f, 0.705882f } },
		{ Color::IndianRed, { 0.803922f, 0.360784f, 0.360784f } },
		{ Color::Indigo, { 0.294118f, 0.000000f, 0.509804f } },
		{ Color::Ivory, { 1.000000f, 1.000000f, 0.941176f } },
		{ Color::Khaki, { 0.941176f, 0.901961f, 0.549020f } },
		{ Color::Lavender, { 0.901961f, 0.901961f, 0.980392f } },
		{ Color::LavenderBlush, { 1.000000f, 0.941176f, 0.960784f } },
		{ Color::LawnGreen, { 0.486275f, 0.988235f, 0.000000f } },
		{ Color::LemonChiffon, { 1.000000f, 0.980392f, 0.803922f } },
		{ Color::LightBlue, { 0.678431f, 0.847059f, 0.901961f } },
		{ Color::LightCoral, { 0.941176f, 0.501961f, 0.501961f } },
		{ Color::LightCyan, { 0.878431f, 1.000000f, 1.000000f } },
		{ Color::LightGoldenYellow, { 0.980392f, 0.980392f, 0.823529f } },
		{ Color::LightGray, { 0.827451f, 0.827451f, 0.827451f } },
		{ Color::LightGreen, { 0.564706f, 0.933333f, 0.564706f } },
		{ Color::LightPink, { 1.000000f, 0.713726f, 0.756863f } },
		{ Color::LightSalmon, { 1.000000f, 0.627451f, 0.478431f } },
		{ Color::LightSeaGreen, { 0.125490f, 0.698039f, 0.666667f } },
		{ Color::LightSkyBlue, { 0.529412f, 0.807843f, 0.980392f } },
		{ Color::LightSlateGray, { 0.466667f, 0.533333f, 0.600000f } },
		{ Color::LightSteelBlue, { 0.690196f, 0.768627f, 0.870588f } },
		{ Color::LightYellow, { 1.000000f, 1.000000f, 0.878431f } },
		{ Color::Lime, { 0.000000f, 1.000000f, 0.000000f } },
		{ Color::LimeGreen, { 0.196078f, 0.803922f, 0.196078f } },
		{ Color::Linen, { 0.980392f, 0.941176f, 0.901961f } },
		{ Color::Magenta, { 1.000000f, 0.000000f, 1.000000f } },
		{ Color::Maroon, { 0.501961f, 0.000000f, 0.000000f } },
		{ Color::MediumAquaMarine, { 0.400000f, 0.803922f, 0.666667f } },
		{ Color::MediumBlue, { 0.000000f, 0.000000f, 0.803922f } },
		{ Color::MediumOrchid, { 0.729412f, 0.333333f, 0.827451f } },
		{ Color::MediumPurple, { 0.576471f, 0.439216f, 0.858824f } },
		{ Color::MediumSeaGreen, { 0.235294f, 0.701961f, 0.443137f } },
		{ Color::MediumSlateBlue, { 0.482353f, 0.407843f, 0.933333f } },
		{ Color::MediumSpringGreen, { 0.000000f, 0.980392f, 0.603922f } },
		{ Color::MediumTurquoise, { 0.282353f, 0.819608f, 0.800000f } },
		{ Color::MediumVioletRed, { 0.780392f, 0.082353f, 0.521569f } },
		{ Color::MidnightBlue, { 0.098039f, 0.098039f, 0.439216f } },
		{ Color::MintCream, { 0.960784f, 1.000000f, 0.980392f } },
		{ Color::MistyRose, { 1.000000f, 0.894118f, 0.882353f } },
		{ Color::Moccasin, { 1.000000f, 0.894118f, 0.709804f } },
		{ Color::NavajoWhite, { 1.000000f, 0.870588f, 0.678431f } },
		{ Color::Navy, { 0.000000f, 0.000000f, 0.501961f } },
		{ Color::OldLace, { 0.992157f, 0.960784f, 0.901961f } },
		{ Color::Olive, { 0.501961f, 0.501961f, 0.000000f } },
		{ Color::OliveDrab, { 0.419608f, 0.556863f, 0.137255f } },
		{ Color::Orange, { 1.000000f, 0.647059f, 0.000000f } },
		{ Color::OrangeRed, { 1.000000f, 0.270588f, 0.000000f } },
		{ Color::Orchid, { 0.854902f, 0.439216f, 0.839216f } },
		{ Color::PaleGoldenRod, { 0.933333f, 0.909804f, 0.666667f } },
		{ Color::PaleGreen, { 0.596078f, 0.984314f, 0.596078f } },
		{ Color::PaleTurquoise, { 0.686275f, 0.933333f, 0.933333f } },
		{ Color::PaleVioletRed, { 0.858824f, 0.439216f, 0.576471f } },
		{ Color::PapayaWhip, { 1.000000f, 0.937255f, 0.835294f } },
		{ Color::PeachPuff, { 1.000000f, 0.854902f, 0.725490f } },
		{ Color::Peru, { 0.803922f, 0.521569f, 0.247059f } },
		{ Color::Pink, { 1.000000f, 0.752941f, 0.796078f } },
		{ Color::Plum, { 0.866667f, 0.627451f, 0.866667f } },
		{ Color::PowderBlue, { 0.690196f, 0.878431f, 0.901961f } },
		{ Color::Purple, { 0.501961f, 0.000000f, 0.501961f } },
		{ Color::RebeccaPurple, { 0.400000f, 0.200000f, 0.600000f } },
		{ Color::Red, { 1.000000f, 0.000000f, 0.000000f } },
		{ Color::RosyBrown, { 0.737255f, 0.560784f, 0.560784f } },
		{ Color::RoyalBlue, { 0.254902f, 0.411765f, 0.882353f } },
		{ Color::SaddleBrown, { 0.545098f, 0.270588f, 0.074510f } },
		{ Color::Salmon, { 0.980392f, 0.501961f, 0.447059f } },
		{ Color::SandyBrown, { 0.956863f, 0.643137f, 0.376471f } },
		{ Color::SeaGreen, { 0.180392f, 0.545098f, 0.341176f } },
		{ Color::SeaShell, { 1.000000f, 0.960784f, 0.933333f } },
		{ Color::Sienna, { 0.627451f, 0.321569f, 0.176471f } },
		{ Color::Silver, { 0.752941f, 0.752941f, 0.752941f } },
		{ Color::SkyBlue, { 0.529412f, 0.807843f, 0.921569f } },
		{ Color::SlateBlue, { 0.415686f, 0.352941f, 0.803922f } },
		{ Color::SlateGray, { 0.439216f, 0.501961f, 0.564706f } },
		{ Color::Snow, { 1.000000f, 0.980392f, 0.980392f } },
		{ Color::SpringGreen, { 0.000000f, 1.000000f, 0.498039f } },
		{ Color::SteelBlue, { 0.274510f, 0.509804f, 0.705882f } },
		{ Color::Tan, { 0.823529f, 0.705882f, 0.549020f } },
		{ Color::Teal, { 0.000000f, 0.501961f, 0.501961f } },
		{ Color::Thistle, { 0.847059f, 0.749020f, 0.847059f } },
		{ Color::Tomato, { 1.000000f, 0.388235f, 0.278431f } },
		{ Color::Turquoise, { 0.250980f, 0.878431f, 0.815686f } },
		{ Color::Violet, { 0.933333f, 0.509804f, 0.933333f } },
		{ Color::Wheat, { 0.960784f, 0.870588f, 0.701961f } },
		{ Color::White, { 1.000000f, 1.000000f, 1.000000f } },
		{ Color::WhiteSmoke, { 0.960784f, 0.960784f, 0.960784f } },
		{ Color::Yellow, { 1.000000f, 1.000000f, 0.000000f } },
		{ Color::YellowGreen, { 0.603922f, 0.803922f, 0.196078f } },
	};
	return ColorMap.at(color);
}

template<>
glm::vec4 sky::GetColor<glm::vec4>(Color color)
{
	return glm::vec4{ GetColor(color), 1.0f };
}

glm::vec4 sky::ColorToNormalized(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
}

glm::vec4 sky::ColorToNormalized(uint32_t value)
{
	auto r = ((uint8_t*)&value)[0];
	auto g = ((uint8_t*)&value)[1];
	auto b = ((uint8_t*)&value)[2];
	auto a = ((uint8_t*)&value)[3];
	return ColorToNormalized(r, g, b, a);
}

uint32_t sky::ColorToUInt32(const glm::vec4& color)
{
	uint32_t result;
	auto* bytes = (uint8_t*)&result;
	bytes[0] = static_cast<uint8_t>(color.r * 255.0f);
	bytes[1] = static_cast<uint8_t>(color.g * 255.0f);
	bytes[2] = static_cast<uint8_t>(color.b * 255.0f);
	bytes[3] = static_cast<uint8_t>(color.a * 255.0f);
	return result;
}

uint32_t sky::ColorToUInt32(const glm::vec3& color)
{
	return ColorToUInt32(glm::vec4{ color, 1.0f });
}
