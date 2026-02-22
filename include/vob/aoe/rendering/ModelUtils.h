#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/Model.h>
#include <vob/aoe/rendering/ModelData.h>


namespace vob::aoegl
{
	constexpr static GraphicInt k_vertexPositionLocation = 0;
	constexpr static GraphicInt k_vertexNormalLocation = 1;
	constexpr static GraphicInt k_vertexUVLocation = 2;
	constexpr static GraphicInt k_vertexTangentLocation = 3;
	constexpr static GraphicInt k_vertexBoneIndicesLocation = 4;
	constexpr static GraphicInt k_vertexBoneWeightsLocation = 5;

	VOB_AOE_API Model createStaticModel(StaticModelData const& a_staticModelData);
	VOB_AOE_API Model createRiggedModel(RiggedModelData const& a_riggedModelData);
}
