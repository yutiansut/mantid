// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidAlgorithms/WFMConvertToTof.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidKernel/ArrayProperty.h"

namespace Mantid {
namespace Algorithms {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(WFMConvertToTof)

using namespace API;
using namespace DataObjects;
using namespace Kernel;

/** Initialize the algorithm's properties.
 */
void WFMConvertToTof::init() {
  declareProperty(make_unique<WorkspaceProperty<EventWorkspace>>(
                      "InputWorkspace", "", Direction::InOut),
                  "An event workspace.");
  declareProperty(
      make_unique<ArrayProperty<double>>("FrameShifts", Direction::Input),
      "Array containing the frame shifts.");
  declareProperty(
      make_unique<ArrayProperty<double>>("FrameParameters", Direction::Input),
      "Array containing the boundaries of each WFM frame.");
}

/** Execute the algorithm.
 */
void WFMConvertToTof::exec() {

  EventWorkspace_sptr event_workspace = getProperty("InputWorkspace");
  const std::vector<double> frame_shifts = getProperty("FrameShifts");
  const std::vector<double> frame_parameters = getProperty("FrameParameters");

  // Process each event list
  auto nlists = event_workspace->getNumberHistograms();
  size_t nframes = frame_parameters.size() / 2;
  bool no_frame_found;
  // Loop over all event lists
  for (size_t i = 0; i < nlists; i++) {
    auto &event_list = event_workspace->getSpectrum(i);
    auto tofs = event_list.getTofs();
    auto nevents = tofs.size();
    std::vector<double> shifts(nevents);
    std::vector<size_t> to_be_removed;
    // Loop over all events in current list
    for (size_t j = 0; j < nevents; j++) {
      // Loop over all WFM frames
      no_frame_found = true;
      for (size_t k = 0; k < nframes; k++) {
        if ((tofs[j] >= frame_parameters[2 * k]) &&
            (tofs[j] <= frame_parameters[2 * k + 1])) {
          shifts[j] = frame_shifts[k];
          no_frame_found = false;
          break;
        }
      }
      if (no_frame_found)
        to_be_removed.push_back(j);
    }
    event_list.convertWFMEventsToTof(shifts);
  }

}

} // namespace Algorithms
} // namespace Mantid
