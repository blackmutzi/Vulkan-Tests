#include "stdafx.h"
#include "GraphicManager.h"

using namespace Graphic;

GraphicManager * GraphicManager::ptrInstance = nullptr;

GraphicManager::GraphicManager()
	: instance(nullptr) , gpu(nullptr) , device( nullptr ) , swapchain( nullptr ) ,
	  queue_family( new QueueFamilyIndex() ) , semaphoreStats( new SemaphoreStats() )
{}

GraphicManager & GraphicManager::getSingleton(void)
{
	if (ptrInstance == nullptr)
	{
		ptrInstance = new GraphicManager();
	}

	return *(ptrInstance);
}

GraphicManager * GraphicManager::getSingletonPtr(void)
{
	if (ptrInstance == nullptr)
	{
		ptrInstance = new GraphicManager();
	}

	return ptrInstance;
}

vk::Instance * GraphicManager::createInstance(void)
{
	//# Init Mode 

	vk::Result result;

	bool validate_layers = false;
	bool validate_extensions = true;

	char const *const enabledInstanceLayers[] = {
		"VK_LAYER_LUNARG_api_dump" ,
		"VK_LAYER_LUNARG_core_validation" ,
		"VK_LAYER_LUNARG_image" ,
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_object_tracker" ,
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_GOOGLE_unique_objects"
	};

	char const *const enabledInstanceExtensions[] = {
		"VK_KHR_surface" ,
		"VK_KHR_win32_surface"
	};


	vk::ApplicationInfo app_info = vk::ApplicationInfo();
	app_info.setPApplicationName(APP_SHORT_NAME);
	app_info.setApplicationVersion(0);
	app_info.setPEngineName(APP_SHORT_NAME);
	app_info.setEngineVersion(0);
	app_info.setApiVersion(VK_API_VERSION_1_0);

	//build instance create info 
	vk::InstanceCreateInfo instance_info = vk::InstanceCreateInfo();
	instance_info.setPApplicationInfo(&app_info);

	if (validate_layers)
	{
		vk::Bool32 validation_found = VK_FALSE;

		// Get Instance Layer Count 
		uint32_t instance_layer_count = 0;
		result = vk::enumerateInstanceLayerProperties(&instance_layer_count, nullptr);
		VERIFY(result == vk::Result::eSuccess);

		// Get Instance Layers 
		std::unique_ptr< vk::LayerProperties[] > instance_layers(new vk::LayerProperties[instance_layer_count]);
		result = vk::enumerateInstanceLayerProperties(&instance_layer_count, instance_layers.get());
		VERIFY(result == vk::Result::eSuccess);

		validation_found = check_layers(ARRAY_SIZE(enabledInstanceLayers), enabledInstanceLayers, instance_layer_count, instance_layers.get());
		VERIFY(validation_found == VK_TRUE);

		instance_info.setEnabledLayerCount(ARRAY_SIZE(enabledInstanceLayers));
		instance_info.setPpEnabledLayerNames(enabledInstanceLayers);

		instance_layers.reset();
	} else {

		instance_info.setEnabledLayerCount(0);
		instance_info.setPpEnabledLayerNames(enabledInstanceLayers);
	}

	if (validate_extensions)
	{
		vk::Bool32 validation_found = VK_FALSE;

		// Get Instance Extensions Count 
		uint32_t instance_extension_count = 0;
		result = vk::enumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
		VERIFY(result == vk::Result::eSuccess);

		// Get Instance Extensions 
		std::unique_ptr< vk::ExtensionProperties[] > instance_extension(new vk::ExtensionProperties[instance_extension_count]);
		result = vk::enumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extension.get());

		validation_found = check_extensions(ARRAY_SIZE(enabledInstanceExtensions), enabledInstanceExtensions, instance_extension_count, instance_extension.get());
		VERIFY(validation_found == VK_TRUE);

		instance_info.setEnabledExtensionCount(ARRAY_SIZE(enabledInstanceExtensions));
		instance_info.setPpEnabledExtensionNames(enabledInstanceExtensions);

		instance_extension.reset();
	}
	
	std::unique_ptr< vk::Instance > unique_instance(new vk::Instance());
	result = vk::createInstance(&instance_info, nullptr, unique_instance.get() );
	instance = unique_instance.release();

	VERIFY(result == vk::Result::eSuccess);
	return instance;
}

