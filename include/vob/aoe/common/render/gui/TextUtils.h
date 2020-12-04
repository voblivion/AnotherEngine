#pragma once

#include <tuple>

#include <vob/aoe/common/render/gui/Font.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/resources/Texture.h>

// TODO : change to std::char8_t when c++20 available

namespace vob::aoe::common
{
	inline std::pair<std::uint32_t, bool> readUnicode(u8string_view& a_utf8Text)
	{
		if (a_utf8Text.size() == 0)
		{
			return { 0, true };
		}

		constexpr char mask = 0b00111111;

		auto const c0 = a_utf8Text[0];
		if ((c0 & 0b10000000) == 0)
		{
			a_utf8Text = a_utf8Text.substr(1);
			return { c0, true };
		}
		if (a_utf8Text.size() < 2)
		{
			return { 0, false };
		}
		auto const c1 = a_utf8Text[1];
		if ((c0 & 0b11100000) == 0b11000000)
		{
			a_utf8Text = a_utf8Text.substr(2);
			return { (c0 & 0b00011111) << 6 | (mask & c1), true };
		}
		if (a_utf8Text.size() < 3)
		{
			return { 0, false };
		}
		auto const c2 = a_utf8Text[2];
		if ((c0 & 0b11110000) == 0b11100000)
		{
			a_utf8Text = a_utf8Text.substr(3);
			return { (c0 & 0b00001111) << 12 | (mask & c1) << 6 | (mask & c2), true };
		}
		if (a_utf8Text.size() < 4)
		{
			return { 0, false };
		}
		auto const c3 = a_utf8Text[3];
		a_utf8Text = a_utf8Text.substr(4);
		return { (c0 & 0b00000111) << 18 | (mask & c1) << 12 | (mask & c2) << 6 | (mask & c3), true };
	}

	constexpr char const* g_lineBreakingChars = "- \t\n";

	inline std::pair<std::string_view, char> extractNextWord(std::string_view& a_text)
	{
		auto const nextWordLen = a_text.find_first_of(g_lineBreakingChars);
		if (nextWordLen < a_text.size())
		{
			auto const nextWord = a_text.substr(0, nextWordLen);
			auto const separator = a_text[nextWordLen];
			a_text = a_text.substr(nextWordLen + 1);
			return std::make_pair(nextWord, separator);
		}

		auto const nextWord = a_text;
		a_text = {};
		return std::make_pair(nextWord, '\x00');
	}

	inline float getWordWidth(
		vec2 const a_renderRatio
		, std::string_view const a_word
		, float const a_size
		, Font const& a_font
		, char a_previousChar
	)
	{
		auto wordWidth = 0.0f;
		for (auto c : a_word)
		{
			if (FontCharacter const* fontCharacter = a_font.findCharacter(c))
			{
				auto const kerning = a_font.getKerningAmount(a_previousChar, c) * a_renderRatio.x;
				wordWidth += fontCharacter->m_xAdvance * a_renderRatio.x + kerning;
				a_previousChar = c;
			}
		}
		return wordWidth;
	}

	inline void addCharToLine(
		vec2 const a_renderRatio
		, FontCharacter const& a_character
		, float const a_size
		, Font const& a_font
		, vec2& a_cursor
		, char const a_previousChar
		, std::vector<GuiVertex>& a_vertices
	)
	{
		auto const kerning = a_font.getKerningAmount(a_previousChar, a_character.m_id) * a_renderRatio.x;
		a_cursor.x += kerning;

		auto const charOffsetTmp = vec2{ a_character.m_offset };
		auto const charSizeTmp = vec2{ a_character.m_size };
		auto const charWidthTmp = vec2{ charSizeTmp.x, 0 };
		auto const charHeightTmp = vec2{ 0, charSizeTmp.y };

		auto tl = charOffsetTmp;
		tl.x *= a_renderRatio.x;
		tl.y *= a_renderRatio.y;
		tl += a_cursor;
		auto tr = charOffsetTmp + charWidthTmp;
		tr.x *= a_renderRatio.x;
		tr.y *= a_renderRatio.y;
		tr += a_cursor;
		auto bl = charOffsetTmp + charHeightTmp;
		bl.x *= a_renderRatio.x;
		bl.y *= a_renderRatio.y;
		bl += a_cursor;
		auto br = charOffsetTmp + charSizeTmp;
		br.x *= a_renderRatio.x;
		br.y *= a_renderRatio.y;
		br += a_cursor;

		auto const textureX = (*a_font.m_pages[a_character.m_page])->m_source.getSize().x;
		auto const textureY = (*a_font.m_pages[a_character.m_page])->m_source.getSize().y;

		auto ttl = vec2{ a_character.m_position };
		ttl.x /= textureX;
		ttl.y /= textureY;
		auto ttr = vec2{ a_character.m_position } +vec2{ a_character.m_size.x, 0 };
		ttr.x /= textureX;
		ttr.y /= textureY;
		auto tbl = vec2{ a_character.m_position } +vec2{ 0, a_character.m_size.y };
		tbl.x /= textureX;
		tbl.y /= textureY;
		auto tbr = vec2{ a_character.m_position } +vec2{ a_character.m_size };
		tbr.x /= textureX;
		tbr.y /= textureY;

		a_vertices.emplace_back(GuiVertex{ tl, ttl });
		a_vertices.emplace_back(GuiVertex{ bl, tbl });
		a_vertices.emplace_back(GuiVertex{ br, tbr });
		a_vertices.emplace_back(GuiVertex{ br, tbr });
		a_vertices.emplace_back(GuiVertex{ tr, ttr });
		a_vertices.emplace_back(GuiVertex{ tl, ttl });

		a_cursor.x += a_character.m_xAdvance * a_renderRatio.x;
	}

