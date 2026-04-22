#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include "gl/GL.h"
#include "glm/glm.hpp"


namespace vob::aoegl
{

	enum class GpuDepthFunc : GraphicEnum
	{
		Less = GL_LESS,
		LessEqual = GL_LEQUAL,
		Equal = GL_EQUAL
	};

	enum class GpuStateChange
	{
		// A change must happen, else we are wasting gpu calls
		SurelyYes,
		// A change often happens, no need to branch
		LikelyYes,
		// A change rarely happens, branch
		LikelyNo,
		// A change must not happen, else something went wrong
		SurelyNo
	};

	struct GpuState
	{
		template<GpuStateChange t_expectedChange, typename TValue, typename TPushChangeFunc>
		void changeState(TValue& a_currentState, TValue a_newState, TPushChangeFunc a_pushChangeFunc)
		{
			if constexpr (t_expectedChange == GpuStateChange::SurelyYes)
			{
				assert(a_currentState != a_newState);
				a_pushChangeFunc(a_newState);
				a_currentState = a_newState;
			}
			else if constexpr (t_expectedChange == GpuStateChange::LikelyYes)
			{
				a_pushChangeFunc(a_newState);
				a_currentState = a_newState;
			}
			else if constexpr (t_expectedChange == GpuStateChange::LikelyNo)
			{
				if (a_currentState != a_newState)
				{
					a_pushChangeFunc(a_newState);
					a_currentState = a_newState;
				}
			}
			else
			{
				assert(a_currentState == a_newState);
			}
		}

		template<GpuStateChange t_expectedChange>
		void enableDepthTest()
		{
			changeState<t_expectedChange>(m_depthTest, TernaryState::True, [](auto) { glEnable(GL_DEPTH_TEST); });
		}

		template<GpuStateChange t_expectedChange>
		void disableDepthTest()
		{
			changeState<t_expectedChange>(
				m_depthTest, TernaryState::False, [](auto) { glDisable(GL_DEPTH_TEST); });
		}

		template<GpuStateChange t_expectedChange>
		void enableDepthWrite()
		{
			changeState<t_expectedChange>(m_depthWrite, TernaryState::True, [](auto) { glDepthMask(GL_TRUE); });
		}

		template<GpuStateChange t_expectedChange>
		void disableDepthWrite()
		{
			changeState<t_expectedChange>(m_depthWrite, TernaryState::False, [](auto) { glDepthMask(GL_FALSE); });
		}

		template<GpuStateChange t_expectedChange>
		void enableBlend()
		{
			changeState<t_expectedChange>(m_blend, TernaryState::True, [](auto) { glEnable(GL_BLEND); });
		}

		template<GpuStateChange t_expectedChange>
		void disableBlend()
		{
			changeState<t_expectedChange>(m_blend, TernaryState::False, [](auto) { glDisable(GL_BLEND); });
		}

		template<GpuStateChange t_expectedChange>
		void setDepthFunc(GpuDepthFunc a_depthFunc)
		{
			changeState<t_expectedChange>(
				m_depthFunc, std::to_underlying(a_depthFunc), [](auto depthFunc) { glDepthFunc(depthFunc); });
		}

		template<GpuStateChange t_expectedChange>
		void setClearDepth(double a_clearDepth)
		{
			changeState<t_expectedChange>(
				m_clearDepth, a_clearDepth, [](auto clearDepth) { glClearDepth(clearDepth); });
		}

		template<GpuStateChange t_expectedChange>
		void enableColorWrite()
		{
			changeState<t_expectedChange>(
				m_colorWrite, TernaryState::True, [](auto) { glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); });
		}

		template<GpuStateChange t_expectedChange>
		void disableColorWrite()
		{
			changeState<t_expectedChange>(
				m_colorWrite, TernaryState::False, [](auto) { glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); });
		}

		template<GpuStateChange t_expectedChange>
		void setClearColor(glm::vec4 a_clearColor)
		{
			changeState<t_expectedChange>(
				m_clearColor, a_clearColor, [](auto clearColor) { glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w); });
		}

		template<GpuStateChange t_expectedChange>
		void setViewport(glm::ivec4 a_viewport)
		{
			changeState<t_expectedChange>(
				m_viewport,
				a_viewport,
				[](auto viewport) { glViewport(viewport.x, viewport.y, viewport.z, viewport.w); });
		}

		template<GpuStateChange t_expectedChange>
		void bindFramebuffer(GraphicId a_framebuffer)
		{
			changeState<t_expectedChange>(
				m_framebuffer,
				a_framebuffer,
				[](auto framebuffer) { glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); });
		}

		template<GpuStateChange t_expectedChange>
		void unbindFramebuffer()
		{
			changeState<t_expectedChange>(m_framebuffer, k_invalidId, [](auto) {});
		}

		template<GpuStateChange t_expectedChange>
		void bindUbo(GraphicId a_index, GraphicId a_ubo)
		{
			changeState<t_expectedChange>(
				m_ubos[a_index],
				a_ubo,
				[a_index](auto ubo) { glBindBufferBase(GL_UNIFORM_BUFFER, a_index, ubo); });
		}

		template<GpuStateChange t_expectedChange>
		void unbindUbo(GraphicId a_index)
		{
			changeState<t_expectedChange>(m_ubos[a_index], k_invalidId, [](auto) {});
		}

		template<GpuStateChange t_expectedChange>
		void bindSsbo(GraphicId a_index, GraphicId a_ssbo)
		{
			changeState<t_expectedChange>(
				m_ssbos[a_index],
				a_ssbo,
				[a_index](auto ssbo) { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, a_index, ssbo); });
		}

		template<GpuStateChange t_expectedChange>
		void unbindSsbo(GraphicId a_index)
		{
			changeState<t_expectedChange>(m_ubos[a_index], k_invalidId, [a_index](auto) {});
		}

		template<GpuStateChange t_expectedChange>
		void bindTexture(GraphicId a_index, GraphicId a_texture)
		{
			changeState<t_expectedChange>(
				m_textures[a_index],
				a_texture,
				[a_index](auto texture) { glBindTextureUnit(a_index, texture); });
		}

		template<GpuStateChange t_expectedChange>
		void unbindTexture(GraphicId a_index)
		{
			changeState<t_expectedChange>(m_textures[a_index], k_invalidId, [a_index](auto) {});
		}

		template<GpuStateChange t_expectedChange>
		void useProgram(GraphicId a_program)
		{
			changeState<t_expectedChange>(m_program, a_program, [](auto program) { glUseProgram(program); });
		}

	private:
		std::array<GraphicId, 36> m_ubos = { k_invalidId };
		std::array<GraphicId, 8> m_ssbos = { k_invalidId };
		std::array<GraphicId, 32> m_textures = { k_invalidId };

		enum class TernaryState : int8_t
		{
			Unknown = -1,
			False = 0,
			True = 1
		};
		TernaryState m_depthTest = TernaryState::Unknown;
		TernaryState m_blend = TernaryState::Unknown;
		TernaryState m_depthWrite = TernaryState::Unknown;
		TernaryState m_colorWrite = TernaryState::Unknown;
		GraphicEnum m_depthFunc = GL_NEVER;
		GraphicId m_framebuffer = k_invalidId;
		GraphicId m_program = k_invalidId;
		glm::ivec4 m_viewport = glm::ivec4{ -1 };
		double m_clearDepth = -1.0;
		glm::vec4 m_clearColor = glm::vec4{ -1.0f };
	};
}
