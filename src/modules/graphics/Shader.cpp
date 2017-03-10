/**
 * Copyright (c) 2006-2017 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
**/

// LOVE
#include "Shader.h"
#include "Graphics.h"
#include "math/MathModule.h"

// glslang
#include "libraries/glslang/glslang/Public/ShaderLang.h"

// C++
#include <string>

// TODO: Use love.graphics to determine actual limits?
static const TBuiltInResource defaultTBuiltInResource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 16384,
	/* .MaxVaryingFloats = */ 128,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 16384,
	/* .MaxDrawBuffers = */ 8,
	/* .MaxVertexUniformVectors = */ 4096,
	/* .MaxVaryingVectors = */ 32,
	/* .MaxFragmentUniformVectors = */ 4096,
	/* .MaxVertexOutputVectors = */ 32,
	/* .MaxFragmentInputVectors = */ 31,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 32,
	/* .MaxComputeImageUniforms = */ 16,
	/* .MaxComputeAtomicCounters = */ 4096,
	/* .MaxComputeAtomicCounterBuffers = */ 8,
	/* .MaxVaryingComponents = */ 128,
	/* .MaxVertexOutputComponents = */ 128,
	/* .MaxGeometryInputComponents = */ 128,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 192,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 144,
	/* .MaxCombinedShaderOutputResources = */ 144,
	/* .MaxImageSamples = */ 32,
	/* .MaxVertexImageUniforms = */ 16,
	/* .MaxTessControlImageUniforms = */ 16,
	/* .MaxTessEvaluationImageUniforms = */ 16,
	/* .MaxGeometryImageUniforms = */ 16,
	/* .MaxFragmentImageUniforms = */ 16,
	/* .MaxCombinedImageUniforms = */ 80,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 4096,
	/* .MaxTessControlAtomicCounters = */ 4096,
	/* .MaxTessEvaluationAtomicCounters = */ 4096,
	/* .MaxGeometryAtomicCounters = */ 4096,
	/* .MaxFragmentAtomicCounters = */ 4096,
	/* .MaxCombinedAtomicCounters = */ 4096,
	/* .MaxAtomicCounterBindings = */ 8,
	/* .MaxVertexAtomicCounterBuffers = */ 8,
	/* .MaxTessControlAtomicCounterBuffers = */ 8,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 8,
	/* .MaxGeometryAtomicCounterBuffers = */ 8,
	/* .MaxFragmentAtomicCounterBuffers = */ 8,
	/* .MaxCombinedAtomicCounterBuffers = */ 8,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 32,
	/* .limits = */ {
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}};

namespace love
{
namespace graphics
{

love::Type Shader::type("Shader", &Object::type);

Shader *Shader::current = nullptr;
Shader *Shader::standardShaders[Shader::STANDARD_MAX_ENUM] = {nullptr};

Shader::Shader(const ShaderSource &source)
	: shaderSource(source)
{
	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx == nullptr)
		throw love::Exception("love.graphics must be initialized to create a Shader.");

