#pragma once
#include "stdafx.h"
#include "TextureManager.h"
#include "DeviceManager.h"

namespace Graphic
{
	typedef struct 
	{
		vk::Buffer buf;
		vk::MemoryAllocateInfo mem_alloc;
		vk::DeviceMemory mem;
		vk::DescriptorBufferInfo buffer_info;

	} Uniform;

	typedef struct
	{
		void * dataPtr;
		vk::DeviceSize size_of_data;

	} UniformBufferData;

	typedef struct
	{
		uint32_t uniformCount;
		const char* const * uniformNames;
		uint32_t * uniformBindLocations;
		UniformBufferData * uniformData;

	} UniformAssemblerInfo;

	typedef struct
	{
		uint32_t textureCount;
		const char * const * textureNames;
		uint32_t * textureBindLocations;

		Texture * textures;

	} TextureAssemblerInfo;

	typedef struct
	{
		UniformAssemblerInfo uniformInfo;
		TextureAssemblerInfo textureInfo;

	} AssemblerInfo;

	class AssemblerManager
	{
	    public:
		AssemblerManager();
		~AssemblerManager();

		static void create_descriptor_layout( AssemblerInfo const & info );
		static void create_descriptor_pool( AssemblerInfo const & info );

		static void create_descriptor_set_uniform(AssemblerInfo const & info , vk::DescriptorBufferInfo * uniform_buffer_info);
		static void create_descriptor_set_image(AssemblerInfo const & info);

		static vk::DescriptorImageInfo * AssemblerManager::create_image_infos(AssemblerInfo const & info);

		vk::DescriptorBufferInfo * create_uniform_buffer( void * data , vk::DeviceSize size_of_data );
		void update_uniform_buffer( void * data , vk::DeviceSize size_of_data );
		void destroy_uniform_buffer();

		static vk::PipelineLayout pipeline_layout;
		static vk::DescriptorSetLayout desc_layout;
		static vk::DescriptorPool desc_pool;
		static vk::DescriptorSet desc_set;

		private:
			Uniform uniform_data;
	};

}