void GraphicManager::createSurface( Win32Surface * surface )
{
	// #Init Mode 

	win32_surface = *surface;

	auto const createInfo = vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(win32_surface.connection)
		.setHwnd(win32_surface.window);

	auto result = instance->createWin32SurfaceKHR(&createInfo, nullptr, &win32_surface.surface);
	VERIFY(result == vk::Result::eSuccess);
}

Win32Surface & GraphicManager::getSurface(void)
{
	return win32_surface;
}

SurfaceFormat & GraphicManager::getSurfaceFormats(void)
{
	// #Init Mode 

	vk::Result result;
	uint32_t formatCount;

	result = gpu->getSurfaceFormatsKHR(win32_surface.surface, &formatCount, nullptr);
	VERIFY(result == vk::Result::eSuccess);

	std::unique_ptr<vk::SurfaceFormatKHR[]> surfFormats(new vk::SurfaceFormatKHR[formatCount]);
	result = gpu->getSurfaceFormatsKHR(win32_surface.surface, &formatCount, surfFormats.get());
	VERIFY(result == vk::Result::eSuccess);

	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == vk::Format::eUndefined)
	{
		surfaceFormat.format = vk::Format::eB8G8R8A8Unorm;
	}
	else
	{
		assert(formatCount >= 1);
		surfaceFormat.format = surfFormats[0].format;
	}

	surfaceFormat.color_space = surfFormats[0].colorSpace;
	surfFormats.reset();

	return surfaceFormat;
}

vk::PhysicalDevice * GraphicManager::getPhysicalDevice(void)
{
	// #Init Mode

	if (gpu == nullptr)
	{
		vk::Result result;
		uint32_t gpu_count = 0;

		result = instance->enumeratePhysicalDevices(&gpu_count, nullptr);
		VERIFY(result == vk::Result::eSuccess);

		std::unique_ptr< vk::PhysicalDevice[] > physical_devices(new vk::PhysicalDevice[gpu_count]);
		result = instance->enumeratePhysicalDevices(&gpu_count, physical_devices.get());
		VERIFY(result == vk::Result::eSuccess);

		gpu = physical_devices.release();

		vk::PhysicalDeviceProperties gpu_props;
		gpu->getProperties(&gpu_props);

		vk::PhysicalDeviceFeatures physDevFeatures;
		gpu->getFeatures(&physDevFeatures);
	}

	return gpu;
}

