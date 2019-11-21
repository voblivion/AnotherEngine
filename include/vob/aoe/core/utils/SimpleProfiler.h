#pragma once

#include <chrono>

#define PROFILER_START() \
	auto _utils_simpleprofiler_t0 = std::chrono::high_resolution_clock::now(); \
	auto _utils_simpleprofiler_t1 = std::chrono::high_resolution_clock::now()

#define PROFILER_RESET() \
	_utils_simpleprofiler_t0 = std::chrono::high_resolution_clock::now();

#define PROFILER_LAP(lapName) \
	_utils_simpleprofiler_t1 = std::chrono::high_resolution_clock::now(); \
	{ \
		auto dt = std::chrono::duration_cast<std::chrono::duration<float>>( \
			_utils_simpleprofiler_t1 - _utils_simpleprofiler_t0); \
		std::cout << lapName ": " << dt.count() << "s" << std::endl; \
	} \
	_utils_simpleprofiler_t0 = _utils_simpleprofiler_t1