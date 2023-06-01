#include <vob/aoe/rendering/systems/render_models_system.h>

#include <vob/aoe/rendering/uniform_util.h>

#include <vob/aoe/spacetime/measures.h>
#ifndef NDEBUG
#include <vob/aoe/input/physical_switch_mapping.h>
#endif

#include <vob/misc/physics/measure_literals.h>
#include <vob/misc/std/message_macros.h>

#include <numbers>

using namespace vob::misph::literals;


namespace vob::aoegl
{
	namespace
	{
#pragma message(VOB_MISTD_TODO "code duplicate")
		template <typename TDirectorWorldComponent, typename TCameraEntities>
		auto get_camera_settings(
			TDirectorWorldComponent const& a_directorWorldComponent,
			TCameraEntities const& a_cameraEntities)
		{
			auto const cameraEntity = a_cameraEntities.find(a_directorWorldComponent.m_activeCamera);
			if (cameraEntity == a_cameraEntities.end())
			{
				return std::make_tuple(glm::mat4{ 1.0f }, std::numbers::pi_v<float> / 2, 0.1f, 1000.0f);
			}

			auto const& transformComponent = cameraEntity->get<aoest::transform_component>();
			auto const& cameraComponent = cameraEntity->get<camera_component>();
			return std::make_tuple(
				transformComponent.m_matrix
				, glm::radians(cameraComponent.m_fovDegree)
				, cameraComponent.m_nearClip
				, cameraComponent.m_farClip);
		}
	}

	render_models_system::render_models_system(aoecs::world_data_provider& a_wdp)
		: m_modelEntities{ a_wdp }
		, m_cameraEntities{ a_wdp }
		, m_windowWorldComponent{ a_wdp }
		, m_directorWorldComponent{ a_wdp }
		, m_meshRenderWorldComponent{ a_wdp }
#ifndef NDEBUG
		, m_mappedInputsWorldComponent{ a_wdp }
#endif
	{
#ifndef NDEBUG
		m_polygonMapping = m_mappedInputsWorldComponent->m_switches.size();
		m_mappedInputsWorldComponent->m_switches.emplace_back(
			mistd::polymorphic_ptr_util::make<aoein::physical_switch_mapping>(
				aoein::physical_switch_reference{ aoein::keyboard::key::W }));
#endif

		glCreateVertexArrays(1, &(m_meshRenderWorldComponent->m_vao));
		glBindVertexArray(m_meshRenderWorldComponent->m_vao);

#define _SETUP_ATTRIB(index, count) \
		glEnableVertexAttribArray(index); \
		glVertexAttribFormat(index, count, GL_FLOAT, GL_FALSE, 0); \
		glVertexAttribBinding(index, index);
		
		_SETUP_ATTRIB(0, 3); // position
		_SETUP_ATTRIB(1, 2); // texture coord
		_SETUP_ATTRIB(2, 3); // normal
		_SETUP_ATTRIB(3, 3); // tangent

#ifdef _TODO_ANIMATION_
		glEnableVertexAttribArray(4); // joint indices
		glVertexAttribIFormat(4, 4, GL_UNSIGNED_INT, 0);
		glVertexAttribBinding(4, 4);
		_SETUP_ATTRIB(5, 4); // joint weights
#endif
#undef _SETUP_ATTRIB
	}

	render_models_system::~render_models_system()
	{
		glDeleteVertexArrays(1, &(m_meshRenderWorldComponent->m_vao));
	}

	void render_models_system::update() const
	{
		auto const& program = m_meshRenderWorldComponent->m_meshProgram;

		// Use program
		glUseProgram(program.m_id);

		// Set scene uniforms
		auto const windowSize = m_windowWorldComponent->m_window.get().get_size();
		const auto [transform, fov, nearClip, farClip] = get_camera_settings(
			*m_directorWorldComponent, *m_cameraEntities);
		uniform_util::set(program.m_viewPositionLocation, aoest::get_translation(transform));
		uniform_util::set(
			program.m_viewProjectionTransformLocation,
			glm::perspective(fov, static_cast<float>(windowSize.x) / windowSize.y, nearClip, farClip)
			* glm::inverse(transform));

		glBindVertexArray(m_meshRenderWorldComponent->m_vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
#ifdef _TODO_ANIMATION_
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
#endif

		// DEBUG ROTATION
		static float r = 0.0f;
		static float rSpeed = 0.0025f;
		r += rSpeed;
		if (r > std::numbers::pi_v<float>)
		{
			r -= std::numbers::pi_v<float>;
		}
		glm::mat4 debugR = glm::rotate(glm::mat4{ 1.0f }, r, glm::vec3{ 0.0f, 1.0f, 0.0f });

#ifndef NDEBUG
		if (m_mappedInputsWorldComponent->m_switches[m_polygonMapping]->changed()
			&& m_mappedInputsWorldComponent->m_switches[m_polygonMapping]->is_pressed())
		{
			m_polygonType = GL_LINE + ((m_polygonType - GL_LINE + 1) % (GL_FILL + 1 - GL_LINE));
		}
		glPolygonMode(GL_FRONT_AND_BACK, m_polygonType);
#endif

		// Render models
		for (auto const& modelEntity : m_modelEntities)
		{
			auto const& modelComponent = modelEntity.get<model_component>();
			auto const& transformComponent = modelEntity.get<aoest::transform_component>();			
			
			// Set model uniforms
			uniform_util::set(program.m_meshTransformLocation, transformComponent.m_matrix * debugR);
			uniform_util::set(
				program.m_meshNormalTransformLocation,
				glm::mat3(glm::transpose(glm::inverse(transformComponent.m_matrix * debugR))));

			// Render model meshes
			for (auto const& texturedMesh : modelComponent.m_model.m_texturedMeshes)
			{
#ifdef _TODO_ANIMATION_
				std::pmr::vector<glm::mat4> pose{ 3, glm::mat4{ 1.0f } };
				static float t = 0.0f;
				t += 0.01f;
				pose.back() = glm::rotate(pose.back(), t, glm::vec3{ 0.0f, 1.0f, 0.0f });
				uniform_util::set(program.m_rigPoseLocation, std::span{ pose });
#endif

				// Bind material textures
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texturedMesh.m_material.m_albedo);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texturedMesh.m_material.m_normal);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, texturedMesh.m_material.m_metallicRoughness);

				// Bind vertex buffers
				glBindVertexBuffer(0, texturedMesh.m_mesh.m_positionVbo, 0, sizeof(glm::vec3));
				glBindVertexBuffer(1, texturedMesh.m_mesh.m_textureCoordVbo, 0, sizeof(glm::vec2));
				glBindVertexBuffer(2, texturedMesh.m_mesh.m_normalVbo, 0, sizeof(glm::vec3));
				glBindVertexBuffer(3, texturedMesh.m_mesh.m_tangentVbo, 0, sizeof(glm::vec3));
#ifdef _TODO_ANIMATION_
				glBindVertexBuffer(4, texturedMesh.m_mesh.m_jointIndicesVbo, 0, sizeof(glm::uvec4));
				glBindVertexBuffer(5, texturedMesh.m_mesh.m_jointWeightsVbo, 0, sizeof(glm::vec4));
#endif
				// Bind element buffer
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texturedMesh.m_mesh.m_ebo);

				// Render mesh
				glDrawElements(GL_TRIANGLES, texturedMesh.m_mesh.m_triangleCount * 3, GL_UNSIGNED_INT, nullptr);
			}
		}
		
		glBindVertexArray(0);

#ifndef NDEBUG
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
	}
}
