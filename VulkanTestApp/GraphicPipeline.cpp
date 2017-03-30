#include "stdafx.h"
#include "GraphicPipeline.h"

using namespace Graphic;

GraphicPipeline::GraphicPipeline(){}
GraphicPipeline::~GraphicPipeline(){}

void GraphicPipeline::setShaderStageInfo(vk::PipelineShaderStageCreateInfo * shaderStageCreateInfo, uint32_t stageCount)
{
	shaderStageInfo = shaderStageCreateInfo;
	shaderStageCount = stageCount;
}


void GraphicPipeline::createLayoutBindings(DescriptorSetLayoutInfo const & info, uint32_t descriptorCount, vk::DescriptorSetLayoutBinding * layout_output)
{
	uint32_t layout_counter = 0;

	// create uniform binding layouts  
	for (uint32_t uniform_count = 0; uniform_count < info.uniformInfo.uniformCount; uniform_count++ , layout_output++)
	{
		layout_output->setBinding(info.uniformInfo.uniformBindLocations[uniform_count]);
		layout_output->setDescriptorType(vk::DescriptorType::eUniformBuffer);
		layout_output->setDescriptorCount(1);
		layout_output->setStageFlags(vk::ShaderStageFlagBits::eVertex);
		layout_output->setPImmutableSamplers(nullptr);

		layout_counter++;
	}
	// create image sampler binding layouts 
	for (uint32_t texture_buffer_count = 0; texture_buffer_count < info.textureInfo.textureCount; texture_buffer_count++, layout_output++)
	{
		layout_output->setBinding(info.textureInfo.textureBindLocations[texture_buffer_count]);
		layout_output->setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		layout_output->setDescriptorCount(1);
		layout_output->setStageFlags(vk::ShaderStageFlagBits::eFragment);
		layout_output->setPImmutableSamplers(nullptr);

		layout_counter++;
	}

	VERIFY(layout_counter == descriptorCount);
}

void GraphicPipeline::createPipelineLayout( DescriptorSetLayoutInfo const & info )
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	uint32_t descriptorCount = info.uniformInfo.uniformCount + info.textureInfo.textureCount;

	//allocate DescriptorSetLayoutBindings 
	std::unique_ptr< vk::DescriptorSetLayoutBinding[] > layoutBindings( new vk::DescriptorSetLayoutBinding[ descriptorCount ] );
	
	//create uniform | image sampler layouts 
	//createLayoutBindings(info, descriptorCount, layoutBindings.get());

	vk::DescriptorSetLayoutBinding const layout_bindings[2] = {
		
		vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(nullptr),

		vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment)
		.setPImmutableSamplers(nullptr) 
	};


	//create DescriptorSetLayout 
	auto const descriptor_layout_info = vk::DescriptorSetLayoutCreateInfo().setBindingCount(2).setPBindings( layout_bindings );
	result = device->createDescriptorSetLayout(&descriptor_layout_info, nullptr, &desc_layout);
	VERIFY(result == vk::Result::eSuccess);

	auto const pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(1).setPSetLayouts(&desc_layout);

	result = device->createPipelineLayout(&pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	VERIFY(result == vk::Result::eSuccess);

	layoutBindings.reset();

}

void GraphicPipeline::createDescriptorPool( DescriptorSetLayoutInfo const & info )
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	vk::DescriptorPoolSize const descriptor_pool[2] = {
		
		   vk::DescriptorPoolSize()
				.setType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1) , // info.uniformInfo.uniformCount 
		  vk::DescriptorPoolSize()
				.setType( vk::DescriptorType::eCombinedImageSampler )
				.setDescriptorCount(1) //info.textureInfo.textureCount
	 };

	auto const descriptor_pool_info = vk::DescriptorPoolCreateInfo()
		.setMaxSets(1)
		.setPoolSizeCount( 2 )
		.setPPoolSizes( descriptor_pool );

	result = device->createDescriptorPool(&descriptor_pool_info, nullptr, &desc_pool);
	VERIFY(result == vk::Result::eSuccess);
}


void GraphicPipeline::setEnvironment(GraphicEnvironment * environment)
{
	setRenderPass(environment->getRenderPass());

	this->environment = environment;
}

