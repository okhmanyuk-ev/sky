#include <Renderer/shader_light.h>
#include <string>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
#include <Renderer/system_gl.h>

using namespace Renderer;

struct ShaderLight::Impl 
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
		struct DirectionalLight
		{
			vec3 direction;
			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
		};

		struct PointLight
		{
			vec3 position;

			float constantAttenuation;
			float linearAttenuation;
			float quadraticAttenuation;

			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
		};

		struct Material
		{
			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
			float shininess;
		};

		layout (std140) uniform ConstantBuffer
		{
			mat4 viewMatrix;
			mat4 projectionMatrix;
			mat4 modelMatrix;

			vec3 eyePosition;
			
			DirectionalLight directionalLight;
			PointLight pointLight;

			Material material;
		};

		uniform sampler2D uTexture;
		uniform bool uRenderTargetBound;

		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		out vec3 vPosition;
		in vec3 aNormal;
		out vec3 vNormal;
		#ifdef HAS_COLOR_ATTRIB
		in vec4 aColor;
		out vec4 vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
		in vec2 aTexCoord;
		out vec2 vTexCoord;
		#endif	
	
		void main()
		{
			vPosition = vec3(modelMatrix * vec4(aPosition, 1.0));
		//	vNormal = mat3(transpose(inverse(modelMatrix))) * aNormal; // -- this was from tutorial
			vNormal = vec3(modelMatrix * vec4(aNormal, 1.0)); // i use this, because no changes with upper
		#ifdef HAS_COLOR_ATTRIB
			vColor = aColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			vTexCoord = aTexCoord;
		#endif
			gl_Position = projectionMatrix * viewMatrix * vec4(vPosition, 1.0);

			if (uRenderTargetBound)
				gl_Position.y *= -1.0;
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec3 vPosition;
		in vec3 vNormal;
		#ifdef HAS_COLOR_ATTRIB
		in vec4 vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
		in vec2 vTexCoord;
		#endif	
	
		vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
		vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDirection);

		out vec4 resultColor;

		void main()
		{
			resultColor = vec4(1.0, 1.0, 1.0, 1.0);
		#ifdef HAS_COLOR_ATTRIB
			resultColor *= vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			resultColor *= texture(uTexture, vTexCoord);
		#endif
			
			vec3 normal = normalize(vNormal);
			vec3 viewDirection = normalize(eyePosition - vPosition);

			vec3 intensity = CalcDirectionalLight(directionalLight, normal, viewDirection);
			intensity += CalcPointLight(pointLight, normal, viewDirection);
		
			resultColor *= vec4(intensity, 1.0);
		}

		vec3 CalcLight(vec3 normal, vec3 viewDirection, vec3 lightDirection, vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular)
		{
			vec3 lightDir = normalize(lightDirection);

			float diff = max(dot(normal, lightDir), 0.0);

			vec3 reflectDir = reflect(-lightDir, normal);
			float spec = pow(max(dot(viewDirection, reflectDir), 0.0), material.shininess);

			vec3 ambient = lightAmbient * material.diffuse;
			vec3 diffuse = lightDiffuse * diff * material.diffuse;
			vec3 specular = lightSpecular * spec * material.specular;

			return ambient + diffuse + specular;
		}

		vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
		{
			return CalcLight(normal, viewDirection, -light.direction, light.ambient, light.diffuse, light.specular);
		}

		vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDirection)
		{
			vec3 lightDir = light.position - vPosition;

			float distance = length(lightDir);
			float linearAttn = light.linearAttenuation * distance;
			float quadraticAttn = light.quadraticAttenuation * (distance * distance);
			float attenuation = 1.0 / (light.constantAttenuation + linearAttn + quadraticAttn);

			return CalcLight(normal, viewDirection, lightDir, light.ambient, light.diffuse, light.specular) * attenuation;
		}
		#endif)";
}

ShaderLight::ShaderLight(const Vertex::Layout& layout)
{
    mImpl = std::make_unique<Impl>();
	mImpl->layout = layout;
	checkRequiredAttribs(requiredAttribs, layout);

	auto source = std::string(shaderSource);

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		source = "#define HAS_COLOR_ATTRIB\n" + source;

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		source = "#define HAS_TEXCOORD_ATTRIB\n" + source;

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
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), nullptr, GL_DYNAMIC_DRAW);

	mImpl->uniformBlock = glGetUniformBlockIndex(mImpl->program, "ConstantBuffer");
	glUniformBlockBinding(mImpl->program, mImpl->uniformBlock, 0);
	
	mImpl->uniformTexture = glGetUniformLocation(mImpl->program, "uTexture");
	mImpl->uniformRenderTargetBound = glGetUniformLocation(mImpl->program, "uRenderTargetBound");
	
	static const std::map<Vertex::Attribute::Type, std::string> attribName = {
		{ Vertex::Attribute::Type::Position, "aPosition" },
		{ Vertex::Attribute::Type::Color, "aColor" },
		{ Vertex::Attribute::Type::TexCoord, "aTexCoord" },
		{ Vertex::Attribute::Type::Normal, "aNormal" }
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

ShaderLight::~ShaderLight()
{
	glDeleteVertexArrays(1, &mImpl->vao);
	glDeleteProgram(mImpl->program);
	glDeleteBuffers(1, &mImpl->ubo);
}

void ShaderLight::apply()
{
	glUseProgram(mImpl->program);
	glBindVertexArray(mImpl->vao);
	glBindBuffer(GL_UNIFORM_BUFFER, mImpl->ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mImpl->ubo);
	
	for (auto& attrib : mImpl->layout.attributes)
	{
		if (mImpl->attribLocations.count(attrib.type) == 0)
			continue;

		glVertexAttribPointer(mImpl->attribLocations[attrib.type], SystemGL::Size[attrib.format],
			SystemGL::Type[attrib.format], SystemGL::Normalize[attrib.format], (GLsizei)mImpl->layout.stride,
			(void*)attrib.offset);
	}
	
	mDirty = true;
}

void ShaderLight::update()
{
	glUniform1ui(mImpl->uniformRenderTargetBound, (GLint)SystemGL::IsRenderTargetBound());

	if (!mDirty)
		return;

	mDirty = false;

    glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), &mConstantBuffer, GL_DYNAMIC_DRAW);

	glUniform1i(mImpl->uniformTexture, 0);
}
#endif
