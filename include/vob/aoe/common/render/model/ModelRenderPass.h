#pragma once
#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/CameraComponent.h>
#include <vob/aoe/common/render/DirectorComponent.h>
#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/common/render/Utils.h>
#include <vob/aoe/common/render/model/ModelRenderComponent.h>
#include <vob/aoe/common/render/model/ModelComponent.h>
#include <vob/aoe/common/window/WindowComponent.h>

namespace vob::aoe::common
{
	class ModelRenderPass
	{
		using CameramanComponents = ecs::ComponentTypeList<
			TransformComponent const
			, CameraComponent const
		>;
		using ModelComponents = ecs::ComponentTypeList<
			TransformComponent const
			, ModelComponent const
		>;
	public:
		// Constructor
		explicit ModelRenderPass(ecs::WorldDataProvider& a_wdp)
			: m_modelRenderComponent{ a_wdp.getWorldComponentRef<ModelRenderComponent>() }
			, m_windowComponent{ a_wdp.getWorldComponentRef<WindowComponent>() }
			, m_directorComponent{ a_wdp.getWorldComponentRef<DirectorComponent>() }
			, m_cameramanEntityList{ a_wdp.getEntityViewList(*this, CameramanComponents{}) }
			, m_modelEntityList{ a_wdp.getEntityViewList(*this, ModelComponents{}) }
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
				, m_windowComponent
				, m_directorComponent
				, m_cameramanEntityList
			))
			{
				return;
			}

			const glm::vec3 lightPosition{ 10 * std::sin(t), 0, 0 };
			shaderProgram.setUniform(shaderProgram.getLightPositionUniformLocation(), lightPosition);
			t += 0.002f;

			auto const ambientColor = m_modelRenderComponent.m_ambientColor;
			shaderProgram.setUniform(shaderProgram.getAmbientColorUniformLocation(), ambientColor);

			m_modelRenderComponent.m_staticModelResourceManager.update();
			m_modelRenderComponent.m_renderTextureResourceManager.update();
			m_modelRenderComponent.m_textureResourceManager.update();

			for (auto const& modelEntity : m_modelEntityList)
			{
				auto const& modelTransformComponent = modelEntity.getComponent<TransformComponent>();
				auto const& modelModelComponent = modelEntity.getComponent<ModelComponent>();

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
					if (material.m_diffuse != nullptr && (*material.m_diffuse)->isReady())
					{
						(*material.m_diffuse)->bind(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, 0);
					}

					glActiveTexture(GL_TEXTURE0 + 1);
					if (material.m_specular != nullptr && (*material.m_specular)->isReady())
					{
						(*material.m_specular)->bind(GL_TEXTURE_2D);
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
		WindowComponent& m_windowComponent;
		DirectorComponent& m_directorComponent;
		ecs::EntityViewList<TransformComponent const, CameraComponent const> const& m_cameramanEntityList;
		ecs::EntityViewList<TransformComponent const, ModelComponent const> const& m_modelEntityList;

		mutable float t = 0;
	};

}
