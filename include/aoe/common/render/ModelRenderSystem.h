#pragma once
#include <aoe/core/ecs/WorldDataProvider.h>
#include "RenderDebugComponent.h"
#include <gl/GL.h>
#include <aoe/common/window/WindowComponent.h>
#include "CameraComponent.h"
#include <aoe/common/space/TransformComponent.h>
#include "DirectorComponent.h"
#include <aoe/common/render/GraphicComponent.h>

namespace aoe
{
	namespace common
	{
		class ModelRenderSystem
		{
		public:
			using CameraComponents = ecs::ComponentTypeList<
				TransformComponent const, CameraComponent const>;
			using ModelComponents = ecs::ComponentTypeList<
				TransformComponent const, GraphicComponent const>;

			explicit ModelRenderSystem(ecs::WorldDataProvider& a_wdp)
				: m_worldWindow{ *a_wdp.getWorldComponent<WindowComponent>() }
				, m_worldRenderDebug{ *a_wdp.getWorldComponent<RenderDebugComponent>() }
				, m_directorComponent{ *a_wdp.getWorldComponent<DirectorComponent>() }
				, m_cameramanList{ a_wdp.getEntityList(CameraComponents{}) }
				, m_modelList{ a_wdp.getEntityList(ModelComponents{}) }
			{
				glEnable(GL_DEPTH_TEST);
			}

			void update() const
			{
				// Retrieve current camera
				auto const* cameraman = m_cameramanList.find(
					m_directorComponent.m_currentCameraman);
				if(cameraman == nullptr && !m_cameramanList.empty())
				{
					cameraman = &m_cameramanList.front();
					m_directorComponent.m_currentCameraman = cameraman->getId();
				}

				if (cameraman != nullptr)
				{
					// Reset canvas
					m_worldRenderDebug.shader->use();
					glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

					// Ambient light
					m_worldRenderDebug.shader->setUniform("u_ambientColor"
						, glm::vec3{ 0.02f });

					m_worldRenderDebug.shader->setUniform("u_lightPosition"
						, glm::vec3{ 10.0f * std::sin(t), 20.0f, 10.0f * std::cos(t) });
					t += 0.0002f;
					t = std::fmod(t, 2.0f*3.141592f);
					glBindVertexArray(0);

					// Get cameraman components
					auto const& cameramanTransform = cameraman->getComponent<TransformComponent>();
					auto const& cameramanCamera = cameraman->getComponent<CameraComponent>();
					auto const windowSize = m_worldWindow.getWindow().getSize();

					// Setup view matrix and position
					glm::mat4 viewMatrix{ 1.0f };
					viewMatrix = glm::translate(viewMatrix, cameramanTransform.m_position);
					viewMatrix *= glm::mat4_cast(cameramanTransform.m_rotation);
					viewMatrix = glm::inverse(viewMatrix);
					m_worldRenderDebug.shader->setUniform("u_view", viewMatrix);
					m_worldRenderDebug.shader->setUniform("u_viewPosition", cameramanTransform.m_position);

					// Setup projection matrix
					auto const projection = glm::perspective(
						glm::radians(cameramanCamera.fov)
						, float(windowSize.x) / windowSize.y
						, cameramanCamera.nearClip, cameramanCamera.farClip);
					m_worldRenderDebug.shader->setUniform("u_projection", projection);

					for(auto t_model : m_modelList)
					{
						// Get model components
						auto const& modelTransform = t_model.getComponent<TransformComponent>();
						if (tt++ % 1000 == 0)
						{
							std::cout << modelTransform.m_position.x << " ";
							std::cout << modelTransform.m_position.y << " ";
							std::cout << modelTransform.m_position.z << std::endl;
							std::cout << cameramanTransform.m_position.x << " ";
							std::cout << cameramanTransform.m_position.y << " ";
							std::cout << cameramanTransform.m_position.z << std::endl;
						}
						auto const& modelGraphic = t_model.getComponent<GraphicComponent>();

						if (modelGraphic.m_model.isValid())
						{
							// Setup model matrices
							glm::mat4 modelMatrix{ 1.0f };
							modelMatrix = glm::translate(modelMatrix, modelTransform.m_position);
							modelMatrix *= glm::mat4_cast(modelTransform.m_rotation);
							m_worldRenderDebug.shader->setUniform("u_model", modelMatrix);
							glm::mat3 const modelNormalMatrix{ glm::transpose(glm::inverse(modelMatrix)) };
							m_worldRenderDebug.shader->setUniform("u_modelNormal", modelNormalMatrix);

							// Render meshes
							auto k = 0u;
							for (auto const& t_mesh : modelGraphic.m_model->m_meshes)
							{
								if (k < modelGraphic.m_materials.size()
									&& modelGraphic.m_materials[k].isValid())
								{
									auto const& t_material = *(modelGraphic.m_materials[k]);

									// Diffuse
									m_worldRenderDebug.shader->setUniform("u_material.m_diffuse", 0);
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
									m_worldRenderDebug.shader->setUniform("u_material.m_specular", 1);
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
								}
							}
						}
					}

					m_worldWindow.getWindow().display();
				}
			}

		private:
			WindowComponent& m_worldWindow;
			RenderDebugComponent& m_worldRenderDebug;
			DirectorComponent& m_directorComponent;
			ecs::SystemEntityList<TransformComponent const
				, CameraComponent const> const& m_cameramanList;
			ecs::SystemEntityList<TransformComponent const
				, GraphicComponent const> const& m_modelList;

			mutable float t{ 2.0f };
			mutable int tt{ 0 };
		};
	}
}
