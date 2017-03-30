#pragma once
#include "stdafx.h"
#include "GraphicManager.h"

namespace Graphic
{
	typedef struct
	{
		vk::Buffer buf;
		vk::MemoryAllocateInfo mem_alloc;
		vk::DeviceMemory mem;
		vk::DescriptorBufferInfo buffer_info;

	} Uniform;

	class GraphicUniform
	{
		public:
			GraphicUniform();
			~GraphicUniform();

			vk::DescriptorBufferInfo * createBuffer(void * data, vk::DeviceSize size_of_data);
			void updateBuffer(void * data, vk::DeviceSize size_of_data);
			void destroyBuffer();
		
		protected:
			bool memory_type_from_properties(uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t *typeIndex);

		private:
			Uniform uniform_data;

	};


}