vk::Device * GraphicManager::createLogicDevice(void)
{
	// #Init Mode 

	uint32_t queue_family_count = 0;
	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t presentQueueFamilyIndex = UINT32_MAX;

	gpu->getQueueFamilyProperties(&queue_family_count, nullptr);

	std::unique_ptr< vk::QueueFamilyProperties[] > queue_props(new vk::QueueFamilyProperties[queue_family_count]);
	gpu->getQueueFamilyProperties(&queue_family_count, queue_props.get());

	// Iterate over each queue to learn whether it supports presenting:
	std::unique_ptr<vk::Bool32[]> supportsPresent(new vk::Bool32[queue_family_count]);
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		gpu->getSurfaceSupportKHR(i, win32_surface.surface, &supportsPresent[i]);
	}

	for (uint32_t i = 0; i < queue_family_count; i++) {
		if (queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			if (graphicsQueueFamilyIndex == UINT32_MAX) {
				graphicsQueueFamilyIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueFamilyIndex = i;
				presentQueueFamilyIndex = i;
				break;
			}
		}
	}

	if (presentQueueFamilyIndex == UINT32_MAX) {
		// If didn't find a queue that supports both graphics and present,
		// then
		// find a separate present queue.
		for (uint32_t i = 0; i < queue_family_count; ++i) {
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueFamilyIndex = i;
				break;
			}
		}
	}

	// Generate error if could not find both a graphics and a present queue
	if (graphicsQueueFamilyIndex == UINT32_MAX ||
		presentQueueFamilyIndex == UINT32_MAX) {
		ERR_EXIT("Could not find both graphics and present queues\n",
			"Swapchain Initialization Failure");
	}


	queue_family->graphics_index = graphicsQueueFamilyIndex;
	queue_family->present_index = presentQueueFamilyIndex;
	queue_family->separate_present_queue = (graphicsQueueFamilyIndex != presentQueueFamilyIndex);


	vk::Result result;
	float const priorities[1] = { 0.0 };
	bool vailidate_device_extensions = false;
	uint32_t device_extension_count = 0;

	char const *const enabledDeviceExtensions[] = {
		"VK_KHR_swapchain" 
		//"VK_NV_glsl_shader" ,
		//"VK_KHR_sampler_mirror_clamp_to_edge",
		//"VK_NV_dedicated_allocation" ,
		//"VK_NV_external_memory" ,
		//"VK_NV_external_memory_win32" ,
		//"VK_NV_win32_keyed_mutex"
	};

	//Look for Device Extensions 
	result = gpu->enumerateDeviceExtensionProperties(nullptr, &device_extension_count, nullptr);
	VERIFY(result == vk::Result::eSuccess);

	if (device_extension_count > 0)
	{
		std::unique_ptr< vk::ExtensionProperties[] > device_extensions(new vk::ExtensionProperties[device_extension_count]);
		result = gpu->enumerateDeviceExtensionProperties(nullptr, &device_extension_count, device_extensions.get());
		VERIFY(result == vk::Result::eSuccess);

		if (vailidate_device_extensions)
		{
			//validate
		}

		//device_extensions.reset();
	}

	vk::DeviceQueueCreateInfo queues[2];
	queues[0].setQueueFamilyIndex( queue_family->graphics_index );
	queues[0].setQueueCount(1);
	queues[0].setPQueuePriorities(priorities);

	uint32_t enabledExtensionCount = ARRAY_SIZE(enabledDeviceExtensions);

	auto deviceInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(queues)
		.setEnabledLayerCount(0)
		.setPpEnabledLayerNames(nullptr)
		.setEnabledExtensionCount( enabledExtensionCount )
		.setPpEnabledExtensionNames(enabledDeviceExtensions)
		.setPEnabledFeatures(nullptr);

	if (queue_family->separate_present_queue)
	{
		queues[1].setQueueFamilyIndex( queue_family->present_index);
		queues[1].setQueueCount(1);
		queues[1].setPQueuePriorities(priorities);
		deviceInfo.setQueueCreateInfoCount(2);
	}

	unique_device.reset( new vk::Device() );
	result = gpu->createDevice(&deviceInfo, nullptr, unique_device.get() );
	device = unique_device.get();
	VERIFY(result == vk::Result::eSuccess);

	unique_graphics_queue.reset(new vk::Queue());
	unique_present_queue.reset(new vk::Queue());

	queue_family->graphics_queue = unique_graphics_queue.get();
	queue_family->present_queue = unique_present_queue.get();

	// Queue Setup 
	device->getQueue( queue_family->graphics_index, 0, queue_family->graphics_queue );

	if (!queue_family->separate_present_queue)
	{
		queue_family->present_queue = queue_family->graphics_queue;
	}
	else
	{
		device->getQueue(queue_family->present_index, 0, queue_family->present_queue );
	}

	return device;
}

