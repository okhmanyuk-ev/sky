#pragma once

namespace Renderer
{
	enum class Blend
	{
		One, // Each component of the color is multiplied by {1, 1, 1, 1}.
		Zero, // Each component of the color is multiplied by {0, 0, 0, 0}.
		SrcColor, // Each component of the color is multiplied by the source color. {Rs, Gs, Bs, As}, where Rs, Gs, Bs, As are color source values.		
		InvSrcColor, // Each component of the color is multiplied by the inverse of the source color. {1 - Rs, 1 - Gs, 1 - Bs, 1 - As}, where Rs, Gs, Bs, As are color source values.
		SrcAlpha, // Each component of the color is multiplied by the alpha value of the source. {As, As, As, As}, where As is the source alpha value.		
		InvSrcAlpha, // Each component of the color is multiplied by the inverse of the alpha value of the source. {1 - As, 1 - As, 1 - As, 1 - As}, where As is the source alpha value.
		DstColor, // Each component color is multiplied by the destination color. {Rd, Gd, Bd, Ad}, where Rd, Gd, Bd, Ad are color destination values.
		InvDstColor, // Each component of the color is multiplied by the inversed destination color. {1 - Rd, 1 - Gd, 1 - Bd, 1 - Ad}, where Rd, Gd, Bd, Ad are color destination values.
		DstAlpha, // Each component of the color is multiplied by the alpha value of the destination. {Ad, Ad, Ad, Ad}, where Ad is the destination alpha value.
		InvDstAlpha, // Each component of the color is multiplied by the inversed alpha value of the destination. {1 - Ad, 1 - Ad, 1 - Ad, 1 - Ad}, where Ad is the destination alpha value.
	//	BlendFactor, // Each component of the color is multiplied by a constant in the BlendFactor
	//	InverseBlendFactor, // Each component of the color is multiplied by a inversed constant in the BlendFactor
	//	SourceAlphaSaturation // Each component of the color is multiplied by either the alpha of the source color, or the inverse of the alpha of the source color, whichever is greater. {f, f, f, 1}, where f = min(As, 1 - As), where As is the source alpha value.
	};

	enum class BlendFunction
	{
		Add, // The function will adds destination to the source. (srcColor * srcBlend) + (destColor * destBlend)	
		Subtract, // The function will subtracts destination from source. (srcColor * srcBlend) - (destColor * destBlend)
		ReverseSubtract, // The function will subtracts source from destination. (destColor * destBlend) - (srcColor * srcBlend) 
		Min, // The function will extracts minimum of the source and destination. min((srcColor * srcBlend),(destColor * destBlend))
		Max // The function will extracts maximum of the source and destination. max((srcColor * srcBlend),(destColor * destBlend))
	};

	struct ColorMask
	{
		bool red = true;
		bool green = true;
		bool blue = true;
		bool alpha = true;
	};

	struct BlendMode
	{
		BlendMode(Blend srcColorBlend, Blend dstColorBlend, Blend srcAlphaBlend, Blend dstAlphaBlend)
		{
			colorSrcBlend = srcColorBlend;
			colorDstBlend = dstColorBlend;
			alphaSrcBlend = srcAlphaBlend;
			alphaDstBlend = dstAlphaBlend;
		}

		BlendMode(Blend srcBlend, Blend dstBlend) : BlendMode(srcBlend, dstBlend, srcBlend, dstBlend) { }

		BlendFunction colorBlendFunction = BlendFunction::Add;
		Blend colorSrcBlend;
		Blend colorDstBlend;

		BlendFunction alphaBlendFunction = BlendFunction::Add;
		Blend alphaSrcBlend;
		Blend alphaDstBlend;

		ColorMask colorMask;
	};

	inline bool operator==(const ColorMask& left, const ColorMask& right)
	{
		return
			left.red == right.red &&
			left.green == right.green &&
			left.blue == right.blue &&
			left.alpha == right.alpha;
	}

	inline bool operator!=(const ColorMask& left, const ColorMask& right)
	{
		return !(left == right);
	}

	inline bool operator==(const BlendMode& left, const BlendMode& right)
	{
		return
			left.colorBlendFunction == right.colorBlendFunction &&
			left.colorSrcBlend == right.colorSrcBlend &&
			left.colorDstBlend == right.colorDstBlend &&
			left.alphaBlendFunction == right.alphaBlendFunction &&
			left.alphaSrcBlend == right.alphaSrcBlend &&
			left.alphaDstBlend == right.alphaDstBlend &&
			left.colorMask == right.colorMask;
	}

	inline bool operator!=(const BlendMode& left, const BlendMode& right)
	{
		return !(left == right);
	}

	namespace BlendStates
	{
		inline const BlendMode Opaque = BlendMode(Blend::One, Blend::Zero);
		inline const BlendMode AlphaBlend = BlendMode(Blend::One, Blend::InvSrcAlpha);
		inline const BlendMode Additive = BlendMode(Blend::SrcAlpha, Blend::One);
		inline const BlendMode NonPremultiplied = BlendMode(Blend::SrcAlpha, Blend::InvSrcAlpha);
	}
}