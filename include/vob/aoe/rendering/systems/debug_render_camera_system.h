#pragma once

#include <vob/aoe/rendering/camera_util.h>
#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>
#include <vob/aoe/rendering/camera_util.h>

#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/spacetime/transform.h>

#include <glm/glm.hpp>


namespace vob::aoegl
{
	class debug_render_camera_system
	{
	public:
		explicit debug_render_camera_system(aoeng::world_data_provider& a_wdp)
			: m_windowWorldComponent{ a_wdp }
			, m_debugMeshWorldComponent { a_wdp }
			, m_cameraEntities{ a_wdp }
		{}

		void update() const
		{
			auto cleanup = [](glm::vec4 const& v) { return glm::vec3{ v / v.w }; };
			auto const windowSize = m_windowWorldComponent->m_window.get().get_size();
			auto const aspectRatio = static_cast<float>(windowSize.x) / windowSize.y;

			auto cameraEntities = m_cameraEntities.get();
			for (auto const cameraEntity : cameraEntities)
			{
				auto const [position, rotation, camera] = cameraEntities.get(cameraEntity);
				auto const transform = aoest::combine(position, rotation);
				auto const viewTransform = glm::perspective(camera.m_fov, aspectRatio, camera.m_nearClip, camera.m_farClip);
				auto const invTransform = glm::inverse(transform);
				auto const viewProjectionTransform = viewTransform * invTransform;
				auto rev = glm::inverse(viewProjectionTransform);

				auto lbf = debug_vertex{ cleanup( rev * glm::vec4{ -1.0f, -1.0f, -1.0f, 1.0f } ), k_red };
				auto lbb = debug_vertex{ cleanup( rev * glm::vec4{ -1.0f, -1.0f, 1.0f, 1.0f } ), k_red };
				auto ltf = debug_vertex{ cleanup( rev * glm::vec4{ -1.0f, 1.0f, -1.0f, 1.0f } ), k_red };
				auto ltb = debug_vertex{ cleanup( rev * glm::vec4{ -1.0f, 1.0f, 1.0f, 1.0f } ), k_red };
				auto rbf = debug_vertex{ cleanup( rev * glm::vec4{ 1.0f, -1.0f, -1.0f, 1.0f } ), k_red };
				auto rbb = debug_vertex{ cleanup( rev * glm::vec4{ 1.0f, -1.0f, 1.0f, 1.0f } ), k_red };
				auto rtf = debug_vertex{ cleanup( rev * glm::vec4{ 1.0f, 1.0f, -1.0f, 1.0f } ), k_red };
				auto rtb = debug_vertex{ cleanup( rev * glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f } ), k_red };



				m_debugMeshWorldComponent->add_line(lbf, lbb);
				m_debugMeshWorldComponent->add_line(ltf, ltb);
				m_debugMeshWorldComponent->add_line(rbf, rbb);
				m_debugMeshWorldComponent->add_line(rtf, rtb);

				m_debugMeshWorldComponent->add_line(lbf, rbf);
				m_debugMeshWorldComponent->add_line(rbf, rtf);
				m_debugMeshWorldComponent->add_line(rtf, ltf);
				m_debugMeshWorldComponent->add_line(ltf, lbf);

				m_debugMeshWorldComponent->add_line(lbb, rbb);
				m_debugMeshWorldComponent->add_line(rbb, rtb);
				m_debugMeshWorldComponent->add_line(rtb, ltb);
				m_debugMeshWorldComponent->add_line(ltb, lbb);
			}
		}

	private:
		aoeng::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
		aoeng::world_component_ref<debug_mesh_world_component> m_debugMeshWorldComponent;

		aoeng::registry_view_ref<
			aoest::position const,
			aoest::rotation const,
			camera_component const
		> m_cameraEntities;
	};
}
