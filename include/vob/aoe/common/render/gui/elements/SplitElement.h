#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>
#include <vob/aoe/common/render/gui/elements/AStandardElement.h>

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::aoe::common
{
	class SplitElement
		: public AStandardElement
	{
    public:
#pragma region Types
        enum class Side
        {
            Top = 0
            , Bottom
            , Left
            , Right
        };
#pragma endregion

#pragma region Attributes
        Side m_firstSide = Side::Top;
        float m_firstSideSize = 0.0f;

		type::dynamic_type_clone<AElement> m_firstChildElement;
		type::dynamic_type_clone<AElement> m_secondChildElement;
#pragma endregion

#pragma region Constructors
		explicit SplitElement(type::dynamic_type_clone_copier const& a_cloner)
			: m_firstChildElement{ a_cloner }
			, m_secondChildElement{ a_cloner }
		{}
#pragma endregion

#pragma region Methods
		bool onEvent(WindowEvent const& a_event, GuiTransform a_transform) override
		{
			auto childTransforms = getChildTransforms(a_transform);
			
			if (m_firstChildElement != nullptr)
			{
				if (m_firstChildElement->onEvent(a_event, childTransforms[0]))
				{
					return true;
				}
			}

			if (m_secondChildElement != nullptr)
			{
				return m_secondChildElement->onEvent(a_event, childTransforms[1]);
			}

			return false;
		}

		virtual void renderContent(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform const a_transform
		) const override
		{
			auto childTransforms = getChildTransforms(a_transform);

			if (m_firstChildElement != nullptr)
			{
				m_firstChildElement->render(a_shaderProgram, a_renderContext, childTransforms[0]);
			}
			if (m_secondChildElement != nullptr)
			{
				m_secondChildElement->render(a_shaderProgram, a_renderContext, childTransforms[1]);
			}
		}

#pragma endregion

        template <typename VisitorType, typename Self>
        static bool accept(VisitorType& a_visitor, Self& a_this)
        {
			AStandardElement::accept(a_visitor, a_this);

            a_visitor.visit(misvi::nvp("First Side", a_this.m_firstSide));
            a_visitor.visit(misvi::nvp("First Side Size", a_this.m_firstSideSize));
            a_visitor.visit(misvi::nvp("First Child Element", a_this.m_firstChildElement));
            a_visitor.visit(misvi::nvp("Second Child Element", a_this.m_secondChildElement));
			return true;
        }

    private:
#pragma region Methods
		std::array<GuiTransform, 2> getChildTransforms(const GuiTransform& a_transform) const
		{
			auto firstChildTransform = a_transform;
			auto secondChildTransform = a_transform;
			switch (m_firstSide)
			{
			case Side::Top:
				firstChildTransform.m_size.y = std::min(a_transform.m_size.y, m_firstSideSize);
                secondChildTransform.m_size.y -= firstChildTransform.m_size.y;
                secondChildTransform.m_position.y += firstChildTransform.m_size.y;
				break;
			case Side::Bottom:
				secondChildTransform.m_size.y = std::min(a_transform.m_size.y, m_firstSideSize);
                firstChildTransform.m_size.y -= secondChildTransform.m_size.y;
                secondChildTransform.m_position.y += firstChildTransform.m_size.y;
                std::swap(firstChildTransform, secondChildTransform);
				break;
			case Side::Left:
				firstChildTransform.m_size.x = std::min(a_transform.m_size.x, m_firstSideSize);
                secondChildTransform.m_size.x -= firstChildTransform.m_size.x;
                secondChildTransform.m_position.x += firstChildTransform.m_size.x;
				break;
			case Side::Right:
				secondChildTransform.m_size.x = std::min(a_transform.m_size.x, m_firstSideSize);
                firstChildTransform.m_size.x -= secondChildTransform.m_size.x;
				secondChildTransform.m_position.x += firstChildTransform.m_size.x;
				std::swap(firstChildTransform, secondChildTransform);
				break;
			}

			return { firstChildTransform, secondChildTransform };
        }
#pragma endregion
	};
}

