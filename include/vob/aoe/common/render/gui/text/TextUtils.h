#pragma once

#include <limits>
#include <tuple>

#include <vob/misc/std/ignorable_assert.h>

#include <vob/aoe/common/render/gui/text/Font.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/resources/Texture.h>


namespace vob::aoe::common
{
    inline auto preCharacterOffset(
        Font const& a_font
        , FontCharacter const& a_character
        , std::optional<char32_t> a_previousCode
    )
    {
        std::int32_t offset = 0;
        if (a_previousCode.has_value())
        {
            offset += a_font.getKerningAmount(a_previousCode.value(), a_character.m_id);
            //offset -= a_font.m_padding.w;
        }
        return offset;
    }

    inline auto postCharacterOffset(
        Font const& a_font
        , FontCharacter const& a_character
    )
    {
        std::int32_t offset = a_font.m_spacing.x + a_character.m_xAdvance;
        //offset += a_font.m_padding.y;
        return offset;
    }

    struct TextCursor
    {
        TextCursor() = default;

        TextCursor(float a_lineHeight, glm::vec2 a_position)
            : m_lineHeight{ a_lineHeight }
            , m_position{ a_position }
        {}

        void advance(float a_offset)
        {
            m_position.x += a_offset;
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

        float m_lineHeight{};
        glm::vec2 m_position{};
    };

	inline glm::vec2 multiply(glm::vec2 const& a_lhs, glm::vec2 const& a_rhs)
	{
		return glm::vec2{ a_lhs.x * a_rhs.x, a_lhs.y * a_rhs.y };
	}

    inline glm::vec2 divide(glm::vec2 const& a_lhs, glm::vec2 const& a_rhs)
    {
        return glm::vec2{ a_lhs.x / a_rhs.x, a_lhs.y / a_rhs.y };
    }

	constexpr std::u32string_view g_blankChars = U" \t\n";
	constexpr char32_t g_spaceCode = U' ';
	constexpr char32_t g_newlineCode = '\n';
	constexpr char32_t g_replacementCode = 0xFFFD; // <?>

	inline std::u32string_view peekWord(std::u32string_view a_view)
    {
        const auto wordCharCount = std::min(a_view.find_first_of(g_blankChars), a_view.size());
        const auto word = a_view.substr(0, wordCharCount);
		return word;
	}

	inline std::u32string_view readWord(std::u32string_view& a_view)
	{
		auto word = peekWord(a_view);
		a_view = a_view.substr(word.size());
		return word;
	}

	inline float getCharWidth(
		Font const& a_font
        , FontCharacter const& a_character
        , std::optional<char32_t> a_previousCode
		, glm::vec2 const a_renderRatio
	)
	{
        return (preCharacterOffset(a_font, a_character, a_previousCode)
            + postCharacterOffset(a_font, a_character)) * a_renderRatio.x;
	}

	inline float getCharWidth(
        Font const& a_font
        , char32_t const a_code
        , FontCharacter const* a_replacementCharacter
        , std::optional<char32_t> a_previousCode
        , glm::vec2 const a_renderRatio
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
        , std::u32string_view a_word
        , FontCharacter const* a_replacementCharacter
        , std::optional<char32_t> a_previousCode
		, glm::vec2 const a_renderRatio
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
        , std::optional<char32_t>& a_previousCode
        , glm::vec2 const a_renderRatio
        , TextCursor& a_cursor
        , std::vector<GuiVertex>& a_vertices
	)
	{
        a_cursor.advance(preCharacterOffset(a_font, a_character, a_previousCode) * a_renderRatio.x);

		auto const offset = glm::vec2{ a_character.m_offset };

		auto const width = glm::vec2{ a_character.m_size.x, 0.0f };
		auto const height = glm::vec2{ 0.0f, a_character.m_size.y };

		// TODO: could pre-compute some of these
		auto const tl = multiply(offset, a_renderRatio) + a_cursor.m_position;
		auto const tr = multiply(offset + width, a_renderRatio) + a_cursor.m_position;
		auto const bl = multiply(offset + height, a_renderRatio) + a_cursor.m_position;
		auto const br = multiply(offset + width + height, a_renderRatio) + a_cursor.m_position;
		
		auto const sfmlTextureSize = (*a_font.m_pages[a_character.m_page])->m_source.getSize();
		auto const textureSize = glm::vec2{ sfmlTextureSize.x, sfmlTextureSize.y };
		auto const position = glm::vec2{ a_character.m_position };

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
        a_cursor.advance(postCharacterOffset(a_font, a_character) * a_renderRatio.x);
	}

    template <typename CharacterOp>
    concept InitAwareTextOp = requires(CharacterOp& a_characterOp)
    {
        {
            a_characterOp.init(
                std::declval<TextCursor>()
                , std::declval<glm::vec2>()
            )
        };
    };

    template <typename CharacterOp>
    concept SkipAwareTextOp = requires(CharacterOp& a_characterOp)
    {
        { a_characterOp.skip() };
    };

