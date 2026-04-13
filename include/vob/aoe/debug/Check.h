#pragma once

#include <iostream>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// TODO: remove this dependency
#include <windows.h>
#undef near
#undef far

namespace vob::aoedb
{
	enum class ErrorBehavior
	{
		Log,
		Break,
		Terminate
	};

	static constexpr auto k_log = ErrorBehavior::Log;
	static constexpr auto k_break = ErrorBehavior::Break;
	static constexpr auto k_terminate = ErrorBehavior::Terminate;
}

// TODO: implement proper logging with severity and all
#define VOB_AOE_CHECK_IMPL(cond, always, behavior, message, ...) \
	([&]<auto t_behavior>() { \
		static bool s_hasHit = false; \
		if (!(cond)) \
		{ \
			if (s_hasHit && !always) \
			{ \
				return false; \
			} \
			s_hasHit = true; \
			std::cerr << std::format(message __VA_OPT__(, ) __VA_ARGS__) << std::endl; \
			if constexpr (t_behavior == vob::aoedb::k_break) \
			{ \
				if (IsDebuggerPresent()) \
				{ \
					__debugbreak(); \
				} \
			} \
			else if constexpr (t_behavior == vob::aoedb::k_terminate) \
			{ \
				if (IsDebuggerPresent()) \
				{ \
					__debugbreak(); \
				} \
				else \
				{ \
					std::terminate(); \
				} \
			} \
			return false; \
		} \
		return true; \
	}).operator()<behavior>()

#define VOB_AOE_CHECK_LOG(cond, message, ...) VOB_AOE_CHECK_IMPL(cond, true, vob::aoedb::k_log, message __VA_OPT__(, ) __VA_ARGS__)
#define VOB_AOE_CHECK_LOG_ONCE(cond, message, ...) VOB_AOE_CHECK_IMPL(cond, false, vob::aoedb::k_log, message __VA_OPT__(, ) __VA_ARGS__)
#define VOB_AOE_CHECK_BREAK(cond, message, ...) VOB_AOE_CHECK_IMPL(cond, true, vob::aoedb::k_break, message __VA_OPT__(, ) __VA_ARGS__)
#define VOB_AOE_CHECK_BREAK_ONCE(cond, message, ...) VOB_AOE_CHECK_IMPL(cond, false, vob::aoedb::k_break, message __VA_OPT__(, ) __VA_ARGS__)
#define VOB_AOE_CHECK_TERMINATE(cond, message, ...) VOB_AOE_CHECK_IMPL(cond, true, vob::aoedb::k_terminate, message __VA_OPT__(, ) __VA_ARGS__)
#ifndef NDEBUG
#define VOB_AOE_CHECK_TERMINATE_SLOW(cond, message, ...) VOB_AOE_CHECK_IMPL(cond, false, vob::aoedb::k_terminate, message __VA_OPT__(, ) __VA_ARGS__)
#else
#define VOB_AOE_CHECK_TERMINATE_SLOW(cond, message, ...) true
#endif
