#pragma once
#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>

#include <vob/aoe/common/_render/Cameracomponent.h>
#include <vob/aoe/common/_render/Directorcomponent.h>
#include <vob/aoe/common/_render/OpenGl.h>
#include <vob/aoe/common/_render/Utils.h>
#include <vob/aoe/common/_render/model/ModelRendercomponent.h>
#include <vob/aoe/common/_render/model/Modelcomponent.h>
#include <vob/aoe/common/window/WorldWindowcomponent.h>

// DEBUG
#include <vob/aoe/common/input/WorldInputcomponent.h>

#include <glm/glm.hpp>

namespace vob::aoe::common
{
	class ModelRenderPass
	{
	public:
		// Constructor
		explicit ModelRenderPass(aoecs::world_data_provider& a_wdp)
			: m_worldInputComponent{ a_wdp.get_world_component<WorldInputComponent const>() }
			, m_modelRenderComponent{ a_wdp.get_world_component<ModelRenderComponent>() }
			, m_worldWindowComponent{ a_wdp.get_world_component<WorldWindowComponent>() }
			, m_directorComponent{ a_wdp.get_world_component<DirectorComponent>() }
			, m_cameramanEntityList{ a_wdp }
			, m_modelEntityList{ a_wdp }
		{
		}

		void run() const
		{
			if (m_modelRenderComponent.m_shaderProgram == nullptr)
			{
				return;
			}
			auto const& shaderProgramHandle = *m_modelRenderComponent.m_shaderProgram;

			m_modelRenderComponent.m_modelShaderProgramResourceManager.update();
			auto const& shaderProgram = *shaderProgramHandle;

			if (!initSceneShaderProgram(
				shaderProgram
				, m_worldWindowComponent
				, m_directorComponent
				, m_cameramanEntityList
			))
			{
				return;
			}

			const glm::vec3 lightPosition{ 32 + 32 * std::sin(t), 32, 32 + 32 * std::cos(t) };
			shaderProgram.setUniform(shaderProgram.getLightPositionUniformLocation(), lightPosition);
			t += 0.01f;

			auto const ambientColor = m_modelRenderComponent.m_ambientColor;
			shaderProgram.setUniform(shaderProgram.getAmbientColorUniformLocation(), ambientColor);

			m_modelRenderComponent.m_staticModelResourceManager.update();
			m_modelRenderComponent.m_renderTextureResourceManager.update();
			m_modelRenderComponent.m_textureResourceManager.update();

			// DEBUG
			Switch r = m_worldInputComponent.m_keyboard.m_keys[Keyboard::Key::R];
			if (r.m_changed && r.m_isActive)
			{
				m_debugMode = static_cast<DebugMode>((static_cast<int>(m_debugMode) + 1)
					% static_cast<int>(DebugMode::Count));
			}
			shaderProgram.setUniform(
				shaderProgram.getUniformLocation("u_debugMode"), static_cast<int>(m_debugMode));

			for (auto const& modelEntity : m_modelEntityList)
			{
				auto const& modelTransformComponent = modelEntity.get<TransformComponent>();
				auto const& modelModelComponent = modelEntity.get<ModelComponent>();

				if (modelModelComponent.m_model == nullptr || !(*modelModelComponent.m_model)->isReady())
				{
					continue;
				}
				auto& model = **modelModelComponent.m_model;

				auto const modelMatrix = modelTransformComponent.m_matrix;
				shaderProgram.setUniform(shaderProgram.getModelUniformLocation(), modelMatrix);

				auto const modelNormalMatrix = glm::transpose((glm::inverse(modelMatrix)));
				shaderProgram.setUniform(shaderProgram.getModelNormalUniformLocation(), modelNormalMatrix);

				for (auto const& mesh : model.m_meshes)
				{
					auto& material = model.m_materials[mesh.getMaterialIndex()];

					glActiveTexture(GL_TEXTURE0 + 0);
					if (material.m_albedo != nullptr && (*material.m_albedo)->isReady())
					{
						(*material.m_albedo)->bind(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, 0);
					}

					glActiveTexture(GL_TEXTURE1);
					if (material.m_normal != nullptr && (*material.m_normal)->isReady())
					{
						(*material.m_normal)->bind(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, 0);
					}

					glActiveTexture(GL_TEXTURE2);
					if (material.m_metallicRoughness != nullptr
						&& (*material.m_metallicRoughness)->isReady())
					{
						(*material.m_metallicRoughness)->bind(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, 0);
					}

					mesh.render();
				}
			}
		}

	//private:
		// Attributes
		WorldInputComponent const& m_worldInputComponent;
		ModelRenderComponent& m_modelRenderComponent;
		WorldWindowComponent& m_worldWindowComponent;
		DirectorComponent& m_directorComponent;
		aoecs::entity_map_observer_list_ref<TransformComponent const, CameraComponent const> m_cameramanEntityList;
		aoecs::entity_map_observer_list_ref<TransformComponent const, ModelComponent const> m_modelEntityList;

		// DEBUG
		mutable float t = 0;
		enum class DebugMode
		{
			Default = 0,
			Normals,
			TangentNormals,
			
			Count
		};
		mutable DebugMode m_debugMode = DebugMode::Default;
	};

}
