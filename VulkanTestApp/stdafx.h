// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschließen
// Windows-Headerdateien:
#include <windows.h>

// C RunTime-Headerdateien
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <stack>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#ifndef NDEBUG
#define VERIFY(x) assert(x)
#else
#define VERIFY(x) ((void)(x))
#endif

#define APP_SHORT_NAME "cube"
#ifdef _WIN32
#define APP_NAME_STR_LEN 80
#endif

// Allow a maximum of two outstanding presentation operations.
#define FRAME_LAG 2
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#ifdef _WIN32
#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        if (!suppress_popups)                                                  \
            MessageBox(nullptr, err_msg, err_class, MB_OK);                    \
        exit(1);                                                               \
    } while (0)
#else
#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        printf(err_msg);                                                       \
        fflush(stdout);                                                        \
        exit(1);                                                               \
    } while (0)
#endif

namespace Graphic
{
	class Vulkan;

	class SurfaceManager;
	class DeviceManager;
	class SemaphoreManager;
	class CommandManager;
	class TextureManager;
	class GraphicManager;

	// Main Pipeline 
	class Pipeline;
	// Object Pipeline 
	class Renderpipeline;

	static bool suppress_popups = false;

}

using namespace std;
