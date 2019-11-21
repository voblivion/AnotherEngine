#pragma once
#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include "DebugRenderComponent.h"
#include <gl/GL.h>
#include <vob/aoe/common/window/WindowComponent.h>
#include "CameraComponent.h"
#include <vob/aoe/common/space/TransformComponent.h>
#include "DirectorComponent.h"
#include <vob/aoe/common/render/GraphicComponent.h>
#include "GameRenderComponent.h"
#include "UIComponent.h"

namespace vob::aoe::common
{
	class RenderSystem
	{
	public:
		using CameraComponents = ecs::ComponentTypeList<
			TransformComponent const
			, CameraComponent const
		>;
		using ModelComponents = ecs::ComponentTypeList<
			TransformComponent const
			, GraphicComponent const
		>;

		explicit RenderSystem(ecs::WorldDataProvider& a_wdp)
			: m_worldWindow{ *a_wdp.getWorldComponent<WindowComponent>() }
			, m_gameRenderComponent{ *a_wdp.getWorldComponent<GameRenderComponent>() }
			, m_debugRenderComponent{ *a_wdp.getWorldComponent<DebugRenderComponent>() }
			, m_directorComponent{ *a_wdp.getWorldComponent<DirectorComponent>() }
			, m_cameramanList{ a_wdp.getEntityList(*this, CameraComponents{}) }
			, m_modelList{ a_wdp.getEntityList(*this, ModelComponents{}) }
		{}

