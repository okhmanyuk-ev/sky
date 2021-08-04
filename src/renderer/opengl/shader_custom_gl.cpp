#include <renderer/shader_custom.h>
#include <string>
#include <stdexcept>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
#include <renderer/system_gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace Renderer;

struct ShaderCustom::Impl
{
	Vertex::Layout layout;
	GLuint program;
	GLuint vao;
	GLuint ubo;
	GLuint uniformBlock;
	GLint uniformTexture;
	std::map<Vertex::Attribute::Type, GLint> attribLocations;
	ConstantBuffer constantBuffer;
	size_t customConstantBufferSize;

	void* appliedConstantBuffer = nullptr;
	void* appliedCustomConstantBuffer = nullptr;
	bool forceDirty = false;
};

ShaderCustom::ShaderCustom(const Vertex::Layout& layout, const std::set<Vertex::Attribute::Type>& requiredAttribs, 
	size_t customConstantBufferSize, const std::string& source)
{
    mImpl = std::make_unique<Impl>();
	mImpl->layout = layout;
	mImpl->customConstantBufferSize = customConstantBufferSize;

	checkRequiredAttribs(requiredAttribs, layout); 
	
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
		throw std::runtime_error(errorLog);
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
		throw std::runtime_error(errorLog);
	}

	mImpl->program = glCreateProgram();
	glAttachShader(mImpl->program, vertexShader);
	glAttachShader(mImpl->program, fragmentShader);
	glLinkProgram(mImpl->program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGenBuffers(1, &mImpl->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, mImpl->ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer) + customConstantBufferSize, nullptr, GL_DYNAMIC_DRAW);

	mImpl->uniformBlock = glGetUniformBlockIndex(mImpl->program, "ConstantBuffer");
	glUniformBlockBinding(mImpl->program, mImpl->uniformBlock, 0);

	mImpl->uniformTexture = glGetUniformLocation(mImpl->program, "uTexture");

	static const std::unordered_map<Vertex::Attribute::Type, std::string> AttribNameMap = {
		{ Vertex::Attribute::Type::Position, "aPosition" },
		{ Vertex::Attribute::Type::Color, "aColor" },
		{ Vertex::Attribute::Type::TexCoord, "aTexCoord" },
		{ Vertex::Attribute::Type::Normal, "aNormal" }
	};

	glGenVertexArrays(1, &mImpl->vao);
	glBindVertexArray(mImpl->vao);

	for (auto& attrib : layout.attributes)
	{
		if (AttribNameMap.count(attrib.type) == 0)
			continue;

		auto location = glGetAttribLocation(mImpl->program, AttribNameMap.at(attrib.type).c_str());
		//assert(location != -1);
		
		if (location == -1)
			continue;

		glEnableVertexAttribArray(location);
		mImpl->attribLocations[attrib.type] = location;
	}
	glBindVertexArray(0);	
	
	mImpl->appliedConstantBuffer = malloc(sizeof(ConstantBuffer));

	if (customConstantBufferSize > 0)
		mImpl->appliedCustomConstantBuffer = malloc(customConstantBufferSize);
}

ShaderCustom::~ShaderCustom()
{
	glDeleteVertexArrays(1, &mImpl->vao);
	glDeleteProgram(mImpl->program);
	glDeleteBuffers(1, &mImpl->ubo);	
	
	free(mImpl->appliedConstantBuffer);

	if (mImpl->appliedCustomConstantBuffer != nullptr)
		free(mImpl->appliedCustomConstantBuffer);
}

void ShaderCustom::apply()
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

	mImpl->forceDirty = true;
}

void ShaderCustom::update()
{
	bool dirty = mImpl->forceDirty;

	if (!dirty && memcmp(mImpl->appliedConstantBuffer, &mConstantBuffer, sizeof(ConstantBuffer)) != 0)
		dirty = true;

	if (mCustomConstantBuffer && !dirty && memcmp(mImpl->appliedCustomConstantBuffer, mCustomConstantBuffer, mImpl->customConstantBufferSize) != 0)
		dirty = true;

	if (!dirty)
		return;

	mImpl->forceDirty = false;
	memcpy(mImpl->appliedConstantBuffer, &mConstantBuffer, sizeof(ConstantBuffer));

	if (mCustomConstantBuffer)
		memcpy(mImpl->appliedCustomConstantBuffer, mCustomConstantBuffer, mImpl->customConstantBufferSize);

	mImpl->constantBuffer = mConstantBuffer;

	if (SystemGL::IsRenderTargetBound())
	{
		auto orientationMatrix = glm::scale(glm::mat4(1.0f), { 1.0f, -1.0f, 1.0f });
		mImpl->constantBuffer.projection = orientationMatrix * mImpl->constantBuffer.projection;
	}

    auto ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(ConstantBuffer) + mImpl->customConstantBufferSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(ptr, &mImpl->constantBuffer, sizeof(ConstantBuffer));
    memcpy((void*)((size_t)ptr + sizeof(ConstantBuffer)), mCustomConstantBuffer, mImpl->customConstantBufferSize);
    glUnmapBuffer(GL_UNIFORM_BUFFER);

	glUniform1i(mImpl->uniformTexture, 0);
}
#endif
