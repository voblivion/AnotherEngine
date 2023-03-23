#include <vob/aoe/rendering/data/model_data_resource_manager.h>


namespace vob::aoegl
{
	namespace
	{
		void allocate_material(
			texture_data_resource_manager& a_textureManager,
			material_data const& a_materialData,
			material& a_material)
		{
			a_material.m_albedo = a_textureManager.add_reference(a_materialData.m_albedo);
			a_material.m_normal = a_textureManager.add_reference(a_materialData.m_normal);
			a_material.m_metallicRoughness = a_textureManager.add_reference(
				a_materialData.m_metallicRoughness);
		}

		void allocate_mesh(
			mesh_data const& a_meshData,
			mesh& a_mesh)
		{
#define _GEN_BUFFER(name) \
		glCreateBuffers(1, &(a_mesh.m_##name##Vbo)); \
		glBindBuffer(GL_ARRAY_BUFFER, a_mesh.m_##name##Vbo); \
		glBufferData( \
			GL_ARRAY_BUFFER, \
			a_meshData.m_##name##s.size() * sizeof(decltype(a_meshData.m_##name##s.front())), \
			a_meshData.m_##name##s.data(), \
			GL_STATIC_DRAW);

			_GEN_BUFFER(position);
			_GEN_BUFFER(textureCoord);
			_GEN_BUFFER(normal);
			_GEN_BUFFER(tangent);
#undef _GEN_BUFFER

			glCreateBuffers(1, &(a_mesh.m_ebo));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, a_mesh.m_ebo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				a_meshData.m_triangles.size() * sizeof(triangle),
				a_meshData.m_triangles.data(),
				GL_STATIC_DRAW);

			a_mesh.m_triangleCount = static_cast<graphic_size>(
				a_meshData.m_triangles.size());
		}

		void allocate_textured_mesh(
			texture_data_resource_manager& a_textureManager,
			textured_mesh_data const& a_texturedMeshData,
			textured_mesh& a_texturedMesh)
		{
			allocate_material(
				a_textureManager, a_texturedMeshData.m_material, a_texturedMesh.m_material);

			allocate_mesh(a_texturedMeshData.m_mesh, a_texturedMesh.m_mesh);
		}

		void allocate_model(
			texture_data_resource_manager& a_textureManager,
			model_data const& a_modelData,
			model& a_model)
		{
			a_model.m_texturedMeshes.resize(a_modelData.m_texturedMeshes.size());
			for (auto i = 0u; i < a_model.m_texturedMeshes.size(); ++i)
			{
				allocate_textured_mesh(
					a_textureManager, a_modelData.m_texturedMeshes[i], a_model.m_texturedMeshes[i]);
			}
		}


		void deallocate_material(
			texture_data_resource_manager& a_textureManager,
			material_data const& a_materialData)
		{
			a_textureManager.remove_reference(a_materialData.m_albedo);
			a_textureManager.remove_reference(a_materialData.m_normal);
			a_textureManager.remove_reference(a_materialData.m_metallicRoughness);
		}

		void deallocate_mesh(mesh& a_mesh)
		{
			glDeleteBuffers(1, &a_mesh.m_ebo);
			glDeleteBuffers(1, &a_mesh.m_positionVbo);
			glDeleteBuffers(1, &a_mesh.m_textureCoordVbo);
			glDeleteBuffers(1, &a_mesh.m_normalVbo);
			glDeleteBuffers(1, &a_mesh.m_tangentVbo);
		}

		void deallocate_textured_mesh(
			texture_data_resource_manager& a_textureManager,
			textured_mesh_data const& a_texturedMeshData,
			textured_mesh& a_texturedMesh)
		{
			deallocate_material(a_textureManager, a_texturedMeshData.m_material);
		}

		void deallocate_model(
			texture_data_resource_manager& a_textureManager,
			model_data const& a_modelData,
			model& a_model)
		{
			for (auto i = 0u; i < a_model.m_texturedMeshes.size(); ++i)
			{
				deallocate_textured_mesh(
					a_textureManager, a_modelData.m_texturedMeshes[i], a_model.m_texturedMeshes[i]);
			}
		}
	}

	model_data_resource_manager::model_data_resource_manager(
		texture_data_resource_manager& a_textureManager)
		: m_textureManager{ a_textureManager }
	{}

	model const& model_data_resource_manager::add_reference(
		std::shared_ptr<model_data const> const& a_data)
	{
		return m_manager.add_reference(
			a_data,
			[this](auto const& a_modelData) {
				model newModel;
				allocate_model(m_textureManager, *a_modelData, newModel);
				return newModel;
			});
	}

	void model_data_resource_manager::remove_reference(
		std::shared_ptr<model_data const> const& a_data)
	{
		m_manager.remove_reference(
			a_data,
			[this](auto const& a_modelData, auto& a_model) {
				deallocate_model(m_textureManager, *a_modelData, a_model);
			});
	}
}
