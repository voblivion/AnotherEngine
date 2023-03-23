#pragma once

#include <array>
#include <iterator>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <iostream>

#include <vob/misc/physics/measure_literals.h>

using namespace vob::misph::literals;


namespace vob::aoepf
{
	template <
		typename TStartIt,
		typename TOpenPriorityQueue,
		typename TVisitedMap,
		typename TClosedSet,
		typename TNeighbors,
		typename TDistance,
		typename THeuristic,
		typename TEnd,
		typename TSolutionFrontInsertIt>
	bool a_star(
		TStartIt a_startBeginIt,
		TStartIt a_startEndIt,
		TOpenPriorityQueue a_openPriorityQueue,
		TVisitedMap a_visitedMap,
		TClosedSet a_closedSet,
		TNeighbors a_neighbors,
		TDistance a_distance,
		THeuristic a_heuristic,
		TEnd a_end,
		TSolutionFrontInsertIt a_solutionFrontInsertIt)
	{
		while (a_startBeginIt != a_startEndIt)
		{
			auto const startNode = a_startBeginIt->m_node;
			auto const startG = a_startBeginIt->m_g;
			auto const startH = a_heuristic(a_startBeginIt->m_node);
			auto const startF = startG + startH;

			a_visitedMap.emplace(startNode, std::nullopt, startG, startH);
			a_openPriorityQueue.emplace(startNode, startH, startF);
			++a_startBeginIt;
		}

		if (a_openPriorityQueue.empty())
		{
			return false;
		}

		while (!a_openPriorityQueue.empty())
		{
			auto current = a_openPriorityQueue.top();
			
			if (a_end(current.m_node))
			{
				// Set solution
				auto solutionNode = std::optional{ current.m_node };
				while (solutionNode)
				{
					a_solutionFrontInsertIt = *solutionNode;
					auto* visitedNode = a_visitedMap.find(*solutionNode);
					solutionNode = visitedNode->m_parent ? std::optional{ *(visitedNode->m_parent) } : std::nullopt;
				}
				return true;
			}

			a_openPriorityQueue.pop();
			a_closedSet.emplace(current.m_node);

			auto const neighbors = a_neighbors(current.m_node);
			if (neighbors.empty()) // DEBUG
			{
				bool d = true;
			}
			for (auto const& neighbor : neighbors)
			{
				if (a_closedSet.contains(neighbor))
				{
					continue;
				}

				auto const neighborG = current.m_f - current.m_h + a_distance(current.m_node, neighbor);

				auto* visitedNeighbor = a_visitedMap.find(neighbor);
				if (visitedNeighbor == nullptr)
				{
					auto const neighborH = a_heuristic(neighbor);

					a_visitedMap.emplace(neighbor, current.m_node, neighborG, neighborH);
					a_openPriorityQueue.emplace(neighbor, neighborH, neighborG + neighborH);
				}
				else if (visitedNeighbor->m_g < neighborG)
				{
					auto const neighborH = visitedNeighbor->m_h;

					visitedNeighbor->m_parent = current.m_node;
					visitedNeighbor->m_g = neighborG;
					a_openPriorityQueue.emplace(neighbor, neighborH, neighborG + neighborH);
				}
			}
		}

		auto solutionNode = std::optional{ a_end.m_bestState };
		while (solutionNode)
		{
			a_solutionFrontInsertIt = *solutionNode;
			auto* visitedNode = a_visitedMap.find(*solutionNode);
			solutionNode = visitedNode->m_parent ? std::optional{ *(visitedNode->m_parent) } : std::nullopt;
		}
		return false;
	}

	template <typename TNode, typename TScore>
	struct start_node
	{
		TNode m_node;
		TScore m_g = {};
	};

	template <typename TNode, typename TScore>
	struct visited_node
	{
		std::optional<TNode> m_parent;
		TScore m_g;
		TScore m_h;
	};

	template <typename TNode, typename TScore>
	struct visited_map
	{
		void emplace(TNode a_node, std::optional<TNode> a_parent, TScore a_g, TScore a_h)
		{
			m_unorderedMap.emplace(a_node, visited_node{ a_parent, a_g, a_h });
		}

