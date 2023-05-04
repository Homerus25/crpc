#pragma once

#include <iostream>

template <typename... Args>
void Log(Args... args) {
  (std::cout << ... << args) << std::endl;
}

template <typename... Args>
void LogErr(Args... args) {
  (std::cerr << ... << args) << std::endl;
}