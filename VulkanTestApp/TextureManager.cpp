#include "stdafx.h"
#include "TextureManager.h"

using namespace Graphic;

DepthImage TextureManager::depth = DepthImage();
Texture TextureManager::textures[1] = { Texture() };
//std::unique_ptr< SwapchainBuffers[] > TextureManager::buffers;
//uint32_t TextureManager::swapchainImageCount;
vk::Result TextureManager::depthImageResult;

TextureManager::TextureManager(){}
TextureManager::~TextureManager(){}

void TextureManager::create_depth_image()
{
	depthImageResult = vk::Result::eNotReady;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	Win32Surface surface = GraphicManager::getSingleton().getSurface();

	vk::Result result;
	uint32_t width  = surface.width;
	uint32_t height = surface.height;

	depth.format = vk::Format::eD16Unorm;

	auto const image = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(depth.format)
		.setExtent({ (uint32_t) width, (uint32_t) height, 1 })
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setInitialLayout(vk::ImageLayout::eUndefined);

	result = device->createImage(&image, nullptr, &depth.image);
	VERIFY(result == vk::Result::eSuccess);

	vk::MemoryRequirements mem_reqs;
	device->getImageMemoryRequirements(depth.image, &mem_reqs);

	depth.mem_alloc.setAllocationSize(mem_reqs.size);
	depth.mem_alloc.setMemoryTypeIndex(0);

	auto const pass = memory_type_from_properties(mem_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits(0), &depth.mem_alloc.memoryTypeIndex);
	VERIFY(pass);

	result = device->allocateMemory(&depth.mem_alloc, nullptr, &depth.mem);
	VERIFY(result == vk::Result::eSuccess);

	result = device->bindImageMemory(depth.image, depth.mem, 0);
	VERIFY(result == vk::Result::eSuccess);

	ImageMemoryBarrierLayout layout;
	layout.image = depth.image;
	layout.aspectMask = vk::ImageAspectFlagBits::eDepth;
	layout.oldLayout = vk::ImageLayout::eUndefined;
	layout.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	layout.srcAccessMask = vk::AccessFlagBits();

	create_image_memory_barrier(layout);

	auto const view = vk::ImageViewCreateInfo()
		.setImage(depth.image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(depth.format)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));

	result = device->createImageView(&view, nullptr, &depth.view);
	VERIFY(result == vk::Result::eSuccess);
}

void TextureManager::destroy_depth_image()
{
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	device->destroyImageView(depth.view, nullptr);
	device->destroyImage(depth.image, nullptr);
	device->freeMemory(depth.mem, nullptr);
}

void TextureManager::destroy_texture_image()
{
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	for ( uint32_t i = 0; i < texture_count; i++) 
	{
		device->destroyImageView(textures[i].view, nullptr);
		device->destroyImage(textures[i].image, nullptr);
		device->freeMemory(textures[i].mem, nullptr);
		device->destroySampler(textures[i].sampler, nullptr);
	}
}

