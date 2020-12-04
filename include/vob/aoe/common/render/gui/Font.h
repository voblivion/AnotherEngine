#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <vob/aoe/core/type/Primitive.h>
#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>

namespace vob::aoe::common
{
	class Texture;

	constexpr char g_invalidCharacterId = -1;

	struct FontCharacter
	{
		u32 m_id = g_invalidCharacterId;
		i16vec2 m_position = {};
		i16vec2 m_size = {};
		i16vec2 m_offset = {};
		u32 m_xAdvance = 0;
		u32 m_page = 0;
		u32 m_channel = 0;
	};

	struct FontKerning
	{
		u32 m_first = g_invalidCharacterId;
		u32 m_second = g_invalidCharacterId;
		i32 m_amount = 0;
	};

	struct Font
		: public type::ADynamicType
	{
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
		std::vector<data::Handle<GraphicResourceHandle<Texture>>> m_pages;

		FontCharacter const* findCharacter(char const a_char) const
		{
			auto const continuousCharacterSetIt = findContinuousCharacterSet(a_char);

			if (continuousCharacterSetIt != m_continuousCharacterSetList.rend())
			{
				return findCharacter(*continuousCharacterSetIt, a_char);
			}

			return nullptr;
		}

		void setCharacter(FontCharacter a_character)
		{
			if (!updateCharacter(a_character))
			{
				addCharacter(a_character);
			}
		}

		void setKerning(FontKerning a_kerning)
		{
			m_kerningAmounts[std::make_pair(a_kerning.m_first, a_kerning.m_second)] = a_kerning.m_amount;
		}

		i32 getKerningAmount(u32 const a_firstCharacterId, u32 const a_secondCharacterId) const
		{
			auto const kerningIt = m_kerningAmounts.find({ a_firstCharacterId, a_secondCharacterId });
			if (kerningIt != m_kerningAmounts.end())
			{
				return kerningIt->second;
			}
			return 0;
		}

	private:
		using ContinuousCharacterSet = std::pair<u32, std::vector<FontCharacter>>;
		using ContinuousCharacterSetList = std::vector<ContinuousCharacterSet>;

		ContinuousCharacterSetList m_continuousCharacterSetList;
		std::unordered_map<std::pair<u32, u32>, i32> m_kerningAmounts;

		FontCharacter* findCharacter(u32 const a_unicode)
		{
			auto const continuousCharacterSetIt = findContinuousCharacterSet(a_unicode);

			if (continuousCharacterSetIt != m_continuousCharacterSetList.rend())
			{
				return findCharacter(*continuousCharacterSetIt, a_unicode);
			}

			return nullptr;
		}

		bool updateCharacter(FontCharacter const& a_character)
		{
			if (auto const existingCharacter = findCharacter(a_character.m_id))
			{
				*existingCharacter = a_character;
				return true;
			}
			return false;
		}

		void addCharacter(FontCharacter const& a_character)
		{
			std::size_t insertSetIndex;
			
			auto const insertSetRIt = findContinuousCharacterSet(a_character.m_id);
			if (insertSetRIt == m_continuousCharacterSetList.rend())
			{
				m_continuousCharacterSetList.emplace(
					m_continuousCharacterSetList.begin()
					, std::make_pair(a_character.m_id, std::vector<FontCharacter>{ a_character })
				);
				insertSetIndex = 0;
			}
			else
			{
				insertSetIndex = std::distance(insertSetRIt, m_continuousCharacterSetList.rend()) - 1;
				auto const offsetInSet = a_character.m_id - insertSetRIt->first;
				if (offsetInSet == insertSetRIt->second.size())
				{
					insertSetRIt->second.emplace_back(a_character);
				}
				else
				{
					insertSetIndex += 1;
					m_continuousCharacterSetList.emplace(
						insertSetRIt.base()
						, std::make_pair(a_character.m_id, std::vector<FontCharacter>{ a_character })
					);
				}
			}

			auto const insertSetIt = m_continuousCharacterSetList.begin() + insertSetIndex;
			auto & insertSetList = insertSetIt->second;
			if (insertSetIt != m_continuousCharacterSetList.end())
			{
				auto const mergeSetIt = insertSetIt + 1;
				if (mergeSetIt < m_continuousCharacterSetList.end())
				{
					auto const& mergeSetList = mergeSetIt->second;
					if (mergeSetIt->first == a_character.m_id + 1)
					{
						insertSetList.reserve(insertSetList.size() + mergeSetList.size());
						insertSetList.insert(insertSetList.begin(), mergeSetList.begin(), mergeSetList.end());
						m_continuousCharacterSetList.erase(mergeSetIt);
					}
				}
			}
		}

		ContinuousCharacterSetList::const_reverse_iterator findContinuousCharacterSet(
			u32 const a_unicode
		) const
		{
			return std::upper_bound(
				m_continuousCharacterSetList.rbegin()
				, m_continuousCharacterSetList.rend()
				, a_unicode
				, [](u32 const a_unicode, auto const& a_continuousCharacterSet)
				{
					return a_unicode >= a_continuousCharacterSet.first;
				}
			);
		}

		static FontCharacter const* findCharacter(
			ContinuousCharacterSet const& a_continuousCharacterSet
			, u32 const a_unicode
		)
		{
			auto const offsetInSet = a_unicode - a_continuousCharacterSet.first;
			if (offsetInSet < a_continuousCharacterSet.second.size())
			{
				return &a_continuousCharacterSet.second[offsetInSet];
			}
			return nullptr;
		}

		ContinuousCharacterSetList::reverse_iterator findContinuousCharacterSet(
			u32 const a_unicode
		)
		{
			return std::upper_bound(
				m_continuousCharacterSetList.rbegin()
				, m_continuousCharacterSetList.rend()
				, a_unicode
				, [](u32 const a_unicode, auto const& a_continuousCharacterSet)
			{
				return a_unicode >= a_continuousCharacterSet.first;
			}
			);
		}

		static FontCharacter* findCharacter(
			ContinuousCharacterSet& a_continuousCharacterSet
			, u32 const a_unicode
		)
		{
			auto const offsetInSet = a_unicode - a_continuousCharacterSet.first;
			if (offsetInSet < a_continuousCharacterSet.second.size())
			{
				return &a_continuousCharacterSet.second[offsetInSet];
			}
			return nullptr;
		}
	};
}

