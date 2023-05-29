#include <vob/aoe/window/window_input_system.h>

#include <vob/aoe/input/keyboard.h>


namespace vob::aoewi
{
	namespace
	{
		void process_event(key_event const& a_keyEvent, aoein::physical_inputs& a_physicalInputs)
		{
			auto& keyState = a_physicalInputs.m_keyboard.m_keys[a_keyEvent.m_key];
			keyState.update(a_keyEvent.m_action != key_event::action::release);
		}

		void process_event(text_event const& a_textEvent, aoein::physical_inputs& a_physicalInputs)
		{

		}

		void process_event(
			mouse_move_event const& a_mouseMoveEvent, aoein::physical_inputs& a_physicalInputs)
		{
			a_physicalInputs.m_mouse.m_move +=
				glm::vec2{ a_mouseMoveEvent.m_position } - a_physicalInputs.m_mouse.m_position;
			a_physicalInputs.m_mouse.m_position = glm::vec2{ a_mouseMoveEvent.m_position };
		}

		void process_event(
			mouse_button_event const& a_mouseButtonEvent, aoein::physical_inputs& a_physicalInputs)
		{
			auto& buttonState = a_physicalInputs.m_mouse.m_buttons[a_mouseButtonEvent.m_button];
			buttonState.update(a_mouseButtonEvent.m_pressed);
		}

		void process_event(
			mouse_scroll_event const& a_mouseScrollEvent, aoein::physical_inputs& a_physicalInputs)
		{

		}

		void process_event(
			mouse_hover_event const& a_mouseHoverEvent, aoein::physical_inputs& a_physicalInputs)
		{

		}

		void process_events(window_interface& a_window, aoein::physical_inputs& a_physicalInputs)
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
		, m_physicalInputsWorldComponent{ a_wdp }
		, m_stopManager{ a_wdp.get_stop_manager() }
	{}

	void window_input_system::update() const
	{
		auto& window = m_windowWorldComponent->m_window.get();
		m_physicalInputsWorldComponent->m_inputs.update();

		process_events(window, m_physicalInputsWorldComponent->m_inputs);
	}
}
