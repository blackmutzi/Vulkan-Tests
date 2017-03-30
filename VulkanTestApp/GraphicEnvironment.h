#pragma once
#include "stdafx.h"
#include "GraphicManager.h"
#include "TextureManager.h"

namespace Graphic
{
	class GraphicEnvironment
	{

	public:
		GraphicEnvironment(uint32_t width , uint32_t height);
		~GraphicEnvironment();

		void createRenderPass(vk::RenderPassCreateInfo * renderPassCreateInfo);
		void createFrameBuffer(uint32_t attachmentCount, vk::ImageView * attachmentImage, vk::Framebuffer * frame_buffer);
		void createFrameBufferEx(vk::FramebufferCreateInfo * fb_info, vk::Framebuffer * frame_buffer);


		void defaultFrameBuffers(void);



		void createFrameBuffers(void);

		vk::RenderPass * getRenderPass(void);
		vk::Framebuffer * getFramebuffers();
		uint32_t getWidth();
		uint32_t getHeight();

		void destroyEnvironment();

	protected:

		uint32_t width, height;
		std::unique_ptr< vk::Framebuffer[] > framebuffers;
		vk::RenderPass * renderPass;

	};

}

