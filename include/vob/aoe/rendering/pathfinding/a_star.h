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
		location m_from = location::bed;
		location m_to = location::bed;
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

#include <vob/misc/std/enum_map.h>

namespace i4
{
	enum class vitale
	{
		stash,
		energy,
		rest,
		count
	};

	enum class enumerate
	{
		location,
		count
	};

	struct state
	{
		// values to maintain in 0..1
		vob::mistd::enum_map<vitale, float> m_vitales{ 0.0f };
		
		// discrete states
		std::vector<int> m_enumerates;
	};

	void end()
	{
		// should be ok to end with vitales within ] init ; init + prod * 1h ]
	}

	float score(state const& a_state)
	{
		vob::mistd::enum_map<vitale, float> factors = { 1.0f, 0.0f, 0.0f };

		float total = 0.0f;
		for (auto i = a_state.m_vitales.begin_index; i < a_state.m_vitales.end_index; ++i)
		{
			total += a_state.m_vitales[i] * factors[i];
		}
		return -total;
	}

	void heuristic()
	{
		// usefull time = remaining time - restore time
		// extra score = - usefull time * production rate
		//
	}

	static constexpr vob::mistd::enum_map<vitale, std::optional<float>> k_maxVitales = { std::nullopt, 1.0f, 1.0f };

	struct action
	{
		vob::mistd::enum_map<vitale, float> m_vitaleChanges;
		vob::mistd::enum_map<enumerate, std::optional<int>> m_fromEnumerates;
		vob::mistd::enum_map<enumerate, std::optional<int>> m_toEnumerates;
	};

	void test()
	{
		glm::mat<4, 4, float> matrix = glm::mat4{ 1.0f };
		glm::inverse(matrix);
		i4::state s;
		s.m_vitales = { 1.0f, 0.75f, 0.75f };

		for (auto i = i4::k_maxVitales.begin_index; i < i4::k_maxVitales.end_index; ++i)
		{
			s.m_vitales[i] = std::min(s.m_vitales[i], i4::k_maxVitales[i].value_or(s.m_vitales[i]));
		}

		float t = i4::score(s);
	}
};

namespace i5
{
	template <size_t t_resourceCount>
	struct solver
	{
		using vec = glm::vec<t_resourceCount, float>;
		using mat = glm::mat<t_resourceCount, t_resourceCount, float>;

		int32_t m_increment = 1;
		int32_t m_maxTime = 48;
		vec m_maxResources = {};
		vec m_startResources = {};
		vec m_endResources = {};
		vec m_locomotion = {};
		vec m_goal = {};

		union
		{
			mat m_actions[2] = {};
			struct
			{
				mat m_productiveActions;
				mat m_locomotiveActions;
			};
		};

		mat m_reverseProductiveActions = {};
	};

	template <size_t t_resourceCount>
	struct state
	{
		using vec = glm::vec<t_resourceCount, float>;

		solver<t_resourceCount> const* m_solver;
		vec m_resources{ 100.0f };
		int32_t m_location = 0;
		int32_t m_time = 0;
		
		friend bool operator==(state const& lhs, state const& rhs)
		{
			return lhs.m_time == rhs.m_time && lhs.m_location == rhs.m_location && lhs.m_resources == rhs.m_resources;
		}
	};

	template <size_t t_resourceCount>
	struct action
	{
		int32_t m_index;
	};

	template <size_t t_resourceCount>
	struct execute
	{
		using state = state<t_resourceCount>;
		using action = action<t_resourceCount>;

		state operator()(state const& a_state, action const& a_action) const
		{
			state cpy = a_state;
			if (a_state.m_location == a_action.m_index)
			{
				cpy.m_resources += cpy.m_solver->m_productiveActions[a_action.m_index] * float(cpy.m_solver->m_increment);
			}
			else
			{
				cpy.m_resources += cpy.m_solver->m_locomotiveActions[a_action.m_index][a_state.m_location] * cpy.m_solver->m_locomotion;
			}

			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				if (cpy.m_solver->m_maxResources[i] > 0)
				{
					cpy.m_resources[i] = std::min(cpy.m_resources[i], cpy.m_solver->m_maxResources[i]);
				}
			}

			cpy.m_location = a_action.m_index;

			cpy.m_time += cpy.m_solver->m_increment;

