#include <vob/aoe/rendering/systems/model_data_resource_system.h>


namespace vob::aoegl
{
	model_data_resource_system::model_data_resource_system(aoeng::world_data_provider& a_wdp)
		: m_modelDataResourceManager{ a_wdp }
		, m_meshRenderWorldComponent{ a_wdp }
	{
		a_wdp.on_construct<model_component, &model_data_resource_system::on_construct>(*this);
		a_wdp.on_destroy<model_component, &model_data_resource_system::on_destroy>(*this);
	}

	void model_data_resource_system::on_construct(aoeng::entity_registry& a_registry, aoeng::entity a_entity)
	{
		auto [modelDataComponent, modelComponent] =
			a_registry.try_get<model_data_component const, model_component>(a_entity);
		if (modelDataComponent == nullptr || modelComponent == nullptr)
		{
			return;
		}

		glBindVertexArray(m_meshRenderWorldComponent->m_vao);

		modelComponent->m_model = m_modelDataResourceManager->add_reference(
			modelDataComponent->m_modelData);

		glBindVertexArray(0);
	}

	void model_data_resource_system::on_destroy(aoeng::entity_registry& a_registry, aoeng::entity a_entity)
	{
		auto [modelDataComponent, modelComponent] =
			a_registry.try_get<model_data_component const, model_component>(a_entity);
		if (modelDataComponent == nullptr || modelComponent == nullptr)
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
