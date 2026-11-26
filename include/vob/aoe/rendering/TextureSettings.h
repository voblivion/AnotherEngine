#pragma once

#include <vob/misc/visitor/accept.h>
#include <vob/misc/visitor/name_value_pair.h>

#include <array>
#include <type_traits>


namespace vob::aoegl
{
	struct TextureSettings
	{
		enum class ColorSpace
		{
			Linear,
			Srgb
		};

		enum class SamplerType
		{
			Simple,
			Array,
			Cube
		};

		enum class Swizzle
		{
			Red,
			Green,
			Blue,
			Alpha,
			Zero,
			One
		};

		ColorSpace colorSpace = ColorSpace::Linear;
		SamplerType samplerType = SamplerType::Simple;
		std::array<Swizzle, 4> swizzle = { Swizzle::Red, Swizzle::Green, Swizzle::Blue, Swizzle::Alpha };
	};

	template <typename TVisitor, typename TSettings>
		requires std::is_same_v<std::remove_cvref_t<TSettings>, TextureSettings>
	bool accept(TVisitor& a_visitor, TSettings& a_settings)
	{
		auto result = true;
		result &= a_visitor.visit(misvi::nvp("colorSpace", a_settings.colorSpace));
		result &= a_visitor.visit(misvi::nvp("samplerType", a_settings.samplerType));
		result &= a_visitor.visit(misvi::nvp("swizzle", a_settings.swizzle));
		return result;
	}
}
