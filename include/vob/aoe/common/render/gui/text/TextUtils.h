#pragma once

#include <tuple>

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/common/render/gui/text/Font.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/resources/Texture.h>

#include <vob/sta/unicode.h>

// TODO : change to std::char8_t when c++20 available

namespace vob::aoe::common
{
    struct TextCursor
    {
        TextCursor(float a_lineHeight, float a_base)
            : m_lineHeight{ a_lineHeight }
            , m_position{ 0, (a_lineHeight + a_base) / 2.0f }
        {}

        void advance(float x)
        {
            m_position.x += x;
        }

        void newLine()
        {
            m_position.x = 0.0f;
            m_position.y += m_lineHeight;
        }

        float getX()
        {
            return m_position.x;
        }

        float m_lineHeight;
        vec2 m_position;
    };

	vec2 multiply(vec2 const& a_lhs, vec2 const& a_rhs)
	{
		return vec2{ a_lhs.x * a_rhs.x, a_lhs.y * a_rhs.y };
	}

    vec2 divide(vec2 const& a_lhs, vec2 const& a_rhs)
    {
        return vec2{ a_lhs.x / a_rhs.x, a_lhs.y / a_rhs.y };
    }

	constexpr sta::utf8_string_view g_blankChars = " \t\n";
	constexpr sta::unicode g_spaceCode = sta::utf8_peek(" ");
	constexpr sta::unicode g_newlineCode = sta::utf8_peek("\n");
	constexpr sta::unicode g_replacementCode = 0xfffd; // <?>

	inline sta::utf8_string_view peekWord(sta::utf8_string_view a_view)
    {
        const auto wordCharCount = std::min(a_view.find_first_of(g_blankChars), a_view.size());
        const auto word = a_view.substr(0, wordCharCount);
		return word;
	}

	inline sta::utf8_string_view readWord(sta::utf8_string_view& a_view)
	{
		auto word = peekWord(a_view);
		a_view = a_view.substr(word.size());
		return word;
	}

	inline float getCharWidth(
		Font const& a_font
		, FontCharacter const& a_character
		, sta::unicode a_previousCode
		, vec2 const a_renderRatio
	)
	{
		auto const kerning = a_font.getKerningAmount(a_previousCode, a_character.m_id);
		return (a_character.m_xAdvance + kerning
			- a_font.m_spacing.x - a_font.m_spacing.y
			- a_font.m_padding.w - a_font.m_padding.y) * a_renderRatio.x;
	}

	inline float getCharWidth(
        Font const& a_font
        , sta::unicode const a_code
        , FontCharacter const* a_replacementCharacter
        , sta::unicode a_previousCode
        , vec2 const a_renderRatio
	)
	{
		auto const* character = a_font.findCharacter(a_code);
		if (character == nullptr)
		{
			character = a_replacementCharacter;
		}
		if (character != nullptr)
		{
			return getCharWidth(a_font, *character, a_previousCode, a_renderRatio);
		}
		return 0.0f;
	}

	inline float getWordWidth(
		Font const& a_font
        , sta::utf8_string_view a_word
        , FontCharacter const* a_replacementCharacter
        , sta::unicode a_previousCode
		, vec2 const a_renderRatio
	)
	{
		auto width = 0.0f;
		for (auto code : a_word)
		{
			width += getCharWidth(
				a_font
                , code
                , a_replacementCharacter
				, a_previousCode
				, a_renderRatio
			);
			a_previousCode = code;
		}
		return width;
	}

	inline void addCharToLine(
		Font const& a_font
        , FontCharacter const& a_character
        , sta::unicode& a_previousCode
        , vec2 const a_renderRatio
        , vec2& a_cursor
        , std::vector<GuiVertex>& a_vertices
	)
	{
		auto const kerning = a_font.getKerningAmount(a_previousCode, a_character.m_id);
		a_cursor.x += (kerning - a_font.m_spacing.x) * a_renderRatio.x;

		auto const offset = vec2{ a_character.m_offset };
		auto const width = vec2{ a_character.m_size.x, 0.0f };
		auto const height = vec2{ 0.0f, a_character.m_size.y };

		// TODO: could pre-compute some of these
		auto const tl = multiply(offset, a_renderRatio) + a_cursor;
		auto const tr = multiply(offset + width, a_renderRatio) + a_cursor;
		auto const bl = multiply(offset + height, a_renderRatio) + a_cursor;
		auto const br = multiply(offset + width + height, a_renderRatio) + a_cursor;
		
		auto const sfmlTextureSize = (*a_font.m_pages[a_character.m_page])->m_source.getSize();
		auto const textureSize = vec2{ sfmlTextureSize.x, sfmlTextureSize.y };
		auto const position = vec2{ a_character.m_position };

		// TODO: could pre-compute some of these
		auto const ttl = divide(position, textureSize);
		auto const ttr = divide(position + width, textureSize);
		auto const tbl = divide(position + height, textureSize);
        auto const tbr = divide(position + width + height, textureSize);

        a_vertices.emplace_back(GuiVertex{ tl, ttl });
        a_vertices.emplace_back(GuiVertex{ bl, tbl });
        a_vertices.emplace_back(GuiVertex{ br, tbr });
        a_vertices.emplace_back(GuiVertex{ br, tbr });
        a_vertices.emplace_back(GuiVertex{ tr, ttr });
        a_vertices.emplace_back(GuiVertex{ tl, ttl });

		a_previousCode = a_character.m_id;
        a_cursor.x += (a_character.m_xAdvance - a_font.m_spacing.y
            - a_font.m_padding.y - a_font.m_padding.w) * a_renderRatio.x;
	}

