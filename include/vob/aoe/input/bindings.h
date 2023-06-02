#pragma once

#include <vob/aoe/input/binding_base.h>
#include <vob/aoe/input/inputs.h>

#include <vob/misc/std/id_map.h>


namespace vob::aoein
{
	template <typename TBindingBase>
	class input_bindings
	{
		using binding_ptr = std::shared_ptr<TBindingBase>;
		using binding_ptr_allocator = std::pmr::polymorphic_allocator<binding_ptr>;
		using binding_map = mistd::id_map<binding_ptr, binding_ptr_allocator>;

	public:
		using binding_id = typename binding_map::key_type;

		explicit input_bindings(binding_ptr_allocator const& a_allocator = {})
			: m_bindings{ a_allocator }
		{}

		TBindingBase const* find(binding_id const a_bindingId) const
		{
			auto const it = m_bindings.find(a_bindingId);
			return it != m_bindings.end() ? it->get() : nullptr;
		}

		TBindingBase const* operator[](binding_id const a_bindingId) const
		{
			return m_bindings[a_bindingId].get();
		}

		binding_id add(std::shared_ptr<TBindingBase> a_binding)
		{
			return m_bindings.emplace(std::move(a_binding));
		}

		void remove(binding_id const a_bindingId)
		{
			m_bindings.erase(a_bindingId);
		}

		void clear()
		{
			m_bindings.clear();
		}

		void update(inputs const& a_inputs, misph::measure_time const a_elapsedTime)
		{
			for (auto& binding : m_bindings)
			{
				binding->update(a_inputs, a_elapsedTime);
			}
		}

	private:
		binding_map m_bindings;
	};

	struct bindings
	{
		using axis_binding_allocator = std::pmr::polymorphic_allocator<axis_binding_base>;
		using axis_bindings = input_bindings<axis_binding_base>;
		using axis_id = typename axis_bindings::binding_id;
		using switch_binding_allocator = std::pmr::polymorphic_allocator<switch_binding_base>;
		using switch_bindings = input_bindings<switch_binding_base>;
		using switch_id = typename switch_bindings::binding_id;

		axis_bindings axes;
		switch_bindings switches;

		explicit bindings(std::pmr::polymorphic_allocator<void> const& a_allocator = {})
			: axes{ axis_binding_allocator{ a_allocator } }
			, switches{ switch_binding_allocator{ a_allocator } }
		{}

		void update(inputs const& a_inputs, misph::measure_time const a_elapsedTime)
		{
			axes.update(a_inputs, a_elapsedTime);
			switches.update(a_inputs, a_elapsedTime);
		}
	};
}
