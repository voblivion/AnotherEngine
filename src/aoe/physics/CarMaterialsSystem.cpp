#include <vob/aoe/physics/CarMaterialsSystem.h>


namespace vob::aoeph
{
	void CarMaterialsSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void CarMaterialsSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		/*struct alignas(16) WheelRenderSceneConfig
		{
			glm::mat4 wheel;
			glm::vec3 albedo;
			float metallic;
			float roughness;
			float steering;
			float distance;
		};*/
		auto const elapsedTime = std::chrono::duration<float>(m_timeContext.get(a_wdap).elapsedTime).count();
		for (auto const [entity, rotation, carCollider, carController, carMaterials] : m_carEntities.get(a_wdap).each())
		{
			auto const forward = rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };

			for (auto& part : carMaterials.parts)
			{
				auto const& wheelState = carCollider.wheels[part.wheelIndex];
				auto const& wheelController = carController.wheels[part.wheelIndex];

				auto const position = wheelState.suspensionAttachPosition
					+ wheelState.rotation * glm::vec3{ 0.0f, -wheelState.suspensionLength, 0.0f };

				part.distance += glm::dot(carCollider.linearVelocity, forward) * elapsedTime;
				while (part.distance < 0)
				{
					part.distance += 2.0f * std::numbers::pi_v<float> *wheelState.radiuses.z;
				}
				while (part.distance > 2.0f * std::numbers::pi_v<float> *wheelState.radiuses.z)
				{
					part.distance -= 2.0f * std::numbers::pi_v<float> *wheelState.radiuses.z;
				}

				auto wheelRenderSceneConfig = WheelRenderSceneConfig{
					aoest::combine(position, wheelState.rotation),
					part.albedo,
					part.metallic,
					part.roughness,
					wheelController.steeringAngle,
					part.distance / wheelState.radiuses.z
				};

				glNamedBufferSubData(part.materialUbo, 0, sizeof(wheelRenderSceneConfig), &wheelRenderSceneConfig);
			}
		}
	}
}
