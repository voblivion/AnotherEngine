#pragma once

#include <vob/aoe/input/Inputs.h>

#include <vob/misc/physics/measure.h>
#include <vob/misc/std/id_map.h>


namespace vob::aoein
{
	struct IInputBinding
	{
		virtual void update(Inputs const& a_inputsContext, misph::measure_time a_elapsedTime) = 0;
	};

	struct IAxisInputBinding : public IInputBinding
	{
		virtual float getChange() const = 0;
		virtual float getValue() const = 0;
	};

	struct ISwitchInputBinding : public IInputBinding
	{
		virtual bool hasChanged() const = 0;
		virtual bool isPressed() const = 0;

		bool wasPressed() const {
			return hasChanged() && isPressed();
		}
		bool wasReleased() const {
			return hasChanged() && !isPressed();
		}
	};

	template <typename TBindingBase>
	class BasicInputBindings
	{
	public:
		using BindingId = typename mistd::id_map<std::shared_ptr<TBindingBase>>::key_type;

		TBindingBase const* find(BindingId const a_bindingId) const
		{
			auto const it = m_bindings.find(a_bindingId);
			return it != m_bindings.end() ? it->get() : nullptr;
		}

		TBindingBase const& operator[](BindingId const a_bindingId) const
		{
			return *m_bindings[a_bindingId];
		}

		BindingId add(std::shared_ptr<TBindingBase> a_binding)
		{
			return m_bindings.emplace(std::move(a_binding));
		}

		void remove(BindingId const a_bindingId)
		{
			m_bindings.erase(a_bindingId);
		}

		void clear()
		{
			m_bindings.clear();
		}

		void update(Inputs const& a_inputsContext, misph::measure_time const a_elapsedTime)
		{
			for (auto& binding : m_bindings)
			{
				binding->update(a_inputsContext, a_elapsedTime);
			}
		}

	private:
		mistd::id_map<std::shared_ptr<TBindingBase>> m_bindings;
	};

	struct InputBindings
	{
		BasicInputBindings<IAxisInputBinding> axes;
		BasicInputBindings<ISwitchInputBinding> switches;

		using AxisId = typename decltype(axes)::BindingId;
		using SwitchId = typename decltype(switches)::BindingId;

		static constexpr AxisId kInvalidAxisId = -1;
		static constexpr SwitchId kInvalidSwitchId = -1;

		void update(Inputs const& a_inputs, misph::measure_time const a_elapsedTime)
		{
			axes.update(a_inputs, a_elapsedTime);
			switches.update(a_inputs, a_elapsedTime);
		}
	};
}
