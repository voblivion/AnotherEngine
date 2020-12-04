#pragma once

#include <vob/sta/string_id.h>

#include <vob/aoe/core/visitor/JsonWriter.h>

namespace vob::aoe::vis
{
	template <typename ContextType>
	inline void accept(common::JsonWriter<ContextType>& a_visitor, sta::string_id& a_id)
	{
		std::string string;
		a_visitor.visit(string);
		a_id.assign(string);
	}
}
