#pragma once

#include <charconv>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/common/data/filesystem/FileSystemDatabase.h>
#include <vob/aoe/common/render/gui/Font.h>

namespace vob::aoe::common
{
	template <typename CharType, typename CharTraitsType, typename AllocatorType>
	bool readNamedLine(
		std::istream& a_inputStream
		, std::string_view const a_expectedName
		, std::basic_istringstream<CharType, CharTraitsType, AllocatorType>& a_line
	)
	{
		using istringstream = std::basic_istringstream<CharType, CharTraitsType, AllocatorType>;
		using string = std::basic_string<CharType, CharTraitsType, AllocatorType>;

		string lineNameStr;
		if (!(a_inputStream >> lineNameStr)
			|| a_expectedName.compare(lineNameStr) != 0)
		{

			ignorable_assert(false && "Invalid .fnt format : missing or misplaced section.");
			return false;
		}

		string lineStr;
		std::getline(a_inputStream, lineStr);
		a_line = { lineStr };
		return true;
	}

	template <typename Type, typename _ = std::enable_if_t<std::is_integral_v<Type>>>
	void parse(std::string_view const a_value, Type& a_integer)
	{
		auto const res = std::from_chars(
			a_value.data()
			, a_value.data() + a_value.size()
			, a_integer
		);
		ignorable_assert(res.ec != std::errc::invalid_argument);
	}

	inline void parse(std::string_view const a_value, bool& a_bool)
	{
		a_bool = !(a_value.empty() || a_value[0] == '0');
	}

	template <glm::length_t t_component, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	std::enable_if_t<t_component >= t_length, std::size_t> parse(
		std::string_view const a_value
		, glm::vec<t_length, Type, t_qualifier>& a_vector
		, std::size_t a_offset = 0
	)
	{
		return -1;
	}

	template <glm::length_t t_component, glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	std::enable_if_t<t_component < t_length, std::size_t> parse(
		std::string_view const a_value
		, glm::vec<t_length, Type, t_qualifier>& a_vector
		, std::size_t const a_offset = 0
	)
	{
		auto const len = a_value.substr(a_offset).find_first_of(',');
		parse(a_value.substr(a_offset, len), a_vector[t_component]);
		return a_offset + len + 1;
	}

	template <glm::length_t t_length, typename Type, glm::qualifier t_qualifier>
	inline void parse(std::string_view const a_value, glm::vec<t_length, Type, t_qualifier>& a_vector)
	{
		auto nextComponentStart = parse<0>(a_value, a_vector);
		nextComponentStart = parse<1>(a_value, a_vector, nextComponentStart);
		nextComponentStart = parse<2>(a_value, a_vector, nextComponentStart);
		nextComponentStart = parse<3>(a_value, a_vector, nextComponentStart);
	}

	template <typename CharType, typename CharTraitsType, typename AllocatorType>
	inline void parse(
		std::string_view const a_value
		, std::basic_string<CharType, CharTraitsType, AllocatorType>& a_string
	)
	{
		if (a_value.size() >= 2 && a_value[0] == '\"' && a_value[a_value.size() - 1] == '\"')
		{
			a_string = a_value.substr(1, a_value.size() - 2);
		}
		else
		{
			ignorable_assert(false);
		}
	}

	struct InfoLineTokenParser
	{
		void operator()(
			std::string_view const a_tokenName
			, std::string_view const a_tokenValue
			, Font& a_font
			, FileSystemDatabase& a_database
			, fs::path const& a_fontPath
		)
		{
			if (a_tokenName.compare("face") == 0)
			{
				parse(a_tokenValue, a_font.m_name);
			}
			else if (a_tokenName.compare("size") == 0)
			{
				parse(a_tokenValue, a_font.m_size);
			}
			else if (a_tokenName.compare("bold") == 0)
			{
				parse(a_tokenValue, a_font.m_isBold);
			}
			else if (a_tokenName.compare("italic") == 0)
			{
				parse(a_tokenValue, a_font.m_isItalic);
			}
			else if (a_tokenName.compare("charset") == 0)
			{
				parse(a_tokenValue, a_font.m_charSet);
			}
			else if (a_tokenName.compare("unicode") == 0)
			{
				parse(a_tokenValue, a_font.m_isUnicode);
			}
			else if (a_tokenName.compare("stretchH") == 0)
			{
				parse(a_tokenValue, a_font.m_horizontalStretch);
			}
			else if (a_tokenName.compare("smooth") == 0)
			{
				parse(a_tokenValue, a_font.m_smooth);
			}
			else if (a_tokenName.compare("aa") == 0)
			{
				parse(a_tokenValue, a_font.m_antiAliasing);
			}
			else if (a_tokenName.compare("padding") == 0)
			{
				parse(a_tokenValue, a_font.m_padding);
			}
			else if (a_tokenName.compare("spacing") == 0)
			{
				parse(a_tokenValue, a_font.m_spacing);
			}
			else
			{
				ignorable_assert(false && "unknown token");
			}
		}
	};