	inline void addCharToLine(
		vec2 const a_renderRatio
		, char const a_char
		, float const a_size
		, Font const& a_font
		, vec2& a_cursor
		, char const a_previousChar
		, std::vector<GuiVertex>& a_vertices
	)
	{
		if (FontCharacter const* character = a_font.findCharacter(a_char))
		{
			addCharToLine(a_renderRatio, *character, a_size, a_font, a_cursor, a_previousChar, a_vertices);
		}
	}

	inline void addWordToLine(
		vec2 const a_renderRatio
		, std::string_view const a_word
		, float const a_size
		, Font const& a_font
		, vec2& a_cursor
		, char a_previousChar
		, std::vector<GuiVertex>& a_vertices
	)
	{
		for (auto const c : a_word)
		{
			addCharToLine(a_renderRatio, c, a_size, a_font, a_cursor, a_previousChar, a_vertices);
			a_previousChar = c;
		}
	}

	inline std::vector<GuiVertex> createTextMeshVertices(
		vec2 const a_areaSize
		, std::string_view a_text
		, float const a_size
		, Font const& a_font
	)
	{
		std::vector<GuiVertex> vertices;
		vertices.reserve(a_text.size() * 6);

		auto const renderRatio = a_size / a_font.m_size * vec2{ 1.0f / a_areaSize.x, 1.0f / a_areaSize.y };

		auto const space = a_font.findCharacter(' ');
		auto const hyphen = a_font.findCharacter('-');

		if (space != nullptr)
		{
			auto const spaceWidth = space->m_xAdvance * renderRatio.x;
			auto const hyphenWidth = hyphen->m_xAdvance * renderRatio.x;

			auto const lineHeight = a_font.m_lineHeight * renderRatio.y;
			vec2 cursor = { 0.0, 0 };

			std::string_view word;
			char separator;
			std::tie(word, separator) = extractNextWord(a_text);

			addWordToLine(renderRatio, word, a_size, a_font, cursor, '\x00', vertices);
			auto lastReadChar = separator;

			while (!a_text.empty())
			{
				std::tie(word, separator) = extractNextWord(a_text);

				auto const wordWidth = getWordWidth(renderRatio, word, a_size, a_font, lastReadChar);
				float prevSeparatorWidth;
				switch (lastReadChar)
				{
				case '-':
					prevSeparatorWidth = hyphenWidth;
					break;
				case '\n':
					prevSeparatorWidth = 0.0f;
					break;
				default:
					prevSeparatorWidth = spaceWidth;
				}
				if (lastReadChar != '\n' && cursor.x + prevSeparatorWidth + wordWidth <= 1.0f)
				{
					addCharToLine(renderRatio, *space, a_size, a_font, cursor, lastReadChar, vertices);
					lastReadChar = separator;
				}
				else
				{
					cursor.x = 0;
					cursor.y += lineHeight;
				}
				addWordToLine(renderRatio, word, a_size, a_font, cursor, lastReadChar, vertices);
				lastReadChar = separator;
			}
		}
		/*
		auto txt = u8string_view{ "\xe6\x98\xa5" };

		auto const code = readUnicode(txt);

		ignorable_assert(code.second && code.first == 0x00e9);
		*/
		return vertices;
	}

}
