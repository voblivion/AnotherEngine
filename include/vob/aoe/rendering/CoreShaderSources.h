#pragma once

#include "vob/aoe/rendering/data/ShaderSource.h"

#include <vob/misc/visitor/name_value_pair.h>

#include <memory>
#include <type_traits>


namespace vob::aoegl
{
	struct CoreShaderSources
	{
		std::shared_ptr<ShaderSource const> geometryVertexShader;
		std::shared_ptr<ShaderSource const> lightClusteringShader;
		std::shared_ptr<ShaderSource const> opaqueShell;
		std::shared_ptr<ShaderSource const> depthShell;
		std::shared_ptr<ShaderSource const> shadowMapShell;
		std::shared_ptr<ShaderSource const> quadVertexShader;
		std::shared_ptr<ShaderSource const> ssaoDepthFragmentShader;
		std::shared_ptr<ShaderSource const> ssaoFragmentShader;
		std::shared_ptr<ShaderSource const> ssaoBlurFragmentShader;
		std::shared_ptr<ShaderSource const> skyShell;
		std::shared_ptr<ShaderSource const> skyIrradianceShader;
		std::shared_ptr<ShaderSource const> ssrFragmentShader;
		std::shared_ptr<ShaderSource const> hizReduce;
		std::shared_ptr<ShaderSource const> ssrPrefilter;
		std::shared_ptr<ShaderSource const> ssrDownsample;
		std::shared_ptr<ShaderSource const> opaqueCompositionFragmentShader;
		std::shared_ptr<ShaderSource const> debugFragmentShader;
		std::shared_ptr<ShaderSource const> debugGeometryVertexShader;
		std::shared_ptr<ShaderSource const> debugGeometryFragmentShader;
		std::shared_ptr<ShaderSource const> bloomDownsample;
		std::shared_ptr<ShaderSource const> bloomUpsample;
		std::shared_ptr<ShaderSource const> bloomCombine;
		std::shared_ptr<ShaderSource const> tonemap;
		std::shared_ptr<ShaderSource const> fxaa;
		std::shared_ptr<ShaderSource const> present;
	};

	template <typename TVisitor, typename TSources>
		requires std::is_same_v<std::remove_cvref_t<TSources>, CoreShaderSources>
	bool accept(TVisitor& a_visitor, TSources& a_sources)
	{
		auto result = true;
		result &= a_visitor.visit(misvi::nvp("geometryVertexShader", a_sources.geometryVertexShader));
		result &= a_visitor.visit(misvi::nvp("lightClusteringShader", a_sources.lightClusteringShader));
		result &= a_visitor.visit(misvi::nvp("opaqueShell", a_sources.opaqueShell));
		result &= a_visitor.visit(misvi::nvp("depthShell", a_sources.depthShell));
		result &= a_visitor.visit(misvi::nvp("shadowMapShell", a_sources.shadowMapShell));
		result &= a_visitor.visit(misvi::nvp("quadVertexShader", a_sources.quadVertexShader));
		result &= a_visitor.visit(misvi::nvp("ssaoDepthFragmentShader", a_sources.ssaoDepthFragmentShader));
		result &= a_visitor.visit(misvi::nvp("ssaoFragmentShader", a_sources.ssaoFragmentShader));
		result &= a_visitor.visit(misvi::nvp("ssaoBlurFragmentShader", a_sources.ssaoBlurFragmentShader));
		result &= a_visitor.visit(misvi::nvp("skyShell", a_sources.skyShell));
		result &= a_visitor.visit(misvi::nvp("skyIrradianceShader", a_sources.skyIrradianceShader));
		result &= a_visitor.visit(misvi::nvp("ssrFragmentShader", a_sources.ssrFragmentShader));
		result &= a_visitor.visit(misvi::nvp("hizReduce", a_sources.hizReduce));
		result &= a_visitor.visit(misvi::nvp("ssrPrefilter", a_sources.ssrPrefilter));
		result &= a_visitor.visit(misvi::nvp("ssrDownsample", a_sources.ssrDownsample));
		result &= a_visitor.visit(misvi::nvp("opaqueCompositionFragmentShader", a_sources.opaqueCompositionFragmentShader));
		result &= a_visitor.visit(misvi::nvp("debugFragmentShader", a_sources.debugFragmentShader));
		result &= a_visitor.visit(misvi::nvp("debugGeometryVertexShader", a_sources.debugGeometryVertexShader));
		result &= a_visitor.visit(misvi::nvp("debugGeometryFragmentShader", a_sources.debugGeometryFragmentShader));
		result &= a_visitor.visit(misvi::nvp("bloomDownsample", a_sources.bloomDownsample));
		result &= a_visitor.visit(misvi::nvp("bloomUpsample", a_sources.bloomUpsample));
		result &= a_visitor.visit(misvi::nvp("bloomCombine", a_sources.bloomCombine));
		result &= a_visitor.visit(misvi::nvp("tonemap", a_sources.tonemap));
		result &= a_visitor.visit(misvi::nvp("fxaa", a_sources.fxaa));
		result &= a_visitor.visit(misvi::nvp("present", a_sources.present));
		return result;
	}
}
