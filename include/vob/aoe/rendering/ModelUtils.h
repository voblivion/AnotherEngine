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

	Model createStaticModel(StaticModelData const& a_staticModelData);
	Model createRiggedModel(RiggedModelData const& a_riggedModelData);
	Model createInstancedModel(StaticModelData const& a_staticModelData);
}
