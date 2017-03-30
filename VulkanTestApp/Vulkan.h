#pragma once
#include "stdafx.h"
#include "pipeline.h"
#include "GraphicEnvironment.h"
#include "Graphicpipeline.h"
#include "GraphicShader.h"
#include "GraphicEventListener.h"

#include "linmath.h"

namespace Graphic
{
	typedef struct
	{
		// Must start with MVP
		float mvp[4][4];
		float position[12 * 3][4];
		float attr[12 * 3][4];

	} TEST_UNIFORM_STRUCT;

	static const float g_vertex_buffer_data[] = {
		-1.0f,-1.0f,-1.0f,  // -X side
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,

		-1.0f,-1.0f,-1.0f,  // -Z side
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,

		-1.0f,-1.0f,-1.0f,  // -Y side
		1.0f,-1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,

		-1.0f, 1.0f,-1.0f,  // +Y side
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,

		1.0f, 1.0f,-1.0f,  // +X side
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,

		-1.0f, 1.0f, 1.0f,  // +Z side
		-1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
	};

	static const float g_uv_buffer_data[] = {
		0.0f, 1.0f,  // -X side
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 1.0f,  // -Z side
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		1.0f, 0.0f,  // -Y side
		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,

		1.0f, 0.0f,  // +Y side
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,

		1.0f, 0.0f,  // +X side
		0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,

		0.0f, 0.0f,  // +Z side
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	class Vulkan : public GraphicEventListener
	{
	public:
		Vulkan();
		~Vulkan();

		void init();
		void prepare();
		void update(float delta);
		void run();
		void cleanup();
		void resize();

	private:
		Pipeline * graphicPipe;
		
		mat4x4 projection_matrix;
		mat4x4 model_matrix;
		mat4x4 view_matrix;

		bool isInitialized;
	};

}

