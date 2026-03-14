#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>


namespace vob::aoexc
{
	class EventPool
	{
	public:
		template <typename TEvent>
		void addEvents(std::vector<TEvent> const& a_events)
		{
			auto eventListEntry = m_eventLists.find(typeid(TEvent));
			if (eventListEntry != m_eventLists.end())
			{
				eventListEntry->second->addEvents(&a_events);
				return;
			}

			m_eventLists.emplace(typeid(TEvent), std::make_shared<EventList<TEvent>>(std::move(a_events)));
		}

		template <typename TEvent>
		void pollEvents(std::vector<TEvent>& a_events) const
		{
			auto eventListEntry = m_eventLists.find(typeid(TEvent));
			if (eventListEntry != m_eventLists.end())
			{
				eventListEntry->second->pollEvents(&a_events);
			}
		}

		void merge(EventPool& a_eventPool)
		{
			for (auto& newEventListEntry : a_eventPool.m_eventLists)
			{
				auto eventListIt = m_eventLists.find(newEventListEntry.first);
				if (eventListIt != m_eventLists.end())
				{
					eventListIt->second->merge(*newEventListEntry.second);
				}
				else
				{
					m_eventLists.emplace(newEventListEntry.first, newEventListEntry.second->stealClone());
				}
			}
		}

	private:
		struct AEventList
		{
			virtual ~AEventList() = default;

			virtual void addEvents(void const* a_events) = 0;
			virtual void pollEvents(void* a_events) = 0;
			virtual void merge(AEventList& a_eventList) = 0;
			virtual std::shared_ptr<AEventList> stealClone() = 0;
		};

		template <typename TEvent>
		struct EventList : AEventList
		{
			explicit EventList(std::vector<TEvent> a_events)
				: m_events{ std::move(a_events) }
			{
			}

			void addEvents(void const* a_events) override
			{
				auto const& newEvents = *static_cast<std::vector<TEvent> const*>(a_events);
				m_events.insert_range(m_events.end(), newEvents);
			}

			void pollEvents(void* a_events) override
			{
				auto& newEvents = *static_cast<std::vector<TEvent>*>(a_events);
				std::swap(newEvents, m_events);
				m_events.clear();
			}

			void merge(AEventList& a_eventList) override
			{
				auto& newEvents = static_cast<EventList<TEvent>&>(a_eventList).m_events;
				m_events.insert_range(m_events.end(), newEvents);
				newEvents.clear();
			}

			std::shared_ptr<AEventList> stealClone() override
			{
				std::vector<TEvent> events;
				std::swap(events, m_events);
				return std::make_shared<EventList<TEvent>>(std::move(events));
			}

			std::vector<TEvent> m_events;
		};

		std::unordered_map<std::type_index, std::shared_ptr<AEventList>> m_eventLists;
	};

}
