#pragma once

#include <type_traits>
#include <vob/aoe/common/_render/gui/elements/TextInputElement.h>
#include <vob/aoe/common/_render/gui/elements/SplitElement.h>

#include "vob/misc/visitor/name_value_pair.h"
#include "vob/misc/visitor/is_visitable.h"


namespace vob::aoe::vis
{
	struct InputElement
	{
		virtual void update() = 0;
	};

	template <typename Type, typename = std::enable_if_t<std::is_arithmetic_v<Type>>>
	struct NumberInputElement : public InputElement
	{
		NumberInputElement(common::TextInputElement& a_source, Type& a_target)
			: m_source{ a_source }
			, m_target{ a_target }
		{}

		virtual void update() override
		{
			std::stringstream stream{ m_source.m_text.data() };
			stream >> m_target;
		}

		common::TextInputElement& m_source;
		Type& m_target;
	};

	struct EditionInterface
	{
		std::vector<std::unique_ptr<InputElement>> m_inputElements;
	};

	class EditorVisitor
	{
	public:
		explicit EditorVisitor(
			std::shared_ptr<common::Font const> a_font
			, data::ADatabase& a_database
			, common::IGraphicResourceManager<common::GuiMesh>& a_guiMeshResourceManager
			, type::dynamic_type_clone_copier& a_cloner)
			: m_font{ a_font }
			, m_database{ a_database }
			, m_guiMeshResourceManager{ a_guiMeshResourceManager }
			, m_cloner{ a_cloner }
		{

		}

		template <typename ValueType>
		EditionInterface generateEditionInterface(
			type::dynamic_type_clone<common::AElement>& a_root
			, ValueType& a_value
		)
		{
			auto& split = a_root.init<common::SplitElement>(m_database, m_cloner);
			split.m_firstSide = common::SplitElement::Side::Top;
			split.m_firstSideSize = 10;
			m_listSplit = &split;
			m_attributeSplit = nullptr;

			visit(a_value);

			return std::move(m_editionInterface);
		}

		template <typename ValueType>
		requires std::is_arithmetic_v<ValueType>
		void visit(ValueType& a_number)
		{
			if (m_attributeSplit != nullptr)
			{
				auto& numberInput = m_attributeSplit->m_secondChildElement.init<common::TextInputElement>(
					m_database
					, m_guiMeshResourceManager
				);
				m_editionInterface.m_inputElements.emplace_back(
					std::make_unique<NumberInputElement<ValueType>>(numberInput, a_number)
				);
				numberInput.m_text = std::to_string(a_number);
				numberInput.setFont(m_font);
				m_attributeSplit = nullptr;
			}
		}

		template <typename ValueType>
		void visit(misvi::name_value_pair<ValueType> a_nameValuePair)
		{
			// new vertical split
			{
				auto& split = m_listSplit->m_secondChildElement.init<common::SplitElement>(m_database, m_cloner);
				split.m_firstSide = common::SplitElement::Side::Top;
				split.m_firstSideSize = 50;
				m_listSplit = &split;
			}

			// new horizontal split
            {
                auto& split = m_listSplit->m_firstChildElement.init<common::SplitElement>(m_database, m_cloner);
				split.m_firstSide = common::SplitElement::Side::Left;
				split.m_firstSideSize = 250;
				m_attributeSplit = &split;

				auto& name = m_attributeSplit->m_firstChildElement.init<common::TextElement>(
					m_database
					, m_guiMeshResourceManager
				);
				name.setFont(m_font);
				name.setText(std::pmr::u32string{ a_nameValuePair.m_name });
			}

			visit(a_nameValuePair.m_value);
		}

		template <typename ValueType>
		requires misvi::is_visitable_free<EditorVisitor, ValueType> && (!std::is_arithmetic_v<ValueType>)
		void visit(ValueType& a_object)
		{
			accept(*this, a_object);
		}

		template <typename ValueType>
		requires misvi::is_visitable_static<EditorVisitor, ValueType>
		void visit(ValueType& a_object)
		{
			ValueType::accept(*this, a_object);
		}

		std::shared_ptr<common::Font const> m_font;
		data::ADatabase& m_database;
		common::IGraphicResourceManager<common::GuiMesh>& m_guiMeshResourceManager;
		type::dynamic_type_clone_copier& m_cloner;
		common::SplitElement* m_listSplit = nullptr;
		common::SplitElement* m_attributeSplit = nullptr;
		EditionInterface m_editionInterface;
	};
}