#pragma once

#include <entt/entt.hpp>


namespace vob::aoeac
{
	struct action_component
	{
		bool m_canInteract = true;
		bool m_isInteracting = false;
		entt::entity m_interactor;
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoeac::action_component>
	bool accept(VisitorType& a_visitor, ThisType& a_this) { return true; }
}
