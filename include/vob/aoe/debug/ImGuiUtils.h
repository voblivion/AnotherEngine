#pragma once

#include <vob/aoe/debug/DebugNameComponent.h>
#include "vob/aoe/debug/DebugNameUtils.h"

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
}