void TextureManager::create_textures()
{
	vk::PhysicalDevice * gpu = GraphicManager::getSingleton().getPhysicalDevice();
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	char const *const tex_files[] = { "lunarg.ppm" };

	bool use_staging_buffer = false;

	vk::Result result;
	vk::FormatProperties props;
	gpu->getFormatProperties(tex_format, &props);

	for (uint32_t i = 0; i < texture_count; i++)
	{

		if ((props.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) && !use_staging_buffer)
		{
			/* Device can texture using linear textures */
			prepare_texture_image(
				tex_files[i], &textures[i],
				vk::ImageTiling::eLinear,
				vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eHostVisible |
				vk::MemoryPropertyFlagBits::eHostCoherent);
		}
		else if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage)
		{
			/* Must use staging buffer to copy linear texture to optimized
			*/
			Texture staging_texture;

			prepare_texture_image(
				tex_files[i], &staging_texture, vk::ImageTiling::eLinear,
				vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible |
				vk::MemoryPropertyFlagBits::eHostCoherent);

			prepare_texture_image(tex_files[i], &textures[i],
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			ImageMemoryBarrierLayout staging_layout;

			staging_layout.image = staging_texture.image;
			staging_layout.aspectMask = vk::ImageAspectFlagBits::eColor;
			staging_layout.oldLayout = staging_texture.imageLayout;
			staging_layout.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			staging_layout.srcAccessMask = vk::AccessFlags();

			create_image_memory_barrier(staging_layout);

			ImageMemoryBarrierLayout texture_layout;

			texture_layout.image = textures[i].image;
			texture_layout.aspectMask = vk::ImageAspectFlagBits::eColor;
			texture_layout.oldLayout = textures[i].imageLayout;
			texture_layout.newLayout = vk::ImageLayout::eTransferDstOptimal;
			texture_layout.srcAccessMask = vk::AccessFlags();

			create_image_memory_barrier(texture_layout);

			auto const subresource =
				vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(0)
				.setBaseArrayLayer(0)
				.setLayerCount(1);

			auto const copy_region =
				vk::ImageCopy()
				.setSrcSubresource(subresource)
				.setSrcOffset({ 0, 0, 0 })
				.setDstSubresource(subresource)
				.setDstOffset({ 0, 0, 0 })
				.setExtent({ (uint32_t)staging_texture.tex_width,
				(uint32_t)staging_texture.tex_height, 1 });

			CommandManager::cmd.copyImage(
				staging_texture.image, vk::ImageLayout::eTransferSrcOptimal,
				textures[i].image, vk::ImageLayout::eTransferDstOptimal, 1,
				&copy_region);

			create_image_memory_barrier(texture_layout);

			//flush
			CommandManager::flush_command_buffer();

			//destroy texture image 
			destroy_texture_image(&staging_texture);
		}
		else
		{
			assert(!"No support for R8G8B8A8_UNORM as texture image format");
		}

		//----------------------------------------------------------------------
		// Create Sampler - aka. Texture Parameter in OpenGL

		auto const samplerInfo = vk::SamplerCreateInfo()

			.setMagFilter(vk::Filter::eNearest)
			.setMinFilter(vk::Filter::eNearest)
			.setMipmapMode(vk::SamplerMipmapMode::eNearest)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setMipLodBias(0.0f)
			.setAnisotropyEnable(VK_FALSE)
			.setMaxAnisotropy(1)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eNever)
			.setMinLod(0.0f)
			.setMaxLod(0.0f)
			.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
			.setUnnormalizedCoordinates(VK_FALSE);

		result = device->createSampler(&samplerInfo, nullptr, &textures[i].sampler);
		VERIFY(result == vk::Result::eSuccess);

		//------------------------------------------------------------------------
		// Create Image View 

		auto const viewInfo = vk::ImageViewCreateInfo()

			.setImage(textures[i].image)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(tex_format)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		result = device->createImageView(&viewInfo, nullptr, &textures[i].view);
		VERIFY(result == vk::Result::eSuccess);

	}// for loop end
}

void TextureManager::destroy_texture_image( Texture * tex_objs) 
{
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	// clean up staging resources
	device->freeMemory(tex_objs->mem, nullptr);
	device->destroyImage(tex_objs->image, nullptr);
}

void TextureManager::create_image_memory_barrier( ImageMemoryBarrierLayout const & layout)
{
	CommandManager::create_command_buffer();

	auto const barrier = vk::ImageMemoryBarrier()
		.setSrcAccessMask(layout.srcAccessMask)
		.setDstAccessMask(layout_access_flags(layout.newLayout))
		.setOldLayout(layout.oldLayout)
		.setNewLayout(layout.newLayout)
		.setSrcQueueFamilyIndex(0)
		.setDstQueueFamilyIndex(0)
		.setImage(layout.image)
		.setSubresourceRange(vk::ImageSubresourceRange(layout.aspectMask, 0, 1, 0, 1));

	   CommandManager::cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlagBits(), 0, nullptr, 0, nullptr, 1,
		&barrier);

	   CommandManager::flush_command_buffer();
}

