#include <vob/aoe/window/window_input_system.h>

#include <vob/aoe/input/inputs.h>


namespace vob::aoewi
{
	namespace
	{
		void process_event(key_event const& a_keyEvent, aoein::inputs& a_physicalInputs)
		{
			auto& keyState = a_physicalInputs.keyboard.keys[a_keyEvent.m_key];
			keyState.update_state(a_keyEvent.m_action != key_event::action::release);
		}

		void process_event(text_event const& a_textEvent, aoein::inputs& a_physicalInputs)
		{

		}

		void process_event(
			mouse_move_event const& a_mouseMoveEvent, aoein::inputs& a_physicalInputs)
		{
			a_physicalInputs.mouse.axes[aoein::mouse::axis::X].update_state(
				static_cast<float>(a_mouseMoveEvent.m_position.x));
			a_physicalInputs.mouse.axes[aoein::mouse::axis::Y].update_state(
				static_cast<float>(a_mouseMoveEvent.m_position.y));
		}

		void process_event(
			mouse_button_event const& a_mouseButtonEvent, aoein::inputs& a_physicalInputs)
		{
			auto& buttonState = a_physicalInputs.mouse.buttons[a_mouseButtonEvent.m_button];
			buttonState.update_state(a_mouseButtonEvent.m_pressed);
		}

		void process_event(
			mouse_scroll_event const& a_mouseScrollEvent, aoein::inputs& a_physicalInputs)
		{

		}

		void process_event(
			mouse_hover_event const& a_mouseHoverEvent, aoein::inputs& a_physicalInputs)
		{

		}

		void process_events(window_interface& a_window, aoein::inputs& a_physicalInputs)
		{
			for (auto const& polledEvent : a_window.get_polled_events())
			{
				std::visit([&a_physicalInputs](auto const& a_event) {
					process_event(a_event, a_physicalInputs);
				}, polledEvent);
			}
		}
	}

	window_input_system::window_input_system(aoecs::world_data_provider& a_wdp)
		: m_windowWorldComponent{ a_wdp }
		, m_inputs{ a_wdp }
		, m_stopManager{ a_wdp.get_stop_manager() }
	{}

	void window_input_system::update() const
	{
		auto& window = m_windowWorldComponent->m_window.get();
		m_inputs->reset_changed_states();

		process_events(window, *m_inputs);
	}
}
