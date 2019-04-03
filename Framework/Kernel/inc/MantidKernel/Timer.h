// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2007 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_KERNEL_TIMER_H_
#define MANTID_KERNEL_TIMER_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidKernel/DllConfig.h"
#include <chrono>
#include <iosfwd>
#include <string>

namespace Mantid {
namespace Kernel {

class MANTID_KERNEL_DLL Timer {
public:
  Timer();
  double elapsed(bool reset = true);
  int64_t elapsedNanoSec(bool reset = true);
// Commented functions are valid only in Linux due to existence of user space and system space
//  int64_t elapsedCPUNanoSec(bool reset = true);
//  double fraction();
  void reset();
  int64_t getStart() const {return m_start;}
  std::string str();
private:
  int64_t m_start; // total nanoseconds
//  int64)t m_CPUstart; // CPU nanoseconds
};

MANTID_KERNEL_DLL std::ostream &operator<<(std::ostream &, const Timer &);

} // namespace Kernel
} // namespace Mantid

#endif /* TIMER_H_ */