void TextureManager::set_image_layout(vk::Image image, vk::ImageAspectFlags aspectMask,
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	vk::AccessFlags srcAccessMask) {


	auto DstAccessMask = [](vk::ImageLayout const &layout)
	{
		vk::AccessFlags flags;

		switch (layout) {
		case vk::ImageLayout::eTransferDstOptimal:
			// Make sure anything that was copying from this image has
			// completed
			flags = vk::AccessFlagBits::eTransferRead;
			break;
		case vk::ImageLayout::eColorAttachmentOptimal:
			flags = vk::AccessFlagBits::eColorAttachmentWrite;
			break;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			flags = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			// Make sure any Copy or CPU writes to image are flushed
			flags = vk::AccessFlagBits::eShaderRead |
				vk::AccessFlagBits::eInputAttachmentRead;
			break;
		case vk::ImageLayout::ePresentSrcKHR:
			flags = vk::AccessFlagBits::eMemoryRead;
			break;
		default:
			break;
		}

		return flags;
	};

	auto const barrier = vk::ImageMemoryBarrier()
		.setSrcAccessMask(srcAccessMask)
		.setDstAccessMask(DstAccessMask(newLayout))
		.setOldLayout(oldLayout)
		.setNewLayout(newLayout)
		.setSrcQueueFamilyIndex(0)
		.setDstQueueFamilyIndex(0)
		.setImage(image)
		.setSubresourceRange(vk::ImageSubresourceRange(
			aspectMask, 0, 1, 0, 1));

	CommandManager::create_command_buffer();

	CommandManager::cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlagBits(), 0, nullptr, 0, nullptr, 1,
		&barrier);

	CommandManager::flush_command_buffer();
}

vk::AccessFlags TextureManager::layout_access_flags(vk::ImageLayout const &layout)
{
	vk::AccessFlags flags;

	switch (layout)
	{
	case vk::ImageLayout::eTransferDstOptimal:
		// Make sure anything that was copying from this image has
		// completed
		flags = vk::AccessFlagBits::eTransferRead;
		break;
	case vk::ImageLayout::eColorAttachmentOptimal:
		flags = vk::AccessFlagBits::eColorAttachmentWrite;
		break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		flags = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;
	case vk::ImageLayout::eShaderReadOnlyOptimal:
		// Make sure any Copy or CPU writes to image are flushed
		flags = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;
		break;
	case vk::ImageLayout::ePresentSrcKHR:
		flags = vk::AccessFlagBits::eMemoryRead;
		break;
	default:
		break;
	}

	return flags;
}