			return cpy;
		}
	};

	template <size_t t_resourceCount>
	state<t_resourceCount> operator+(state<t_resourceCount> const& a_state, action<t_resourceCount> const& a_action)
	{
		return execute<t_resourceCount>{}(a_state, a_action);
	}

	template <size_t t_resourceCount>
	bool is_valid(state<t_resourceCount> const& a_state)
	{
		if (a_state.m_time > a_state.m_solver->m_maxTime)
		{
			return false;
		}

		for (int32_t i = 0; i < t_resourceCount; ++i)
		{
			if (a_state.m_resources[i] <= 0.0f)
			{
				return false;
			}
		}
		return true;
	}

	template <size_t t_resourceCount>
	struct neighbors
	{
		using state = state<t_resourceCount>;
		using action = action<t_resourceCount>;

		std::vector<state> operator()(state const& a_state) const
		{
			std::vector<state> res;
			res.reserve(t_resourceCount);

			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				state cpy = a_state + action{ i };
				if (is_valid(cpy))
				{
					res.push_back(cpy);
				}
			}

			return res;
		}
	};

	template <size_t t_resourceCount>
	glm::vec<t_resourceCount, float> const catchup_times(state<t_resourceCount> const& a_state)
	{
		using state = state<t_resourceCount>;
		using action = action<t_resourceCount>;

		auto missingResources = a_state.m_solver->m_endResources - a_state.m_resources;
		for (int32_t i = 0; i < t_resourceCount; ++i)
		{
			if (a_state.m_solver->m_goal[i] > 0.0f)
			{
				missingResources[i] = 0.0f;
			}
		}

		auto const catchupTimes = a_state.m_solver->m_reverseProductiveActions * missingResources;

		return catchupTimes;
	}

	template <size_t t_resourceCount>
	float sum_values(glm::vec<t_resourceCount, float> const& values)
	{
		float sum = 0.0f;
		for (int32_t i = 0; i < t_resourceCount; ++i)
		{
			sum += values[i];
		}
		return sum;
	}

	template <size_t t_resourceCount>
	struct heuristic
	{
		using state = state<t_resourceCount>;
		using action = action<t_resourceCount>;

		float operator()(state const& a_state) const
		{
			auto const catchupTimes = catchup_times(a_state);
			auto newResources = a_state.m_resources + a_state.m_solver->m_productiveActions * catchupTimes;

			auto freeTime = float(a_state.m_solver->m_maxTime - a_state.m_time);
			freeTime -= sum_values(catchupTimes);
			if (freeTime < 0.0f)
			{
				return 0.0f;
			}

			std::vector<std::pair<int32_t, float>> goalRatios;
			goalRatios.reserve(t_resourceCount);
			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				if (a_state.m_solver->m_goal[i] > 0)
				{
					goalRatios.emplace_back(i, newResources[i] / a_state.m_solver->m_goal[i]);
				}
			}
			std::sort(goalRatios.begin(), goalRatios.end(), [](auto const& lhs, auto const& rhs) {
				return lhs.second < rhs.second;
				});
			auto currentEndIt = goalRatios.begin();
			while (goalRatios.front().second < goalRatios.back().second && freeTime > 0.0f)
			{
				currentEndIt = std::upper_bound(currentEndIt + 1, goalRatios.end(), goalRatios.front().second,
					[](auto const lhs, auto const rhs) {
						return lhs <= rhs.second;
					});

				auto const neededRatio = currentEndIt->second - goalRatios.begin()->second;

				glm::vec<t_resourceCount, float> neededRes{ 0.0f };
				for (auto it = goalRatios.begin(); it != currentEndIt; ++it)
				{
					neededRes[it->first] = a_state.m_solver->m_goal[it->first];
				}
				neededRes *= neededRatio;

				auto const neededTimes = a_state.m_solver->m_reverseProductiveActions * neededRes;
				auto const neededTime = sum_values(neededTimes);
				auto const spentTime = std::min(freeTime, neededTime);
				auto const spentTimes = neededTimes * spentTime / neededTime;
				for (auto it = goalRatios.begin(); it != currentEndIt; ++it)
				{
					it->second += neededRatio * spentTime / neededTime;
				}
				freeTime -= spentTime;
			}

			auto lowestRatio = goalRatios.front().second;
			if (freeTime > 0.0f)
			{
				auto const bonusTimes = a_state.m_solver->m_reverseProductiveActions * a_state.m_solver->m_goal;
				auto const bonusTime = sum_values(bonusTimes);
				lowestRatio += freeTime / bonusTime;
			}

			return -lowestRatio;
		}
	};

	template <size_t t_resourceCount>
	struct end
	{
		using state = state<t_resourceCount>;
		using action = action<t_resourceCount>;

		int32_t m_bestTime = std::numeric_limits<int32_t>::max();
		float m_bestScore = 0.0f;

		state m_bestState;

		bool operator()(state const& a_state)
		{
			if (m_bestTime == std::numeric_limits<int32_t>::max() || m_bestTime < a_state.m_time)
			{
				m_bestScore = heuristic<t_resourceCount>{}(a_state);
				m_bestTime = a_state.m_time;
			}
			else if (m_bestTime == a_state.m_time)
			{
				auto const score = heuristic<t_resourceCount>{}(a_state);
				if (score < m_bestScore)
				{
					m_bestState = a_state;
					m_bestScore = score;
				}
			}

			if (a_state.m_time != a_state.m_solver->m_maxTime)
			{
				return false;
			}

			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				if (a_state.m_solver->m_goal[i] > 0.0f)
				{
					continue;
				}

				if (a_state.m_resources[i] < a_state.m_solver->m_startResources[i])
				{
					return false;
				}

				if (a_state.m_resources[i] >= a_state.m_solver->m_endResources[i])
				{
					return false;
				}
			}

			return true;
		}
	};

	template <size_t t_resourceCount>
	struct distance
	{
		float operator()(state<t_resourceCount> const& lhs, state<t_resourceCount> const& rhs) const
		{
			float dist = 0.0f;
			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				if (lhs.m_solver->m_goal[i] > 0.0f)
				{
					dist += rhs.m_resources[i] - lhs.m_resources[i];
				}
			}
			return dist;
		}
	};

	template <size_t t_resourceCount>
	struct hash
	{
		size_t operator()(vob::aoepf::open_node<state<3>, float> const& value) const
		{
			size_t res = 0;
			res += value.m_node.m_time << 48;
			res += value.m_node.m_location << 32;

			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				res = (~res) | *reinterpret_cast<const int32_t*>(&value.m_node.m_resources[i]);
			}

			return res;
		}
	};

	template <size_t t_resourceCount>
	struct equal_to
	{
		bool operator()(vob::aoepf::open_node<state<3>, float> const& lhs, vob::aoepf::open_node<state<3>, float> const& rhs)
		{
			return lhs.m_node.m_location == rhs.m_node.m_location && lhs.m_node.m_resources == rhs.m_node.m_resources
				&& lhs.m_node.m_time == rhs.m_node.m_time;
		}
	};
}