		visited_node<TNode, TScore>* find(TNode const& a_node)
		{
			auto it = m_unorderedMap.find(a_node);
			return it == m_unorderedMap.end() ? nullptr : &(it->second);
		}

		std::unordered_map<TNode, visited_node<TNode, TScore>> m_unorderedMap;
	};

	template <typename TNode, typename TScore>
	struct empty_closed_set
	{
		void emplace(TNode a_node)
		{

		}

		bool contains(TNode const& a_node)
		{
			return false;
		}
	};

	template <typename TNode, typename TScore>
	struct open_node
	{
		TNode m_node;
		TScore m_h;
		TScore m_f;

		friend bool operator<(open_node const& a_lhs, open_node const& a_rhs)
		{
			return a_lhs.m_f > a_rhs.m_f;
		}
	};

	template <typename TNode, typename TScore>
	using open_priority_queue = std::priority_queue<open_node<TNode, TScore>>;

	template <typename TNode>
	using closed_set = std::unordered_set<TNode>;
}

namespace i1
{
	using activity_level = decltype(0_cal / 1_min);

	enum class location
	{
		bed,
		kitchen,
		field
	};

	struct action
	{
		location m_from;
		location m_to;
		vob::misph::measure_time m_duration = 0_min;
		activity_level m_energyRate = 0_cal / 1_min;
		activity_level m_harvestRate = 0_cal / 1_min;
		vob::misph::measure_scalar m_restRate{ 0.0f };
	};

	action k_rest{			location::bed,		location::bed,		5_min,	-1_cal / 1_min, 0_cal / 1_min,		vob::misph::measure_scalar{ 2.0f } };
	action k_restToEat{		location::bed,		location::kitchen,	20_min, -2_cal / 1_min, 0_cal / 1_min,		vob::misph::measure_scalar{ -1.0f } };
	action k_restToWork{	location::bed,		location::field,	35_min, -3_cal / 1_min, 0_cal / 1_min,		vob::misph::measure_scalar{ -1.0f } };
	action k_eat{			location::kitchen,	location::kitchen,	10_min, 8_cal / 1_min,	-10_cal / 1_min,	vob::misph::measure_scalar{ 0.0f } };
	action k_eatToRest{		location::kitchen,	location::bed,		15_min, -2_cal / 1_min, 0_cal / 1_min,		vob::misph::measure_scalar{ -1.0f } };
	action k_eatToWork{		location::kitchen,	location::field,	30_min, -3_cal / 1_min, 0_cal / 1_min,		vob::misph::measure_scalar{ -1.0f } };
	action k_work{			location::field,	location::field,	15_min, -3_cal / 1_min, 10_cal / 1_min,		vob::misph::measure_scalar{ -1.5f } };
	action k_workToRest{	location::field,	location::bed,		40_min, -3_cal / 1_min, 0_cal / 1_min,		vob::misph::measure_scalar{ -1.0f } };
	action k_workToEat{		location::field,	location::bed,		40_min, -3_cal / 1_min, 0_cal / 1_min,		vob::misph::measure_scalar{ -1.0f } };

	struct state
	{
		location m_location = location::bed;
		float m_h = 0.0f;
		vob::misph::measure_time m_timeOfDay = 0_min;
		vob::misph::measure_energy m_energy = 1000_cal;
		vob::misph::measure_energy m_harvest = 1000_cal;
		vob::misph::measure_time m_rest = 16_h;

		friend bool operator==(state const& a_lhs, state const& a_rhs)
		{
			return a_lhs.m_location == a_rhs.m_location
				&& a_lhs.m_timeOfDay == a_rhs.m_timeOfDay
				&& a_lhs.m_energy == a_rhs.m_energy
				&& a_lhs.m_harvest == a_rhs.m_harvest
				&& a_lhs.m_rest == a_rhs.m_rest;
		}
	};

	struct heuristic
	{
		state m_startState{};

		float operator()(state const& a_state)
		{
			auto const energy = a_state.m_energy - m_startState.m_energy;
			auto const harvest = a_state.m_harvest - m_startState.m_harvest;
			auto const rest = a_state.m_rest - m_startState.m_rest;

			auto const dEnergy = std::max(-energy / k_eat.m_energyRate, energy / k_work.m_energyRate);
			auto const dRest = std::max(-rest / k_rest.m_restRate, rest / k_work.m_restRate);
			auto const dHarvest = std::max(-harvest / k_work.m_harvestRate, 0_min);
			return std::max(dEnergy, std::max(dRest, dHarvest)).get_value();
		}
	};