void GraphicPipeline::setRenderPass( vk::RenderPass * pass )
{
	renderPass = pass;
}

vk::Pipeline * GraphicPipeline::createPipeline( DescriptorSetLayoutInfo const & descriptor_layout_info )
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	//Make Pipeline Layout 
	createPipelineLayout( descriptor_layout_info );

	vk::PipelineCacheCreateInfo const pipelineCacheInfo;
	result = device->createPipelineCache(&pipelineCacheInfo, nullptr, &pipelineCache);
	VERIFY(result == vk::Result::eSuccess);

	vk::PipelineVertexInputStateCreateInfo const vertexInputInfo;

	auto const inputAssemblyInfo =
		vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

	auto const viewportInfo = vk::PipelineViewportStateCreateInfo()
		.setViewportCount(1)
		.setScissorCount(1);

	auto const multisampleInfo = vk::PipelineMultisampleStateCreateInfo();

	auto const stencilOp = vk::StencilOpState()
		.setFailOp(vk::StencilOp::eKeep)
		.setPassOp(vk::StencilOp::eKeep)
		.setCompareOp(vk::CompareOp::eAlways);

	auto const depthStencilInfo =
		vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setStencilTestEnable(VK_FALSE)
		.setFront(stencilOp)
		.setBack(stencilOp);

	vk::PipelineColorBlendAttachmentState const colorBlendAttachments[1] = {
		vk::PipelineColorBlendAttachmentState().setColorWriteMask(
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA) };


	auto const colorBlendInfo = vk::PipelineColorBlendStateCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(colorBlendAttachments);

	vk::DynamicState const dynamicStates[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	auto const dynamicStateInfo = vk::PipelineDynamicStateCreateInfo()
		.setPDynamicStates(dynamicStates)
		.setDynamicStateCount(2);

	auto const rasterInfo = vk::PipelineRasterizationStateCreateInfo()
				.setDepthClampEnable(VK_FALSE)
				.setRasterizerDiscardEnable(VK_FALSE)
				.setPolygonMode(vk::PolygonMode::eFill)
				.setCullMode(vk::CullModeFlagBits::eBack)
				.setFrontFace(vk::FrontFace::eCounterClockwise)
				.setDepthBiasEnable(VK_FALSE)
				.setLineWidth(1.0f);


	auto const pipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setStageCount( shaderStageCount )						// Shader Anzahl: Vertex + Fragment (2)
		.setPStages( shaderStageInfo )							// Shader Information
		.setPVertexInputState(&vertexInputInfo)					// ??? - Vertex Data only direct?? maybe .. sind aber in den Uniforms
		.setPInputAssemblyState(&inputAssemblyInfo)				// Draw with Triangles 
		.setPViewportState(&viewportInfo)						// Anzahl of Viewports & Scissors 
		.setPRasterizationState(&rasterInfo)					// Pixel verarbeitung 
		.setPMultisampleState(&multisampleInfo)					// ??? - hat was mit MultiSampler Images zu tun 
		.setPDepthStencilState(&depthStencilInfo)				// Depth & Stencil Settings ( DepthTestEnable = true , dan ist der 3D Modus aktiv )
		.setPColorBlendState(&colorBlendInfo)					// ??? - Colour Blend Settings ( OpenGL hatte das auch - muss ich noch mal nach Lesen für was das war )
		.setPDynamicState(&dynamicStateInfo)					// Dynamische Zustände Aktivieren ( ViewPort & Scissor must have )
		.setLayout(pipelineLayout)								// Pipeline Layout: binding locations von Buffern ( uniform, images ) 
		.setRenderPass( *(renderPass) );						// Colour , Depth Framebuffer Image Attachments Settings 

	unique_pipeline.reset(new vk::Pipeline());
	result = device->createGraphicsPipelines(pipelineCache, 1, &pipelineInfo, nullptr, unique_pipeline.get() );
	VERIFY(result == vk::Result::eSuccess);

	updateDescriptorSets( descriptor_layout_info );
	createDrawBuild( environment );

	return unique_pipeline.get();
}

UniformCollection GraphicPipeline::getUniformBuffers(void)
{
	return uniforms;
}

