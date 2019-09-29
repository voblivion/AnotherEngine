#pragma once

#include <aoe/core/data/ALoader.h>
#include <aoe/core/standard/Allocator.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <aoe/common/render/Model.h>
#include <aoe/common/render/AssimpUtils.h>
#include <aoe/common/Export.h>

namespace aoe
{
	namespace common
	{

		class AOE_COMMON_API AssimpLoader final
			: public data::ALoader
		{
		public:
			explicit AssimpLoader(std::pmr::memory_resource* m_resource
				= std::pmr::get_default_resource());

			// Methods
			virtual std::shared_ptr<sta::ADynamicType> load(
				std::istream& a_inputStream) override;

		private:
			// Attributes
			Assimp::Importer m_importer;
			std::pmr::memory_resource* m_resource;
		};
	}
}
