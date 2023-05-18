#pragma once

#include <iostream>

template <typename... Args>
void Log(Args... args) {
#ifdef CRPC_LOG
  (std::cout << ... << args) << std::endl;
#endif
}

template <typename... Args>
void LogErr(Args... args) {
  (std::cerr << ... << args) << std::endl;
}