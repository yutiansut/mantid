// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidKernel/Timer.h"
#include <chrono>
#include <ostream>
#include <sstream>

#ifdef __linux__
#include <time.h>
#endif  // __linux__


namespace Mantid {
namespace Kernel {

#if defined(__linux__)
uint64_t timespecToNanoSec(const timespec& time) {
  return time.tv_nsec + time.tv_sec * 1'000'000'000;
}

uint64_t currentWallClockNanoSec(clockid_t clock_id) {
  timespec time;
  clock_gettime(clock_id, &time);
  return timespecToNanoSec(time);
}

Timer::Timer() {
  m_start = currentWallClockNanoSec(CLOCK_MONOTONIC);
//  m_CPUstart = currentWallClockNanoSec(CLOCK_PROCESS_CPUTIME_ID);
}


double Timer::elapsed(bool reset){
  return static_cast<double>(elapsedNanoSec(reset)) / 1'000'000'000.0;
}

int64_t Timer::elapsedNanoSec(bool reset) {
  auto res = currentWallClockNanoSec(CLOCK_MONOTONIC) - m_start;
  if(reset)
    m_start = currentWallClockNanoSec(CLOCK_MONOTONIC);
  return res;
}

// Commented functions are valid only in Linux due to existence of user space and system space
//int64_t Timer::elapsedCPUNanoSec(bool reset) {
//}
//
//double Timer::fraction() {
//}

void Timer::reset() {
  m_start = currentWallClockNanoSec(CLOCK_MONOTONIC);
}
#elif defined(__WINDOWS__)
// need the implementation for windows
#else
// implementation for macos
#endif // __LINUX__


std::string Timer::str() {
  std::stringstream buffer;
  buffer << this->elapsed(false) << "s";
  return buffer.str();
}

/// Convenience function to provide for easier debug printing.
std::ostream &operator<<(std::ostream &out, Timer &obj) {
  out << obj.str();
  return out;
}

} // namespace Kernel
} // namespace Mantid
