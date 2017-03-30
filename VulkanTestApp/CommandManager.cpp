#include "stdafx.h"
#include "CommandManager.h"

using namespace Graphic;

vk::CommandPool CommandManager::cmd_pool;
vk::CommandBuffer CommandManager::cmd;

CommandManager::CommandManager()
{
}


CommandManager::~CommandManager()
{
}

void CommandManager::create_command_buffer()
{
	QueueFamilyIndex * queue_family = GraphicManager::getSingleton().getQueueFamily();
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	if (!cmd_pool)
	{ 
		auto const cmd_pool_info = vk::CommandPoolCreateInfo().setQueueFamilyIndex( queue_family->graphics_index );
		auto result =device->createCommandPool(&cmd_pool_info, nullptr, &cmd_pool);
		VERIFY(result == vk::Result::eSuccess);
	}
	
	//cmd = Buffer for Initialize Commands
	if (!cmd)
	{
		auto const cmd_buffer_info = vk::CommandBufferAllocateInfo()
			.setCommandPool(cmd_pool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		auto const cmd_buffer_begin_info = vk::CommandBufferBeginInfo()
			.setPInheritanceInfo(nullptr);


		vk::Result result = device->allocateCommandBuffers(&cmd_buffer_info, &cmd);
		VERIFY(result == vk::Result::eSuccess);

		result = cmd.begin(&cmd_buffer_begin_info);
		VERIFY(result == vk::Result::eSuccess);
	}
}

void CommandManager::flush_command_buffer()
{
		QueueFamilyIndex * queue_family = GraphicManager::getSingleton().getQueueFamily();
		vk::Device * device = GraphicManager::getSingleton().getDevice();

		if (!cmd) {
			return;
		}

		// Beende den Command Buffer 
		auto result = cmd.end();
		VERIFY(result == vk::Result::eSuccess);

		auto const fenceInfo = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits(0));
		vk::Fence fence;
		device->createFence(&fenceInfo, nullptr, &fence);

		auto const submitInfo =vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers( &cmd );

		// Command Buffer Befehle ausführen 
		result = queue_family->graphics_queue->submit(1, &submitInfo, fence);
		VERIFY(result == vk::Result::eSuccess);

		result = device->waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
		VERIFY(result == vk::Result::eSuccess);

		device->freeCommandBuffers(cmd_pool, 1, &cmd);
		device->destroyFence(fence, nullptr);

		cmd = vk::CommandBuffer();
}