	auto& operator+=(state& a_state, action const& a_action)
	{
		a_state.m_location = a_action.m_to;
		a_state.m_timeOfDay += a_action.m_duration;
		a_state.m_energy += a_action.m_energyRate * a_action.m_duration;
		a_state.m_harvest += a_action.m_harvestRate * a_action.m_duration;
		a_state.m_rest += a_action.m_restRate * a_action.m_duration;
		a_state.m_rest = std::max(a_state.m_rest, 24_h);
		a_state.m_h = heuristic{}(a_state);
		return a_state;
	}

	auto operator+(state const& a_state, action const& a_action)
	{
		state newState = a_state;
		newState += a_action;
		return newState;
	}

	auto operator-(state const& a_target, state const& a_source)
	{
		action newAction;
		newAction.m_from = a_source.m_location;
		newAction.m_to = a_target.m_location;
		newAction.m_duration = a_target.m_timeOfDay - a_source.m_timeOfDay;
		newAction.m_energyRate = (a_target.m_energy - a_source.m_energy) / newAction.m_duration;
		newAction.m_harvestRate = (a_target.m_harvest - a_source.m_harvest) / newAction.m_duration;
		newAction.m_restRate = (a_target.m_rest - a_source.m_rest) / newAction.m_duration;
		return newAction;
	}

	struct neighbors
	{
		std::vector<action> m_actions{
			k_rest, k_restToEat, k_restToWork,
			k_eat, k_eatToRest, k_eatToWork,
			k_work, k_workToRest, k_workToEat };

		std::vector<state> operator()(state const& a_state)
		{
			std::vector<state> neighbors;
			neighbors.reserve(m_actions.size());
			for (auto const& a : m_actions)
			{
				if (a_state.m_location != a.m_from)
				{
					continue;
				}
				if (a_state.m_energy + a.m_energyRate * a.m_duration < 0_cal)
				{
					continue;
				}
				if (a_state.m_harvest + a.m_harvestRate * a.m_duration < 0_cal)
				{
					continue;
				}
				if (a_state.m_rest + a.m_restRate * a.m_duration < 0_min)
				{
					continue;
				}
				if (a_state.m_timeOfDay + a.m_duration > 24_h)
				{
					continue;
				}
				neighbors.push_back(a_state + a);
			}

			return neighbors;
		}
	};

	struct distance
	{
		float operator()(state const& a_source, state const& a_target)
		{
			return (a_target - a_source).m_duration.get_value();
		}
	};

	struct end
	{
		state m_startState{};
		vob::misph::measure_energy m_desiredHarvest = 0_cal;

		std::size_t m_maxEvalCount = 2'000'000;
		std::size_t m_evalCount = 0;

		state m_bestState;

		bool operator()(state const& a_state)
		{
			if (m_evalCount == 0 || m_bestState.m_timeOfDay < a_state.m_timeOfDay ||
				(std::abs(m_bestState.m_timeOfDay - a_state.m_timeOfDay) < 5_min
					&& heuristic{}(m_bestState) > heuristic{}(a_state)))
			{
				m_bestState = a_state;
			}

			if (m_maxEvalCount == ++m_evalCount)
			{
				return true;
			}

			if (a_state.m_timeOfDay < 24_h - 1_min)
			{
				return false;
			}

			if (a_state.m_location != state{}.m_location)
			{
				return false;
			}

			if (a_state.m_timeOfDay < 24_h - 1_min)
			{
				return false;
			}

			if (a_state.m_harvest - state{}.m_harvest < m_desiredHarvest)
			{
				return false;
			}

			if (a_state.m_energy < state{}.m_energy * 0.99f)
			{
				return false;
			}

			if (a_state.m_rest < state{}.m_rest * 0.99f)
			{
				return false;
			}

			return true;
		}
	};
}

