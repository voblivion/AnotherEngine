#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/sta/memory.h>

namespace vob::aoe::sync
{
	class VOB_AOE_API ATask
	{
		enum class State
		{
			Done,
			Pending
		};
	public:
		// Constructors
		virtual ~ATask() = default;

		// Methods
		void addDependency(ATask& a_task);

		void preUpdate();

		void update();

		void waitDone();

	protected:
		// Methods
		virtual void doUpdate() = 0;

	private:
		// Attributes
		State m_state{ State::Done };
		std::mutex m_mutex;
		std::condition_variable m_sync;
		std::vector<std::reference_wrapper<ATask>> m_dependencies;

		// Methods
		void setState(State const a_state);
	};

	using TaskList = std::pmr::vector<sta::polymorphic_ptr<ATask>>;

	struct TaskDescription
	{
		std::size_t m_id;
		std::vector<std::size_t> m_dependencies;
	};
}