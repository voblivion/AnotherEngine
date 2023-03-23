#pragma once

#include <fstream>

#include <vob/aoe/core/data/ADatabase.h>

#include <vob/aoe/common/data/filesystem/AFileSystemLoader.h>
#include <vob/aoe/common/_render/Manager.h>
#include <vob/aoe/common/_render/model/static_model.h>
#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

#include <assimp/Importer.hpp>

namespace vob::aoe::common
{
	class VOB_AOE_API static_model_loader final
		: public AFileSystemLoader
	{
	public:
		// Constructors
		explicit static_model_loader(
			data::ADatabase& a_database
			, FileSystemIndexer& a_fileSystemIndexer
			, IGraphicResourceManager<static_model>& a_staticModelResourceManager
		);

		// Methods
		bool canLoad(std::filesystem::path const& a_path) const override;

		std::shared_ptr<type::ADynamicType> load(std::filesystem::path const& a_path) const override;

	private:
		// Attributes
		data::ADatabase& m_database;
		FileSystemIndexer& m_fileSystemIndexer;
		IGraphicResourceManager<static_model>& m_staticModelResourceManager;

		static void open(std::ifstream& a_file, std::filesystem::path const& a_path)
		{
			a_file.open(a_path, std::ios::binary | std::ios::in);
		}
	};
}
