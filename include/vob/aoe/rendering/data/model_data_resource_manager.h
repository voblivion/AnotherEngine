#pragma once

#include <vob/aoe/rendering/data/texture_data_resource_manager.h>
#include <vob/aoe/rendering/data/model_data.h>
#include <vob/aoe/rendering/resources/model.h>

#include <vob/aoe/common/data_resource_manager.h>

#include <vob/aoe/api.h>


namespace vob::aoegl
{
	class VOB_AOE_API model_data_resource_manager
	{
	public:
		explicit model_data_resource_manager(texture_data_resource_manager& a_textureManager);

		model const& add_reference(std::shared_ptr<model_data const> const& a_data);

		void remove_reference(std::shared_ptr<model_data const> const& a_data);

	private:

		texture_data_resource_manager& m_textureManager;

		aoeco::data_resource_manager<std::shared_ptr<model_data const>, model> m_manager;
	};
}