namespace std
{
	template <size_t t_resourceCount>
	struct hash<i5::state<t_resourceCount>>
	{
		std::size_t operator()(i5::state<t_resourceCount> const& a_state) const
		{
			size_t res = 0;
			res |= size_t(a_state.m_time) << 48;
			res |= size_t(a_state.m_location) << 32;

			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				res = (~res) | *reinterpret_cast<const uint32_t*>(&a_state.m_resources[i]);
			}

			return res;
		}
	};

	template <size_t t_resourceCount>
	struct hash< vob::aoepf::open_node<i5::state<t_resourceCount>, float>>
	{
		std::size_t operator()(vob::aoepf::open_node<i5::state<t_resourceCount>, float> const& a_node) const
		{
			size_t res = 0;
			res += a_node.m_node.m_time << 48;
			res += a_node.m_node.m_location << 32;

			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				res = (~res) | *reinterpret_cast<const int32_t*>(&a_node.m_node.m_resources[i]);
			}

			return res;
		}
	};

	template <size_t t_resourceCount>
	struct hash< vob::aoepf::visited_node<i5::state<t_resourceCount>, float>>
	{
		std::size_t operator()(vob::aoepf::visited_node<i5::state<t_resourceCount>, float> const& a_node) const
		{
			size_t res = 0;
			res += a_node.m_node.m_time << 48;
			res += a_node.m_node.m_location << 32;

			for (int32_t i = 0; i < t_resourceCount; ++i)
			{
				res = (~res) | *reinterpret_cast<const int32_t*>(&a_node.m_node.m_resources[i]);
			}

			return res;
		}
	};
}

namespace i5
{
	void test()
	{
		using namespace vob::aoepf;

		solver<3> solver;
		{
			// energy / rest / stash
			// eat / sleep / work 4
			solver.m_increment = 1; // 0.5h
			solver.m_maxTime = 48; // 0.5h
			solver.m_maxResources = glm::vec3{ 96.0f, 72.0f, -1.0f };
			solver.m_startResources = glm::vec3{ 24.0f, 48.0f, 100.0f };
			solver.m_endResources = glm::vec3{ 32.0f, 52.0f, 0.0f };
			solver.m_locomotion = glm::vec3{ -1.0f, -1.0f, 0.0f };
			solver.m_goal = glm::vec3{ 0.0f, 0.0f, 1.0f };
			solver.m_productiveActions = glm::mat3(
				glm::vec3{ 8.0f, -1.0f, -8.0f }, glm::vec3{ -0.1f, 2.0f, 0.0f }, glm::vec3{ -2.0f, -1.0f, 16.0f });
			solver.m_reverseProductiveActions = glm::inverse(solver.m_productiveActions);
			solver.m_locomotiveActions = glm::mat3(
				glm::vec3{ 1.0f, 1.0f, 1.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f });
		}

		state<3> initState{ &solver };
		{
			initState.m_resources = solver.m_startResources;
			initState.m_location = 0;
			initState.m_time = 0;
		}

		auto initNode = start_node<state<3>, float>{ initState, 0.0f };

		auto queue = open_priority_queue<state<3>, float>();
		auto map = visited_map<state<3>, float>();
		auto set = closed_set<state<3>>();

		std::vector<state<3>> solution;
		
		{
			auto tmp = neighbors<3>{}(initState);
			if (tmp.empty())
			{
				return;
			}
		}

		vob::aoepf::a_star(
			&initNode, (&initNode) + 1,
			queue, map, set,
			neighbors<3>{}, distance<3>{}, heuristic<3>{}, end<3>{},
			std::back_inserter(solution));
	}
}