	struct CommonLineTokenParser
	{
		void operator()(
			std::string_view const a_tokenName
			, std::string_view const a_tokenValue
			, Font& a_font
			, FileSystemDatabase& a_database
			, fs::path const& a_fontPath
		)
		{
			if (a_tokenName.compare("lineHeight") == 0)
			{
				parse(a_tokenValue, a_font.m_lineHeight);
			}
			else if (a_tokenName.compare("base") == 0)
			{
				parse(a_tokenValue, a_font.m_base);
			}
			else if (a_tokenName.compare("scaleW") == 0)
			{
				parse(a_tokenValue, a_font.m_scale.x);
			}
			else if (a_tokenName.compare("scaleH") == 0)
			{
				parse(a_tokenValue, a_font.m_scale.y);
			}
			else if (a_tokenName.compare("pages") == 0)
			{
				ignorable_assert(a_font.m_pages.empty());
				std::size_t pages;
				parse(a_tokenValue, pages);
				a_font.m_pages.resize(pages, data::Handle<GraphicResourceHandle<Texture>>{ a_database });
			}
			else if (a_tokenName.compare("packed") == 0)
			{
				parse(a_tokenValue, a_font.m_isPacked);
			}
			else
			{
				ignorable_assert(false && "unsupported token");
			}
		}
	};

	struct PageLineTokenParser
	{
		void operator()(
			std::string_view const a_tokenName
			, std::string_view const a_tokenValue
			, data::Handle<GraphicResourceHandle<Texture>>& a_page
			, FileSystemDatabase& a_database
			, fs::path const& a_fontPath
		)
		{
			if (a_tokenName.compare("id") == 0)
			{
				// TODO : noop, I assume it's in order but is it ok ?
			}
			else if (a_tokenName.compare("file") == 0)
			{
				u8string filePathStr;
				parse(a_tokenValue, filePathStr);
				auto const filePath = pathFromFilePath(filePathStr, a_fontPath);

				auto& loader = a_database.getMultiFileSystemLoader();
				auto& indexer = loader.getIndexer();
				a_page.setId(indexer.getId(filePath));
			}
			else
			{
				ignorable_assert(false && "unsupported token");
			}
		}
	};

	struct CountLineTokenParser
	{
		void operator()(
			std::string_view const a_tokenName
			, std::string_view const a_tokenValue
			, std::size_t& a_charCount
			, FileSystemDatabase& a_database
			, fs::path const& a_fontPath
			)
		{
			if (a_tokenName.compare("count") == 0)
			{
				parse(a_tokenValue, a_charCount);
			}
			else
			{
				ignorable_assert(false && "unsupported token");
			}
		}
	};

	struct CharLineTokenParser
	{
		void operator()(
			std::string_view const a_tokenName
			, std::string_view const a_tokenValue
			, FontCharacter& a_character
			, FileSystemDatabase& a_database
			, fs::path const& a_fontPath
			)
		{
			if (a_tokenName.compare("id") == 0)
			{
				parse(a_tokenValue, a_character.m_id);
			}
			else if (a_tokenName.compare("x") == 0)
			{
				parse(a_tokenValue, a_character.m_position.x);
			}
			else if (a_tokenName.compare("y") == 0)
			{
				parse(a_tokenValue, a_character.m_position.y);
			}
			else if (a_tokenName.compare("width") == 0)
			{
				parse(a_tokenValue, a_character.m_size.x);
			}
			else if (a_tokenName.compare("height") == 0)
			{
				parse(a_tokenValue, a_character.m_size.y);
			}
			else if (a_tokenName.compare("xoffset") == 0)
			{
				parse(a_tokenValue, a_character.m_offset.x);
			}
			else if (a_tokenName.compare("yoffset") == 0)
			{
				parse(a_tokenValue, a_character.m_offset.y);
				// a_character.m_offset.y *= -1;
			}
			else if (a_tokenName.compare("xadvance") == 0)
			{
				parse(a_tokenValue, a_character.m_xAdvance);
			}
			else if (a_tokenName.compare("page") == 0)
			{
				parse(a_tokenValue, a_character.m_page);
			}
			else if (a_tokenName.compare("chnl") == 0)
			{
				parse(a_tokenValue, a_character.m_channel);
			}
			else
			{
				ignorable_assert(false && "unsupported token");
			}
		}
	};

