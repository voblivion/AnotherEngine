#pragma once

#include <mutex>
#include <vector>

#include <aoe/core/Export.h>
#include <aoe/core/standard/ADynamicType.h>
#include <aoe/core/standard/Memory.h>

namespace aoe
{
	namespace sync
	{
		class AOE_CORE_API ATask
			: public sta::ADynamicType
		{
			enum class State
			{
				Done,
				Pending
			};
		public:
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

		using TaskList = std::pmr::vector<sta::PolymorphicPtr<ATask>>;

		struct TaskDescription
		{
			std::size_t m_id;
			std::vector<std::size_t> m_dependencies;
		};
	}
}