// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2012 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_MDALGORITHMS_MERGEMD_H_
#define MANTID_MDALGORITHMS_MERGEMD_H_

#include "MantidAPI/Algorithm.h"
#include "MantidAPI/IMDEventWorkspace_fwd.h"
#include "MantidDataObjects/MDEventWorkspace.h"
#include "MantidMDAlgorithms/BoxControllerSettingsAlgorithm.h"

namespace Mantid {
namespace MDAlgorithms {

/** Merge several MDWorkspaces into one.

  @date 2012-01-17
*/
class DLLExport MergeMD : public BoxControllerSettingsAlgorithm {
public:
  enum struct MergeType {DEFAULT, INDEXED};
public:
  const std::string name() const override;
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "Merge several MDWorkspaces into one.";
  }

  int version() const override;
  const std::vector<std::string> seeAlso() const override {
    return {"MergeMDFiles", "AccumulateMD"};
  }
  const std::string category() const override;

private:
  void init() override;
  void exec() override;
  void createOutputWorkspace(std::vector<std::string> &inputs);

  template <typename MDE, size_t nd>
  void doPlus(typename Mantid::DataObjects::MDEventWorkspace<MDE, nd>::sptr ws);

  /// The robust version of merging algorithm
  void doMergeDefault();


  template <size_t ND, template <size_t> class MDEventType>
  std::vector<MDEventType<ND>> collectEvents(std::vector<size_t> wsIdx) const;
  /**
   * The version of algorithm, that uses spatial index
   * morton number to increse the performance
   */
  void doMergeIndexed();

  /// Vector of input MDWorkspaces
  std::vector<Mantid::API::IMDEventWorkspace_sptr> m_workspaces;

  /// Vector of number of experimentalInfos for each input workspace
  std::vector<uint16_t> experimentInfoNo = {0};

  /// Output MDEventWorkspace
  Mantid::API::IMDEventWorkspace_sptr out;

  MergeType mergeType = MergeType::DEFAULT;
};

namespace mdo = Mantid::DataObjects;

template <size_t ND, template <size_t> class MDEventType>
std::vector<MDEventType<ND>> MergeMD::collectEvents(std::vector<size_t> wsIdxs) const {
  size_t evCnt = std::accumulate(wsIdxs.cbegin(), wsIdxs.cend(), 0ul, [this](size_t& a, const size_t& i){
    return a += this->m_workspaces[i]->getNPoints();
  });
  std::vector<MDEventType<ND>> res;
  res.reserve(evCnt);
  for(auto&& idx: wsIdxs) {
    auto ws = boost::static_pointer_cast<mdo::MDEventWorkspace<MDEventType<ND>, ND>>(m_workspaces[idx]);
    auto data = ws->getBox();
    std::vector<API::IMDNode *> boxes;
    data->getBoxes(boxes, 1000, true);
    for (auto&& ibox: boxes) {
      auto box = static_cast<mdo::MDBox<MDEventType<ND>, ND>*>(ibox);
      if (box && !box->getIsMasked()) {
        const auto &events = box->getConstEvents();
        res.insert(res.end(), events.begin(), events.end());
      }
    }
  }
  return res;
}

} // namespace MDAlgorithms
} // namespace Mantid

#endif /* MANTID_MDALGORITHMS_MERGEMD_H_ */
