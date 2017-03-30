#pragma once
#include "stdafx.h"
#include "GraphicManager.h"

namespace Graphic
{

class CommandManager
{
public:
	CommandManager();
	~CommandManager();

	static void create_command_buffer();
	static void flush_command_buffer();

	static vk::CommandPool cmd_pool;
	static vk::CommandBuffer cmd;
};

}

