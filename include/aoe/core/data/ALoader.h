#pragma once

#include <memory>
#include <aoe/core/Export.h>
#include <aoe/core/standard/ADynamicType.h>

namespace aoe
{
	namespace data
	{
		class AOE_CORE_API ALoader
			: public sta::ADynamicType
		{
		public:
			// Methods
			virtual std::shared_ptr<sta::ADynamicType> load(
				std::istream& a_inputStream) = 0;
		};
	}
}
