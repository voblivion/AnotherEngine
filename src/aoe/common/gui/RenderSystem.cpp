#include <vob/aoe/common/gui/RenderSystem.h>

namespace vob::aoe::common::gui
{
	RenderSystem::RenderSystem(ecs::WorldDataProvider& a_wdp)
		: m_canvasList{ a_wdp.getEntityList(*this, CanvasComponents{}) }
		, m_objectEntityList{ a_wdp.getEntityList(*this, ObjectComponents{}) }
	{}

	void RenderSystem::update() const
	{
		for (auto const& t_canvas : m_canvasList)
		{
			render(t_canvas);
		}
	}

	void RenderSystem::render(
		CanvasComponents::EntityType const& a_canvas
	) const
	{
		auto const& t_hierarchy = a_canvas.getComponent<HierarchyComponent>();
		auto const& t_canvas = a_canvas.getComponent<CanvasComponent>();
		for (auto const& t_child : t_hierarchy.m_children)
		{
			auto const t_object = m_objectEntityList.find(t_child);
			if (t_object != nullptr)
			{
				render(*t_object, t_canvas.m_size);
			}
		}
	}

	void RenderSystem::render(
		ObjectEntity const& a_objectEntity
		, Vector2 const& a_canvasSize
	) const
	{
		auto& t_objectComponent = a_objectEntity.getComponent<ObjectComponent>();
		if (t_objectComponent.m_object != nullptr)
		{
			t_objectComponent.m_object->render(
				a_objectEntity
				, m_objectEntityList
				, {}
				, a_canvasSize
			);
		}
	}
}
