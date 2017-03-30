#pragma once
#include "stdafx.h"

#include "CommandManager.h"
#include "TextureManager.h"
#include "GraphicPipeline.h"
#include "GraphicEnvironment.h"
#include "GraphicEventListener.h"
#include "GraphicManager.h"

namespace Graphic
{
	using Pipelines = std::vector< GraphicPipeline * >;
	using Environments = std::vector< GraphicEnvironment * >;

	class Pipeline
	{
		public:
			Pipeline();
			~Pipeline();

			void init();
			void prepareInit();
			void resize();
			void update( float delta );
			void draw();
			void finish();
		
			void setGraphicEventListener( GraphicEventListener * listener );

			void addPipeline(GraphicPipeline * pipeline);
			void addEnvironment(GraphicEnvironment * env);

			Pipelines getGraphicPipelines(void);

		private:

			GraphicEventListener * eventListener;

			Pipelines pipelines;
			Environments environments;

	};
}

