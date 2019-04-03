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

NewTimer::NewTimer() {
  m_start = currentWallClockNanoSec(CLOCK_MONOTONIC);
//  m_CPUstart = currentWallClockNanoSec(CLOCK_PROCESS_CPUTIME_ID);
}


double NewTimer::elapsed(bool reset){
  return static_cast<double>(elapsedNanoSec(reset)) / 1'000'000'000.0;
}

int64_t NewTimer::elapsedNanoSec(bool reset) {
  auto res = currentWallClockNanoSec(CLOCK_MONOTONIC) - m_start;
  if(reset)
    m_start = currentWallClockNanoSec(CLOCK_MONOTONIC);
  return res;
}

// Commented functions are valid only in Linux due to existence of user space and system space
//int64_t NewTimer::elapsedCPUNanoSec(bool reset) {
//}
//
//double NewTimer::fraction() {
//}

void NewTimer::reset() {
  m_start = currentWallClockNanoSec(CLOCK_MONOTONIC);
}
#elif defined(__WINDOWS__)
// need the implementation for windows
#else
// implementation for macos
#endif // __LINUX__


/** Constructor.
 *  Instantiating the object starts the timer.
 */
Timer::Timer() { m_start = std::chrono::high_resolution_clock::now(); }

/** Returns the wall-clock time elapsed in seconds since the Timer object's
 *creation, or the last call to elapsed
 *
 * @param reset :: set to true to reset the clock (default)
 * @return time in seconds
 */
float Timer::elapsed(bool reset) {
  float retval = elapsed_no_reset();
  if (reset)
    this->reset();
  return retval;
}

/** Returns the wall-clock time elapsed in seconds since the Timer object's
 *creation, or the last call to elapsed
 *
 * @return time in seconds
 */
float Timer::elapsed_no_reset() const {
  const auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> duration = now - m_start;

  return duration.count();
}

/// Explicitly reset the timer.
void Timer::reset() { m_start = std::chrono::high_resolution_clock::now(); }

/// Convert the elapsed time (without reseting) to a string.
std::string Timer::str() const {
  std::stringstream buffer;
  buffer << this->elapsed_no_reset() << "s";
  return buffer.str();
}

/// Convenience function to provide for easier debug printing.
std::ostream &operator<<(std::ostream &out, const Timer &obj) {
  out << obj.str();
  return out;
}

} // namespace Kernel
} // namespace Mantid
