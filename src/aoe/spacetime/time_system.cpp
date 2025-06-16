#include <vob/aoe/spacetime/time_system.h>

namespace vob::aoest
{
	void time_system::update() const
	{
		auto const currentTime = std::chrono::high_resolution_clock::now();

		auto& tickStartTime = m_presentationTimeContext->tick_start_time;
		auto const elapsedTime = currentTime - tickStartTime;

		m_presentationTimeContext->elapsed_time = elapsedTime;
		m_presentationTimeContext->tick_start_time = currentTime;

		if (m_simulationTimeContext->play_for_duration < 0.0_s)
		{
			m_simulationTimeContext->elapsed_time = elapsedTime;
			m_simulationTimeContext->tick_start_time += elapsedTime;
		}
		else if (m_simulationTimeContext->play_for_duration > 0.0_s)
		{
			m_simulationTimeContext->elapsed_time = std::min(
				m_simulationTimeContext->play_for_duration,
				m_presentationTimeContext->elapsed_time);

			m_simulationTimeContext->tick_start_time += m_simulationTimeContext->elapsed_time;

			m_simulationTimeContext->play_for_duration -= m_simulationTimeContext->elapsed_time;
		}
		else
		{
			m_simulationTimeContext->elapsed_time = 0_s;
		}
	}
}