namespace std
{
	template <>
	struct hash<i1::state>
	{
		std::size_t operator()(i1::state const& a_state) const
		{
			auto i = static_cast<std::uint32_t>(a_state.m_energy.get_value());
			i ^= static_cast<std::uint32_t>(a_state.m_harvest.get_value());
			i ^= static_cast<std::uint32_t>(a_state.m_rest.get_value());
			i ^= static_cast<std::uint32_t>(a_state.m_timeOfDay.get_value());
			return i;
		}
	};
}

#include <vob/misc/std/enum_map.h>

namespace i2
{
	struct location
	{
		enum e
		{
			bed,
			kitchen,
			field
		};
	};

	struct enum_indices
	{
		enum
		{
			location = 0,
			count
		};
	};

	struct number_indices
	{
		enum
		{
			restfullness = 0,
			energy,
			wealth,
			stamina,
			count
		};
	};

	using enum_values = location::e;
	using enum_opt_values = std::optional<location::e>;
	using number_values = std::array<int, number_indices::count>;
	using number_f_values = std::array<float, number_indices::count>;
	using number_opt_values = std::array<std::optional<int>, number_indices::count>;
	
	struct state
	{
		float m_h = 0.0f;
		enum_values m_enums{ location::bed };
		int m_timeOfDay = 0;
		number_values m_numbers{ 1200 /* 20h */, 480 /* 12h * 2cal */, 5760 /* 48h * 2cal */, 300 /* 5h * 2 */};

		friend bool operator==(state const& a_lhs, state const& a_rhs)
		{
			return a_lhs.m_enums == a_rhs.m_enums && a_lhs.m_numbers == a_rhs.m_numbers;
		}

		auto& operator=(state const& a_rhs)
		{
			m_h = a_rhs.m_h;
			m_enums = a_rhs.m_enums;
			m_timeOfDay = a_rhs.m_timeOfDay;
			m_numbers = a_rhs.m_numbers;
			return *this;
		}
	};

	constexpr number_values k_goalEnums = {};

	struct heuristic
	{
		number_f_values m_moreDecays{ 1.5, 3, 0, 0 };
		number_values m_moreCosts{ -15, -15, -5, 0 };
		number_f_values m_lessDecays{ 1.5, 3, 0, 0 };
		number_values m_lessCosts{ -15, -15, +1, -1 };

		float operator()(state const& a_state)
		{
			auto const startingState = state{};

			float cost = 0.0f;

			auto remainingTime = 60 * 24 - a_state.m_timeOfDay;
			for (auto i = 0ull; i < startingState.m_numbers.size(); ++i)
			{
				auto const diff = a_state.m_numbers[i] - startingState.m_numbers[i];
				if (diff < 0)
				{
					cost -= std::max(0.0f, -diff - m_moreDecays[i] * remainingTime) * m_moreCosts[i];
				}
				else
				{
					cost -= std::max(0.0f, diff + m_lessDecays[i] * remainingTime) * m_lessCosts[i];
				}
			}

			return cost;
		}
	};

	struct distance
	{
		int m_timeDist = 1;

		float operator()(state const& a_source, state const& a_target)
		{
			return 1.0f * ((a_target.m_timeOfDay - a_source.m_timeOfDay) * m_timeDist);
		}
	};
	
	struct action
	{
		enum_opt_values m_enumFroms{};
		enum_opt_values m_enumTos{};
		int m_duration = 30;
		number_values m_numberChanges{};
	};

	number_opt_values k_numberMaxs{ 2 * 60 * 48, 2 * 60 * 48, std::nullopt, 300 };
	auto& operator+=(state& a_state, action const& a_action)
	{
		a_state.m_enums = a_action.m_enumTos.value_or(a_state.m_enums);
		/*for (auto i = 0ull; i < a_action.m_enumTos.size(); ++i)
		{
			a_state.m_enums[i] = a_action.m_enumTos[i].value_or(a_state.m_enums[i]);
		}*/

		for (auto i = 0ull; i < a_action.m_numberChanges.size(); ++i)
		{
			a_state.m_numbers[i] += a_action.m_numberChanges[i] * a_action.m_duration;

			a_state.m_numbers[i] = std::min(a_state.m_numbers[i], k_numberMaxs[i].value_or(a_state.m_numbers[i]));
		}

		a_state.m_timeOfDay += a_action.m_duration;

		a_state.m_h = heuristic{}(a_state);
		return a_state;
	}