    template <typename CharacterOp>
    inline void tryProcessChar(
        Font const& a_font
        , char32_t a_code
        , FontCharacter const* a_replacementCharacter
        , glm::vec2 a_renderRatio
        , std::optional<char32_t>& a_previousCode
        , TextCursor& a_cursor
        , CharacterOp& a_characterOp
    )
    {
        auto character = a_font.findCharacter(a_code);
        if (character == nullptr)
        {
            character = a_replacementCharacter;
        }
        if (character != nullptr)
        {
            a_characterOp(a_font, *character, a_renderRatio, a_previousCode, a_cursor);
            a_previousCode = character->m_id;
        }
        else
        {
            if constexpr (SkipAwareTextOp<CharacterOp>)
            {
                a_characterOp.skip();
            }
        }
    }

    template <typename CharacterOp>
    inline void processText(
        Font const& a_font
        , std::u32string_view a_text
        , std::u32string_view::size_type a_beginIndex
        , std::u32string_view::size_type a_endIndex
        , float const a_size
        , glm::vec2 const a_areaSize
        , std::optional<glm::vec2> a_cursor
        , std::optional<char32_t>& a_previousCode
        , CharacterOp& a_characterOp
    )
    {
        if (a_areaSize.x == 0.0f || a_areaSize.y == 0.0f || a_font.m_size == 0.0f)
        {
            return;
        }

        auto const fontRatio = a_size / a_font.m_size;
        auto const renderRatio = fontRatio * divide(glm::vec2{ 1.0f, 1.0f }, a_areaSize);
        auto const replacement = a_font.findCharacter(g_replacementCode);
        auto const base = a_font.m_base * renderRatio.y;
        auto const lineHeight = a_font.m_lineHeight * renderRatio.y;
        auto const textBegin = a_text.begin() + a_beginIndex;
        auto const textEnd = a_text.begin() + a_endIndex;

        auto text = a_text.substr(a_beginIndex);
        auto cursor = TextCursor{
            lineHeight
            , a_cursor.value_or(glm::vec2{ 0.0f, (lineHeight + base) / 2.0f })
        };

        if constexpr (InitAwareTextOp<CharacterOp>)
        {
            a_characterOp.init(cursor, renderRatio);
        }

        auto textIt = textBegin;
        while (textIt != textEnd)
        {
            auto word = peekWord(text);
            if (!word.empty())
            {
                auto wordWidth = getWordWidth(a_font, word, replacement, a_previousCode, renderRatio);
                if (cursor.getX() + wordWidth > 1.0f && a_previousCode.has_value())
                {
                    a_characterOp.newLine(cursor);
                    a_previousCode = std::nullopt;
                    wordWidth = getWordWidth(a_font, word, replacement, a_previousCode, renderRatio);
                }

                tryProcessChar(
                    a_font
                    , *textIt++
                    , replacement
                    , renderRatio
                    , a_previousCode
                    , cursor
                    , a_characterOp
                );
                while (textIt != word.end() && textIt != textEnd && cursor.getX()
                    + getCharWidth(a_font, *textIt, replacement, a_previousCode, renderRatio) < 1.0f)
                {
                    tryProcessChar(
                        a_font
                        , *textIt++
                        , replacement
                        , renderRatio
                        , a_previousCode
                        , cursor
                        , a_characterOp
                    );
                }
                text = text.substr(std::distance(textBegin, textIt));
            }

            if (textIt == textEnd)
            {
                break;
            }

            auto possibleBlank = text.front();
            if (possibleBlank == g_newlineCode)
            {
                a_characterOp.newLine(cursor);
                a_previousCode = std::nullopt;
                text = text.substr(1);
                ++textIt;
            }
            else if (g_blankChars.find(possibleBlank) != std::u32string_view::npos)
            {
                auto blankWidth = getCharWidth(
                    a_font
                    , possibleBlank
                    , replacement
                    , a_previousCode
                    , renderRatio
                );
                if (cursor.getX() + blankWidth > 1.0f)
                {
                    a_characterOp.newLine(cursor);
                    a_previousCode = std::nullopt;
                }
                tryProcessChar(
                    a_font
                    , possibleBlank
                    , replacement
                    , renderRatio
                    , a_previousCode
                    , cursor
                    , a_characterOp
                );
                text = text.substr(1);
                ++textIt;
            }
        }
    }

    struct GenerateCharVertices
    {
        void init(TextCursor a_cursor, glm::vec2 a_renderRatio)
        {
            m_cursorPosition = a_cursor.m_position;
        }

        void newLine(TextCursor& a_cursor)
        {
            a_cursor.newLine();
            m_cursorPosition = a_cursor.m_position;
        }

        void operator()(
            Font const& a_font
            , FontCharacter const& a_character
            , glm::vec2 a_renderRatio
            , std::optional<char32_t> a_previousCode
            , TextCursor& a_cursor
        )
        {
            addCharToLine(
                a_font
                , a_character
                , a_previousCode
                , a_renderRatio
                , a_cursor
                , m_vertices
            );
            m_cursorPosition = a_cursor.m_position;
        }