    inline void addCharToLine(
        Font const& a_font
        , sta::unicode const a_code
        , FontCharacter const* a_replacementCharacter
        , sta::unicode& a_previousCode
        , vec2 const a_renderRatio
        , vec2& a_cursor
        , std::vector<GuiVertex>& a_vertices
	)
	{
		auto character = a_font.findCharacter(a_code);
		if (character == nullptr)
		{
			character = a_replacementCharacter;
		}
		if (character != nullptr)
		{
			addCharToLine(
				a_font
				, *character
				, a_previousCode
				, a_renderRatio
				, a_cursor
				, a_vertices
			);
		}
	}

    inline void addWordToLine(
        Font const& a_font
        , sta::utf8_string_view a_word
        , FontCharacter const* a_replacementCharacter
        , sta::unicode& a_previousCode
        , vec2 const a_renderRatio
        , vec2& a_cursor
        , std::vector<GuiVertex>& a_vertices
    )
    {
        for (auto code : a_word)
        {
            addCharToLine(
                a_font
                , code
                , a_replacementCharacter
                , a_previousCode
                , a_renderRatio
                , a_cursor
                , a_vertices
            );
        }
    }

    inline std::vector<GuiVertex> createTextMesh(
        Font const& a_font
        , sta::utf8_string_view a_text
        , std::size_t a_start
        , std::size_t a_end
        , float const a_size
		, std::optional<float> const a_lineHeight
        , vec2 const a_areaSize
        , std::optional<TextCursor>& a_cursor
    )
    {
        if (a_areaSize.x == 0.0f || a_areaSize.y == 0.0f || a_font.m_size == 0.0f)
        {
            return {};
        }

        std::vector<GuiVertex> vertices;
        vertices.reserve((a_end - a_start) * 6);

		auto const fontRatio = a_size / a_font.m_size;
		auto const renderRatio = fontRatio * divide(vec2{ 1.0f, 1.0f }, a_areaSize);
        auto const replacement = a_font.findCharacter(g_replacementCode);
        auto const base = a_font.m_base * renderRatio.y;
        auto const lineHeight = a_lineHeight.value_or(a_font.m_lineHeight) * renderRatio.y;
        if (!a_cursor.has_value())
        {
            a_cursor = TextCursor{ lineHeight, base };
        }
        auto& cursor = a_cursor.value();
		sta::unicode lastCode = a_start == 0 ? 0 : *(a_text.begin() + (a_start - 1));
        
        auto ignoreNewLine = a_start == 0 || !g_blankChars.contains(a_text[a_start - 1]);
        auto start = a_text.begin() + a_start;
        auto jt = start;
        auto end = a_text.begin() + a_end;
        a_text = a_text.substr(a_start);

		while (jt != end)
		{
			auto word = peekWord(a_text);
			if (!word.empty())
			{
				// Add new line if necessary
				auto wordWidth = getWordWidth(a_font, word, replacement, lastCode, renderRatio);
				if (cursor.getX() + wordWidth > 1.0f
                    && (jt != start || !ignoreNewLine))
				{
                    cursor.newLine();
					lastCode = g_newlineCode;
					wordWidth = getWordWidth(a_font, word, replacement, lastCode, renderRatio);
				}

				// Put as many characters from word as possible
                auto it = word.begin();
				addCharToLine(a_font, *it++, replacement, lastCode, renderRatio, cursor.m_position, vertices);
                ++jt;
				while (it != word.end() && jt != end
					&& cursor.getX() + getCharWidth(a_font, *it, replacement, lastCode, renderRatio) < 1.0f)
				{
					addCharToLine(a_font, *it++, replacement, lastCode, renderRatio, cursor.m_position, vertices);
                    ++jt;
				}
				a_text = a_text.substr(it, a_text.end()); // it is not from same utf8_string_view...
			}

            if (jt == end)
            {
                break;
            }

			auto blank = a_text.front();
            if (blank == g_newlineCode)
            {
                cursor.newLine();
                lastCode = g_newlineCode;
                a_text = a_text.substr(1);
                ++jt;
            }
            else if (g_blankChars.contains(blank))
            {
                if (cursor.getX() + getCharWidth(a_font, blank, replacement, lastCode, renderRatio) > 1.0f)
                {
                    cursor.newLine();
                    lastCode = g_newlineCode;
                }

                addCharToLine(a_font, blank, replacement, lastCode, renderRatio, cursor.m_position, vertices);
                a_text = a_text.substr(1);
                ++jt;
            }
		}

		return vertices;
    }

