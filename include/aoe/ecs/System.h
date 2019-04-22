#pragma once

#include <aoe/standard/ADynamicType.h>

namespace aoe
{
	namespace ecs
	{
		class ASystem
			: public sta::ADynamicType
		{
		public:
			virtual void update() const = 0;
		};
	}
}
