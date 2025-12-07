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
		void addEvents(std::vector<TEvent> a_events)
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
		void pollEvents(std::vector<TEvent>& a_events)
		{
			auto eventListEntry = m_eventLists.find(typeid(TEvent));
			if (eventListEntry != m_eventLists.end())
			{
				eventListEntry->second->pollEvents(&a_events);
			}
		}

		void merge(EventPool a_eventPool)
		{
			for (auto& newEventListEntry : a_eventPool.m_eventLists)
			{
				auto eventListEntry = m_eventLists.find(newEventListEntry.first);
				if (eventListEntry != m_eventLists.end())
				{
					eventListEntry->second->merge(*newEventListEntry.second);
					continue;
				}

				m_eventLists.emplace(newEventListEntry.first, std::move(newEventListEntry.second));
			}
		}

	private:
		struct AEventList
		{
			virtual ~AEventList() = default;

			virtual void addEvents(void* a_events) = 0;
			virtual void pollEvents(void* a_events) = 0;
			virtual void merge(AEventList& a_eventList) = 0;
		};

		template <typename TEvent>
		struct EventList : AEventList
		{
			explicit EventList(std::vector<TEvent> a_events)
				: m_events{ std::move(a_events) }
			{
			}

			void addEvents(void* a_events) override
			{
				auto& newEvents = *static_cast<std::vector<TEvent>*>(a_events);
				m_events.insert_range(m_events.end(), newEvents);
			}

			void merge(AEventList& a_eventList) override
			{
				auto& newEvents = static_cast<EventList<TEvent>&>(a_eventList).m_events;
				m_events.insert_range(m_events.end(), newEvents);
			}

			void pollEvents(void* a_events) override
			{
				auto& newEvents = *static_cast<std::vector<TEvent>*>(a_events);
				std::swap(newEvents, m_events);
			}

			std::vector<TEvent> m_events;
		};

		std::unordered_map<std::type_index, std::shared_ptr<AEventList>> m_eventLists;
	};

}