void GraphicManager::createSwapchain(void)
{
	// #Prepare Mode 
	
	vk::SwapchainKHR oldSwapchain;

	vk::SurfaceCapabilitiesKHR surfCapabilities;
	auto result = gpu->getSurfaceCapabilitiesKHR(win32_surface.surface, &surfCapabilities);
	VERIFY(result == vk::Result::eSuccess);

	uint32_t presentModeCount;
	result = gpu->getSurfacePresentModesKHR(win32_surface.surface, &presentModeCount, nullptr);
	VERIFY(result == vk::Result::eSuccess);

	presentModes.reset( new vk::PresentModeKHR[presentModeCount] );
	result = gpu->getSurfacePresentModesKHR(win32_surface.surface, &presentModeCount, presentModes.get());
	VERIFY(result == vk::Result::eSuccess);

	vk::Extent2D swapchainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = win32_surface.width;
		swapchainExtent.height = win32_surface.height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
		win32_surface.width = surfCapabilities.currentExtent.width;
		win32_surface.height = surfCapabilities.currentExtent.height;
	}

	// The FIFO present mode is guaranteed by the spec to be supported
	// and to have no tearing.  It's a great default present mode to use.
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

	uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
	// If maxImageCount is 0, we can ask for as many images as we want,
	// otherwise
	// we're limited to maxImageCount
	if ((surfCapabilities.maxImageCount > 0) &&
		(desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount)) {
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
	}

	vk::SurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
	{
		preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else
	{
		preTransform = surfCapabilities.currentTransform;
	}

	auto const swapchain_ci = vk::SwapchainCreateInfoKHR()
		.setSurface(win32_surface.surface)
		.setMinImageCount(desiredNumberOfSwapchainImages)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.color_space)
		.setImageExtent({ swapchainExtent.width, swapchainExtent.height })
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setPreTransform(preTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(swapchainPresentMode)
		.setClipped(true)
		.setOldSwapchain(oldSwapchain);

	if( swapchain != nullptr )
			device->destroySwapchainKHR( *(swapchain) , nullptr);
		

	unique_swapchain.reset(new vk::SwapchainKHR());
	result = device->createSwapchainKHR(&swapchain_ci, nullptr, unique_swapchain.get() );
	swapchain = unique_swapchain.get();

	VERIFY(result == vk::Result::eSuccess);
}

void GraphicManager::createSwapchainbuffers(void)
{
	// #Prepare Mode

	vk::Result result;
	result = device->getSwapchainImagesKHR( *(swapchain) , &swapchainImageCount, nullptr);
	VERIFY(result == vk::Result::eSuccess);

	unique_swapchainImages.reset( new vk::Image[swapchainImageCount] );
	result = device->getSwapchainImagesKHR(*(swapchain), &swapchainImageCount, unique_swapchainImages.get());
	VERIFY(result == vk::Result::eSuccess);

	unique_swapchain_buffers.reset(new SwapchainBuffers[swapchainImageCount]);

	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		auto const color_image_view = vk::ImageViewCreateInfo()
			.setImage( unique_swapchainImages[i] )
			.setViewType(vk::ImageViewType::e2D)
			.setFormat( getSurfaceFormats().format )
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		unique_swapchain_buffers[i].image = unique_swapchainImages[i];

		result = device->createImageView(&color_image_view, nullptr, &unique_swapchain_buffers[i].view);
		VERIFY(result == vk::Result::eSuccess);

		TextureManager::set_image_layout( unique_swapchain_buffers[i].image , vk::ImageAspectFlagBits::eColor,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits());
	}
}


void GraphicManager::createSemaphore(void)
{
	// #Init Mode 
	
	vk::Result result;

	semaphoreStats->quit = false;
	semaphoreStats->curFrame = 0;
	semaphoreStats->frame_index = 0;

	// Create semaphores to synchronize acquiring presentable buffers before
	// rendering and waiting for drawing to be complete before presenting
	auto const semaphoreCreateInfo = vk::SemaphoreCreateInfo();

	// Create fences that we can use to throttle if we get too far
	// ahead of the image presents
	vk::FenceCreateInfo const fence_ci;

	for (uint32_t i = 0; i < FRAME_LAG; i++)
	{
		device->createFence(&fence_ci, nullptr, &semaphoreStats->fences[i]);
		semaphoreStats->fencesInited[i] = false;

		result = device->createSemaphore(&semaphoreCreateInfo, nullptr, &semaphoreStats->image_acquired_semaphores[i]);
		VERIFY(result == vk::Result::eSuccess);

		result = device->createSemaphore(&semaphoreCreateInfo, nullptr, &semaphoreStats->draw_complete_semaphores[i]);
		VERIFY(result == vk::Result::eSuccess);

		if ( queue_family->separate_present_queue)
		{
			result = device->createSemaphore(&semaphoreCreateInfo, nullptr, &semaphoreStats->image_ownership_semaphores[i]);
			VERIFY(result == vk::Result::eSuccess);
		}
	}
}


