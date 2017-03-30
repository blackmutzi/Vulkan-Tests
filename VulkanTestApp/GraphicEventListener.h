#pragma once
#include "stdafx.h"

namespace Graphic
{
	class GraphicEventListener
	{
		public:
			virtual ~GraphicEventListener() = default;

			virtual void prepare() = 0;
			virtual void update(float delta) = 0;
	};
}
