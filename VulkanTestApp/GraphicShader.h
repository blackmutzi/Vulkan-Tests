#pragma once
#include "stdafx.h"
#include "GraphicManager.h"

namespace Graphic
{
	class GraphicShader
	{
		public:

			GraphicShader();
			~GraphicShader();

			void createShaderStageInfo(vk::PipelineShaderStageCreateInfo * shaderStageCreateInfo, uint32_t stageCount, const char * vertex_filename, const char * fragment_filename);
			vk::PipelineShaderStageCreateInfo * getShaderStageInfo(void);
			uint32_t getShaderStageCount(void);

			vk::ShaderModule prepare_shader(const char * filename);

	private:

		vk::PipelineShaderStageCreateInfo * shaderStageInfo;
		uint32_t shaderStageCount;

		vk::ShaderModule prepare_shader_module(const void *code, size_t size);
		char * read_spv(const char *filename, size_t *psize);

	};

}

