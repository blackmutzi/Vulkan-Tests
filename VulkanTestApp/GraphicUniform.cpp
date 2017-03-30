#include "stdafx.h"
#include "GraphicUniform.h"

using namespace Graphic;

GraphicUniform::GraphicUniform(){}
GraphicUniform::~GraphicUniform(){}

vk::DescriptorBufferInfo * GraphicUniform::createBuffer(void * data, vk::DeviceSize size_of_data)
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	// Create Uniform Buffer 
	auto const buf_info = vk::BufferCreateInfo()
		.setSize( size_of_data )
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

	result = device->createBuffer(&buf_info, nullptr, &uniform_data.buf);
	VERIFY(result == vk::Result::eSuccess);

	// Get Memory Requirements 
	vk::MemoryRequirements mem_reqs;
	device->getBufferMemoryRequirements(uniform_data.buf, &mem_reqs);

	uniform_data.mem_alloc.setAllocationSize(mem_reqs.size);
	uniform_data.mem_alloc.setMemoryTypeIndex(0);

	bool pass = memory_type_from_properties(
		mem_reqs.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent,
		&uniform_data.mem_alloc.memoryTypeIndex);
	VERIFY(pass);

	// Allocate memory 
	result = device->allocateMemory(&uniform_data.mem_alloc, nullptr, &(uniform_data.mem));
	VERIFY(result == vk::Result::eSuccess);

	// Bind Buffer Memory 
	result = device->bindBufferMemory(uniform_data.buf, uniform_data.mem, 0);
	VERIFY(result == vk::Result::eSuccess);

	// Map Memory 
	auto pData = device->mapMemory(uniform_data.mem, 0, uniform_data.mem_alloc.allocationSize);
	VERIFY(pData.result == vk::Result::eSuccess);

	memcpy(pData.value, data, size_of_data );

	device->unmapMemory(uniform_data.mem);

	// Info 
	uniform_data.buffer_info.buffer = uniform_data.buf;
	uniform_data.buffer_info.offset = 0;
	uniform_data.buffer_info.range  = size_of_data;

	return &uniform_data.buffer_info;
}

void GraphicUniform::updateBuffer(void * data, vk::DeviceSize size_of_data)
{
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	auto pData = device->mapMemory(uniform_data.mem, 0, uniform_data.mem_alloc.allocationSize, vk::MemoryMapFlags());
	VERIFY(pData.result == vk::Result::eSuccess);

	//Update
	memcpy(pData.value, &data, size_of_data);
    device->unmapMemory(uniform_data.mem);
}

void GraphicUniform::destroyBuffer()
{
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	device->destroyBuffer(uniform_data.buf, nullptr);
	device->freeMemory(uniform_data.mem, nullptr);
}

bool GraphicUniform::memory_type_from_properties(uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t *typeIndex) 
{
	vk::PhysicalDevice * gpu = GraphicManager::getSingleton().getPhysicalDevice();

	// Get Memory information and properties
	vk::PhysicalDeviceMemoryProperties memory_properties;
	gpu->getMemoryProperties(&memory_properties);

	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memory_properties.memoryTypes[i].propertyFlags &
				requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}

	// No memory types matched, return failure
	return false;
}


