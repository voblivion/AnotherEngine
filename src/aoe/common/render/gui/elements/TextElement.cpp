#include <vob/aoe/common/render/gui/elements/TextElement.h>

namespace vob::aoe::common
{
    void TextElement::setText(sta::utf8_string a_text)
    {
        m_text = std::move(a_text);
        m_hasChanged = true;
    }

    void TextElement::setFont(std::shared_ptr<aoe::common::Font const> a_font)
    {
        m_font = std::move(a_font);
        m_hasChanged = true;
    }

    auto const& TextElement::getFont() const
    {
        return m_font;
    }

    void TextElement::setSize(float const a_size)
    {
        m_size = a_size;
        m_hasChanged = true;
    }

    bool TextElement::onEvent(WindowEvent const& a_event, GuiTransform a_transform)
    {
        if (AStandardElement::onEvent(a_event, a_transform))
        {
            return true;
        }

        std::visit([this, a_transform](auto const& a_event)
            {
                using EventType = std::decay_t<decltype(a_event)>;

                if constexpr (std::is_same_v<EventType, MouseButtonEvent>)
                {
                    auto const& mouseButtonEvent =
                        static_cast<MouseButtonEvent const&>(a_event);

                    if (mouseButtonEvent.m_button == GLFW_MOUSE_BUTTON_LEFT)
                    {
                        m_hasChanged = true;
                        auto relativePos = m_mousePos - a_transform.m_position;
                        std::optional<sta::unicode> previousCode = std::nullopt;
                        auto index = getIndexAtPos(
                            *m_font
                            , m_text
                            , relativePos
                            , m_size
                            , a_transform.m_size
                            , previousCode
                        );
                        if (mouseButtonEvent.m_pressed)
                        {
                            m_isSelecting = true;
                            m_selectionStart = index;
                            m_selectionEnd = index;
                        }
                        else
                        {
                            m_isSelecting = false;
                            m_selectionEnd = index;
                        }
                        m_hasChanged = true;
                    }
                }
                if constexpr (std::is_same_v<EventType, MouseMoveEvent>)
                {
                    auto const mouseMoveEvent =
                        static_cast<MouseMoveEvent const&>(a_event);
                    m_mousePos = vec2{ mouseMoveEvent.m_position };

                    auto relativePos = m_mousePos - a_transform.m_position;
                    std::optional<sta::unicode> previousCode = std::nullopt;
                    if (m_isSelecting)
                    {
                        m_selectionEnd = getIndexAtPos(
                            *m_font
                            , m_text
                            , relativePos
                            , m_size
                            , a_transform.m_size
                            , previousCode
                        );
                        m_hasChanged = true;
                    }
                }
            }, a_event);

        return false;
    }

    void TextElement::renderContent(
        GuiShaderProgram const& a_shaderProgram
        , GuiRenderContext& a_renderContext
        , GuiTransform const a_transform
    ) const
    {
        if (!m_preSelectionMesh->isReady()
            || !m_selectionMesh->isReady()
            || !m_postSelectionMesh->isReady()
            || m_font == nullptr
            || m_font->m_pages[0] == nullptr
            || !(*m_font->m_pages[0])->isReady())
        {
            return;
        }

        if (needsTextMeshUpdate(a_transform))
        {
            std::optional<vec2> cursor = std::nullopt;
            std::optional<sta::unicode> previousCode = std::nullopt;

            auto vertices = createTextMesh(
                *m_font
                , m_text
                , 0
                , std::min(m_selectionStart, m_selectionEnd)
                , m_size
                , a_transform.m_size
                , cursor
                , previousCode
            );
            m_preSelectionMesh->setVertices(vertices.data(), static_cast<u32>(vertices.size()));
            vertices = createTextMesh(
                *m_font
                , m_text
                , std::min(m_selectionStart, m_selectionEnd)
                , std::max(m_selectionStart, m_selectionEnd)
                , m_size
                , a_transform.m_size
                , cursor
                , previousCode
            );
            m_selectionMesh->setVertices(vertices.data(), static_cast<u32>(vertices.size()));
            vertices = createTextMesh(
                *m_font
                , m_text
                , std::max(m_selectionStart, m_selectionEnd)
                , m_text.size()
                , m_size
                , a_transform.m_size
                , cursor
                , previousCode
            );
            m_postSelectionMesh->setVertices(vertices.data(), static_cast<u32>(vertices.size()));
            m_hasChanged = false;
            m_lastRenderedSize = a_transform.m_size;
        }

        glActiveTexture(GL_TEXTURE0 + 0);
        (*m_font->m_pages[0])->bind(GL_TEXTURE_2D);
        a_shaderProgram.setRenderType(GuiRenderType::DistanceFieldFill);
        a_shaderProgram.setElementPosition(a_transform.m_position);
        a_shaderProgram.setElementSize(a_transform.m_size);

        a_shaderProgram.setColor(m_color);
        m_preSelectionMesh.resource()->render();

        a_shaderProgram.setColor(vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        m_selectionMesh.resource()->render();

        a_shaderProgram.setColor(m_color);
        m_postSelectionMesh.resource()->render();
    }
}