        std::vector<GuiVertex>& m_vertices;
        glm::vec2 m_cursorPosition;
    };

    inline std::vector<GuiVertex> createTextMesh(
        Font const& a_font
        , std::u32string_view a_text
        , std::size_t a_start
        , std::size_t a_end
        , float const a_size
        , glm::vec2 const a_areaSize
        , std::optional<glm::vec2>& a_cursor
        , std::optional<char32_t>& a_previousCode
    )
    {
        std::vector<GuiVertex> vertices;
        vertices.reserve((a_end - a_start) * 6);

        GenerateCharVertices op{ vertices };
        processText(
            a_font
            , a_text
            , a_start
            , a_end
            , a_size
            , a_areaSize
            , a_cursor
            , a_previousCode
            , op
        );
        a_cursor = op.m_cursorPosition;

        return vertices;
    }

    struct AdvanceCharWidth
    {
        void init(TextCursor a_cursor, glm::vec2 a_renderRatio)
        {
            m_cursorPosition = a_cursor.m_position;
        }

        void newLine(TextCursor& a_cursor)
        {
            a_cursor.newLine();
            m_cursorPosition = a_cursor.m_position;
            m_lastCharacterPostOffset = 0.0f;
        }

        void operator()(
            Font const& a_font
            , FontCharacter const& a_character
            , glm::vec2 a_renderRatio
            , std::optional<char32_t> a_previousCode
            , TextCursor& a_cursor
        )
        {
            a_cursor.advance(preCharacterOffset(a_font, a_character, a_previousCode) * a_renderRatio.x);
            a_cursor.advance(postCharacterOffset(a_font, a_character) * a_renderRatio.x);
            m_cursorPosition = a_cursor.m_position;
            m_lastCharacterPostOffset = (/*a_font.m_padding.z +*/ a_font.m_padding.y + a_font.m_spacing.x) * a_renderRatio.x;
        }

        glm::vec2 m_cursorPosition;
        float m_lastCharacterPostOffset = 0.0f;
    };

    inline std::pair<glm::vec2, glm::vec2> getCursorTransform(
        Font const& a_font
        , std::u32string_view a_text
        , std::size_t a_cursorIndex
        , float const a_size
        , glm::vec2 const a_areaSize
    )
    {
        std::optional<char32_t> previousCode;
        AdvanceCharWidth op;
        processText(
            a_font
            , a_text
            , 0
            , a_cursorIndex
            , a_size
            , a_areaSize
            , std::nullopt
            , previousCode
            , op
        );

        return std::make_pair(
            multiply(op.m_cursorPosition - glm::vec2{ op.m_lastCharacterPostOffset, 0.0f }, a_areaSize)
            , glm::vec2{ 1.0f * a_size / 24.0f, a_font.m_base * a_size / a_font.m_size }
        );
    }

    struct StoreClosestPos
    {
        StoreClosestPos(glm::vec2 a_position, float a_size)
            : m_position{ a_position }
            , m_offset{ 0.0f, a_size / 2.0f }
        {
        }

        void init(TextCursor a_cursor, glm::vec2 a_renderRatio)
        {
            m_offset *= a_renderRatio;
            m_position = m_position * a_renderRatio - m_offset;

            m_bestIndex = m_currentIndex;
            m_bestDistance = glm::length(a_cursor.m_position - m_position );
        }

        void newLine(TextCursor& a_cursor)
        {
            a_cursor.newLine();
        }

        void skip()
        {
            ++m_currentIndex;
        }

        void operator()(
            Font const& a_font
            , FontCharacter const& a_character
            , glm::vec2 a_renderRatio
            , std::optional<char32_t> a_previousCode
            , TextCursor& a_cursor
        )
        {
            auto distance = glm::length(a_cursor.m_position - m_position);
            if (distance < m_bestDistance)
            {
                m_bestDistance = distance;
                m_bestIndex = m_currentIndex;
            }

            a_cursor.advance(getCharWidth(a_font, a_character, a_previousCode, a_renderRatio));
            ++m_currentIndex;
        }

        glm::vec2 m_position;
        glm::vec2 m_offset;
        float m_bestDistance = std::numeric_limits<float>::max();
        std::u32string_view::size_type m_bestIndex = -1;
        std::u32string_view::size_type m_currentIndex = 0;
    };

    inline auto getIndexAtPos(
        Font const& a_font
        , std::u32string_view a_text
        , glm::vec2 a_position
        , float const a_size
        , glm::vec2 const a_areaSize
        , std::optional<char32_t>& a_previousCode
    )
    {
        StoreClosestPos op{ a_position, a_size };
        processText(
            a_font
            , a_text
            , 0
            , a_text.size()
            , a_size
            , a_areaSize
            , std::nullopt
            , a_previousCode
            , op
        );
        return op.m_bestIndex;
    }
}
