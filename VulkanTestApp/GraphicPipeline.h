#pragma once
#include "stdafx.h"
#include "TextureManager.h"
#include "GraphicUniform.h"
#include "GraphicShader.h"
#include "GraphicEnvironment.h"
#include "GraphicManager.h"

namespace Graphic
{
	typedef struct
	{
		void * dataPtr;
		vk::DeviceSize size_of_data;

	} UniformBufferData;

	typedef struct
	{
		uint32_t uniformCount;
		uint32_t * uniformBindLocations;

		UniformBufferData * uniformData;

	} DescriptorLayoutUniformInfo;

	typedef struct
	{
		uint32_t textureCount;
		uint32_t * textureBindLocations;

		Texture * textures;

	} DescriptorLayoutTextureInfo;

	typedef struct
	{
		DescriptorLayoutUniformInfo uniformInfo;
		DescriptorLayoutTextureInfo textureInfo;

	} DescriptorSetLayoutInfo;


	using UniformCollection = std::vector< GraphicUniform * >;

	class GraphicPipeline
	{
	public:
		GraphicPipeline();
		~GraphicPipeline();

		void setEnvironment( GraphicEnvironment * environment );
		void setShaderStageInfo(vk::PipelineShaderStageCreateInfo * shaderStageCreateInfo , uint32_t stageCount );

		vk::Pipeline * createPipeline( DescriptorSetLayoutInfo const & info );
		vk::Pipeline * getPipeline( void );
		void destroyPipeline(void);
		
		UniformCollection getUniformBuffers(void);

	protected:
		void setRenderPass(vk::RenderPass * renderPass);

		void createDrawBuild( GraphicEnvironment * environment);

		void createLayoutBindings(DescriptorSetLayoutInfo const & info, uint32_t descriptorCount, vk::DescriptorSetLayoutBinding * layout_output);
		void createPipelineLayout(DescriptorSetLayoutInfo const & info);
		void createDescriptorPool(DescriptorSetLayoutInfo const & info);
		void updateDescriptorSets(DescriptorSetLayoutInfo const & info);

		void create_descriptor_set_image(DescriptorSetLayoutInfo const & info);
		void create_descriptor_set_uniforms(DescriptorSetLayoutInfo const & info );
	
		void draw_build_cmd(SwapchainBuffers * swapchain_buffer, uint32_t current_buffer, GraphicEnvironment  * environment);
		//void draw_build_cmd( vk::CommandBuffer * buffer , uint32_t current_buffer , GraphicEnvironment * environment );

	protected:

		UniformCollection uniforms;

		GraphicEnvironment * environment;
		std::unique_ptr< vk::Pipeline > unique_pipeline;

		vk::PipelineCache pipelineCache;
		vk::RenderPass * renderPass;
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout desc_layout;
		vk::DescriptorPool desc_pool;
		vk::DescriptorSet desc_set;
		vk::PipelineShaderStageCreateInfo * shaderStageInfo;
		uint32_t shaderStageCount;
	};
}

