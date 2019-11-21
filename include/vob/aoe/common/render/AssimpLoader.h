#pragma once

#include <vob/aoe/core/data/ALoader.h>
#include <assimp/Importer.hpp>
#include <vob/aoe/common/opengl/Manager.h>
#include <vob/aoe/common/opengl/resources/StaticModel.h>

namespace vob::aoe::ogl
{
	class VOB_AOE_API AssimpLoader final
		: public data::ALoader
	{
	public:
		explicit AssimpLoader(
			Manager<StaticModel>& a_staticModelResourceManager
			, std::pmr::memory_resource* a_memoryResource = std::pmr::get_default_resource()
		);

		// Methods
		std::shared_ptr<type::ADynamicType> load(std::istream& a_inputStream) override;

	private:
		// Attributes
		Assimp::Importer m_importer;
		Manager<StaticModel>& m_staticModelResourceManager;
		std::pmr::memory_resource* m_memoryResource;
	};
}
