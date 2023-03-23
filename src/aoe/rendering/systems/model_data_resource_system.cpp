#include <vob/aoe/rendering/systems/model_data_resource_system.h>


namespace vob::aoegl
{
	model_data_resource_system::model_data_resource_system(aoecs::world_data_provider& a_wdp)
		: m_modelDataResourceManager{ a_wdp }
		, m_meshRenderWorldComponent{ a_wdp }
	{
		a_wdp.observe_spawns(*this);
		a_wdp.observe_despawns(*this);
	}

	void model_data_resource_system::on_spawn(aoecs::entity_list::entity_view a_entity) const
	{
		auto const modelDataComponent = a_entity.get_component<model_data_component const>();
		if (modelDataComponent == nullptr)
		{
			return;
		}

		auto const modelComponent = a_entity.get_component<model_component>();
		if (modelComponent == nullptr)
		{
			return;
		}

		glBindVertexArray(m_meshRenderWorldComponent->m_vao);

		modelComponent->m_model = m_modelDataResourceManager->add_reference(
			modelDataComponent->m_modelData);

		glBindVertexArray(0);
	}

	void model_data_resource_system::on_despawn(aoecs::entity_list::entity_view a_entity) const
	{
		auto const modelDataComponent = a_entity.get_component<model_data_component const>();
		if (modelDataComponent == nullptr)
		{
			return;
		}

		auto const modelComponent = a_entity.get_component<model_component>();
		if (modelComponent == nullptr)
		{
			return;
		}


		glBindVertexArray(m_meshRenderWorldComponent->m_vao);

		m_modelDataResourceManager->remove_reference(
			modelDataComponent->m_modelData);

		glBindVertexArray(0);
	}

	void model_data_resource_system::update() const
	{

	}
}