	bool gles = gfx->getRenderer() == Graphics::RENDERER_OPENGLES;
	std::string err;
	if (!validate(gfx, gles, source, false, err))
		throw love::Exception("%s", err.c_str());
}

Shader::~Shader()
{
	for (int i = 0; i < STANDARD_MAX_ENUM; i++)
	{
		if (this == standardShaders[i])
			standardShaders[i] = nullptr;
	}

	if (current == this)
		attachDefault(STANDARD_DEFAULT);
}

void Shader::attachDefault(StandardShader defaultType)
{
	Shader *defaultshader = standardShaders[defaultType];

	if (defaultshader == nullptr)
	{
		current = nullptr;
		return;
	}

	if (current != defaultshader)
		defaultshader->attach();
}

bool Shader::isDefaultActive()
{
	for (int i = 0; i < STANDARD_MAX_ENUM; i++)
	{
		if (current == standardShaders[i])
			return true;
	}

	return false;
}

TextureType Shader::getMainTextureType() const
{
	const UniformInfo *info = getUniformInfo(BUILTIN_TEXTURE_MAIN);
	return info != nullptr ? info->textureType : TEXTURE_MAX_ENUM;
}

void Shader::checkMainTextureType(TextureType textype) const
{
	const UniformInfo *info = getUniformInfo(BUILTIN_TEXTURE_MAIN);

	if (info != nullptr && info->textureType != TEXTURE_MAX_ENUM && info->textureType != textype)
	{
		const char *textypestr = "unknown";
		const char *shadertextypestr = "unknown";
		Texture::getConstant(textype, textypestr);
		Texture::getConstant(info->textureType, shadertextypestr);
		throw love::Exception("Texture's type (%s) must match the type of the shader's main texture type (%s).", textypestr, shadertextypestr);
	}
}

bool Shader::validate(Graphics *gfx, bool gles, const ShaderSource &source, bool checkWithDefaults, std::string &err)
{
	if (source.vertex.empty() && source.pixel.empty())
	{
		err = "Error validating shader: no source code!";
		return false;
	}

	bool supportsGLSL3 = gfx->isSupported(Graphics::FEATURE_GLSL3);

	int defaultversion = gles ? 100 : 120;
	EProfile defaultprofile = gles ? EEsProfile : ENoProfile;

	glslang::TShader vshader(EShLangVertex);
	glslang::TShader pshader(EShLangFragment);

	// TProgram must be destroyed before TShader.
	glslang::TProgram program;

	auto addshader = [&](glslang::TShader &s, const std::string &src, ShaderStage stage) -> bool
	{
		if (src.empty())
			return true;

		const char *csrc = src.c_str();
		int srclen = (int) src.length();
		s.setStringsWithLengths(&csrc, &srclen, 1);

		bool forcedefault = false;
		if (src.find("#define LOVE_GLSL1_ON_GLSL3") != std::string::npos)
			forcedefault = true;

		bool forwardcompat = supportsGLSL3 && !forcedefault;

		if (!s.parse(&defaultTBuiltInResource, defaultversion, defaultprofile, forcedefault, forwardcompat, EShMsgSuppressWarnings))
		{
			const char *stagename;
			getConstant(stage, stagename);
			err = "Error validating " + std::string(stagename) + " shader:\n\n"
				+ std::string(s.getInfoLog()) + "\n" + std::string(s.getInfoDebugLog());
			return false;
		}

		program.addShader(&s);
		return true;
	};

	const ShaderSource &defaults = gfx->getCurrentDefaultShaderCode();
	const std::string &vertcode = (checkWithDefaults && source.vertex.empty()) ? defaults.vertex : source.vertex;
	const std::string &pixcode = (checkWithDefaults && source.pixel.empty()) ? defaults.pixel : source.pixel;

	if (!addshader(vshader, vertcode, STAGE_VERTEX))
		return false;

	if (!addshader(pshader, pixcode, STAGE_PIXEL))
		return false;

	if (!program.link(EShMsgDefault))
	{
		err = "Cannot compile shader:\n\n" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
		return false;
	}

	return true;
}

bool Shader::initialize()
{
	return glslang::InitializeProcess();
}

void Shader::deinitialize()
{
	glslang::FinalizeProcess();
}

bool Shader::getConstant(const char *in, Language &out)
{
	return languages.find(in, out);
}

bool Shader::getConstant(Language in, const char *&out)
{
	return languages.find(in, out);
}

bool Shader::getConstant(const char *in, ShaderStage &out)
{
	return stageNames.find(in, out);
}

bool Shader::getConstant(ShaderStage in, const char *&out)
{
	return stageNames.find(in, out);
}

bool Shader::getConstant(const char *in, BuiltinUniform &out)
{
	return builtinNames.find(in, out);
}

bool Shader::getConstant(BuiltinUniform in, const char *&out)
{
	return builtinNames.find(in, out);
}

StringMap<Shader::Language, Shader::LANGUAGE_MAX_ENUM>::Entry Shader::languageEntries[] =
{
	{ "glsl1", LANGUAGE_GLSL1 },
	{ "essl1", LANGUAGE_ESSL1 },
	{ "glsl3", LANGUAGE_GLSL3 },
	{ "essl3", LANGUAGE_ESSL3 },
};

StringMap<Shader::Language, Shader::LANGUAGE_MAX_ENUM> Shader::languages(Shader::languageEntries, sizeof(Shader::languageEntries));

StringMap<Shader::ShaderStage, Shader::STAGE_MAX_ENUM>::Entry Shader::stageNameEntries[] =
{
	{ "vertex", STAGE_VERTEX },
	{ "pixel",  STAGE_PIXEL  },
};

StringMap<Shader::ShaderStage, Shader::STAGE_MAX_ENUM> Shader::stageNames(Shader::stageNameEntries, sizeof(Shader::stageNameEntries));

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM>::Entry Shader::builtinNameEntries[] =
{
	{ "MainTex",                   BUILTIN_TEXTURE_MAIN                },
	{ "love_VideoYChannel",        BUILTIN_TEXTURE_VIDEO_Y             },
	{ "love_VideoCbChannel",       BUILTIN_TEXTURE_VIDEO_CB            },
	{ "love_VideoCrChannel",       BUILTIN_TEXTURE_VIDEO_CR            },
	{ "TransformMatrix",           BUILTIN_MATRIX_TRANSFORM            },
	{ "ProjectionMatrix",          BUILTIN_MATRIX_PROJECTION           },
	{ "TransformProjectionMatrix", BUILTIN_MATRIX_TRANSFORM_PROJECTION },
	{ "NormalMatrix",              BUILTIN_MATRIX_NORMAL               },
	{ "love_PointSize",            BUILTIN_POINT_SIZE                  },
	{ "love_ScreenSize",           BUILTIN_SCREEN_SIZE                 },
};

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM> Shader::builtinNames(Shader::builtinNameEntries, sizeof(Shader::builtinNameEntries));

} // graphics
} // love