	struct KerningLineTokenParser
	{
		void operator()(
			std::string_view const a_tokenName
			, std::string_view const a_tokenValue
			, FontKerning& a_kerning
			, FileSystemDatabase& a_database
			, fs::path const& a_fontPath
			)
		{
			if (a_tokenName.compare("first") == 0)
			{
				parse(a_tokenValue, a_kerning.m_first);
			}
			else if (a_tokenName.compare("second") == 0)
			{
				parse(a_tokenValue, a_kerning.m_second);
			}
			else if (a_tokenName.compare("amount") == 0)
			{
				parse(a_tokenValue, a_kerning.m_amount);
			}
			else
			{
				ignorable_assert(false && "unsupported token");
			}
		}
	};

	template <typename TokenParserType, typename ObjectType>
	inline bool readToken(
		std::istream& a_inputStream
		, ObjectType& a_object
		, FileSystemDatabase& a_database
		, fs::path const& a_fontPath
	)
	{
		u8string token;
		if (a_inputStream >> token)
		{
			auto const tokenName = token.substr(0, token.find_first_of('='));
			auto const tokenValue = token.substr(token.find_first_of('=') + 1);
			TokenParserType{}(tokenName, tokenValue, a_object, a_database, a_fontPath);
			return true;
		}
		return false;
	}

	template <typename TokenParserType, typename ObjectType>
	inline void parseLine(
		std::istream& a_inputStream
		, ObjectType& a_object
		, FileSystemDatabase& a_database
		, fs::path const& a_fontPath
	)
	{
		while (readToken<TokenParserType>(a_inputStream, a_object, a_database, a_fontPath)) {}
	}

	class FontLoader
		: public AFileSystemLoader
	{
	public:
		explicit FontLoader(FileSystemDatabase& a_database)
			: m_database{ a_database }
		{}

		virtual bool canLoad(fs::path const& a_path) const override
		{
			return std::wcscmp(a_path.extension().c_str(), L".fnt") == 0;
		}

		virtual std::shared_ptr<type::ADynamicType> load(fs::path const& a_path) const override
		{
			Font font;
			std::ifstream file{ a_path, std::ios::in };
			std::istringstream line{};
			auto token = u8string{};

			if (!readNamedLine(file, "info", line))
			{
				return nullptr;
			}
			parseLine<InfoLineTokenParser>(line, font, m_database, a_path);

			if (!readNamedLine(file, "common", line))
			{
				return nullptr;
			}
			parseLine<CommonLineTokenParser>(line, font, m_database, a_path);

			for (auto& page : font.m_pages)
			{
				if (!readNamedLine(file, "page", line))
				{
					return nullptr;
				}
				parseLine<PageLineTokenParser>(line, page, m_database, a_path);
			}

			if (!readNamedLine(file, "chars", line))
			{
				return nullptr;
			}
			std::size_t charCount = 0;
			parseLine<CountLineTokenParser>(line, charCount, m_database, a_path);

			for (auto k = 0; k < charCount; ++k)
			{
				if (!readNamedLine(file, "char", line))
				{
					return nullptr;
				}
				FontCharacter character;
				parseLine<CharLineTokenParser>(line, character, m_database, a_path);
				if (character.m_id == g_invalidCharacterId)
				{
					ignorable_assert(false && "invalid character");
					return nullptr;
				}
				font.setCharacter(character);
			}

			if (!readNamedLine(file, "kernings", line))
			{
				return nullptr;
			}
			std::size_t kerningsCount = 0;
			parseLine<CountLineTokenParser>(line, kerningsCount, m_database, a_path);

			for (auto k = 0; k < kerningsCount; ++k)
			{
				if (!readNamedLine(file, "kerning", line))
				{
					return nullptr;
				}

				FontKerning kerning;
				parseLine<KerningLineTokenParser>(line, kerning, m_database, a_path);
				if (kerning.m_first == g_invalidCharacterId || kerning.m_second == g_invalidCharacterId)
				{
					ignorable_assert(false && "invalid kerning");
					return nullptr;
				}
				font.setKerning(kerning);
			}

			return std::make_shared<Font>(std::move(font));
		}

	private:
		FileSystemDatabase& m_database;
	};
}
