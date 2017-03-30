#include "stdafx.h"
#include "GraphicShader.h"

using namespace Graphic;

GraphicShader::GraphicShader(){}
GraphicShader::~GraphicShader(){}

vk::PipelineShaderStageCreateInfo * GraphicShader::getShaderStageInfo(void)
{
	return shaderStageInfo;
}

uint32_t GraphicShader::getShaderStageCount(void)
{
	return shaderStageCount;
}

void GraphicShader::createShaderStageInfo(vk::PipelineShaderStageCreateInfo * shaderStageCreateInfo, uint32_t stageCount, const char * vertex_filename, const char * fragment_filename)
{
	if (shaderStageCreateInfo == nullptr)
	{
		//Demo Version 

		vk::ShaderModule vertex_shader   = prepare_shader(vertex_filename);
		vk::ShaderModule fragment_shader = prepare_shader(fragment_filename);

		std::unique_ptr< vk::PipelineShaderStageCreateInfo[] > stageInfo(new vk::PipelineShaderStageCreateInfo[2] );

		stageInfo[0].setStage(vk::ShaderStageFlagBits::eVertex);
		stageInfo[0].setModule(vertex_shader);
		stageInfo[0].setPName("main");
		stageInfo[0].setPNext(&stageInfo[1]);

		stageInfo[1].setStage(vk::ShaderStageFlagBits::eFragment);
		stageInfo[1].setModule(fragment_shader);
		stageInfo[1].setPName("main");

		shaderStageInfo = stageInfo.release();
		shaderStageCount = 2;
		
		//vk::Device * device = GraphicManager::getSingleton().getDevice();
		//device->destroyShaderModule(vertex_shader, nullptr);
		//device->destroyShaderModule(fragment_shader, nullptr);
	}
	else
	{
		shaderStageInfo = shaderStageCreateInfo;
		shaderStageCount = stageCount;
	}
}


vk::ShaderModule GraphicShader::prepare_shader_module(const void *code, size_t size)
{
	vk::Result result;

	auto const moduleCreateInfo = vk::ShaderModuleCreateInfo().setCodeSize(size).setPCode((uint32_t const *)code);

	vk::ShaderModule module;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	result = device->createShaderModule(&moduleCreateInfo, nullptr, &module);
	VERIFY(result == vk::Result::eSuccess);

	return module;
}

vk::ShaderModule GraphicShader::prepare_shader(const char * filename)
{
	size_t size = 0;
	void *vertShaderCode = read_spv(filename, &size);

	vk::ShaderModule shader_module = prepare_shader_module(vertShaderCode, size);

	free(vertShaderCode);

	return shader_module;
}

char * GraphicShader::read_spv(const char *filename, size_t *psize)
{
	FILE *fp = fopen(filename, "rb");

	if (!fp)
	{
		return nullptr;
	}

	fseek(fp, 0L, SEEK_END);
	long int size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	void *shader_code = malloc(size);
	size_t retval = fread(shader_code, size, 1, fp);
	VERIFY(retval == 1);

	*psize = size;

	fclose(fp);

	return (char *)shader_code;
}