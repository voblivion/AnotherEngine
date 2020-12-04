#pragma once

#include <fstream>

#include <vob/aoe/common/data/filesystem/AFileSystemLoader.h>
#include <assimp/Importer.hpp>
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/render/model/StaticModel.h>
#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

namespace vob::aoe::common
{
	class VOB_AOE_API ModelLoader final
		: public AFileSystemLoader
	{
	public:
		// Constructors
		explicit ModelLoader(
			data::ADatabase& a_database
			, FileSystemIndexer& a_fileSystemIndexer
			, IGraphicResourceManager<StaticModel>& a_staticModelResourceManager
		);

		// Methods
		bool canLoad(std::filesystem::path const& a_path) const override;

		std::shared_ptr<type::ADynamicType> load(std::filesystem::path const& a_path) const override;

	private:
		// Attributes
		data::ADatabase& m_database;
		FileSystemIndexer& m_fileSystemIndexer;
		IGraphicResourceManager<StaticModel>& m_staticModelResourceManager;

		static void open(std::ifstream& a_file, std::filesystem::path const& a_path)
		{
			a_file.open(a_path, std::ios::binary | std::ios::in);
		}
	};
}
