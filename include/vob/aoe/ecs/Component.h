#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/core/type/ADynamicType.h>

#include <vob/misc/type/clone.h>


namespace vob::aoecs
{
	class AComponent
		: public aoe::type::ADynamicType
	{
	public:
	};

	class VOB_AOE_API component
	{
	public:
		virtual ~component() = default;
	};

	using component_cloner = misty::pmr::clone_copier<component>;
	
	template <typename TComponent>
	using component_clone = misty::pmr::clone<TComponent, component_cloner>;
}
