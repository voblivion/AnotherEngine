#pragma once

#include <vob/aoe/engine/game.h>

#include "entt/entt.hpp"


namespace vob::aoeng
{
	using EcsWorldQuery = std::function<void(entt::registry&)>;

	class EcsWorldDataAccessRegistrar
	{
	private:
		friend class EcsWorld;

		explicit EcsWorldDataAccessRegistrar() = default;
	};

	class EcsWorldDataAccessProvider
	{
	public:
		explicit EcsWorldDataAccessProvider(entt::registry& a_registry, std::pmr::vector<EcsWorldQuery>& a_queryQueue, IGameController& a_gameController)
			: m_registry{ a_registry }
			, m_queryQueue{ a_queryQueue }
			, m_gameController{ a_gameController }
		{
		}

	private:
		template <typename TContext>
		friend class EcsWorldContextRef;

		template <typename... TComponents>
		friend class EcsWorldViewRef;

		friend class EcsWorldQueryQueueRef;

		friend class EcsWorldGameControllerRef;

		entt::registry& m_registry;
		std::pmr::vector<EcsWorldQuery>& m_queryQueue;
		IGameController& m_gameController;
	};

	template <typename TContext>
	class EcsWorldContextRef
	{
	public:
		void init([[maybe_unused]] EcsWorldDataAccessRegistrar& a_wdar) const
		{
			// TODO: notify wdar of resource used.
		}

		TContext& get(EcsWorldDataAccessProvider const& a_wdap) const
		{
			return a_wdap.m_registry.ctx().get<TContext>();
		}
	};

	template <typename... TComponents>
	class EcsWorldViewRef
	{
	public:
		void init([[maybe_unused]] EcsWorldDataAccessRegistrar& a_wdar) const
		{
			// TODO: notify wdar of resource used.
		}

		decltype(auto) get(EcsWorldDataAccessProvider const& a_wdap) const
		{
			return a_wdap.m_registry.view<TComponents...>();
		}

		template <typename... TExcludes>
		decltype(auto) get(EcsWorldDataAccessProvider const& a_wdap, entt::exclude_t<TExcludes...> a_excludes = {}) const
		{
			return a_wdap.m_registry.view<TComponents...>(a_excludes);
		}
	};

	class EcsWorldQueryQueueRef
	{
	public:
		void init([[maybe_unused]] EcsWorldDataAccessRegistrar& a_wdar) const
		{
			// TODO: notify wdar of resource used.
		}

		const std::pmr::vector<EcsWorldQuery>& get(EcsWorldDataAccessProvider const& a_wdap) const
		{
			return a_wdap.m_queryQueue;
		}
	};

	class EcsWorldGameControllerRef
	{
	public:
		void init([[maybe_unused]] EcsWorldDataAccessRegistrar& a_wdar) const
		{
			// TODO: notify wdar of resource used.
		}

		IGameController& get(EcsWorldDataAccessProvider const& a_wdap) const
		{
			return a_wdap.m_gameController;
		}
	};
}
