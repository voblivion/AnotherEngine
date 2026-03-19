#include <vob/aoe/rendering/ModelUtils.h>

#include <vob/misc/std/container_util.h>

#include <cstddef>
#include <cstdint>


namespace vob::aoegl
{
	namespace
	{
		template <typename T, typename = void, typename = void>
		struct has_rig_data : std::false_type {};

		template <typename T>
		struct has_rig_data<T, std::void_t<decltype(T::boneIndices)>, std::void_t<decltype(T::boneWeights)>> : std::true_type {};

		template<typename TModelData>
		Model createModel(TModelData const& a_modelData)
		{
			Model model;
			for (auto const& meshData : a_modelData.meshes)
			{
				auto& mesh = model.meshes.emplace_back();
				glCreateVertexArrays(1, &mesh.vao);
				glCreateBuffers(1, &mesh.vbo);
				glCreateBuffers(1, &mesh.ebo);

				auto const& vertices = meshData.vertices;
				auto const& indices = meshData.indices;

				using VertexType = std::remove_cvref_t<decltype(vertices.front())>;
				using IndexType = std::remove_cvref_t<decltype(indices.front())>;

				glNamedBufferStorage(
					mesh.vbo, vertices.size() * sizeof(VertexType), vertices.data(), 0 /* flags */);
				glVertexArrayVertexBuffer(
					mesh.vao, 0 /* binding index */, mesh.vbo, 0 /* offset */, sizeof(VertexType));

				glNamedBufferStorage(
					mesh.ebo, indices.size() * sizeof(IndexType), indices.data(), 0 /* flags */);
				glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

				glEnableVertexArrayAttrib(mesh.vao, k_vertexPositionLocation);
				glVertexArrayAttribFormat(
					mesh.vao,
					k_vertexPositionLocation,
					3 /* size */,
					GL_FLOAT,
					GL_FALSE /* normalized */,
					offsetof(VertexType, position));
				glVertexArrayAttribBinding(mesh.vao, k_vertexPositionLocation, 0 /* binding index */);

				glEnableVertexArrayAttrib(mesh.vao, k_vertexNormalLocation);
				glVertexArrayAttribFormat(
					mesh.vao,
					k_vertexNormalLocation,
					3 /* size */,
					GL_FLOAT,
					GL_FALSE /* normalized */,
					offsetof(VertexType, normal));
				glVertexArrayAttribBinding(mesh.vao, k_vertexNormalLocation, 0 /* binding index */);

				glEnableVertexArrayAttrib(mesh.vao, k_vertexUVLocation);
				glVertexArrayAttribFormat(
					mesh.vao,
					k_vertexUVLocation,
					2 /* size */,
					GL_FLOAT,
					GL_FALSE /* normalized */,
					offsetof(VertexType, uv));
				glVertexArrayAttribBinding(mesh.vao, k_vertexUVLocation, 0 /* binding index */);

				glEnableVertexArrayAttrib(mesh.vao, k_vertexTangentLocation);
				glVertexArrayAttribFormat(
					mesh.vao,
					k_vertexTangentLocation,
					3 /* size */,
					GL_FLOAT,
					GL_FALSE /* normalized */,
					offsetof(VertexType, tangent));
				glVertexArrayAttribBinding(mesh.vao, k_vertexTangentLocation, 0 /* binding index */);

				if constexpr (has_rig_data<VertexType>::value)
				{
					glEnableVertexArrayAttrib(mesh.vao, k_vertexBoneIndicesLocation);
					glVertexArrayAttribIFormat(
						mesh.vao,
						k_vertexBoneIndicesLocation,
						4 /* size */,
						GL_INT,
						offsetof(VertexType, boneIndices));
					glVertexArrayAttribBinding(mesh.vao, k_vertexBoneIndicesLocation, 0 /* binding index */);

					glEnableVertexArrayAttrib(mesh.vao, k_vertexBoneWeightsLocation);
					glVertexArrayAttribFormat(
						mesh.vao,
						k_vertexBoneWeightsLocation,
						4 /* size */,
						GL_FLOAT,
						GL_FALSE /* normalized */,
						offsetof(VertexType, boneWeights));
					glVertexArrayAttribBinding(mesh.vao, k_vertexBoneWeightsLocation, 0 /* binding index */);
				}

				mesh.indexCount = mistd::isize(indices);

				for (auto const& vertex : vertices)
				{
					auto const vertexLengthSq = glm::dot(vertex.position, vertex.position);
					if (model.boundingRadius * model.boundingRadius < vertexLengthSq)
					{
						model.boundingRadius = std::sqrt(vertexLengthSq);
					}
				}
			}

			return model;
		}
	}

	Model createStaticModel(StaticModelData const& a_staticModelData)
	{
		return createModel(a_staticModelData);
	}

	Model createRiggedModel(RiggedModelData const& a_riggedModelData)
	{
		return createModel(a_riggedModelData);
	}
}
