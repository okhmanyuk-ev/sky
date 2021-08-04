#include "shadertoy.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
	const char* shaderSource =
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
		R"(
		layout (std140) uniform ConstantBuffer
		{
			mat4 uViewMatrix;
			mat4 uProjectionMatrix;
			mat4 uModelMatrix;

			vec3 iResolution;
			float iTime;
			vec4 iMouse;
		};

		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		out vec3 vPosition;
	
		void main()
		{
			vPosition = aPosition;
			gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec3 vPosition;
		
		out vec4 fragColor;

		void mainImage(out vec4 fragColor, in vec2 fragCoord);

		void main()
		{
			mainImage(fragColor, vPosition.xy);	
		}
		#endif
		)";
#endif
}

Shadertoy::Shadertoy(const Vertex::Layout& layout, const std::string& src) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position }, sizeof(CustomConstantBuffer), 
		shaderSource + src)
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}
