#pragma once
#include <chrono>
#include <format>
#include <iostream>

// Uncomment to create console and enable logging
#undef NDEBUG

#ifdef NDEBUG
#define LOG(...) (void)0
#define LOGW(...) (void)0
#define TIMER_START (void)0
#define TIMER_END (void)0
#else
#define LOG(...)                                                               \
  std::cout << "[" << __FILE__ << ":" << __LINE__ << "] "                      \
            << std::format(__VA_ARGS__) << std::endl
#define LOGW(...)                                                              \
  std::wcout << "[" << __FILE__ << ":" << __LINE__ << "] "                     \
             << std::format(__VA_ARGS__) << std::endl
#define TIMER_START                                                            \
  std::chrono::steady_clock::time_point _start =                               \
      std::chrono::steady_clock::now();
#define TIMER_END                                                              \
  LOG("Timer took {}ms",                                                       \
      std::chrono::duration_cast<std::chrono::milliseconds>(                   \
          std::chrono::steady_clock::now() - _start)                           \
          .count());
#endif
