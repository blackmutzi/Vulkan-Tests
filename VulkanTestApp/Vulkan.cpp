#include "Vulkan.h"

using namespace Graphic;

Vulkan::Vulkan()
	:isInitialized(false)
{}
Vulkan::~Vulkan(){}

void Vulkan::init()
{
	graphicPipe = new Pipeline();
	graphicPipe->init();
	graphicPipe->setGraphicEventListener(this);

	isInitialized = true;
}

void Vulkan::prepare()
{
    //Create Textures in Prepare 
	graphicPipe->prepareInit();

	TextureManager::create_depth_image();
	TextureManager::create_textures();

	// ---------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------
	// Prepare Uniform Buffers / Image Buffers

	uint32_t uniform_location[1] = { 0 };
	uint32_t texture_location[1] = { 1 };

	uint32_t uniform_count = 1;
	std::unique_ptr< UniformBufferData[] > uniformData(new UniformBufferData[uniform_count]);

	TEST_UNIFORM_STRUCT data;
	mat4x4 VP, MVP;

	vec3 eye = { 0.0f, 3.0f, 5.0f };
	vec3 origin = { 0, 0, 0 };
	vec3 up = { 0.0f, 1.0f, 0.0 };

	//spin_angle = 4.0f;
	//spin_increment = 0.2f;
	//pause = false;

	mat4x4_perspective(projection_matrix, (float)degreesToRadians(45.0f), 1.0f, 0.1f, 100.0f);
	mat4x4_look_at(view_matrix, eye, origin, up);
	mat4x4_identity(model_matrix);

	projection_matrix[1][1] *= -1; // Flip projection matrix from GL to Vulkan orientation.

	mat4x4_mul(VP, projection_matrix, view_matrix);
	mat4x4_mul(MVP, VP, model_matrix);

	memcpy(data.mvp, MVP, sizeof(MVP));

	for (int32_t i = 0; i < 12 * 3; i++) 
	{
		data.position[i][0] = g_vertex_buffer_data[i * 3];
		data.position[i][1] = g_vertex_buffer_data[i * 3 + 1];
		data.position[i][2] = g_vertex_buffer_data[i * 3 + 2];
		data.position[i][3] = 1.0f;
		data.attr[i][0] = g_uv_buffer_data[2 * i];
		data.attr[i][1] = g_uv_buffer_data[2 * i + 1];
		data.attr[i][2] = 0;
		data.attr[i][3] = 0;
	}

	uniformData[0].dataPtr = &data;
	uniformData[0].size_of_data = sizeof(data);

	DescriptorSetLayoutInfo assembler_info = DescriptorSetLayoutInfo();
	assembler_info.uniformInfo.uniformCount = uniform_count;
	assembler_info.uniformInfo.uniformBindLocations = uniform_location;
	assembler_info.uniformInfo.uniformData = uniformData.release();

	assembler_info.textureInfo.textureCount = 1;
	assembler_info.textureInfo.textureBindLocations = texture_location;
	assembler_info.textureInfo.textures = &TextureManager::textures[0];

	// ---------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------
	// Prepare Shader Code for Graphic Pipeline 
	GraphicShader shader;
	shader.createShaderStageInfo(nullptr, 0, "cube-vert.spv", "cube-frag.spv");

	// ---------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------
	// Prepare Environment like Renderpass for GraphicPipeline

	GraphicManager * manager = GraphicManager::getSingletonPtr();

	GraphicEnvironment * env = new GraphicEnvironment( manager->getSurface().width , manager->getSurface().height );
	env->createRenderPass(nullptr);         // use default: only Demo version
	env->defaultFrameBuffers();

	graphicPipe->addEnvironment( env);

	// ---------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------
	// Create Graphic Pipeline  
	GraphicPipeline * pipeline = new GraphicPipeline();
	pipeline->setEnvironment( env );
	pipeline->setShaderStageInfo(shader.getShaderStageInfo(), shader.getShaderStageCount());
	pipeline->createPipeline(assembler_info);

	graphicPipe->addPipeline(pipeline);
}

void Vulkan::update(float delta)
{
	//for (GraphicPipeline * pipeline : graphicPipe->getGraphicPipelines())
	//{
		//for (GraphicUniform uniform_buffer : pipeline->getUniformBuffers() )
		//{
			//only Cube Rotation

		//	float spin_angle = 4.0f;
		//		float spin_increment = 0.2f;

		//	mat4x4 VP;
	    //		mat4x4_mul(VP, projection_matrix, view_matrix);

			// Rotate 22.5 degrees around the Y axis
		//	mat4x4 Model;
		//	mat4x4_dup(Model, model_matrix);
		//	mat4x4_rotate(model_matrix, Model, 0.0f, 1.0f, 0.0f, (float) degreesToRadians(spin_angle));

		//	mat4x4 MVP;
		//		mat4x4_mul(MVP, VP, model_matrix);
		//	uniform_buffer.updateBuffer( &MVP[0][0] , sizeof(MVP) );
		//}
	//}
}

void Vulkan::run()
{
	update(1.0f);
	graphicPipe->draw();
}

void Vulkan::resize()
{
	if (!isInitialized)
				return;

	graphicPipe->resize();
}

void Vulkan::cleanup()
{
	graphicPipe->finish();
}