vk::Result GraphicManager::acquireNextImage(void)
{
	//# Draw Mode 

	vk::Result result;
	uint32_t frame_index = semaphoreStats->frame_index;

	if (semaphoreStats->fencesInited[frame_index])
	{
		device->waitForFences(1 , &semaphoreStats->fences[frame_index], VK_TRUE, UINT64_MAX);
		device->resetFences(  1 , &semaphoreStats->fences[frame_index] );
	}

	result = device->acquireNextImageKHR( *(swapchain) , UINT64_MAX, semaphoreStats->image_acquired_semaphores[frame_index], semaphoreStats->fences[frame_index], &semaphoreStats->current_buffer);
	semaphoreStats->fencesInited[frame_index] = true;

	return result;
}

vk::Result GraphicManager::graphicQueueSubmit(void)
{
	//# Draw Mode 

	vk::Result result;
	uint32_t frame_index		 = semaphoreStats->frame_index;
	uint32_t current_buffer      = semaphoreStats->current_buffer;
	vk::CommandBuffer * cmd_test = &unique_swapchain_buffers[current_buffer].cmd;

	vk::PipelineStageFlags const pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	auto const submit_info = vk::SubmitInfo()
		.setPWaitDstStageMask( &pipe_stage_flags )
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&semaphoreStats->image_acquired_semaphores[frame_index])
		.setCommandBufferCount(1)
		.setPCommandBuffers(cmd_test)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&semaphoreStats->draw_complete_semaphores[frame_index]);

	result = getQueueFamily()->graphics_queue->submit(1, &submit_info, vk::Fence() );
	VERIFY(result == vk::Result::eSuccess);

	auto const presentInfo = vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&semaphoreStats->draw_complete_semaphores[frame_index])
		.setSwapchainCount(1)
		.setPSwapchains( swapchain )
		.setPImageIndices( &current_buffer );

	result = getQueueFamily()->present_queue->presentKHR( &presentInfo );
	
	semaphoreStats->frame_index += 1;
	semaphoreStats->frame_index %= FRAME_LAG;

	return result;
}

vk::Instance * GraphicManager::getInstance(void)
{
	return instance;
}

vk::Device * GraphicManager::getDevice(void)
{
	return device;
}

vk::SwapchainKHR * GraphicManager::getSwapchain(void)
{
	return swapchain;
}

QueueFamilyIndex * GraphicManager::getQueueFamily(void)
{
	return queue_family;
}

SemaphoreStats * GraphicManager::getSemaphore(void)
{
	return semaphoreStats;
}

uint32_t GraphicManager::getSwapchainBufferCount(void)
{
	return swapchainImageCount;
}
SwapchainBuffers * GraphicManager::getSwapchainBuffers()
{
	return unique_swapchain_buffers.get();
}

vk::Bool32 GraphicManager::check_layers(uint32_t check_count,
	char const *const *const check_names,
	uint32_t layer_count, vk::LayerProperties *layers)
{

	for (uint32_t i = 0; i < check_count; i++) {
		vk::Bool32 found = VK_FALSE;
		for (uint32_t j = 0; j < layer_count; j++) {
			if (!strcmp(check_names[i], layers[j].layerName)) {
				found = VK_TRUE;
				break;
			}
		}
		if (!found) {
			fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
			return 0;
		}
	}
	return VK_TRUE;
}

vk::Bool32 GraphicManager::check_extensions(uint32_t check_count,
	char const *const *const check_names,
	uint32_t layer_count, vk::ExtensionProperties *layers)
{

	for (uint32_t i = 0; i < check_count; i++) {
		vk::Bool32 found = VK_FALSE;
		for (uint32_t j = 0; j < layer_count; j++) {
			if (!strcmp(check_names[i], layers[j].extensionName)) {
				found = VK_TRUE;
				break;
			}
		}
		if (!found) {
			fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
			return 0;
		}
	}
	return VK_TRUE;
}


