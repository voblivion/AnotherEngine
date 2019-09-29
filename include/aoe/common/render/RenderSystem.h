#pragma once
#include <aoe/core/ecs/WorldDataProvider.h>
#include "RenderDebugComponent.h"
#include <gl/GL.h>
#include <aoe/common/window/WindowComponent.h>
#include "CameraComponent.h"
#include <aoe/common/space/TransformComponent.h>
#include "DirectorComponent.h"
#include <aoe/common/render/GraphicComponent.h>
#include "SceneRenderComponent.h"
#include <aoe/common/imgui/imgui_sfml.h>
#include "aoe/common/imgui/imgui.h"

namespace aoe
{
	namespace common
	{
		class RenderSystem
		{
		public:
			using CameraComponents = ecs::ComponentTypeList<
				TransformComponent const, CameraComponent const>;
			using ModelComponents = ecs::ComponentTypeList<
				TransformComponent const, GraphicComponent const>;

			explicit RenderSystem(ecs::WorldDataProvider& a_wdp)
				: m_worldWindow{ *a_wdp.getWorldComponent<WindowComponent>() }
				, m_sceneRenderComponent{ *a_wdp.getWorldComponent<SceneRenderComponent>() }
				, m_worldRenderDebug{ *a_wdp.getWorldComponent<RenderDebugComponent>() }
				, m_directorComponent{ *a_wdp.getWorldComponent<DirectorComponent>() }
				, m_cameramanList{ a_wdp.getEntityList(*this, CameraComponents{}) }
				, m_modelList{ a_wdp.getEntityList(*this, ModelComponents{}) }
			{}

			void update() const
			{
				auto const camera = m_cameramanList.find(
					m_directorComponent.m_currentCamera
				);

				// Retrieve current camera
				if (camera != nullptr)
				{
					// Reset canvas
					m_sceneRenderComponent.m_renderTexture.startRenderTo();
					glEnable(GL_DEPTH_TEST);
					glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
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
					{
						m_worldRenderDebug.m_modelShader->m_glProgram.startUsing();

						// TODO Lights
						m_worldRenderDebug.m_modelShader->setUniform("u_ambientColor"
							, glm::vec3{ 0.02f });
						m_worldRenderDebug.m_modelShader->setUniform("u_lightPosition"
							, glm::vec3{ 10.0f * std::sin(t), 20.0f, 10.0f * std::cos(t) });
						t += 0.0002f;
						t = std::fmod(t, 2.0f*3.141592f);
						m_worldRenderDebug.m_modelShader->setUniform("u_view", viewMatrix);
						m_worldRenderDebug.m_modelShader->setUniform("u_viewPosition", viewPos);
						m_worldRenderDebug.m_modelShader->setUniform("u_projection", projectionMatrix);
						for (auto t_model : m_modelList)
						{
							// Get model components
							auto const& modelTransform = t_model.getComponent<TransformComponent>();
							auto const& modelGraphic = t_model.getComponent<GraphicComponent>();

							if (modelGraphic.m_model.isValid())
							{
								// Setup model matrices
								glm::mat4 modelMatrix{ 1.0f };
								modelMatrix = glm::translate(modelMatrix, modelTransform.m_position);
								modelMatrix *= glm::mat4_cast(modelTransform.m_rotation);
								m_worldRenderDebug.m_modelShader->setUniform("u_model", modelMatrix);
								glm::mat3 const modelNormalMatrix{ glm::transpose(glm::inverse(modelMatrix)) };
								m_worldRenderDebug.m_modelShader->setUniform("u_modelNormal", modelNormalMatrix);

								// Render meshes
								auto k = 0u;
								for (auto const& t_mesh : modelGraphic.m_model->m_meshes)
								{
									if (k < modelGraphic.m_materials.size()
										&& modelGraphic.m_materials[k].isValid())
									{
										auto const& t_material = *(modelGraphic.m_materials[k]);

										// Diffuse
										m_worldRenderDebug.m_modelShader->setUniform("u_material.m_diffuse", 0);
										glActiveTexture(GL_TEXTURE0);
										if (t_material.m_diffuse.isValid())
										{
											glBindTexture(GL_TEXTURE_2D, t_material.m_diffuse->m_texture.getNativeHandle());
										}
										else
										{
											glBindTexture(GL_TEXTURE_2D, 0);
										}

										// Specular
										m_worldRenderDebug.m_modelShader->setUniform("u_material.m_specular", 1);
										glActiveTexture(GL_TEXTURE1);
										if (t_material.m_diffuse.isValid())
										{
											glBindTexture(GL_TEXTURE_2D, t_material.m_diffuse->m_texture.getNativeHandle());
										}
										else
										{
											glBindTexture(GL_TEXTURE_2D, 0);
										}

										glBindVertexArray(t_mesh.m_vertexArrayObject);
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

					// Lines rendering
					{
						m_worldRenderDebug.m_debugShader->m_glProgram.startUsing();

						m_worldRenderDebug.m_debugShader->setUniform("u_view", viewMatrix);
						m_worldRenderDebug.m_debugShader->setUniform("u_projection", projectionMatrix);

						auto& t_mesh = m_worldRenderDebug.m_debugMesh;
						t_mesh.update();

						glBindVertexArray(t_mesh.m_vertexArrayObject);
						glDrawElements(GL_LINES
							, static_cast<std::uint32_t>(t_mesh.m_lines.size())
							, GL_UNSIGNED_INT, nullptr);
						t_mesh.reset();
					}

					m_sceneRenderComponent.m_renderTexture.stopRenderTo();
				}
				{
					/*ImGui::SFML::Update(m_worldWindow.getWindow()
						, static_cast<sf::Vector2f>(m_worldWindow.getWindow().getSize())
						, sf::milliseconds(16));
					ImGui::ShowTestWindow();*/

					glDisable(GL_DEPTH_TEST);
					glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT);
					m_sceneRenderComponent.m_postProcessShader->m_glProgram.startUsing();
					m_sceneRenderComponent.m_renderShape.draw();
					//ImGui::SFML::Render();
					m_worldWindow.getWindow().display();
				}
			}

		private:
			WindowComponent& m_worldWindow;
			SceneRenderComponent& m_sceneRenderComponent;
			RenderDebugComponent& m_worldRenderDebug;
			DirectorComponent& m_directorComponent;
			ecs::SystemEntityList<RenderSystem, TransformComponent const
				, CameraComponent const> const& m_cameramanList;
			ecs::SystemEntityList<RenderSystem, TransformComponent const
				, GraphicComponent const> const& m_modelList;

			// TODO
			mutable float t{ 2.0f };
			mutable int tt{ 0 };
		};
	}
}
