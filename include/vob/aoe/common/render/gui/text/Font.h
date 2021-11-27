#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include <vob/aoe/api.h>

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/resources/Texture.h>

namespace vob::aoe::common
{
    class Texture;

	struct FontCharacter
	{
		char32_t m_id = 0xFFFF;
		glm::i16vec2 m_position = {};
		glm::i16vec2 m_size = {};
		glm::i16vec2 m_offset = {};
		std::uint32_t m_xAdvance = 0;
		std::uint32_t m_page = 0;
		std::uint32_t m_channel = 0;
	};

	struct CharacterSequence
	{
		char32_t m_first;
		char32_t m_second;
	};

	struct FontKerning
	{
		CharacterSequence m_sequence;
		std::int32_t m_amount = 0;
	};

	constexpr bool operator==(CharacterSequence const& a_lhs, CharacterSequence const& a_rhs)
	{
		return a_lhs.m_first == a_rhs.m_first && a_lhs.m_second == a_rhs.m_second;
	}

	struct CharacterSequence_Hash
	{
		std::size_t operator()(CharacterSequence const& a_characterSequence) const
		{
			return (static_cast<std::size_t>(a_characterSequence.m_first) << 32) | a_characterSequence.m_second;
		}
	};

	struct VOB_AOE_API Font
		: public type::ADynamicType
	{
		// Attributes
		std::pmr::string m_name;
		std::pmr::string m_charSet;
		std::uint32_t m_size = 0;
		bool m_isBold = false;
		bool m_isItalic = false;
		bool m_isUnicode = false;
		bool m_isPacked = false;
		std::uint32_t m_horizontalStretch = 100;
		std::uint32_t m_smooth = 1;
		std::uint32_t m_antiAliasing = 1;
		glm::ivec4 m_padding = {};
		glm::ivec2 m_spacing = {};
		std::uint32_t m_lineHeight = 0;
		std::uint32_t m_base = 0;
		glm::uvec2 m_scale = {};
		std::vector<std::shared_ptr<GraphicResourceHandle<Texture> const>> m_pages;

		// Methods
		FontCharacter const* findCharacter(char32_t const a_unicode) const;
        void setCharacter(FontCharacter const& a_character);
        std::int32_t getKerningAmount(char32_t const a_firstCode, char32_t const a_secondCode) const;
		void setKerning(FontKerning a_kerning);

    private:
		// Types
        using ContinuousCharacterSet = std::pair<char32_t, std::vector<FontCharacter>>;
        using ContinuousCharacterSetList = std::vector<ContinuousCharacterSet>;

		// Attributes
		ContinuousCharacterSetList m_continuousCharacterSetList;
		std::unordered_map<CharacterSequence, std::int32_t, CharacterSequence_Hash> m_kerningAmounts;
	};
}