		void update() const
		{
			m_worldWindow.getWindow().setActive(true);
			m_gameRenderComponent.m_renderTextureManager.update();
			m_gameRenderComponent.m_shaderProgramResourceManager.update();
			m_gameRenderComponent.m_simpleStaticMeshManager.update();
			m_gameRenderComponent.m_staticModelResourceManager.update();
			m_gameRenderComponent.m_textureManager.update();

			auto const camera = m_cameramanList.find(
				m_directorComponent.m_currentCamera
			);

			// 3D scene rendering
			if (camera != nullptr)
			{
				// Reset scene canvas
				m_gameRenderComponent.m_gameRenderTexture.resource().startRenderTo();
				glEnable(GL_DEPTH_TEST);
				glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				// Compute render matrices
				glm::vec3 viewPos{ 1.0f };
				glm::mat4 viewMatrix{ 1.0f };
				glm::mat4 projectionMatrix{ 1.0f };
				{
					// Get cameraman components
					auto const& cameramanTransform = camera->getComponent<TransformComponent>();
					auto const& cameramanCamera = camera->getComponent<CameraComponent>();
					auto const windowSize = m_worldWindow.getWindow().getSize();

					// Setup view matrix and position
					viewMatrix = glm::translate(viewMatrix, cameramanTransform.m_position);
					viewMatrix *= glm::mat4_cast(cameramanTransform.m_rotation);
					viewMatrix = glm::inverse(viewMatrix);
					viewPos = cameramanTransform.m_position;

					// Setup projection matrix
					projectionMatrix = glm::perspective(
						glm::radians(cameramanCamera.fov)
						, float(windowSize.x) / windowSize.y
						, cameramanCamera.nearClip, cameramanCamera.farClip);
				}

				// Models rendering
				if (m_gameRenderComponent.m_modelShader->resource().isReady())
				{
					m_gameRenderComponent.m_modelShader->resource().use();

					// TODO Lights
					m_gameRenderComponent.m_modelShader->resource().setUniform("u_ambientColor"
						, glm::vec3{ 0.02f });
					m_gameRenderComponent.m_modelShader->resource().setUniform("u_lightPosition"
						, glm::vec3{ 10.0f * std::sin(t), 20.0f, 10.0f * std::cos(t) });
					t += 0.0002f;
					t = std::fmod(t, 2.0f*3.141592f);
					m_gameRenderComponent.m_modelShader->resource().setUniform("u_view", viewMatrix);
					m_gameRenderComponent.m_modelShader->resource().setUniform("u_viewPosition", viewPos);
					m_gameRenderComponent.m_modelShader->resource().setUniform("u_projection", projectionMatrix);
					for (auto t_model : m_modelList)
					{
						// Get model components
						auto const& modelTransform = t_model.getComponent<TransformComponent>();
						auto const& modelGraphic = t_model.getComponent<GraphicComponent>();

						if (modelGraphic.m_model.isValid() && modelGraphic.m_model->resource().isReady())
						{
							// Setup model matrices
							glm::mat4 modelMatrix{ 1.0f };
							modelMatrix = glm::translate(modelMatrix, modelTransform.m_position);
							modelMatrix *= glm::mat4_cast(modelTransform.m_rotation);
							m_gameRenderComponent.m_modelShader->resource().setUniform("u_model", modelMatrix);
							glm::mat3 const modelNormalMatrix{ glm::transpose(glm::inverse(modelMatrix)) };
							m_gameRenderComponent.m_modelShader->resource().setUniform("u_modelNormal", modelNormalMatrix);

							// Render meshes
							auto k = 0u;
							for (auto const& t_mesh : modelGraphic.m_model->resource().m_meshes)
							{
								if (k < modelGraphic.m_materials.size()
									&& modelGraphic.m_materials[k].isValid())
								{
									auto const& t_material = *(modelGraphic.m_materials[k]);

									// Diffuse
									m_gameRenderComponent.m_modelShader->resource().setUniform("u_material.m_diffuse", 0);
									glActiveTexture(GL_TEXTURE0);
									if (t_material.m_diffuse.isValid() && t_material.m_diffuse->resource().isReady())
									{
										glBindTexture(GL_TEXTURE_2D, t_material.m_diffuse->resource().m_texture.value().getNativeHandle());
									}
									else
									{
										glBindTexture(GL_TEXTURE_2D, 0);
									}

									// Specular
									m_gameRenderComponent.m_modelShader->resource().setUniform("u_material.m_specular", 1);
									glActiveTexture(GL_TEXTURE1);
									if (t_material.m_diffuse.isValid() && t_material.m_diffuse->resource().isReady())
									{
										glBindTexture(GL_TEXTURE_2D, t_material.m_diffuse->resource().m_texture.value().getNativeHandle());
									}
									else
									{
										glBindTexture(GL_TEXTURE_2D, 0);
									}

									glBindVertexArray(t_mesh.m_vao.m_id);
									glDrawElements(GL_TRIANGLES
										, static_cast<std::uint32_t>(t_mesh.m_faces.size())
										, GL_UNSIGNED_INT, nullptr);
									++k;
									glBindVertexArray(0);
								}
							}
						}
					}
				}

				// Debug rendering
				if (m_debugRenderComponent.m_shader->resource().isReady())
				{
					m_debugRenderComponent.m_shader->resource().use();

					m_debugRenderComponent.m_shader->resource().setUniform("u_view", viewMatrix);
					m_debugRenderComponent.m_shader->resource().setUniform("u_projection", projectionMatrix);

					auto& t_mesh = m_debugRenderComponent.m_debugMesh;
					t_mesh.update();

					glBindVertexArray(t_mesh.m_vertexArrayObject);
					glDrawElements(GL_LINES
						, static_cast<std::uint32_t>(t_mesh.m_lines.size())
						, GL_UNSIGNED_INT, nullptr);
					t_mesh.reset();
				}

				m_gameRenderComponent.m_gameRenderTexture.resource().stopRenderTo();
			}

			// VOB
			if (m_gameRenderComponent.m_postProcessShader->resource().isReady())
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				m_gameRenderComponent.m_postProcessShader->resource().use();
				// glEnable(GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				m_gameRenderComponent.m_renderShape.resource().update();
				m_gameRenderComponent.m_renderShape.resource().draw();
			}
			m_worldWindow.getWindow().display();
		}

	private:
		WindowComponent& m_worldWindow;
		GameRenderComponent& m_gameRenderComponent;
		DebugRenderComponent& m_debugRenderComponent;

		DirectorComponent& m_directorComponent;
		ecs::EntityList<TransformComponent const, CameraComponent const> const& m_cameramanList;

		ecs::EntityList<TransformComponent const, GraphicComponent const> const& m_modelList;

		// TODO
		mutable float t{ 2.0f };
		mutable int tt{ 0 };
	};
}