    inline std::pair<vec2, vec2> getCursorTransform(
        Font const& a_font
        , sta::utf8_string_view a_text
        , std::size_t a_cursorIndex
        , float const a_size
        , std::optional<float> const a_lineHeight
        , vec2 const a_areaSize
	)
    {
        if (a_areaSize.x == 0.0f || a_areaSize.y == 0.0f || a_font.m_size == 0.0f)
        {
            return {};
        }

        auto const fontRatio = a_size / a_font.m_size;
		auto const renderRatio = fontRatio * divide(vec2{ 1.0f, 1.0f }, a_areaSize);
        auto const replacement = a_font.findCharacter(g_replacementCode);
		auto const base = a_font.m_base * renderRatio.y;
        auto const lineHeight = a_lineHeight.value_or(a_font.m_lineHeight) * renderRatio.y;
        vec2 cursor{ 0.0f, lineHeight - base };
		sta::unicode lastCode = 0;

		while (!a_text.empty() && a_cursorIndex > 0)
		{
			auto word = peekWord(a_text);
			if (!word.empty())
			{
				// Add new line if necessary
				auto wordWidth = getWordWidth(a_font, word, replacement, lastCode, renderRatio);
				if (cursor.x + wordWidth > 1.0f)
                {
                    cursor = { 0.0f, cursor.y + lineHeight };
                    lastCode = g_newlineCode;
                    wordWidth = getWordWidth(a_font, word, replacement, lastCode, renderRatio);
                }

                // Put as many characters from word as possible
                auto it = word.begin();
				cursor.x += getCharWidth(a_font, *it, replacement, lastCode, renderRatio);
				lastCode = *it++;
				--a_cursorIndex;
                while (it != word.end() && a_cursorIndex > 0
                    && cursor.x + getCharWidth(a_font, *it, replacement, lastCode, renderRatio) < 1.0f)
                {
                    cursor.x += getCharWidth(a_font, *it, replacement, lastCode, renderRatio);
					lastCode = *it++;
					--a_cursorIndex;
                }
                a_text = a_text.substr(it, a_text.end()); // it is not from same utf8_string_view...
            }

			if (a_cursorIndex == 0)
			{
				break;
			}

            auto blank = a_text.front();
            if (blank == g_newlineCode)
            {
                cursor = { 0.0f, cursor.y + lineHeight };
                lastCode = g_newlineCode;
                a_text = a_text.substr(1);
				--a_cursorIndex;
            }
            else if (g_blankChars.contains(blank))
            {
                if (cursor.x + getCharWidth(a_font, blank, replacement, lastCode, renderRatio) > 1.0f)
                {
                    cursor = { 0.0f, cursor.y + lineHeight };
                    lastCode = g_newlineCode;
                }

                cursor.x += getCharWidth(a_font, blank, replacement, lastCode, renderRatio);
				lastCode = blank;
                a_text = a_text.substr(1);
                --a_cursorIndex;
            }
		}

#ifdef WHATEVER
		sta::utf8_string_view word = readWord(a_text).substr(0, a_cursorIndex);
		cursor.x += getWordWidth(a_font, word, replacement, lastCode, renderRatio);
		a_cursorIndex -= word.size();
		while (!a_text.empty() && a_cursorIndex > 0)
		{
			auto const blank = a_text.front();
			auto const blankWidth = getCharWidth(
				a_font
				, blank
				, replacement
				, lastCode
				, renderRatio
			);
			a_text = a_text.substr(1);

			auto lastCodeCopy = lastCode;
			word = readWord(a_text);
			auto wordWidth = getWordWidth(a_font, word, replacement, lastCode, renderRatio);
			if (blank == g_newlineCode || cursor.x + blankWidth + wordWidth > 1.0f)
			{
				cursor = { 0.0f, cursor.y + lineHeight };
				lastCode = g_newlineCode;
			}
			else
			{
				cursor.x += blankWidth;
			}
			if (--a_cursorIndex == 0)
			{
				break;
			}

			word = word.substr(0, a_cursorIndex);
			wordWidth = getWordWidth(a_font, word, replacement, lastCodeCopy, renderRatio);
			cursor.x += wordWidth;
			a_cursorIndex -= word.size();
		}
#endif
		return std::make_pair(
			multiply(cursor, a_areaSize)
				+ vec2{ 0.0f, (a_font.m_base * fontRatio - a_size) }
			, vec2{ 1.0f * a_size / 24.0f, a_font.m_base * fontRatio }
		);
	}

