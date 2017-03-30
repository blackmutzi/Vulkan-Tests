#pragma once
#include "stdafx.h"
#include "TextureManager.h"

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

		vk::Queue * graphics_queue;
		vk::Queue * present_queue;

	} QueueFamilyIndex;

	typedef struct
	{
		bool quit;

		uint32_t curFrame;
		uint32_t frame_index;
		uint32_t current_buffer;

		vk::Fence fences[FRAME_LAG];
		bool fencesInited[FRAME_LAG];

		vk::Semaphore image_acquired_semaphores[ FRAME_LAG ];
		vk::Semaphore draw_complete_semaphores[ FRAME_LAG ];
		vk::Semaphore image_ownership_semaphores[ FRAME_LAG ];

	} SemaphoreStats;

	typedef struct {
		vk::Image image;
		vk::CommandBuffer cmd;
		vk::CommandBuffer graphics_to_present_cmd;
		vk::ImageView view;
	} SwapchainBuffers;

	class GraphicManager
	{
		private:
			GraphicManager();

		public:
			~GraphicManager() = default;
			 GraphicManager(const GraphicManager &) = default;

			static GraphicManager * getSingletonPtr(void);
			static GraphicManager & getSingleton(void);

			vk::Instance * createInstance(void);
			vk::Device * createLogicDevice(void);
			void createSurface( Win32Surface * surface );
			void createSwapchain(void);
			void createSemaphore(void);
			void createSwapchainbuffers(void);

			vk::PhysicalDevice * getPhysicalDevice(void);
			SurfaceFormat & getSurfaceFormats(void);
			Win32Surface & getSurface(void);

			vk::Instance * getInstance(void);
			vk::Device * getDevice(void);

			vk::SwapchainKHR * getSwapchain(void);
			QueueFamilyIndex * getQueueFamily(void);
			SemaphoreStats * getSemaphore(void);

			vk::Result acquireNextImage(void);
			vk::Result graphicQueueSubmit(void);

			uint32_t getSwapchainBufferCount(void);
			SwapchainBuffers * getSwapchainBuffers();

		private:

			vk::Bool32 check_layers(uint32_t check_count, char const *const *const check_names, uint32_t layer_count, vk::LayerProperties *layers);
			vk::Bool32 check_extensions(uint32_t check_count, char const *const *const check_names, uint32_t layer_count, vk::ExtensionProperties *layers);

			static GraphicManager * ptrInstance;
			vk::Instance * instance;
			vk::PhysicalDevice * gpu;
			vk::Device * device;
			vk::SwapchainKHR * swapchain;

			std::unique_ptr< vk::Queue > unique_graphics_queue;
			std::unique_ptr< vk::Queue > unique_present_queue;
			std::unique_ptr< vk::Device > unique_device;
			std::unique_ptr<vk::PresentModeKHR[]> presentModes;
			std::unique_ptr< vk::SwapchainKHR > unique_swapchain;
			std::unique_ptr< SwapchainBuffers[] > unique_swapchain_buffers;
			std::unique_ptr<vk::Image[]> unique_swapchainImages;
			uint32_t swapchainImageCount;

			QueueFamilyIndex * queue_family;
			SemaphoreStats * semaphoreStats;
			Win32Surface win32_surface;
			SurfaceFormat surfaceFormat;
	};

}