	auto operator+(state const& a_state, action const& a_action)
	{
		state newState = a_state;
		newState += a_action;
		return newState;
	}

	action k_rest{
		enum_opt_values{location::bed}, enum_opt_values{location::bed}, 30, number_values{ 4, -4, 0, +5 } };
	action k_restToEat{
		enum_opt_values{location::bed}, enum_opt_values{location::kitchen}, 30, number_values{ -2, -4, 0, -1 } };
	action k_restToWork{
		enum_opt_values{location::bed}, enum_opt_values{location::field}, 30, number_values{ -2, -4, 0, -1 } };
	action k_eat{
		enum_opt_values{location::kitchen}, enum_opt_values{location::kitchen}, 30, number_values{ -2, 16, -20, +5 } };
	action k_eatToRest{
		enum_opt_values{location::kitchen}, enum_opt_values{location::bed}, 30, number_values{ -2, -4, 0, -1 } };
	action k_eatToWork{
		enum_opt_values{location::kitchen}, enum_opt_values{location::field}, 30, number_values{ -2, -4, 0, -1 } };
	action k_work{
		enum_opt_values{location::field}, enum_opt_values{location::field}, 30, number_values{ -2, -4, 30, -2 } };
	action k_workToRest{
		enum_opt_values{location::field}, enum_opt_values{location::bed}, 30, number_values{ -2, -4, 0, -1 } };
	action k_workToEat{
		enum_opt_values{location::field}, enum_opt_values{location::bed}, 30, number_values{ -2, -4, 0, -1 } };

	struct neighbors
	{
		std::vector<action> m_actions{ k_rest, k_restToEat, k_restToWork, k_eat, k_eatToRest, k_eatToWork, k_work, k_workToRest, k_workToRest };

		number_opt_values m_numberMins{ 0, 0, 0, 0 };
		number_opt_values m_numberMaxs{ 2 * 60 * 48, 2 * 60 * 48, std::nullopt, std::nullopt };

		std::vector<state> operator()(state const& a_state) const
		{
			std::vector<state> neighborStates;
			neighborStates.reserve(m_actions.size());

			for (auto const& action : m_actions)
			{
				if (a_state.m_timeOfDay + action.m_duration > 60 * 24)
				{
					continue;
				}

				if (action.m_enumFroms.value_or(a_state.m_enums) != a_state.m_enums)
				{
					continue;
				}
				/*auto matchEnumRequirements = true;
				for (auto i = 0ull; i < action.m_enumFroms.size(); ++i)
				{
					if (action.m_enumFroms[i].value_or(a_state.m_enums[i]) != a_state.m_enums[i])
					{
						matchEnumRequirements = false;
						break;
					}
				}
				if (!matchEnumRequirements)
				{
					continue;
				}*/

				auto const neighborState = a_state + action;
				bool matchNumberRequirements = true;
				for (auto i = 0ull; i < action.m_numberChanges.size(); ++i)
				{
					if (neighborState.m_numbers[i] < m_numberMins[i].value_or(neighborState.m_numbers[i]))
					{
						matchNumberRequirements = false;
						break;
					}

					if (neighborState.m_numbers[i] > m_numberMaxs[i].value_or(neighborState.m_numbers[i]))
					{
						matchNumberRequirements = false;
						break;
					}
				}
				if (!matchNumberRequirements)
				{
					continue;
				}

				neighborStates.push_back(neighborState);
			}

			return neighborStates;
		}
	};

	struct end
	{
		std::size_t m_maxEvalCount = 2'000'000;
		std::size_t m_evalCount = 0;

		state m_bestState{};

		bool operator()(state const& a_state)
		{
			if (m_evalCount == 0 || m_bestState.m_timeOfDay < a_state.m_timeOfDay ||
				(std::abs(m_bestState.m_timeOfDay - a_state.m_timeOfDay) == 0
					&& heuristic{}(m_bestState) > heuristic{}(a_state)))
			{
				m_bestState = a_state;
			}

			if (m_maxEvalCount == ++m_evalCount)
			{
				return true;
			}

			return a_state.m_enums == state{}.m_enums && a_state.m_timeOfDay == 60 * 24;
		}
	};
}

