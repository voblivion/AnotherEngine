#pragma once

#include <vob/aoe/rendering/color.h>
#include <vob/aoe/rendering/graphic_types.h>
#include <vob/aoe/rendering/uniform_util.h>
#include <vob/aoe/rendering/resources/debug_mesh.h>
#include <vob/aoe/rendering/resources/material.h>
#include <vob/aoe/rendering/resources/mesh.h>
#include <vob/aoe/rendering/resources/program.h>
#include <vob/aoe/rendering/resources/quad.h>
#include <vob/aoe/rendering/resources/render_texture.h>
#include <vob/aoe/rendering/resources/textured_mesh.h>

#include <vob/aoe/spacetime/measures.h>

#include <glm/glm.hpp>


namespace vob::aoegl
{
	struct vertex_attribute_index
	{
		static constexpr graphic_index position = 0;
		static constexpr graphic_index texture_coord = 2;
		static constexpr graphic_index normal = 1;
		static constexpr graphic_index tangent = 3;
		static constexpr graphic_index joint_indices = 4;
		static constexpr graphic_index joint_weights = 5;
	};

	struct vertex_buffer_index
	{
		static constexpr graphic_index static_attributes = 0;
		static constexpr graphic_index rigged_attributes = 1;
	};

	struct vertex_attribute_buffer_index
	{
		static constexpr graphic_index position = vertex_buffer_index::static_attributes;
		static constexpr graphic_index texture_coord = vertex_buffer_index::static_attributes;
		static constexpr graphic_index normal = vertex_buffer_index::static_attributes;
		static constexpr graphic_index tangent = vertex_buffer_index::static_attributes;
		static constexpr graphic_index joint_indices = vertex_buffer_index::rigged_attributes;
		static constexpr graphic_index joint_weights = vertex_buffer_index::rigged_attributes;
	};

	class context
	{
	public:
		class program_context
		{
		public:
			virtual ~program_context() = default;
		};

		class scene_program_context : public program_context
		{
		public:
			explicit scene_program_context(scene_program a_sceneProgram)
				: m_sceneProgram{ a_sceneProgram }
			{}

			void set_view_position(aoest::length_vector const& a_viewPosition);
			void set_view_projection_transform(glm::mat4 const& a_viewProjectionTransform);

		private:
			scene_program m_sceneProgram;
		};

		class debug_program_context : public scene_program_context
		{
		public:
			explicit debug_program_context(debug_program a_debugProgram)
				: scene_program_context{ a_debugProgram }
				, m_debugProgram{ a_debugProgram }
			{}

			void render(debug_mesh const& a_debugMesh);

		private:
			debug_program m_debugProgram;
		};

		class mesh_program_context : public scene_program_context
		{
		public:
			explicit mesh_program_context(mesh_program a_meshProgram)
				: scene_program_context{ a_meshProgram }
				, m_meshProgram{ a_meshProgram }
			{}

			void set_material(material const& a_material);
			void set_sun_color(rgb const& a_ambientColor);
			void set_sun_direction(aoest::length_vector const& a_sunDirection);
			void set_mesh_transform(glm::mat4 const& a_meshTransform);
			void set_mesh_normal_transform(glm::mat4 const& a_meshNormalTransform);
			void set_is_rigged(bool a_isRigged);
			void set_rig_pose(std::span<glm::mat4 const> a_rigPose);

		private:
			mesh_program m_meshProgram;
		};

		class post_process_program_context : public program_context
		{
		public:
			explicit post_process_program_context(post_process_program a_postProcessProgram)
				: m_postProcessProgram{ a_postProcessProgram }
			{}

			void set_window_size(glm::vec2 const& a_windowSize);

			void set_texture(graphic_id a_textureId);

			void render(quad const& a_quad);
			
		private:
			post_process_program m_postProcessProgram;
		};

		void use_framebuffer(graphic_id a_framebuffer, rgba const& a_clearColor = k_cyan);

		void use_program(debug_program const& a_debugProgram);

		void use_program(mesh_program const& a_meshProgram);

		void use_program(post_process_program const& a_postProcessProgram);

		scene_program_context* get_scene_program_context();
		debug_program_context* get_debug_program_context();
		mesh_program_context* get_mesh_program_context();
		post_process_program_context* get_post_process_program_context();

	private:
		graphic_id m_framebuffer = 0;
		std::shared_ptr<program_context> m_programContext;
	};
}
