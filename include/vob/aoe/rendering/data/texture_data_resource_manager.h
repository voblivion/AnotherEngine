#pragma once

#include <vob/aoe/rendering/data/texture_data.h>
#include <vob/aoe/rendering/graphic_types.h>

#include <vob/aoe/common/data_resource_manager.h>

#include <vob/aoe/api.h>

#include <memory>


namespace vob::aoegl
{
	class VOB_AOE_API texture_data_resource_manager
	{
	public:
		graphic_id const& add_reference(std::shared_ptr<texture_data const> const& a_textureData);

		void remove_reference(std::shared_ptr<texture_data const> const& a_textureData);

	private:
		aoeco::data_resource_manager<std::shared_ptr<texture_data const>, graphic_id> m_manager;
	};
}
