#pragma once

#include <vob/aoe/api.h>

#include <vob/misc/type/clone.h>

namespace vob::aoe::type
{
	class VOB_AOE_API ADynamicType
	{
	public:
		virtual ~ADynamicType() = default;
	};

	using dynamic_type_clone_copier = misty::pmr::clone_copier<ADynamicType>;

	template <typename TValue>
	using dynamic_type_clone = misty::pmr::clone<TValue, dynamic_type_clone_copier>;
}