#include <glm/matrix.hpp>

namespace i6
{
	constexpr auto k_endOfDay = 24_h;
	constexpr auto k_maxRest = 24_h;
	constexpr auto k_maxEnergy = 2'000_cal;
	
	constexpr auto k_sleepRestRate = 2_min / 1_min;
	constexpr auto k_sleepEnergyRate = -0.5_cal / 1_min;
	constexpr auto k_sleepStashRate = 0_cal / 1_min;

	enum class location
	{
		bed,
		kitchen,
		field,
		count
	};

	constexpr auto k_startLocation = location::bed;
	constexpr auto k_startRest = 16_h;
	constexpr auto k_startEnergy = 1'000_cal;
	constexpr auto k_startStorage = 10'000_cal;

	struct state
	{
		// world
		vob::misph::measure_time m_timeOfDay = 0_h;

		// agent
		location m_location = k_startLocation;
		vob::misph::measure_time m_rest = k_startRest;
		vob::misph::measure_energy m_energy = k_startEnergy;
		vob::misph::measure_energy m_storage = k_startStorage;
	};

	std::ostream& operator<<(std::ostream& o_out, state const& a_state)
	{
		o_out << a_state.m_timeOfDay << " | ";
		switch (a_state.m_location)
		{
		case location::bed:
			o_out << "bed | ";
			break;
		case location::kitchen:
			o_out << "kitchen | ";
			break;
		case location::field:
			o_out << "field | ";
			break;
		default:
			break;
		}
		o_out << a_state.m_rest - k_startRest << " | ";
		o_out << a_state.m_energy - k_startEnergy << " | ";
		o_out << a_state.m_storage - k_startStorage;
		return o_out;
	}

	constexpr bool operator==(state const& a_lhs, state const& a_rhs)
	{
		return a_lhs.m_timeOfDay == a_rhs.m_timeOfDay &&
			a_lhs.m_location == a_rhs.m_location &&
			a_lhs.m_rest == a_rhs.m_rest &&
			a_lhs.m_energy == a_rhs.m_energy &&
			a_lhs.m_storage == a_rhs.m_storage;
	}

	struct action
	{
		vob::misph::measure_time m_duration = 0_h;
		location m_newLocation = location::bed;
		vob::misph::measure_time m_restChange = 0_h;
		vob::misph::measure_energy m_energyChange = 0_cal;
		vob::misph::measure_energy m_storageChange = 0_cal;
	};

	struct base_action_rate
	{
		using time_rate = decltype(1_min / 1_min);
		using energy_rate = decltype(1_cal / 1_min);

		time_rate m_rest = 0.0f;
		energy_rate m_energy = 0_cal / 1_min;
		energy_rate m_storage = 0_cal / 1_min;
	};

	constexpr auto k_baseSleepRate = base_action_rate{ 2.0f, -0.5_cal / 1_min, 0_cal / 1_min };
	constexpr auto k_baseEatRate = base_action_rate{ -0.5f, 8_cal / 1_min, -10_cal / 1_min };
	constexpr auto k_baseWorkRate = base_action_rate{ -1.25f, -2_cal / 1_min, 5_cal / 1_min };
	constexpr auto k_walkRate = base_action_rate{ -1.0f, -1_cal / 1_min, 0_cal / 1_min };

	action create_action(
		location a_newLocation,
		base_action_rate a_baseActionRate,
		vob::misph::measure_time a_baseActionDuration,
		vob::misph::measure_time a_walkDuration)
	{
		return action{
			a_walkDuration + a_baseActionDuration,
			a_newLocation,
			a_baseActionRate.m_rest * a_baseActionDuration + k_walkRate.m_rest * a_walkDuration,
			a_baseActionRate.m_energy * a_baseActionDuration + k_walkRate.m_energy * a_walkDuration,
			a_baseActionRate.m_storage * a_baseActionDuration + k_walkRate.m_storage * a_walkDuration
		};
	}

	using location_time_map = vob::mistd::enum_map<location, vob::misph::measure_time>;
	constexpr vob::mistd::enum_map<location, location_time_map> k_walkDurations{
		location_time_map{0_min, 15_min, 30_min},
		location_time_map{15_min, 0_min, 30_min},
		location_time_map{30_min, 30_min, 0_min}
	};

	constexpr vob::mistd::enum_map<location, base_action_rate> k_baseActionRates{
		k_baseSleepRate, k_baseEatRate, k_baseWorkRate
	};

