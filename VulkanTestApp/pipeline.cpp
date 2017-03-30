#include "stdafx.h"
#include "pipeline.h"

using namespace Graphic;

Pipeline::Pipeline() {}
Pipeline::~Pipeline(){}

void Pipeline::init()
{
	GraphicManager * manager = GraphicManager::getSingletonPtr();

	manager->getPhysicalDevice();
	manager->getSurfaceFormats();

	manager->createLogicDevice();
	manager->createSemaphore();
}

void Pipeline::addEnvironment(GraphicEnvironment * env)
{
	environments.push_back(env);
}

void Pipeline::addPipeline(GraphicPipeline * pipeline)
{
	pipelines.push_back(pipeline);
}

void Pipeline::setGraphicEventListener(GraphicEventListener * listener)
{
	this->eventListener = listener;
}

void Pipeline::prepareInit()
{
	GraphicManager::getSingletonPtr()->createSwapchain();
	GraphicManager::getSingletonPtr()->createSwapchainbuffers();
}

void Pipeline::draw()
{
	vk::Result result;
	GraphicManager * manager = GraphicManager::getSingletonPtr();
	
	result = manager->acquireNextImage();
	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		// swapchain is out of date (e.g. the window was resized) and
		// must be recreated:

		manager->getSemaphore()->frame_index += 1;
		manager->getSemaphore()->frame_index %= FRAME_LAG;

		resize();
		draw();

		return;
	}
	VERIFY(result == vk::Result::eSuccess);

	//Graphic Queue Submit 
	result = manager->graphicQueueSubmit();
	if (result == vk::Result::eErrorOutOfDateKHR) 
	{
		// swapchain is out of date (e.g. the window was resized) and
		// must be recreated:
		resize();
	}
	VERIFY(result == vk::Result::eSuccess);
}

void Pipeline::resize()
{
	if ( pipelines.empty() || environments.empty() )
			return;

	this->finish();

	if (this->eventListener != nullptr)
			this->eventListener->prepare();
}

void Pipeline::update(float delta)
{
	if (this->eventListener != nullptr)
			this->eventListener->update(delta);
}

void Pipeline::finish()
{
	vk::Result result;
	vk::Device * device = GraphicManager::getSingleton().getDevice();
	result = device->waitIdle();
	VERIFY(result == vk::Result::eSuccess);

	TextureManager::destroy_depth_image();
	TextureManager::destroy_texture_image();


	// destroy graphic pipelines 
	for (GraphicPipeline * pipeline : pipelines)
	{
		pipeline->destroyPipeline();

		// destroy uniform buffers 
		for (GraphicUniform * uniform : pipeline->getUniformBuffers())
		{
			uniform->destroyBuffer();
		}

		delete pipeline;
	}

	// destroy Environments
	for (GraphicEnvironment * env : environments)
	{
		env->destroyEnvironment();

		delete env;
	}
}

Pipelines Pipeline::getGraphicPipelines(void)
{
	return pipelines;
}
