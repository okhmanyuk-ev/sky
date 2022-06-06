#include <renderer/shader_custom.h>
#include <string>
#include <stdexcept>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
#include <renderer/system_gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <renderer/shader_compiler.h>

using namespace Renderer;

struct ShaderCross::Impl
{
	Vertex::Layout layout;
	GLuint program;
	GLuint vao;
	std::map<Vertex::Attribute::Type, GLint> attribLocations;
};

ShaderCross::ShaderCross(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code)
{
    mImpl = std::make_unique<Impl>();
	mImpl->layout = layout;

	std::vector<std::string> defines;
	AddLocationDefines(layout, defines);
	defines.push_back("FLIP_TEXCOORD_Y");

	auto vertex_shader_spirv = Renderer::CompileGlslToSpirv(Renderer::ShaderStage::Vertex, vertex_code, defines);
	auto fragment_shader_spirv = Renderer::CompileGlslToSpirv(Renderer::ShaderStage::Fragment, fragment_code, defines);

	auto glsl_vert = Renderer::CompileSpirvToGlsl(vertex_shader_spirv);
	auto glsl_frag = Renderer::CompileSpirvToGlsl(fragment_shader_spirv);

	auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
	auto v = glsl_vert.c_str();
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
		throw std::runtime_error(errorLog);
	}

	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	auto f = glsl_frag.c_str();
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
		throw std::runtime_error(errorLog);
	}

	mImpl->program = glCreateProgram();
	glAttachShader(mImpl->program, vertexShader);
	glAttachShader(mImpl->program, fragmentShader);
	glLinkProgram(mImpl->program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGenVertexArrays(1, &mImpl->vao);
	glBindVertexArray(mImpl->vao);

	int i = 0;

	for (auto& attrib : layout.attributes)
	{
		glEnableVertexAttribArray(i);
		mImpl->attribLocations[attrib.type] = i;
		i++;
	}
	glBindVertexArray(0);
}

ShaderCross::~ShaderCross()
{
	glDeleteVertexArrays(1, &mImpl->vao);
	glDeleteProgram(mImpl->program);
}

void ShaderCross::apply()
{
	glUseProgram(mImpl->program);
	glBindVertexArray(mImpl->vao);

	for (auto& attrib : mImpl->layout.attributes)
	{
		if (mImpl->attribLocations.count(attrib.type) == 0)
			continue;

		glVertexAttribPointer(mImpl->attribLocations.at(attrib.type), SystemGL::Size.at(attrib.format),
			SystemGL::Type.at(attrib.format), SystemGL::Normalize.at(attrib.format), (GLsizei)mImpl->layout.stride,
			(void*)attrib.offset);
	}
}

#endif
