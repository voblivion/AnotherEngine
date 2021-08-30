#include <vob/aoe/common/render/gui/text/Font.h>

namespace vob::aoe::common
{
    namespace
    {
        template <typename ContinuousCharacterSetList>
        auto findContinuousCharacterSet(
            ContinuousCharacterSetList& a_continuousCharacterSetList
            , sta::unicode const a_unicode
        )
        {
            return std::upper_bound(
                a_continuousCharacterSetList.rbegin()
                , a_continuousCharacterSetList.rend()
                , a_unicode
                , [](auto const a_unicode, auto& a_continuousCharacterSet)
                {
                    return a_unicode >= a_continuousCharacterSet.first;
                }
            );
        }

        template <typename ContinuousCharacterSet>
        FontCharacter const* findCharacterInSet(
            ContinuousCharacterSet& a_continuousCharacterSet
            , sta::unicode const a_unicode
        )
        {
            auto const offsetInSet = a_unicode - a_continuousCharacterSet.first;
            if (offsetInSet < a_continuousCharacterSet.second.size())
            {
                return &a_continuousCharacterSet.second[offsetInSet];
            }
            return nullptr;
        }
    }

    FontCharacter const* Font::findCharacter(sta::unicode const a_unicode) const
    {
        auto const continuousCharacterSetIt = findContinuousCharacterSet(
            m_continuousCharacterSetList
            , a_unicode
        );
        if (continuousCharacterSetIt != m_continuousCharacterSetList.rend())
        {
            return findCharacterInSet(*continuousCharacterSetIt, a_unicode);
        }

        return nullptr;
    }

    void Font::setCharacter(FontCharacter const& a_character)
    {
        auto character = findCharacter(a_character.m_id);

        // Update character
        if (character != nullptr)
        {
            const_cast<FontCharacter&>(*character) = a_character;
            return;
        }
        // Add character
        else
        {
            std::size_t insertSetIndex;
            auto const insertSetRIt = findContinuousCharacterSet(
                m_continuousCharacterSetList
                , a_character.m_id
            );
            // Create new set in front
            if (insertSetRIt == m_continuousCharacterSetList.rend())
            {
                insertSetIndex = 0;
                m_continuousCharacterSetList.emplace(
                    m_continuousCharacterSetList.begin()
                    , std::make_pair(
                        a_character.m_id
                        , std::vector<FontCharacter>{ a_character }
                    )
                );
            }
            else
            {
                // Add Character at the end of set
                insertSetIndex = std::distance(
                    insertSetRIt
                    , m_continuousCharacterSetList.rend()
                ) - 1;
                auto const offsetInSet = a_character.m_id - insertSetRIt->first;
                if (offsetInSet == insertSetRIt->second.size())
                {
                    insertSetRIt->second.emplace_back(a_character);
                }
                // Create new set after
                else
                {
                    insertSetIndex += 1;
                    m_continuousCharacterSetList.emplace(
                        insertSetRIt.base()
                        , std::make_pair(
                            a_character.m_id
                            , std::vector<FontCharacter>{ a_character }
                        )
                    );
                }
            }

            // Merge sets
            auto const insertSetIt = m_continuousCharacterSetList.begin() + insertSetIndex;
            auto& insertSetList = insertSetIt->second;
            auto const mergeSetIt = insertSetIt + 1;
            if (mergeSetIt != m_continuousCharacterSetList.end())
            {
                auto const& mergeSetList = mergeSetIt->second;
                if (mergeSetIt->first == a_character.m_id + 1)
                {
                    insertSetList.reserve(insertSetList.size() + mergeSetList.size());
                    insertSetList.insert(
                        insertSetList.end()
                        , mergeSetList.begin()
                        , mergeSetList.end()
                    );
                    m_continuousCharacterSetList.erase(mergeSetIt);
                }
            }
        }
    }

    i32 Font::getKerningAmount(
        sta::unicode const a_firstCode
        , sta::unicode const a_secondCode
    ) const
    {
        auto const kerningIt = m_kerningAmounts.find(
            std::make_pair(a_firstCode, a_secondCode)
        );
        if (kerningIt != m_kerningAmounts.end())
        {
            return kerningIt->second;
        }
        return 0;
    }

    void Font::setKerning(FontKerning a_kerning)
    {
        m_kerningAmounts[a_kerning.m_sequence] = a_kerning.m_amount;
    }
}