#pragma once

#include <vob/aoe/core/visitor/JsonWriter.h>
#include <string>


namespace vob::aoe::vis
{
	inline void accept(json::JsonWriter& a_visitor, sta::string_id& a_id)
	{
		std::pmr::string string;
		a_visitor.visit(string);
		a_id.assign(string);
	}
}
