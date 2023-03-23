#pragma once

#include <vob/aoe/data/id.h>

#include <memory>


namespace vob::aoedt
{
	template <typename TData>
	class database
	{
	public:
		virtual std::shared_ptr<TData const> find(id const a_id) const = 0;
	};
}
