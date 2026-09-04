#pragma once

#include "vob/aoe/rendering/Model.h"
#include "vob/aoe/rendering/data/ModelData.h"


namespace vob::aoegl
{
	// TODO: move to glsl-shared file.
	constexpr static GraphicInt k_vertexPositionLocation = 0;
	constexpr static GraphicInt k_vertexNormalLocation = 1;
	constexpr static GraphicInt k_vertexUVLocation = 2;
	constexpr static GraphicInt k_vertexTangentLocation = 3;
	constexpr static GraphicInt k_vertexBoneIndicesLocation = 4;
	constexpr static GraphicInt k_vertexBoneWeightsLocation = 5;
	constexpr static GraphicInt k_instanceRow0Location = 4;

	std::shared_ptr<GpuMesh> createStaticMesh(
		GpuDeleteQueue& a_deleteQueue, StaticMeshData const& a_staticMeshData, float& a_boundingRadius);
	std::shared_ptr<GpuMesh> createRiggedMesh(
		GpuDeleteQueue& a_deleteQueue, RiggedMeshData const& a_riggedMeshData, float& a_boundingRadius);
	Model createStaticModel(GpuDeleteQueue& a_deleteQueue, StaticModelData const& a_staticModelData);
	Model createRiggedModel(GpuDeleteQueue& a_deleteQueue, RiggedModelData const& a_riggedModelData);
	Model createInstancedModel(GpuDeleteQueue& a_deleteQueue, StaticModelData const& a_staticModelData);
}