	action neighbor_action(location a_oldLocation, location a_newLocation, vob::misph::measure_time a_baseActionTime)
	{
		auto const walkDuration = k_walkDurations[a_oldLocation][a_newLocation];
		auto const baseActionRate = k_baseActionRates[a_newLocation];
		return create_action(a_newLocation, baseActionRate, a_baseActionTime, walkDuration);
	}

	template <typename T = void>
	struct between
	{
		[[nodiscard]] constexpr bool operator()(const T& a_value, const T& a_min, const T& a_max)
		{
			return std::less_equal<T>{}(a_min, a_value) && std::less_equal<T>{}(a_value, a_max);
		}
	};

	template <>
	struct between<void>
	{
		template <typename TValue, typename TMin, typename TMax>
		[[nodiscard]] constexpr auto operator()(TValue&& a_value, TMin&& a_min, TMax&& a_max) const
		{
			return std::less_equal<>{}(a_min, a_value) && std::less_equal<>{}(a_value, a_max);
		}

		using is_transparent = int;
	};

	constexpr bool can_execute(state const& a_state, action const& a_action)
	{
		if (!between{}(a_state.m_timeOfDay + a_action.m_duration, 0_h, k_endOfDay))
		{
			return false;
		}

		if (!between{}(a_state.m_rest + a_action.m_restChange, 0_h, k_maxRest))
		{
			return false;
		}

		if (!between{}(a_state.m_energy + a_action.m_energyChange, 0_cal, k_maxEnergy))
		{
			return false;
		}

		if (!std::greater{}(a_state.m_storage + a_action.m_storageChange, 0_cal))
		{
			return false;
		}

		return true;
	}

	constexpr auto& operator+=(state& a_state, action const& a_action)
	{
		assert(can_execute(a_state, a_action));

		a_state.m_timeOfDay += a_action.m_duration;
		a_state.m_location = a_action.m_newLocation;
		a_state.m_rest += a_action.m_restChange;
		a_state.m_energy += a_action.m_energyChange;
		a_state.m_storage += a_action.m_storageChange;
		return a_state;
	}
	
	constexpr auto operator+(state const& a_state, action const& a_action)
	{
		auto newState = a_state;
		newState += a_action;
		return newState;
	}

	constexpr auto k_baseActionDuration = 15_min;

	struct neighbors
	{
		std::vector<state> operator()(state const& a_state) const
		{
			std::vector<state> res;

			auto const sleepAction = neighbor_action(a_state.m_location, location::bed, k_baseActionDuration);
			if (can_execute(a_state, sleepAction))
			{
				res.emplace_back(a_state + sleepAction);
			}

			auto const eatAction = neighbor_action(a_state.m_location, location::kitchen, k_baseActionDuration);
			if (can_execute(a_state, eatAction))
			{
				res.emplace_back(a_state + eatAction);
			}

			auto const workAction = neighbor_action(a_state.m_location, location::field, k_baseActionDuration);
			if (can_execute(a_state, workAction))
			{
				res.emplace_back(a_state + workAction);
			}

			return res;
		}
	};

	struct end
	{
		state m_bestState;
		vob::misph::measure_time m_bestTime = 0_min;

#ifndef NDEBUG
		int32_t m_iterations = 0;
#endif

		bool operator()(state const& a_state)
		{
#ifndef NDEBUG
			if (++m_iterations % 1000 == 0)
			{
				std::cout << a_state << std::endl;
			}
#endif
			if (m_bestTime < a_state.m_timeOfDay)
			{
				m_bestTime = a_state.m_timeOfDay;
				m_bestState = a_state;
			}

			if (!between{}(a_state.m_timeOfDay, k_endOfDay - 1_min, k_endOfDay + 1_min))
			{
				return false;
			}

			if (a_state.m_location != k_startLocation)
			{
				return false;
			}

			if (!between{}(a_state.m_rest, k_startRest, k_startRest + k_baseSleepRate.m_rest * k_baseActionDuration))
			{
				return false;
			}

			if (!between{}(a_state.m_energy, k_startEnergy, k_startEnergy + k_baseEatRate.m_energy * k_baseActionDuration))
			{
				return false;
			}

			if (!std::greater{}(a_state.m_storage, k_startStorage))
			{
				return false;
			}

			return true;
		}
	};

	struct distance
	{
		float operator()(state const& a_lhs, state const& a_rhs) const
		{
			return (a_rhs.m_timeOfDay - a_lhs.m_timeOfDay).get_value();
		}
	};

	struct heuristic
	{
		float operator()(state const& a_state) const
		{
			return -((a_state.m_storage - k_startStorage) / k_baseWorkRate.m_storage).get_value();
		}
	};
}

