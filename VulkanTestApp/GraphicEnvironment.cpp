#include "stdafx.h"
#include "GraphicEnvironment.h"

using namespace Graphic;

GraphicEnvironment::GraphicEnvironment( uint32_t pwidth , uint32_t pheight )
	: renderPass(nullptr) , width(pwidth) , height(pheight)
{}

GraphicEnvironment::~GraphicEnvironment(){}

uint32_t GraphicEnvironment::getWidth(void)
{
	return width;
}

uint32_t GraphicEnvironment::getHeight(void)
{
	return height;
}

vk::Framebuffer *  GraphicEnvironment::getFramebuffers()
{
	return framebuffers.get();
}

vk::RenderPass * GraphicEnvironment::getRenderPass(void)
{
	return renderPass;
}

void GraphicEnvironment::createRenderPass(vk::RenderPassCreateInfo * renderPassCreateInfo)
{
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::Format depth_format = vk::Format::eD16Unorm;

	const vk::AttachmentDescription attachments[2] = {
		
		vk::AttachmentDescription()
		.setFlags(vk::AttachmentDescriptionFlagBits::eMayAlias)
		.setFormat(format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR),

		vk::AttachmentDescription()
		.setFlags(vk::AttachmentDescriptionFlagBits::eMayAlias)
		.setFormat( depth_format )
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout( vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setFinalLayout( vk::ImageLayout::eDepthStencilAttachmentOptimal) 
	};

	auto const color_reference =
		vk::AttachmentReference().setAttachment(0).setLayout( vk::ImageLayout::eColorAttachmentOptimal );

	auto const depth_reference =
		vk::AttachmentReference().setAttachment(1).setLayout( vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto const subpass =
		vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setInputAttachmentCount(0)
		.setPInputAttachments(nullptr)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_reference)
		.setPResolveAttachments(nullptr)
		.setPDepthStencilAttachment(&depth_reference)
		.setPreserveAttachmentCount(0)
		.setPPreserveAttachments(nullptr);

	auto const rp_info = vk::RenderPassCreateInfo()
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(0)
		.setPDependencies(nullptr);


	renderPass = new vk::RenderPass();
	auto result = device->createRenderPass( &rp_info , nullptr, renderPass );
	VERIFY(result == vk::Result::eSuccess);
}

void Graphic::GraphicEnvironment::createFrameBuffers()
{
	vk::Result result;
	GraphicManager * manager = GraphicManager::getSingletonPtr();
	vk::Device     * device  = manager->getDevice();
	uint32_t swapchainBufferCount = manager->getSwapchainBufferCount();
	SwapchainBuffers * swapchainBuffer = manager->getSwapchainBuffers();

	vk::ImageView attachments[2];

	auto const fb_info = vk::FramebufferCreateInfo()
		.setRenderPass( *(renderPass) )
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setWidth((uint32_t)width)
		.setHeight((uint32_t)height)
		.setLayers(1);

	framebuffers.reset(new vk::Framebuffer[ swapchainBufferCount ]);

	for (uint32_t i = 0; i < swapchainBufferCount; i++)
	{
		attachments[0] = swapchainBuffer[i].view;
		attachments[1] = TextureManager::depth.view;
		
		result = device->createFramebuffer(&fb_info, nullptr, &framebuffers[i]);
		VERIFY(result == vk::Result::eSuccess);
	}
}

void GraphicEnvironment::defaultFrameBuffers()
{
	GraphicManager * manager = GraphicManager::getSingletonPtr();
	uint32_t swapchainBufferCount = manager->getSwapchainBufferCount();
	SwapchainBuffers * swapchainBuffer = manager->getSwapchainBuffers();

	framebuffers.reset(new vk::Framebuffer[swapchainBufferCount]);
	
	uint32_t attachmentCount = 2;
	std::unique_ptr< vk::ImageView[] > attachments( new vk::ImageView[attachmentCount] );

	for (uint32_t i = 0; i < swapchainBufferCount; i++)
	{
		attachments[0] = swapchainBuffer[i].view;
		attachments[1] = TextureManager::depth.view;

		createFrameBuffer(attachmentCount, attachments.get(), &framebuffers[i]);
	}

	attachments.reset();
}

void GraphicEnvironment::createFrameBuffer( uint32_t attachmentCount , vk::ImageView * attachmentImage ,  vk::Framebuffer * frame_buffer )
{
	VERIFY(attachmentImage != nullptr);
	VERIFY(frame_buffer != nullptr);
	VERIFY(renderPass != nullptr);
	VERIFY(attachmentCount > 0);

	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	auto const fb_info = vk::FramebufferCreateInfo()
		.setRenderPass(*(renderPass))
		.setAttachmentCount(attachmentCount)
		.setPAttachments(attachmentImage)
		.setWidth((uint32_t)width)
		.setHeight((uint32_t)height)
		.setLayers(1);

	result = device->createFramebuffer(&fb_info, nullptr, frame_buffer);
	VERIFY(result == vk::Result::eSuccess);
}

void GraphicEnvironment::createFrameBufferEx( vk::FramebufferCreateInfo * fb_info , vk::Framebuffer * frame_buffer )
{
	VERIFY(frame_buffer != nullptr);
	VERIFY(fb_info != nullptr);

	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	result = device->createFramebuffer(fb_info, nullptr, frame_buffer);
	VERIFY(result == vk::Result::eSuccess);
}



void GraphicEnvironment::destroyEnvironment()
{

}
