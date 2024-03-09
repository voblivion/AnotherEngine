#include <vob/aoe/rendering/systems/model_data_resource_system.h>


namespace vob::aoegl
{
	model_data_resource_system::model_data_resource_system(aoeng::world_data_provider& a_wdp)
		: m_modelDataResourceManager{ a_wdp }
		, m_meshRenderWorldComponent{ a_wdp }
	{
		a_wdp.on_construct<&model_data_resource_system::on_construct, model_data_component>(*this);
		a_wdp.on_destroy<&model_data_resource_system::on_destroy, model_data_component, model_component>(*this);
	}

	void model_data_resource_system::on_construct(aoeng::registry& a_registry, aoeng::entity a_entity)
	{
		auto const& modelDataComponent = a_registry.get<model_data_component const>(a_entity);
		auto& modelComponent = a_registry.emplace_or_replace<model_component>(a_entity);
		
		glBindVertexArray(m_meshRenderWorldComponent->m_vao);

		modelComponent.m_model = m_modelDataResourceManager->add_reference(
			modelDataComponent.m_modelData);

		glBindVertexArray(0);
	}

	void model_data_resource_system::on_destroy(aoeng::registry& a_registry, aoeng::entity a_entity)
	{
		auto [modelDataComponent, modelComponent] =
			a_registry.try_get<model_data_component const, model_component>(a_entity);
		if (modelComponent == nullptr || modelDataComponent == nullptr)
		{
			return;
		}
		glBindVertexArray(m_meshRenderWorldComponent->m_vao);

		m_modelDataResourceManager->remove_reference(modelDataComponent->m_modelData);
		modelComponent->m_model = model{};

		glBindVertexArray(0);
	}

	void model_data_resource_system::update() const
	{

	}
}