	inline std::size_t getMouseIndex(
		Font const& a_font
		, sta::utf8_string_view a_text
		, vec2 a_mousePos
		, float const a_size
		, std::optional<float> const a_lineHeight
		, vec2 const a_areaSize
	)
    {
        if (a_areaSize.x == 0.0f || a_areaSize.y == 0.0f || a_font.m_size == 0.0f)
        {
            return {};
        }

        auto const fontRatio = a_size / a_font.m_size;
        auto const renderRatio = fontRatio * divide(vec2{ 1.0f, 1.0f }, a_areaSize);
        auto const replacement = a_font.findCharacter(g_replacementCode);
        auto const base = a_font.m_base * renderRatio.y;
        auto const lineHeight = a_lineHeight.value_or(a_font.m_lineHeight) * renderRatio.y;
        vec2 cursor{ 0.0f, lineHeight - base };
        sta::unicode lastCode = 0;

		auto cursorCenterOffset = vec2{
			1.0f * a_size / 24.0f
			, (a_font.m_base * fontRatio - a_size + a_font.m_base * fontRatio)
		};

        std::size_t currentIndex = 0;
        auto currentDistance = glm::length(cursorCenterOffset - a_mousePos);
		std::size_t bestIndex = currentIndex++;
		auto bestDistance = currentDistance;

        while (!a_text.empty())
        {
            auto word = peekWord(a_text);
            if (!word.empty())
            {
                // Add new line if necessary
                auto wordWidth = getWordWidth(a_font, word, replacement, lastCode, renderRatio);
                if (cursor.x + wordWidth > 1.0f)
                {
                    cursor = { 0.0f, cursor.y + lineHeight };
                    lastCode = g_newlineCode;
                    wordWidth = getWordWidth(a_font, word, replacement, lastCode, renderRatio);
                }

                // Put as many characters from word as possible
                auto it = word.begin();
                cursor.x += getCharWidth(a_font, *it, replacement, lastCode, renderRatio);
                lastCode = *it++;
                currentDistance = glm::length(
					cursorCenterOffset + multiply(cursor, a_areaSize) - a_mousePos
                );
				if (currentDistance < bestDistance)
				{
					bestDistance = currentDistance;
					bestIndex = currentIndex;
				}
				++currentIndex;
                while (it != word.end()
                    && cursor.x + getCharWidth(a_font, *it, replacement, lastCode, renderRatio) < 1.0f)
                {
                    cursor.x += getCharWidth(a_font, *it, replacement, lastCode, renderRatio);
                    lastCode = *it++;
                    currentDistance = glm::length(
                        cursorCenterOffset + multiply(cursor, a_areaSize) - a_mousePos
                    );
                    if (currentDistance < bestDistance)
                    {
                        bestDistance = currentDistance;
                        bestIndex = currentIndex;
                    }
                    ++currentIndex;
                }
                a_text = a_text.substr(it, a_text.end()); // it is not from same utf8_string_view...
            }

            auto blank = a_text.front();
            if (blank == g_newlineCode)
            {
                cursor = { 0.0f, cursor.y + lineHeight };
                lastCode = g_newlineCode;
                currentDistance = glm::length(
                    cursorCenterOffset + multiply(cursor, a_areaSize) - a_mousePos
                );
                if (currentDistance < bestDistance)
                {
                    bestDistance = currentDistance;
                    bestIndex = currentIndex;
                }
                ++currentIndex;
                a_text = a_text.substr(1);
            }
            else if (g_blankChars.contains(blank))
            {
                if (cursor.x + getCharWidth(a_font, blank, replacement, lastCode, renderRatio) > 1.0f)
                {
                    cursor = { 0.0f, cursor.y + lineHeight };
                    lastCode = g_newlineCode;
                }

                cursor.x += getCharWidth(a_font, blank, replacement, lastCode, renderRatio);
                lastCode = blank;
                currentDistance = glm::length(
                    cursorCenterOffset + multiply(cursor, a_areaSize) - a_mousePos
				);
                if (currentDistance < bestDistance)
                {
                    bestDistance = currentDistance;
                    bestIndex = currentIndex;
                }
                ++currentIndex;
                a_text = a_text.substr(1);
            }
        }

		return bestIndex;
	}
}
