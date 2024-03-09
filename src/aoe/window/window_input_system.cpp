#include <vob/aoe/window/window_input_system.h>

#include <vob/aoe/input/inputs.h>

#include <iostream>

namespace vob::aoewi
{
	namespace
	{
		void process_event(
			key_event const& a_keyEvent,
			aoewi::window_input_world_component const& inputWorldComponent,
			aoein::inputs& a_physicalInputs)
		{
			if (inputWorldComponent.m_shouldIgnoreKeyboardKeyEvents)
			{
				return;
			}

			auto& keyState = a_physicalInputs.keyboard.keys[a_keyEvent.m_key];
			keyState.update_state(a_keyEvent.m_action != key_event::action::release);
		}

		void process_event(
			text_event const& a_textEvent,
			aoewi::window_input_world_component const& inputWorldComponent,
			aoein::inputs& a_physicalInputs)
		{

		}

		void process_event(
			mouse_move_event const& a_mouseMoveEvent,
			aoewi::window_input_world_component const& inputWorldComponent,
			aoein::inputs& a_physicalInputs)
		{
			a_physicalInputs.mouse.axes[aoein::mouse::axis::X].update_state(
				static_cast<float>(a_mouseMoveEvent.m_position.x));
			a_physicalInputs.mouse.axes[aoein::mouse::axis::Y].update_state(
				static_cast<float>(a_mouseMoveEvent.m_position.y));
		}

		void process_event(
			mouse_button_event const& a_mouseButtonEvent,
			aoewi::window_input_world_component const& inputWorldComponent,
			aoein::inputs& a_physicalInputs)
		{
			if (inputWorldComponent.m_shouldIgnoreMouseButtonEvents)
			{
				return;
			}

			auto& buttonState = a_physicalInputs.mouse.buttons[a_mouseButtonEvent.m_button];
			buttonState.update_state(a_mouseButtonEvent.m_pressed);
		}

		void process_event(
			mouse_scroll_event const& a_mouseScrollEvent,
			aoewi::window_input_world_component const& inputWorldComponent,
			aoein::inputs& a_physicalInputs)
		{
			auto button = a_mouseScrollEvent.m_move.y > 0.0f
				? aoein::mouse::button::ScrollUp : aoein::mouse::button::ScrollDown;
			auto& buttonState = a_physicalInputs.mouse.buttons[button];
			buttonState.update_state(true);
		}

		void process_event(
			mouse_hover_event const& a_mouseHoverEvent,
			aoewi::window_input_world_component const& inputWorldComponent,
			aoein::inputs& a_physicalInputs)
		{

		}

		void process_events(
			window_interface& a_window,
			const aoewi::window_input_world_component& inputWorldComponent,
			aoein::inputs& a_physicalInputs)
		{
			for (auto const& polledEvent : a_window.get_polled_events())
			{
				std::visit([&inputWorldComponent, &a_physicalInputs](auto const& a_event) {
					process_event(a_event, inputWorldComponent, a_physicalInputs);
				}, polledEvent);
			}
		}
	}

	window_input_system::window_input_system(aoeng::world_data_provider& a_wdp)
		: m_windowWorldComponent{ a_wdp }
		, m_windowInputWorldComponent{ a_wdp }
		, m_inputs{ a_wdp }
		, m_shouldStop{ a_wdp }
	{}

	void window_input_system::update() const
	{
		auto& window = m_windowWorldComponent->m_window.get();
		m_inputs->reset_changed_states();

		process_events(window, *m_windowInputWorldComponent, *m_inputs);

		for (int g = 0; g < aoein::k_maxGamepadCount; ++g)
		{
			auto& gamepad = m_inputs->gamepads[g];

			const bool isGamepadPresent = window.is_gamepad_present(g);
			gamepad.is_connected.update_state(isGamepadPresent);
			if (isGamepadPresent)
			{
				for (auto button : gamepad.buttons.keys())
				{
					gamepad.buttons[button].update_state(window.is_gamepad_button_pressed(g, button));
				}
				for (auto axis : gamepad.axes.keys())
				{
					gamepad.axes[axis].update_state(window.get_gamepad_axis_value(g, axis));
				}
			}
		}
	}
}
