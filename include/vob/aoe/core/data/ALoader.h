#pragma once

#include <memory>
#include <vob/aoe/core/type/ADynamicType.h>


namespace vob::aoe::data
{
	class VOB_AOE_API ALoader
		: public type::ADynamicType
	{
	public:
		// Methods
		virtual std::shared_ptr<type::ADynamicType> load(
			std::istream& a_inputStream) = 0;
	};
}
