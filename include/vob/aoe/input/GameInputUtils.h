#pragma once

#include <vob/aoe/input/GameInputBindingContext.h>
#include <vob/aoe/input/GameInputContext.h>
#include <vob/aoe/input/InputBinding.h>


namespace vob::aoein
{
	namespace GameInputUtils
	{
		inline GameInputValueId addInputValueBinding(
			GameInputContext& a_gameInputCtx, GameInputBindingContext& a_inputBindingCtx, std::shared_ptr<AInputValueBinding> a_binding, float a_defaultValue = 0)
		{
			auto const id = a_gameInputCtx.registerValue(a_defaultValue);
			a_inputBindingCtx.mapping.values.emplace_back(id, std::move(a_binding));
			return id;
		}

		inline GameInputEventId addInputEventBinding(
			GameInputContext& a_gameInputCtx, GameInputBindingContext& a_inputBindingCtx, std::shared_ptr<AInputEventBinding> a_binding)
		{
			auto const id = a_gameInputCtx.registerEvent();
			a_inputBindingCtx.mapping.events.emplace_back(id, std::move(a_binding));
			return id;
		}

		inline void setInputMapping(
			GameInputContext& a_gameInputCtx, GameInputBindingContext& a_inputBindingCtx, InputMapping a_mapping)
		{
			for (auto const& [id, binding] : a_inputBindingCtx.mapping.values)
			{
				a_gameInputCtx.resetValue(id);
			}

			a_inputBindingCtx.mapping = std::move(a_mapping);
		}
	}

}