namespace std
{
	template <>
	struct hash<i2::state>
	{
		std::size_t operator()(i2::state const& a_state) const
		{
			std::size_t i = 0;
			for (auto const& number : a_state.m_numbers)
			{
				i ^= number;
			}
			i ^= static_cast<std::size_t>(a_state.m_enums);
			/*for (auto const& e : a_state.m_enums)
			{
				i ^= e;
			}*/
			i ^= a_state.m_timeOfDay;
			return i;
		}
	};
}

#include <variant>

namespace i3
{
	using namespace vob::misph;

	template<typename T>
	bool equals(
		T const& lhs,
		T const& rhs,
		T const& epsilon = std::numeric_limits<T>::epsilon())
	{
		return std::abs(lhs - rhs) < epsilon;
	}

	enum class action_type
	{
		none,
		sleep,
		eat,
		work
	};

	struct state
	{
		static constexpr auto k_startingEnergy = 2_cal * 24 * 60;
		static constexpr auto k_maxEnergy = 3 * 2_cal * 24 * 60;
		static constexpr auto k_startingRest = 16_h;
		static constexpr auto k_maxRest = 32_h;
		static constexpr auto k_startingStash = 2_cal * 2 * 24 * 60;

		enum class location
		{
			bed,
			kitchen,
			field
		};

		action_type m_previousAction = action_type::none;

		location m_location = location::bed;

		measure_time m_timeOfDay = 0_h;

		measure_energy m_energy = k_startingEnergy;
		measure_time m_rest = k_startingRest;
		measure_energy m_stash = k_startingStash;
	};

	struct heuristic
	{

		float operator()(state const& a_state)
		{
			auto const startingState = state{};

			float cost = 0.0f;

			auto remainingTime = 24_h - a_state.m_timeOfDay;

			return cost;
		}
	};

	struct action_base
	{
		static constexpr auto k_baseEnergyUsed = 1_cal / 1_min;
		static constexpr auto k_activityEnergyUsed = 1_cal / 1_min;
		static constexpr auto k_travelActivityLevel = 0.5f;

		state apply(state const& a_source, measure_time a_duration) const
		{
			state target = a_source;
			target.m_previousAction = get_type();
			target.m_timeOfDay += a_duration;

			a_duration = apply_travel_logic(a_source, target, a_duration);

			apply_activity_logic(a_source, target, a_duration);
			apply_special_logic(a_source, target, a_duration);

			return target;
		}

		virtual action_type get_type() const = 0;

		decltype(auto) calculate_energy_use_rate() const
		{
			return k_baseEnergyUsed + k_activityEnergyUsed * get_activity_level();
		}

		void apply_activity_logic(
			state const& a_source, state& a_target, measure_time a_duration) const
		{
			a_target.m_energy -= calculate_energy_use_rate() * a_duration;
		}

		measure_time apply_travel_logic(
			state const& a_source, state& a_target, measure_time a_duration) const
		{
			a_target.m_location = get_location();
			if (a_target.m_location != a_source.m_location)
			{
				auto const travelDuration = calculate_travel_duration(a_source);
				a_duration -= travelDuration;
				auto const travelEnergyUsed =
					k_baseEnergyUsed + k_activityEnergyUsed * k_travelActivityLevel;
				a_target.m_energy -= travelEnergyUsed * travelDuration;
			}

			return a_duration;
		}

		virtual state::location get_location() const = 0;

		virtual measure_time calculate_travel_duration(state const& a_source) const = 0;

		virtual measure_scalar get_activity_level() const = 0;

		virtual void apply_special_logic(
			state const& a_source, state& a_target, measure_time a_duration) const = 0;
	};

	struct sleep_action : public action_base
	{
		static constexpr auto k_nightStart = 12_h;
		static constexpr auto k_daySleepRatio = 1.8f;
		static constexpr auto k_nightSleepRatio = 2.2f;

		virtual action_type get_type() const final
		{
			return action_type::sleep;
		}

		virtual state::location get_location() const final
		{
			return state::location::bed;
		}

		virtual measure_time calculate_travel_duration(state const& a_source) const final
		{
			return a_source.m_location == state::location::field ? 30_min : 15_min;
		}

		virtual measure_scalar get_activity_level() const final
		{
			return 0.0f;
		}