void TextureManager::prepare_texture_image(const char *filename, Texture *tex_obj, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags required_props)
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	int32_t tex_width;
	int32_t tex_height;

	if (!loadTexture(filename, nullptr, nullptr, &tex_width, &tex_height)) {
		ERR_EXIT("Failed to load textures", "Load Texture Failure");
	}

	tex_obj->tex_width = tex_width;
	tex_obj->tex_height = tex_height;

	// ----------------------- create image ----------------------------------------
	// -----------------------------------------------------------------------------

	auto const image_create_info =
		vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setExtent({ (uint32_t)tex_width, (uint32_t)tex_height, 1 })
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(tiling)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setInitialLayout(vk::ImageLayout::ePreinitialized);

	result = device->createImage(&image_create_info, nullptr, &tex_obj->image);
	VERIFY(result == vk::Result::eSuccess);

	// ----------------------- attach memory to the image object -------------------
	// -----------------------------------------------------------------------------
	vk::MemoryRequirements mem_reqs;

	// create memory allocation size 
	device->getImageMemoryRequirements(tex_obj->image, &mem_reqs);
	tex_obj->mem_alloc.setAllocationSize(mem_reqs.size);
	tex_obj->mem_alloc.setMemoryTypeIndex(0);

	// check memory properties 
	auto pass = memory_type_from_properties(mem_reqs.memoryTypeBits, required_props, &tex_obj->mem_alloc.memoryTypeIndex);
	VERIFY(pass == true);

	// allocate memory 
	result = device->allocateMemory(&tex_obj->mem_alloc, nullptr, &(tex_obj->mem));
	VERIFY(result == vk::Result::eSuccess);

	// bind memory 
	result = device->bindImageMemory(tex_obj->image, tex_obj->mem, 0);
	VERIFY(result == vk::Result::eSuccess);

	// ----------------------- write rgb data in the memory ------------------------
	// -----------------------------------------------------------------------------

	if (required_props & vk::MemoryPropertyFlagBits::eHostVisible)
	{
		auto const subres = vk::ImageSubresource()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setArrayLayer(0);

		// Get Image Subresource Layout 
		vk::SubresourceLayout layout;
		device->getImageSubresourceLayout(tex_obj->image, &subres, &layout);

		// Get Device Memory Pointer 
		auto data = device->mapMemory(tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize);
		VERIFY(data.result == vk::Result::eSuccess);

		// Read RGBA Data from file and write in the Memory 
		if (!loadTexture(filename, (uint8_t *)data.value, &layout, &tex_width, &tex_height ) ) 
			fprintf(stderr, "Error loading texture: %s\n", filename);
		
		// write finish - unmap memory
		device->unmapMemory(tex_obj->mem);
	}

	//Test Bereich: Get RGBA Data 
	//auto read_data = device->mapMemory(tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize);
	//uint8_t rgba[262144];
	//std::unique_ptr< uint8_t[] > store(new uint8_t(4 * tex_width * tex_height));
	//memcpy(&rgba[0], (const void*)read_data.value, tex_obj->mem_alloc.allocationSize);
	//device->unmapMemory(tex_obj->mem);
	//Test Bereich Ende 

	// ----------------------- make image memory barrier ---------------------------
	// -----------------------------------------------------------------------------

	tex_obj->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	
	set_image_layout(tex_obj->image, vk::ImageAspectFlagBits::eColor,
		vk::ImageLayout::ePreinitialized, tex_obj->imageLayout,
		vk::AccessFlagBits::eHostWrite);

	//tex_obj->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	//ImageMemoryBarrierLayout layout;
	//layout.image = tex_obj->image;
	//layout.aspectMask = vk::ImageAspectFlagBits::eColor;
	//layout.oldLayout = vk::ImageLayout::ePreinitialized;
	//layout.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	//layout.srcAccessMask = vk::AccessFlagBits::eHostWrite;
	//create_image_memory_barrier(layout);
}

bool TextureManager::loadTexture(const char *filename, uint8_t *rgba_data, vk::SubresourceLayout *layout, int32_t *width, int32_t *height) 
{
	FILE *fPtr = fopen(filename, "rb");
	if (!fPtr) {
		return false;
	}

	char header[256];
	char *cPtr = fgets(header, 256, fPtr); // P6
	if (cPtr == nullptr || strncmp(header, "P6\n", 3)) {
		fclose(fPtr);
		return false;
	}

	do {
		cPtr = fgets(header, 256, fPtr);
		if (cPtr == nullptr) {
			fclose(fPtr);
			return false;
		}
	} while (!strncmp(header, "#", 1));

	sscanf(header, "%u %u", width, height);
	if (rgba_data == nullptr) {
		fclose(fPtr);
		return true;
	}

	char *result = fgets(header, 256, fPtr); // Format
	VERIFY(result != nullptr);
	if (cPtr == nullptr || strncmp(header, "255\n", 3)) {
		fclose(fPtr);
		return false;
	}

	for (int y = 0; y < *height; y++) {
		uint8_t *rowPtr = rgba_data;

		for (int x = 0; x < *width; x++) {
			size_t s = fread(rowPtr, 3, 1, fPtr);
			(void)s;
			rowPtr[3] = 255; /* Alpha of 1 */
			rowPtr += 4;
		}

		rgba_data += layout->rowPitch;
	}

	fclose(fPtr);
	return true;
}


bool TextureManager::memory_type_from_properties(uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t *typeIndex) 
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
