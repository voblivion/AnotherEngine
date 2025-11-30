#pragma once

#include <vob/aoe/input/GameInputBindingContext.h>
#include <vob/aoe/input/GameInputContext.h>
#include <vob/aoe/input/InputBinding.h>


namespace vob::aoein
{
	namespace GameInputUtils
	{
		GameInputValueId addInputValueBinding(
			GameInputContext& a_gameInputCtx, GameInputBindingContext& a_inputBindingCtx, std::shared_ptr<AInputValueBinding> a_binding, float a_defaultValue = 0)
		{
			auto const id = a_gameInputCtx.registerValue(a_defaultValue);
			a_inputBindingCtx.values.emplace_back(id, std::move(a_binding));
			return id;
		}

		GameInputEventId addInputEventBinding(
			GameInputContext& a_gameInputCtx, GameInputBindingContext& a_inputBindingCtx, std::shared_ptr<AInputEventBinding> a_binding)
		{
			auto const id = a_gameInputCtx.registerEvent();
			a_inputBindingCtx.events.emplace_back(id, std::move(a_binding));
			return id;
		}
	}

}
