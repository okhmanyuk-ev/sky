#include "shadertoy.h"

using namespace Renderer;
using namespace Renderer::Shaders;

/*namespace
{
	const char* shaderSource =
//#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
		R"(
		layout (std140) uniform ConstantBuffer
		{
			mat4 uViewMatrix;
			mat4 uProjectionMatrix;
			mat4 uModelMatrix;

			vec3 iResolution;
			float iTime;
			vec4 iMouse;
			int iFrame;
		};

		uniform sampler2D uTexture;

		#define iChannel0 uTexture
		#define iChannel1 uTexture
		#define iChannel2 uTexture
		#define iChannel3 uTexture

		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		out vec3 vPosition;

		#ifdef HAS_TEXCOORD_ATTRIB
		in vec2 aTexCoord;
		out vec2 vTexCoord;
		#endif	

		void main()
		{		
			vPosition = aPosition;
		#ifdef HAS_TEXCOORD_ATTRIB
			vTexCoord = aTexCoord;
		#endif
			gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec3 vPosition;

		#ifdef HAS_TEXCOORD_ATTRIB
		in vec2 vTexCoord;
		#endif	

		out vec4 fragColor;

		void mainImage(out vec4 fragColor, in vec2 fragCoord);

		void main()
		{
		#ifdef HAS_TEXCOORD_ATTRIB
			mainImage(fragColor, vTexCoord);	
		#else
			mainImage(fragColor, vPosition.xy);	
		#endif
		}
		#endif
		)";
//#endif
}

Shadertoy::Shadertoy(const Vertex::Layout& layout, const std::string& src, const std::set<Flag>& flags) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position }, sizeof(CustomConstantBuffer),
		MakeDefinesFromFlags(layout, shaderSource + src, flags))
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}

Shadertoy::Shadertoy(const Vertex::Layout& layout, const std::string& src) : Shadertoy(layout, src, MakeFlagsFromLayout(layout))
{
	//
};

std::set<Shadertoy::Flag> Shadertoy::MakeFlagsFromLayout(const Vertex::Layout& layout)
{
	std::set<Flag> result = { };

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		result.insert(Flag::Colored);

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		result.insert(Flag::Textured);

	return result;
}

std::string Shadertoy::MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags)
{
	auto result = source;

	if (flags.count(Flag::Colored) > 0)
	{
		assert(layout.hasAttribute(Renderer::Vertex::Attribute::Type::Color));
		result = "#define HAS_COLOR_ATTRIB\n" + result;
	}

	if (flags.count(Flag::Textured) > 0)
	{
		assert(layout.hasAttribute(Renderer::Vertex::Attribute::Type::TexCoord));
		result = "#define HAS_TEXCOORD_ATTRIB\n" + result;
	}

	return result;
}*/