#pragma once
#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/Cameracomponent.h>
#include <vob/aoe/common/render/Directorcomponent.h>
#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/common/render/Utils.h>
#include <vob/aoe/common/render/model/ModelRendercomponent.h>
#include <vob/aoe/common/render/model/Modelcomponent.h>
#include <vob/aoe/common/window/WorldWindowcomponent.h>

// DEBUG
#include <vob/aoe/common/input/WorldInputcomponent.h>

namespace vob::aoe::common
{
	class ModelRenderPass
	{
		using CameramanComponents = aoecs::ComponentTypeList<
			TransformComponent const
			, CameraComponent const
		>;
		using ModelComponents = aoecs::ComponentTypeList<
			TransformComponent const
			, ModelComponent const
		>;
	public:
		// Constructor
		explicit ModelRenderPass(aoecs::WorldDataProvider& a_wdp)
			: m_modelRenderComponent{ a_wdp.getWorldComponentRef<ModelRenderComponent>() }
			, m_worldWindowComponent{ a_wdp.getWorldComponentRef<WorldWindowComponent>() }
			, m_directorComponent{ a_wdp.getWorldComponentRef<DirectorComponent>() }
			, m_cameramanEntityList{ a_wdp.getentity_view_list(*this, CameramanComponents{}) }
			, m_modelEntityList{ a_wdp.getentity_view_list(*this, ModelComponents{}) }
			, m_worldInputComponent{ a_wdp.getWorldComponentRef<WorldInputComponent>() }
		{}

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
				auto const& modelTransformComponent = modelEntity.get_component<TransformComponent>();
				auto const& modelModelComponent = modelEntity.get_component<ModelComponent>();

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
		ModelRenderComponent& m_modelRenderComponent;
		WorldWindowComponent& m_worldWindowComponent;
		DirectorComponent& m_directorComponent;
		aoecs::entity_view_list<TransformComponent const, CameraComponent const> const& m_cameramanEntityList;
		aoecs::entity_view_list<TransformComponent const, ModelComponent const> const& m_modelEntityList;

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
		WorldInputComponent& m_worldInputComponent;
	};

}