namespace std
{
	template <>
	struct hash<i6::state>
	{
		size_t operator()(i6::state const& a_state) const
		{
			auto t = static_cast<uint64_t>((a_state.m_timeOfDay / i6::k_baseActionDuration).get_value());
			auto l = static_cast<uint64_t>(a_state.m_location);
			auto r = static_cast<uint64_t>(a_state.m_rest.get_value() * 10);
			auto e = static_cast<uint64_t>(a_state.m_energy.get_value() * 10);
			auto s = static_cast<uint64_t> (a_state.m_storage.get_value() * 10);
			
			return s | e << 8 | r << 16 | t << 24 | t << 30;
		}
	};
}

namespace i6
{
	void test()
	{
		using namespace vob::aoepf;

		auto initNode = start_node<state, float>{ state{}, 0.0f };
		auto queue = open_priority_queue<state, float>{};
		auto map = visited_map<state, float>{};
		auto set = closed_set<state>{};

		std::vector<state> solution;
		solution.reserve(static_cast<size_t>(std::ceil((24_h / k_baseActionDuration).get_value())));

		vob::aoepf::a_star(
			&initNode, (&initNode) + 1,
			queue, map, set,
			neighbors{}, distance{}, heuristic{}, end{},
			std::back_inserter(solution));
	}
}

#include <vob/misc/std/vector2d.h>
#include <iomanip>

namespace i7
{
	using namespace vob;
	// - have things to optimize
	// - have an idea of how much time you need to spend in each activity to reach some goal (mmm...)
	// - update this time iterations after iterations
	// - try to reach maxs to minimize "travel"
	// - travel leads to effective time; see how to incorporate this knowledge

	constexpr float k_endOfDay = 24.0f;

	enum class location
	{
		bed,
		kitchen,
		field,
		count
	};

	std::ostream& operator<<(std::ostream& o_os, location const a_location)
	{
		switch (a_location)
		{
		case location::bed:
			return o_os << "b";
		case location::kitchen:
			return o_os << "k";
		case location::field:
			return o_os << "f";
		default:
			return o_os << "?";
		}
	}

	enum class vital
	{
		rest,
		energy,
		count
	};

	std::ostream& operator<<(std::ostream& o_os, vital const a_vital)
	{
		switch (a_vital)
		{
		case vital::rest:
			return o_os << "r";
		case vital::energy:
			return o_os << "e";
		default:
			return o_os << "?";
		}
	}

	template <typename T>
	using vital_map = mistd::enum_map<vital, T>;

	template <typename T>
	using location_map = mistd::enum_map<location, T>;

	constexpr location k_startLocation = location::bed;

	struct state
	{
		float m_timeOfDay = 0.0f;
		location m_location = k_startLocation;

		vital_map<float> m_vitals = { 16.0f, 8.0f };
		float m_wealth = 100.0f;

		location_map<float> m_remainingActivityTimes = {8.0f, 5.0f, 11.0f};
	};

	std::ostream& operator<<(std::ostream& o_os, state const& a_state)
	{
		o_os << std::fixed;
		o_os << std::setprecision(2);
		o_os << "t: " << a_state.m_timeOfDay << ", loc: " << a_state.m_location << ", vitals: {";
		for (auto v : mistd::enum_range<vital>{})
		{
			o_os << v << ": " << a_state.m_vitals[v] << ", ";
		}
		return o_os << "}, w: " << a_state.m_wealth;
	}

	struct action
	{
		vital_map<float> m_vitals = { 0.0f, 0.0f };
		float m_wealth = 0.0f;
	};

	struct activity
	{
		float m_time = 0.0f;
		action m_action = {};
	};

	constexpr action k_sleep = action{ vital_map<float>{+2.0f, -1.0f}, -0.0f };
	constexpr action k_eat = action{ vital_map<float>{-0.5f, +10.0f}, -12.0f };
	constexpr action k_work = action{ vital_map<float>{-1.1f, -2.0f}, +15.0f };

	constexpr vital_map<float> k_maxVitals = { 24.0f, 16.0f };

	constexpr mistd::enum_map<location, mistd::enum_map<location, activity>> k_moves = {
		mistd::enum_map<location, activity>{
			activity{},
			activity{0.25f, action{vital_map<float>{-1.0f, -1.0f}, 0.0f}},
			activity{0.50f, action{vital_map<float>{-1.0f, -1.0f}, 0.0f}}
		},
		mistd::enum_map<location, activity>{
			activity{0.25f, action{vital_map<float>{-1.0f, -1.0f}, 0.0f}},
			activity{},
			activity{0.50f, action{vital_map<float>{-1.0f, -1.0f}, 0.0f}}
		},
		mistd::enum_map<location, activity>{
			activity{0.25f, action{vital_map<float>{-1.0f, -1.0f}, 0.0f}},
			activity{0.25f, action{vital_map<float>{-1.0f, -1.0f}, 0.0f}},
			activity{}
		}
	};

