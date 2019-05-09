// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2012 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_ALGORITHMS_WFMCONVERTTOTOF_H_
#define MANTID_ALGORITHMS_WFMCONVERTTOTOF_H_

#include "MantidAPI/Algorithm.h"
#include "MantidKernel/System.h"

namespace Mantid {
namespace Algorithms {

/**
 * Add a peak to a PeaksWorkspace.

  @date 2012-10-16
 */
class DLLExport WFMConvertToTof : public API::Algorithm {
public:
  /// Algorithm's name for identification
  const std::string name() const override { return "WFMConvertToTof"; };
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "Convert the raw detector arrival times to real time-of-flight.";
  }
  /// Algorithm's version for identification
  int version() const override { return 1; };
  /// Algorithm's category for identification
  const std::string category() const override {
    return "Wave-frame multiplication";
  }

private:
  /// Initialise the properties
  void init() override;
  /// Run the algorithm
  void exec() override;
};

} // namespace Algorithms
} // namespace Mantid

#endif /* MANTID_ALGORITHMS_WFMCONVERTTOTOF_H_ */
