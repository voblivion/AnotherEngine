#include "vob/aoe/rendering/systems/ReleaseUnusedGpuResourcesSystem.h"

#include "vob/aoe/rendering/resources/GpuMaterial.h"
#include "vob/aoe/rendering/resources/GpuShader.h"


namespace vob::aoegl
{
	void ReleaseUnusedGpuResourcesSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		
	}

	void ReleaseUnusedGpuResourcesSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& gpuResourceRegistriesCtx = m_gpuResourceRegistriesContext.get(a_wdap);

		auto& bufferRegistry = *gpuResourceRegistriesCtx.bufferRegistry;
		bufferRegistry.processUnusedResources([](GpuBuffer gpuBuffer)
			{
				glDeleteBuffers(1, &gpuBuffer.bo);
			});

		auto& materialRegistry = *gpuResourceRegistriesCtx.materialRegistry;
		materialRegistry.processUnusedResources([](GpuMaterial gpuMaterial)
			{
				if (gpuMaterial.paramsUbo != k_invalidId)
				{
					glDeleteBuffers(1, &gpuMaterial.paramsUbo);
				}
				glDeleteTextures(mistd::isize(gpuMaterial.textureIds), gpuMaterial.textureIds.data());
			});

		auto& meshRegistry = *gpuResourceRegistriesCtx.meshRegistry;
		meshRegistry.processUnusedResources([](GpuMesh gpuMesh)
			{
				glDeleteVertexArrays(1, &gpuMesh.vao);
				glDeleteBuffers(1, &gpuMesh.vbo);
				glDeleteBuffers(1, &gpuMesh.ebo);
			});

		auto& shaderRegistry = *gpuResourceRegistriesCtx.shaderRegistry;
		shaderRegistry.processUnusedResources([](GpuShader gpuShader)
			{
				glDeleteProgram(gpuShader.staticProgram);
				glDeleteProgram(gpuShader.riggedProgram);
				glDeleteProgram(gpuShader.instancedProgram);
			});
	}
}
