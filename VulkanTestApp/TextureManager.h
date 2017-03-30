#pragma once

#include "stdafx.h"
#include "GraphicManager.h"
#include "CommandManager.h"

namespace Graphic
{
	typedef struct
	{
		vk::Image image;
		vk::ImageAspectFlags aspectMask;
		vk::ImageLayout oldLayout;
		vk::ImageLayout newLayout;
		vk::AccessFlags srcAccessMask;

	} ImageMemoryBarrierLayout;

	typedef struct
	{
		vk::Sampler sampler;
		vk::Image image;
		vk::ImageLayout imageLayout;

		vk::MemoryAllocateInfo mem_alloc;
		vk::DeviceMemory mem;
		vk::ImageView view;

		int32_t tex_width;
		int32_t tex_height;

	} Texture ;

	typedef struct 
	{
		vk::Format format;
		vk::Image image;
		vk::MemoryAllocateInfo mem_alloc;
		vk::DeviceMemory mem;
		vk::ImageView view;

	} DepthImage;

	static const vk::Format tex_format = vk::Format::eR8G8B8A8Unorm;
	
	class TextureManager
	{
		public:
			TextureManager();
			~TextureManager();

			static void create_depth_image();
			static void create_textures();

			static void destroy_depth_image();

			static void destroy_texture_image(Texture * tex_objs);
			static void destroy_texture_image();

			static void create_image_memory_barrier(ImageMemoryBarrierLayout const & layout);
			
			static void prepare_texture_image(const char *filename, Texture *tex_obj, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags required_props);
			
			static bool loadTexture(const char *filename, uint8_t *rgba_data, vk::SubresourceLayout *layout, int32_t *width, int32_t *height);

			static vk::AccessFlags layout_access_flags(vk::ImageLayout const &layout);

			static DepthImage depth;
			static Texture textures[1];
			static const int32_t texture_count = 1;
			static vk::Result depthImageResult;

			static void set_image_layout(vk::Image image, vk::ImageAspectFlags aspectMask,
				vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
				vk::AccessFlags srcAccessMask);

		protected:

			static bool memory_type_from_properties(uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t *typeIndex);
	};

}