vk::Pipeline * GraphicPipeline::getPipeline(void)
{
	return unique_pipeline.get();
}

void GraphicPipeline::createDrawBuild( GraphicEnvironment * environment )
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	QueueFamilyIndex * queue_family = GraphicManager::getSingleton().getQueueFamily();
	
	vk::CommandPool cmd_pool;
	auto const cmd_pool_info = vk::CommandPoolCreateInfo().setQueueFamilyIndex(queue_family->graphics_index);
	result = device->createCommandPool(&cmd_pool_info, nullptr, &cmd_pool);
	VERIFY(result == vk::Result::eSuccess);
	
	auto const cmd_info = vk::CommandBufferAllocateInfo()
		.setCommandPool( cmd_pool )
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	uint32_t swapchainImageCount = GraphicManager::getSingletonPtr()->getSwapchainBufferCount();
	SwapchainBuffers * buffers   = GraphicManager::getSingletonPtr()->getSwapchainBuffers();

	for (uint32_t i = 0; i < swapchainImageCount; ++i) 
	{
		result = device->allocateCommandBuffers(&cmd_info , &buffers[i].cmd);
		VERIFY(result == vk::Result::eSuccess);

		draw_build_cmd( &buffers[i] , i ,  environment );
	}
}

void GraphicPipeline::draw_build_cmd(SwapchainBuffers * swapchain_buffer , uint32_t current_buffer ,  GraphicEnvironment  * environment)
{
	uint32_t width  = environment->getWidth();
	uint32_t height = environment->getHeight();

	auto const commandInfo = vk::CommandBufferBeginInfo().setFlags( vk::CommandBufferUsageFlagBits::eSimultaneousUse );

	 vk::ClearValue const clearValues[2] = {
		vk::ClearColorValue( std::array<float, 4> ({ 0.2f, 0.2f, 0.2f, 0.2f }) ),
		vk::ClearDepthStencilValue(1.0f, 0u) 
	 };

	 vk::Framebuffer framebuffer = environment->getFramebuffers()[current_buffer];
	 vk::Rect2D rect = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height));

	 vk::CommandBuffer * commandBuffer = &swapchain_buffer->cmd;
	 vk::Image swapchain_image		   =  swapchain_buffer->image;

	 auto const passInfo = vk::RenderPassBeginInfo()
		 .setRenderPass( *(renderPass) )
		 .setFramebuffer( framebuffer )
		 .setRenderArea( rect )
		 .setClearValueCount(2)
		 .setPClearValues(clearValues);


	 auto result = commandBuffer->begin(&commandInfo);
	 VERIFY(result == vk::Result::eSuccess);


	 auto const image_memory_barrier = vk::ImageMemoryBarrier()
		 .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
		 .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		 .setOldLayout(vk::ImageLayout::ePresentSrcKHR)
		 .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
		 .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		 .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		 .setImage( swapchain_image )
		 .setSubresourceRange(vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	 commandBuffer->pipelineBarrier(
		 vk::PipelineStageFlagBits::eColorAttachmentOutput,
		 vk::PipelineStageFlagBits::eColorAttachmentOutput,
		 vk::DependencyFlagBits(), 0, nullptr, 0, nullptr, 1,
		 &image_memory_barrier);

	 commandBuffer->beginRenderPass(&passInfo, vk::SubpassContents::eInline);
	 commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *( unique_pipeline.get() ) );
	 commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &desc_set , 0, nullptr);

	 auto const viewport = vk::Viewport()
		 .setWidth((float) width )
		 .setHeight((float) height )
		 .setMinDepth((float)0.0f)
		 .setMaxDepth((float)1.0f);

	 commandBuffer->setViewport(0, 1, &viewport);

	 vk::Rect2D const scissor(vk::Offset2D(0, 0), vk::Extent2D(width, height));
	 commandBuffer->setScissor(0, 1, &scissor);

	 commandBuffer->draw(12 * 3, 1, 0, 0);
	 commandBuffer->endRenderPass();

	 result = commandBuffer->end();
	 VERIFY(result == vk::Result::eSuccess);
}

