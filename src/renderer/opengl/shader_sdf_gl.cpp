#include <Renderer/shader_sdf.h>
#include <string>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
#include <Renderer/system_gl.h>

using namespace Renderer;

struct ShaderSDF::Impl
{
	Vertex::Layout layout;
	GLuint program;
	GLuint vao;
	GLuint ubo;
	GLuint uniformBlock;
	GLint uniformTexture;
	GLint uniformRenderTargetBound;
	std::map<Vertex::Attribute::Type, GLint> attribLocations;
};

namespace
{
	const char* shaderSource = R"(
		layout (std140) uniform ConstantBuffer
		{
			mat4 uViewMatrix;
			mat4 uProjectionMatrix;
			mat4 uModelMatrix;
			vec4 uColor;
			float uMinValue;
			float uMaxValue;
			float uSmoothFactor;
		};
	
		uniform sampler2D uTexture;
		uniform bool uRenderTargetBound;
		
		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		in vec2 aTexCoord;

		out vec2 vTexCoord;

		void main()
		{
			vTexCoord = aTexCoord;
			gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
			
			if (uRenderTargetBound)
				gl_Position.y *= -1.0;
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec2 vTexCoord;

		out vec4 fragColor;

		void main()
		{
			float distance = texture(uTexture, vTexCoord).a;
			float minAlpha = smoothstep(uMinValue - uSmoothFactor, uMinValue + uSmoothFactor, distance);
			float maxAlpha = smoothstep(uMaxValue + uSmoothFactor, uMaxValue - uSmoothFactor, distance);
			vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
			if (maxAlpha > 0.0 && minAlpha > 0.0)
			{
				if (maxAlpha > 0.0)
				{
					result = vec4(uColor.rgb, uColor.a * maxAlpha);
				}
				if (minAlpha > 0.0 && minAlpha < maxAlpha)
				{
					result = vec4(uColor.rgb, uColor.a * minAlpha);
				}
			}
			fragColor = result;
		}
		#endif)";
}

ShaderSDF::ShaderSDF(const Vertex::Layout& layout)
{
    mImpl = std::make_unique<Impl>();
	mImpl->layout = layout;
	checkRequiredAttribs(RequiredAttribs, layout);

	auto source = std::string(shaderSource);

	auto vertexSource = "#define VERTEX_SHADER\n" + source;
	auto fragmentSource = "#define FRAGMENT_SHADER\n" + source;

#if defined(RENDERER_GL44)
	vertexSource = "#version 440 core\n" + vertexSource;
	fragmentSource = "#version 440 core\n" + fragmentSource;
#elif defined(RENDERER_GLES3)
	vertexSource = "precision highp float;\n" + vertexSource;
	fragmentSource = "precision highp float;\n" + fragmentSource;
	vertexSource = "#version 300 es\n" + vertexSource;
	fragmentSource = "#version 300 es\n" + fragmentSource;
#endif

	auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
	auto v = vertexSource.c_str();
	glShaderSource(vertexShader, 1, &v, NULL);
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
		std::string errorLog;
		errorLog.resize(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);
		assert(false);
	}

	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	auto f = fragmentSource.c_str();
	glShaderSource(fragmentShader, 1, &f, NULL);
	glCompileShader(fragmentShader);

	isCompiled = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
		std::string errorLog;
		errorLog.resize(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errorLog[0]);
		assert(false);
	}

	mImpl->program = glCreateProgram();
	glAttachShader(mImpl->program, vertexShader);
	glAttachShader(mImpl->program, fragmentShader);
	glLinkProgram(mImpl->program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	glGenBuffers(1, &mImpl->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, mImpl->ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), nullptr, GL_STATIC_DRAW);

	mImpl->uniformBlock = glGetUniformBlockIndex(mImpl->program, "ConstantBuffer");
	glUniformBlockBinding(mImpl->program, mImpl->uniformBlock, 0);

	mImpl->uniformTexture = glGetUniformLocation(mImpl->program, "uTexture");
	mImpl->uniformRenderTargetBound = glGetUniformLocation(mImpl->program, "uRenderTargetBound");
	
	static const std::map<Vertex::Attribute::Type, std::string> attribName = {
		{ Vertex::Attribute::Type::Position, "aPosition" },
		{ Vertex::Attribute::Type::TexCoord, "aTexCoord" }
	};

	glGenVertexArrays(1, &mImpl->vao);
	glBindVertexArray(mImpl->vao);

	for (auto& attrib : layout.attributes)
	{
		if (attribName.count(attrib.type) == 0)
			continue;

		auto location = glGetAttribLocation(mImpl->program, attribName.at(attrib.type).c_str());
		//assert(location != -1);
		
		if (location == -1)
			continue;

		glEnableVertexAttribArray(location);
		mImpl->attribLocations[attrib.type] = location;
	}
}

ShaderSDF::~ShaderSDF()
{
	glDeleteVertexArrays(1, &mImpl->vao);
	glDeleteProgram(mImpl->program);
	glDeleteBuffers(1, &mImpl->ubo);
}

void ShaderSDF::apply()
{
	glUseProgram(mImpl->program);
	glBindVertexArray(mImpl->vao);
	glBindBuffer(GL_UNIFORM_BUFFER, mImpl->ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mImpl->ubo);

	for (auto& attrib : mImpl->layout.attributes)
	{
		if (mImpl->attribLocations.count(attrib.type) == 0)
			continue;

		glVertexAttribPointer(mImpl->attribLocations.at(attrib.type), SystemGL::Size.at(attrib.format),
			SystemGL::Type.at(attrib.format), SystemGL::Normalize.at(attrib.format), (GLsizei)mImpl->layout.stride,
			(void*)attrib.offset);
	}

	mNeedUpdate = true;
}

void ShaderSDF::update()
{
	glUniform1ui(mImpl->uniformRenderTargetBound, (GLint)SystemGL::IsRenderTargetBound());

	if (!mNeedUpdate)
		return;

	mNeedUpdate = false;
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), &mConstantBufferData, GL_STATIC_DRAW);
	glUniform1i(mImpl->uniformTexture, 0);
}
#endif