		virtual void apply_special_logic(
			state const& a_source, state& a_target, measure_time a_duration) const final
		{
			auto const daySleepDuration =
				std::max(a_duration, k_nightStart - a_source.m_timeOfDay);
			auto const nightSleepDuration = a_duration - daySleepDuration;

			a_target.m_rest +=
				daySleepDuration * k_daySleepRatio + nightSleepDuration * k_nightSleepRatio;
		}
	};

	struct eat_action : public action_base
	{
		static constexpr auto k_energyIngestionRate = 10_cal / 1_min;
		static constexpr auto k_stashUseRate = 12_cal / 1_min;

		virtual action_type get_type() const final
		{
			return action_type::eat;
		}

		virtual measure_time calculate_travel_duration(state const& a_source) const final
		{
			return a_source.m_location == state::location::bed ? 15_min : 30_min;
		}

		virtual state::location get_location() const final
		{
			return state::location::kitchen;
		}

		virtual measure_scalar get_activity_level() const final
		{
			return 0.0f;
		}

		virtual void apply_special_logic(
			state const& a_source, state& a_target, measure_time a_duration) const final
		{
			auto const availableEnergyRoom = state::k_maxEnergy - a_target.m_energy;
			auto const energyIngested =
				std::min(availableEnergyRoom, k_energyIngestionRate * a_duration);


			a_target.m_energy += energyIngested;
			a_target.m_stash -= energyIngested / k_energyIngestionRate * k_stashUseRate;
		}
	};

	struct work_action : public action_base
	{
		static constexpr auto k_nightStart = 12_h;
		static constexpr auto k_dayProductionRate = 20_cal / 1_min;
		static constexpr auto k_nightProductionRate = 10_cal / 1_min;

		virtual action_type get_type() const final
		{
			return action_type::work;
		}

		virtual measure_time calculate_travel_duration(state const& a_source) const final
		{
			return a_source.m_location == state::location::bed ? 15_min : 30_min;
		}

		virtual state::location get_location() const final
		{
			return state::location::kitchen;
		}

		virtual measure_scalar get_activity_level() const final
		{
			return 1.5f;
		}

		virtual void apply_special_logic(
			state const& a_source, state& a_target, measure_time a_duration) const final
		{
			auto const dayProdDuration =
				std::max(a_duration, k_nightStart - a_source.m_timeOfDay);
			auto const nightProdDuration = a_duration - dayProdDuration;

			auto const energyUseRate = calculate_energy_use_rate();
			auto const startEnergy = a_source.m_energy;
			auto const stopEnergy = startEnergy - energyUseRate * a_duration;
			auto const workEnergy = startEnergy > state::k_startingEnergy
				? startEnergy : (startEnergy + stopEnergy) * 0.5f;
			auto const hungerRate =
				std::abs(workEnergy - state::k_startingEnergy) / state::k_startingEnergy;
			auto const workRate = 1.0f - hungerRate.get_value();

			auto const baseProd =
				dayProdDuration * k_dayProductionRate + nightProdDuration * k_nightProductionRate;

			a_target.m_stash += baseProd * workRate;
		}
	};

	struct action
	{
		measure_time m_duration;
		std::shared_ptr<action_base> m_action;
	};

	auto& operator+=(state& a_state, action const& a_action)
	{
		a_state = a_action.m_action->apply(a_state, a_action.m_duration);
		return a_state;
	}

	auto operator+(state const& a_state, action const& a_action)
	{
		state newState = a_state;
		newState += a_action;
		return newState;
	}
}

#include <glm/glm.hpp>

namespace vob::aoepf
{
	void test()
	{
		start_node<i2::state, float> startState{};
		open_priority_queue<i2::state, float> openPriorityQueue;
		visited_map<i2::state, float> visitedMap;
		empty_closed_set<i2::state, float> closedSet;

		std::vector<i2::state> solution;

		a_star(
			&startState,
			(&startState) + 1,
			openPriorityQueue,
			visitedMap,
			closedSet,
			i2::neighbors{},
			i2::distance{},
			i2::heuristic{},
			i2::end{},
			std::back_insert_iterator(solution));

		i2::heuristic{}(solution[0]);
	}
}
