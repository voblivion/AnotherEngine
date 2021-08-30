#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include <vob/sta/utility.h>

#include <vob/aoe/api.h>

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/type/Primitive.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/resources/Texture.h>

namespace vob::aoe::common
{
    class Texture;

	struct FontCharacter
	{
		sta::unicode m_id = sta::invalid_unicode;
		i16vec2 m_position = {};
		i16vec2 m_size = {};
		i16vec2 m_offset = {};
		u32 m_xAdvance = 0;
		u32 m_page = 0;
		u32 m_channel = 0;
	};

	struct FontKerning
	{
		std::pair<sta::unicode, sta::unicode> m_sequence;
		i32 m_amount = 0;
	};

	struct VOB_AOE_API Font
		: public type::ADynamicType
	{
		// Attributes
		u8string m_name;
		u8string m_charSet;
		u32 m_size = 0;
		bool m_isBold = false;
		bool m_isItalic = false;
		bool m_isUnicode = false;
		bool m_isPacked = false;
		u32 m_horizontalStretch = 100;
		u32 m_smooth = 1;
		u32 m_antiAliasing = 1;
		ivec4 m_padding = {};
		ivec2 m_spacing = {};
		u32 m_lineHeight = 0;
		u32 m_base = 0;
		uvec2 m_scale = {};
		std::vector<std::shared_ptr<GraphicResourceHandle<Texture> const>> m_pages;

		// Methods
		FontCharacter const* findCharacter(sta::unicode const a_unicode) const;
        void setCharacter(FontCharacter const& a_character);
        i32 getKerningAmount(sta::unicode const a_firstCode, sta::unicode const a_secondCode) const;
		void setKerning(FontKerning a_kerning);

    private:
		// Types
        using ContinuousCharacterSet = std::pair<sta::unicode, std::vector<FontCharacter>>;
        using ContinuousCharacterSetList = std::vector<ContinuousCharacterSet>;

		// Attributes
		ContinuousCharacterSetList m_continuousCharacterSetList;
		std::unordered_map<std::pair<sta::unicode, sta::unicode>, i32> m_kerningAmounts;
	};
}