	state quick_apply_no_limit(state const& a_state, activity const& a_activity, location const a_location)
	{
		state newState = a_state;
		newState.m_timeOfDay += a_activity.m_time;
		newState.m_location = a_location;
		for (int32_t i = 0; i < a_state.m_vitals.size(); ++i)
		{
			newState.m_vitals[i] += a_activity.m_time * a_activity.m_action.m_vitals[i];
		}
		newState.m_wealth += a_activity.m_time * a_activity.m_action.m_wealth;
		return newState;
	}

	state quick_apply(state const& a_state, activity const& a_activity, location const a_location)
	{
		state newState = quick_apply_no_limit(a_state, a_activity, a_location);
		newState.m_remainingActivityTimes[a_location] -= a_activity.m_time;
		return newState;
	}

	state apply(state const& a_state, activity const& a_activity, location const a_location)
	{
		return quick_apply(quick_apply_no_limit(a_state, k_moves[a_state.m_location][a_location], a_location), a_activity, a_location);
	}

	float calc_max_activity_time(state const& a_state, action const& a_action, location a_location)
	{
		state copy = quick_apply_no_limit(a_state, k_moves[a_state.m_location][a_location], a_location);
		
		// 1. make sure we don't get past a day
		state last = quick_apply_no_limit(copy, k_moves[a_location][k_startLocation], k_startLocation);
		if (last.m_timeOfDay > 24.0f)
		{
			return 0.0f;
		}

		// 2. calculate limiting vital / wealth
		float result = a_action.m_wealth < 0.0f ? copy.m_wealth / -a_action.m_wealth : std::numeric_limits<float>::max();
		for (auto v : mistd::enum_range<vital>{})
		{
			auto worstLeaveDiff = 0.0f;
			auto bestLeaveDiff = 0.0f;
			for (auto l : mistd::enum_range<location>{})
			{
				worstLeaveDiff = std::max(worstLeaveDiff, -k_moves[a_location][l].m_action.m_vitals[v] * k_moves[a_location][l].m_time);
				bestLeaveDiff = std::min(worstLeaveDiff, -k_moves[a_location][l].m_action.m_vitals[v] * k_moves[a_location][l].m_time);
			}

			if (a_action.m_vitals[v] > 0.0f)
			{
				result = std::min(result, (k_maxVitals[v] - bestLeaveDiff - copy.m_vitals[v]) / a_action.m_vitals[v]);
			}
			else if (a_action.m_vitals[v] < 0.0f)
			{
				result = std::min(result, (copy.m_vitals[v] - worstLeaveDiff) / -a_action.m_vitals[v]);
			}
		}

		// 3. cap to remaining activity time
		result = std::min(a_state.m_remainingActivityTimes[a_location], result);

		return std::max(0.0f, std::min(result, 24.0f - k_moves[a_location][k_startLocation].m_time - copy.m_timeOfDay));
	}

	struct neighbors
	{
		std::vector<state> operator()(state const& a_state) const
		{
			std::vector<state> res;
			res.reserve(3);
			
			auto const sleepTime = calc_max_activity_time(a_state, k_sleep, location::bed);
			if (sleepTime > 0.0f)
			{
				res.emplace_back(apply(a_state, activity{ sleepTime, k_sleep }, location::bed));
			}

			auto const eatTime = calc_max_activity_time(a_state, k_eat, location::kitchen);
			if (eatTime > 0.0f)
			{
				res.emplace_back(apply(a_state, activity{ eatTime, k_eat }, location::kitchen));
			}

			auto const workTime = calc_max_activity_time(a_state, k_work, location::field);
			if (workTime > 0.0f)
			{
				res.emplace_back(apply(a_state, activity{ workTime, k_work }, location::field));
			}

			return res;
		}
	};

	struct end
	{
		state m_bestState;

#ifndef NDEBUG
		int32_t m_iterations = 0;
#endif

		bool operator()(state const& a_state)
		{
#ifndef NDEBUG
			if (++m_iterations % 1000 == 0)
			{
				std::cout << a_state << std::endl;
			}
#endif

			return true;
		}
	};

	float score(state const& a_lhs)
	{
		return std::abs(a_lhs.m_vitals[vital::rest] - state{}.m_vitals[vital::rest])
			+ std::abs(a_lhs.m_vitals[vital::energy] - state{}.m_vitals[vital::energy])
			- a_lhs.m_wealth / 10.0f;
	}

