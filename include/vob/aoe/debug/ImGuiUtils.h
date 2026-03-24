#pragma once

#include "vob/aoe/debug/DebugNameComponent.h"
#include "vob/aoe/debug/DebugNameUtils.h"

#include "vob/misc/std/enum_traits.h"

#include "entt/entt.hpp"
#include "imgui.h"


namespace vob::aoedb
{
	template<typename TView>
	bool ImGuiEntityCombo(
		char const* a_label,
		entt::entity* a_selectedEntity,
		TView a_entities,
		entt::view<entt::get_t<DebugNameComponent const>> a_debugNameEntities)
	{
		bool valueChanged = false;
		if (ImGui::BeginCombo(a_label, getImmediateUseDebugNameCStr(a_debugNameEntities, *a_selectedEntity)))
		{
			auto const isNullSelected = !a_entities.contains(*a_selectedEntity);
			if (ImGui::Selectable(aoedb::getImmediateUseDebugNameCStr(a_debugNameEntities, entt::null), isNullSelected))
			{
				valueChanged = a_entities.contains(*a_selectedEntity);
				*a_selectedEntity = entt::null;

				if (isNullSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			for (auto entity : a_entities)
			{
				auto const isSelected = *a_selectedEntity == entity;
				if (ImGui::Selectable(getImmediateUseDebugNameCStr(a_debugNameEntities, entity), isSelected))
				{
					valueChanged = *a_selectedEntity != entity;
					*a_selectedEntity = entity;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
		return valueChanged;
	}

	template <typename TEnum, size_t t_maxStringSize = 20>
	bool ImGuiEnumCombo(char const* a_label, TEnum* a_currentEnumValue, ImGuiComboFlags a_flags = 0)
	{
		bool valueChanged = false;

		auto const toSmallStr = [](std::string_view a_stringView)
			{
				auto const size = std::min(a_stringView.size(), t_maxStringSize);
				std::array<char, t_maxStringSize + 1> smallStr;
				std::memcpy(smallStr.data(), a_stringView.data(), size);
				smallStr[size] = 0;
				return smallStr;
			};

		auto const currentValueName = mistd::enum_traits<TEnum>::cast(*a_currentEnumValue).value_or("");
		auto const currentValueStr = toSmallStr(currentValueName.substr(currentValueName.rfind(":") + 1));
		if (ImGui::BeginCombo(a_label, currentValueStr.data(), a_flags))
		{
			for (auto const [enumValue, enumValueName] : mistd::enum_traits<TEnum>::valid_value_name_pairs)
			{
				auto const enumValueStr = toSmallStr(enumValueName.substr(enumValueName.rfind(":") + 1));
				if (ImGui::Selectable(enumValueStr.data(), enumValue == *a_currentEnumValue))
				{
					valueChanged = *a_currentEnumValue != enumValue;
					*a_currentEnumValue = enumValue;
				}

				if (*a_currentEnumValue == enumValue)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		return valueChanged;
	}
}