void GraphicPipeline::create_descriptor_set_uniforms( DescriptorSetLayoutInfo const & info )
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	
	uint32_t start_count = 0;
	uint32_t end_count = info.uniformInfo.uniformCount;
	uint32_t dest_binding = 0;

	vk::WriteDescriptorSet uniform_write;
	UniformBufferData * uniform_data = info.uniformInfo.uniformData;
	GraphicUniform * graphic_uniform = nullptr;
	vk::DescriptorBufferInfo * pBufferInfo = nullptr;

	for (start_count = 0; start_count < end_count; start_count++ , uniform_data++)
	{
		graphic_uniform = new GraphicUniform();
		pBufferInfo		= graphic_uniform->createBuffer(uniform_data->dataPtr, uniform_data->size_of_data);
	
		uniform_write.setDstSet(desc_set);										 // bereits allocate DescriptorSet
		uniform_write.setDstBinding( 0 );										 // DescriptorSet binding location(0)
	
		uniform_write.setDescriptorType(vk::DescriptorType::eUniformBuffer);	 // es handelt sich um einen Uniform Buffer 
		uniform_write.setDescriptorCount(1);									 // die Anzahl der UniformBuffern sind hier gemeint  
		uniform_write.setPBufferInfo( pBufferInfo );							 // Uniform Buffer Information ( kann auch ein Array sein )

		device->updateDescriptorSets( 1 , &uniform_write , 0 , nullptr);		 // Update / Write die Information in den Descriptor Set 
		uniforms.push_back( graphic_uniform );
	}
}

void GraphicPipeline::create_descriptor_set_image(DescriptorSetLayoutInfo const & info)
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	
	uint32_t start_count = 0;
	uint32_t end_count = info.textureInfo.textureCount;
	uint32_t dest_binding = 0;

	vk::WriteDescriptorSet write_image;
	vk::DescriptorImageInfo pImageInfo;

	for (start_count = 0; start_count < end_count; start_count++)
	{
		pImageInfo.setSampler(info.textureInfo.textures[start_count].sampler);
		pImageInfo.setImageView(info.textureInfo.textures[start_count].view);
		pImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		write_image.setDstSet(desc_set);												// bereits allocate DescriptionSet 
		write_image.setDstBinding(0);													// DescriptorSet binding location(0)
		
		write_image.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);		// Descriptor Buffer Type: Image 
		write_image.setDescriptorCount(1);											    // Anzahl der Images 
		write_image.setPImageInfo( &pImageInfo );										// Information von den Images ( kann auch ein Array sein )

		device->updateDescriptorSets(1, &write_image , 0, nullptr);						// Update / Write die Information in den Descriptor Set 
	}
}

void GraphicPipeline::updateDescriptorSets(DescriptorSetLayoutInfo const & info)
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	createDescriptorPool( info );

	auto const alloc_info = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(desc_pool)									// In dem Descriptor Pool befindet sich die Anzahl der Uniform & Image Buffer 
																		//  ||-> ist definiert: Maximum Descriptor Set Anzahl die erstellt werden dürfen (1)
		.setDescriptorSetCount(1)										// Die Anzahl der Descriptor Sets ( die gerade erstellt werden ) (1)
		.setPSetLayouts(&desc_layout);									// Description Layout: beinhaltet die Buffer Binding Locations (0) = Uniform Buffer , (1) Image 

	result = device->allocateDescriptorSets(&alloc_info, &desc_set);	// desc_set = das erstelle Descriptor Set 
	VERIFY(result == vk::Result::eSuccess);

	create_descriptor_set_uniforms(info);								// Update das Descriptor Set mit (1) Uniform Buffer
	create_descriptor_set_image(info);									// Update das Descriptor Set mit (1) Image Buffer 
}

void GraphicPipeline::destroyPipeline(void)
{
	vk::Device * device = GraphicManager::getSingleton().getDevice();

	device->destroyPipeline( *( unique_pipeline.get() ) , nullptr );

	device->destroyDescriptorPool(desc_pool, nullptr);          //check
	device->destroyDescriptorSetLayout(desc_layout, nullptr);   //check
	device->destroyPipelineCache(pipelineCache, nullptr);		//check
	device->destroyPipelineLayout(pipelineLayout, nullptr);		//check
}