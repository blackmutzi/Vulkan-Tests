#pragma once
#include "stdafx.h"

namespace Graphic
{
	#define FRAME_LAG 2 

	typedef struct
	{
		HINSTANCE connection;
		HWND window;
		uint32_t width;
		uint32_t height;
		const char * window_title;

		vk::SurfaceKHR surface;

	} Win32Surface;

	typedef struct
	{
		vk::Format format;
		vk::ColorSpaceKHR color_space;

	} SurfaceFormat;

	typedef struct
	{
		uint32_t graphics_index;
		uint32_t present_index;
		bool separate_present_queue;

		vk::Queue graphics_queue;
		vk::Queue present_queue;

	} QueueFamilyIndex;

	typedef struct
	{
		bool quit;
		uint32_t curFrame;
		uint32_t frame_index;

		vk::Fence fences[FRAME_LAG];
		bool fencesInited[FRAME_LAG];

		vk::Semaphore image_acquired_semaphores[2];
		vk::Semaphore draw_complete_semaphores[2];
		vk::Semaphore image_ownership_semaphores[2];

	} SemaphoreStats;

	class GraphicManager
	{
		private:
			GraphicManager();

		public:
			~GraphicManager() = default;
			 GraphicManager(const GraphicManager &) = default;


			static GraphicManager * getSingletonPtr(void);
			static GraphicManager & getSingleton(void);

			void setInstance(vk::Instance * instance);
			void setPhysicalDevice(vk::PhysicalDevice * physical_device);

			vk::Instance * createInstance2(void);
			vk::Instance * createInstance(void);
			vk::Device * createDevice(void);
			void createSurface( Win32Surface * surface );
			void createSwapchain(void);
			void createSemaphore(void);

			vk::PhysicalDevice * getPhysicalDevice(void);
			SurfaceFormat & getSurfaceFormats(void);
			Win32Surface & getSurface(void);

			vk::Instance * getInstance(void);
			vk::Device * getDevice(void);

			vk::SwapchainKHR & getSwapchain(void);
			QueueFamilyIndex & getQueueFamily(void);
			SemaphoreStats & getSemaphore(void);

		private:

			vk::Bool32 check_layers(uint32_t check_count, char const *const *const check_names, uint32_t layer_count, vk::LayerProperties *layers);
			vk::Bool32 check_extensions(uint32_t check_count, char const *const *const check_names, uint32_t layer_count, vk::ExtensionProperties *layers);

			static GraphicManager * ptrInstance;
			vk::Instance * instance;
			vk::PhysicalDevice * gpu;
			vk::Device * device;
			vk::SwapchainKHR swapchain;

			QueueFamilyIndex queue_family;
			Win32Surface win32_surface;
			SurfaceFormat surfaceFormat;
			SemaphoreStats semaphoreStats;
	};

}