	void test_process(std::vector<state> ancestors, state const& a_state, state& o_bestState, float& o_bestScore, std::vector<state>& o_bestAncestors)
	{
		auto const ns = neighbors{}(a_state);
		if (ns.empty())
		{
			auto const s = score(a_state);
			if (s < o_bestScore && a_state.m_timeOfDay >= 24.0f - 0.5f)
			{
				o_bestState = a_state;
				o_bestScore = s;
				o_bestAncestors = ancestors;
			}

			/* for (auto const& state : ancestors)
			{
				std::cout << state.m_location << " -> ";
			}
			std::cout << a_state.m_location << std::endl;
			std::cout << '\t' << a_state << std::endl; */
		}

		ancestors.push_back(a_state);
		for (auto const& n : ns)
		{
			test_process(ancestors, n, o_bestState, o_bestScore, o_bestAncestors);
		}
	}

	void test()
	{
		location_map<float> activityTimes = { 8.0f, 5.0f, 11.0f };
		glm::mat3 timeToCost(
			glm::vec3{ k_sleep.m_vitals[vital::rest], k_sleep.m_vitals[vital::energy], k_sleep.m_wealth },
			glm::vec3{ k_eat.m_vitals[vital::rest], k_eat.m_vitals[vital::energy], k_eat.m_wealth },
			glm::vec3{ k_work.m_vitals[vital::rest], k_work.m_vitals[vital::energy], k_work.m_wealth }
		);
		glm::mat3 costToTime = glm::inverse(timeToCost);

		float bestF = std::numeric_limits<float>::max();
		location_map<float> bestActivityTimes;

		for (int i = 0; i < 10; ++i)
		{
			state s;
			s.m_remainingActivityTimes = activityTimes;
			state b;
			float f = std::numeric_limits<float>::max();
			std::vector<state> a;
			test_process({}, s, b, f, a);
			if (f < bestF)
			{
				bestF = f;
				bestActivityTimes = activityTimes;
			}

			std::cout << "f: " << std::format("{: >6.2f}", f);
			std::cout << " ||| r: " << std::format("{: >5.2f}", state{}.m_vitals[vital::rest]) << " -> " << std::format("{: >5.2f}", b.m_vitals[vital::rest]);
			std::cout << " | e: " << std::format("{: >5.2f}", state{}.m_vitals[vital::energy]) << " -> " << std::format("{: >5.2f}", b.m_vitals[vital::energy]);
			std::cout << " ||| b: " << std::format("{: >5.2f}", activityTimes[0] - b.m_remainingActivityTimes[0]);
			std::cout << " | k: " << std::format("{: >5.2f}", activityTimes[1] - b.m_remainingActivityTimes[1]);
			std::cout << " | f: " << std::format("{: >5.2f}", activityTimes[2] - b.m_remainingActivityTimes[2]);
			std::cout << " ||| w: " << std::format("{: >5.2f}", b.m_wealth);
			std::cout << std::endl;

			glm::vec3 costDiff = 0.5f * glm::vec3(
				state{}.m_vitals[vital::rest] - b.m_vitals[vital::rest],
				state{}.m_vitals[vital::energy] - b.m_vitals[vital::energy],
				0.0f);
			glm::vec3 timeDiff = costToTime * costDiff;
			activityTimes[location::bed] += timeDiff.x;
			activityTimes[location::kitchen] += timeDiff.y;
			activityTimes[location::field] -= timeDiff.x + timeDiff.y;
		}


		state s;
		s.m_remainingActivityTimes = bestActivityTimes;
		state b;
		float f = std::numeric_limits<float>::max();
		std::vector<state> as;
		test_process({}, s, b, f, as);
		{
			std::cout << "----------------" << std::endl;
			std::cout << bestActivityTimes[0] << " | ";
			std::cout << bestActivityTimes[1] << " | ";
			std::cout << bestActivityTimes[2] << std::endl;
			std::cout << "----------------" << std::endl;
			std::cout << "l: ";
			for (auto const& a : as)
			{
				std::cout << "   " << a.m_location << "   -> ";
			}
			std::cout << "   " << b.m_location << std::endl;
		}

		std::cout << "t: ";
		float prevTimeOfDay = 0.0f;
		for (auto const& a : as)
		{
			std::cout << std::format("{: >6.2f}", a.m_timeOfDay - prevTimeOfDay) << " -> ";
			prevTimeOfDay = a.m_timeOfDay;
		}
		std::cout << std::format("{: >6.2f}", b.m_timeOfDay - prevTimeOfDay) << std::endl;
		for (auto const v : mistd::enum_range<vital>{})
		{
			std::cout << v << ": ";
			for (auto const& a : as)
			{
				std::cout << std::format("{: >6.2f}", a.m_vitals[v]) << " -> ";
			}
			std::cout << std::format("{: >6.2f}", b.m_vitals[v]) << std::endl;
		}
		std::cout << "w: ";
		for (auto const& a : as)
		{
			std::cout << std::format("{: >6.2f}", a.m_wealth) << " -> ";
		}
		std::cout << std::format("{: >6.2f}", b.m_wealth) << std::endl;
	}
}

namespace vob::aoepf
{
	void test()
	{
		i7::test();
	}
}
