#include <vob/aoe/common/render/gui/elements/TextInputElement.h>

#include <string_view>


namespace vob::aoe::common
{
    bool TextInputElement::onEvent(WindowEvent const& a_event, GuiTransform a_transform)
    {
        if (TextElement::onEvent(a_event, a_transform))
        {
            return true;
        }

        std::visit([this](auto const& a_event)
            {
                using EventType = std::decay_t<decltype(a_event)>;

                if constexpr (std::is_same_v<EventType, TextEvent>)
                {
                    auto const& textEvent = static_cast<TextEvent const&>(a_event);
                    insert(textEvent.m_unicode);
                }
                if constexpr (std::is_same_v<EventType, MouseButtonEvent>)
                {
                    auto const& mouseButtonEvent =
                        static_cast<MouseButtonEvent const&>(a_event);
                    if (mouseButtonEvent.m_button == GLFW_MOUSE_BUTTON_LEFT)
                    {
                        if (mouseButtonEvent.m_pressed)
                        {

                        }
                    }
                }
                if constexpr (std::is_same_v<EventType, MouseMoveEvent>)
                {
                    auto const mouseMoveEvent =
                        static_cast<MouseMoveEvent const&>(a_event);
                    m_mousePosition = mouseMoveEvent.m_position;
                    if (m_isMouseLeftDown)
                    {

                    }
                }
                if constexpr (std::is_same_v<EventType, KeyEvent>)
                {
                    auto const& keyEvent = static_cast<KeyEvent const&>(a_event);
                    if (keyEvent.m_action == KeyEvent::Action::Release)
                    {
                        return;
                    }

                    switch (keyEvent.m_keyCode)
                    {
                    case GLFW_KEY_BACKSPACE:
                    {
                        auto tmp = std::min(m_selectionStart, m_selectionEnd);
                        m_selectionEnd = std::max(m_selectionStart, m_selectionEnd);
                        m_selectionStart = tmp;
                        if (m_selectionEnd != 0)
                        {
                            if (m_selectionStart != m_selectionEnd)
                            {
                                erase(m_selectionEnd - m_selectionStart);
                            }
                            else if (keyEvent.m_modifierMask.hasModifier(Modifier::Control))
                            {
                                auto wordSize = m_selectionEnd - prevWordStart();
                                m_selectionEnd -= wordSize;
                                m_selectionStart = m_selectionEnd;
                                erase(wordSize);
                            }
                            else
                            {
                                --m_selectionEnd;
                                m_selectionStart = m_selectionEnd;
                                erase(1);
                            }
                        }
                        break;
                    }
                    case GLFW_KEY_DELETE:
                    {
                        if (m_selectionEnd < m_text.size())
                        {
                            if (m_selectionStart != m_selectionEnd)
                            {
                                erase(m_selectionEnd - m_selectionStart);
                            }
                            else if (keyEvent.m_modifierMask.hasModifier(Modifier::Control))
                            {
                                erase(nextWordStart() - m_selectionEnd);
                            }
                            else
                            {
                                erase(1);
                            }
                        }
                        break;
                    }
                    case GLFW_KEY_LEFT:
                    {
                        if (m_selectionEnd != 0)
                        {
                            if (keyEvent.m_modifierMask.hasModifier(Modifier::Control))
                            {
                                m_selectionEnd = prevWordStart();
                            }
                            else
                            {
                                --m_selectionEnd;
                            }
                            if (!keyEvent.m_modifierMask.hasModifier(Modifier::Shift))
                            {
                                m_selectionStart = m_selectionEnd;
                            }
                            m_hasChanged = true;
                            m_needsCursorUpdate = true;
                        }
                        break;
                    }
                    case GLFW_KEY_RIGHT:
                    {
                        if (m_selectionEnd < m_text.size())
                        {
                            if (keyEvent.m_modifierMask.hasModifier(Modifier::Control))
                            {
                                m_selectionEnd = nextWordStart();
                            }
                            else
                            {
                                ++m_selectionEnd;
                            }
                            if (!keyEvent.m_modifierMask.hasModifier(Modifier::Shift))
                            {
                                m_selectionStart = m_selectionEnd;
                            }
                            m_hasChanged = true;
                            m_needsCursorUpdate = true;
                        }
                        break;
                    }
                    case GLFW_KEY_ENTER:
                    {
                        insert(g_newlineCode);
                        break;
                    }
                    case GLFW_KEY_PAGE_UP:
                    {
                        if (keyEvent.m_modifierMask.hasModifier(Modifier::Shift))
                        {
                            m_lineHeight = m_lineHeight.value_or(m_font->m_lineHeight) + 2;
                        }
                        else
                        {
                            m_size += 2;
                        }
                        m_hasChanged = true;
                        break;
                    }
                    case GLFW_KEY_PAGE_DOWN:
                    {
                        if (keyEvent.m_modifierMask.hasModifier(Modifier::Shift))
                        {
                            m_lineHeight = m_lineHeight.value_or(m_font->m_lineHeight) - 2;
                        }
                        else
                        {
                            m_size -= 2;
                        }
                        m_hasChanged = true;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                    }

                }
            }, a_event);
        return false;
    }

    void TextInputElement::renderContent(
        GuiShaderProgram const& a_shaderProgram
        , GuiRenderContext& a_renderContext
        , GuiTransform a_transform) const
    {
        if (m_font == nullptr
            || m_font->m_pages[0] == nullptr
            || !(*m_font->m_pages[0])->isReady())
        {
            return;
        }

        if (m_needsCursorUpdate || m_hasChanged)
        {
            m_cursorTransform = getCursorTransform(
                *m_font
                , m_text
                , m_selectionEnd
                , m_size
                , a_transform.m_size
            );

            m_lastCursorUpdateTime = a_renderContext.m_frameStartTime;
            m_needsCursorUpdate = false;
        }

        auto timeSinceCursorUpdate = a_renderContext.m_frameStartTime - m_lastCursorUpdateTime;
        auto halfSecondsSinceCursorUpdate = timeSinceCursorUpdate.count() / 500000000;
        if (halfSecondsSinceCursorUpdate % 2 == 0)
        {
            a_shaderProgram.setRenderType(GuiRenderType::QuadFill);
            a_shaderProgram.setOuterCornerRadius(glm::vec4{});
            a_shaderProgram.setElementPosition(
                a_transform.m_position + m_cursorTransform.first
            );
            a_shaderProgram.setElementSize(m_cursorTransform.second);
            a_shaderProgram.setColor(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
            a_renderContext.m_quad->render();
        }

        TextElement::renderContent(a_shaderProgram, a_renderContext, a_transform);
    }

    void TextInputElement::insert(char32_t a_unicode)
    {
        auto tmp = std::min(m_selectionStart, m_selectionEnd);
        m_selectionEnd = std::max(m_selectionStart, m_selectionEnd);
        m_selectionStart = tmp;
        erase(m_selectionEnd - m_selectionStart);
        m_text.insert(m_text.begin() + (m_selectionEnd++), a_unicode);
        m_selectionStart = m_selectionEnd;
        m_hasChanged = true;
        m_needsCursorUpdate = true;
    }

    void TextInputElement::erase(std::size_t a_count)
    {
        m_text.erase(m_selectionStart, a_count);
        m_selectionEnd = m_selectionStart;
        m_needsCursorUpdate = true;
        m_hasChanged = true;
    }

    size_t TextInputElement::prevWordStart() const
    {
        auto text = std::u32string_view{ m_text }.substr(0, m_selectionEnd);
        auto wordEnd = text.find_last_not_of(g_spaceCode) + 1;
        text = text.substr(0, wordEnd);
        auto wordStart = text.find_last_of(g_spaceCode);
        return wordStart == std::u32string_view::npos ? 0 : wordStart;
    }

    size_t TextInputElement::nextWordStart() const
    {
        auto text = std::u32string_view{ m_text }.substr(m_selectionEnd);
        auto wordEnd = std::min(text.size(), text.find_first_of(g_spaceCode));
        text = text.substr(wordEnd);
        auto wordStart = text.find_first_not_of(g_spaceCode);
        return wordStart == std::u32string_view::npos
            ? m_text.size()
            : m_selectionEnd + wordEnd + wordStart;
    }
}