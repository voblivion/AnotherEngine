#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/gui/CanvasComponent.h>
#include <vob/aoe/common/gui/ObjectComponent.h>


namespace vob::aoe::common::gui
{
	struct RenderSystem
	{
		// Alias
		using CanvasComponents = ecs::ComponentTypeList<
			CanvasComponent const
			, HierarchyComponent const
		>;

		using ObjectComponents = ecs::ComponentTypeList<
			ObjectComponent const
			, HierarchyComponent const
		>;

		// Constructors
		explicit VOB_AOE_API RenderSystem(ecs::WorldDataProvider& a_wdp);

		// Methods
		void VOB_AOE_API update() const;

		void VOB_AOE_API render(
			CanvasComponents::EntityType const& a_canvas
		) const;

		void VOB_AOE_API render(
			ObjectEntity const& a_objectEntity
			, Vector2 const& a_canvasSize
		) const;

	private:
		// Attributes
		ecs::EntityList<CanvasComponent const, HierarchyComponent const> const& m_canvasList;

		ObjectEntityList const& m_objectEntityList;
	};
}
