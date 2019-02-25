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
#include "MantidDataObjects/MDEventFactory.h"

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

  template <size_t ND, template <size_t> class MDEventType>
  static std::vector<API::IMDNode*> zOrderedLeafes(API::IMDNode* top);

  template <size_t ND, class IntT, class MortonT>
  static std::vector<size_t> zPermutation(size_t split);

  template <size_t ND, template <size_t> class MDEventType>
  void doMergeIndexed(const std::vector<size_t>& wsIndexes,
      std::vector<size_t>::const_iterator lastIter);

  template <size_t ND>
  void doMergeIndexed(const std::vector<size_t>& wsIndexes,
      std::vector<size_t>::const_iterator lastIter);

  template <size_t maxDim>
  void doMergeIndexedLoop(const std::vector<size_t>& wsIndexes,
      std::vector<size_t>::const_iterator lastIter);
  /**
   * The version of algorithm, that uses spatial index
   * morton number to increase the performance
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

template <size_t ND, class IntT, class MortonT>
std::vector<size_t> MergeMD::zPermutation(const size_t split) {
  using namespace morton_index;
  IntT count = split;
  for(IntT i = 1; i < ND; ++i) count *= split;
  std::vector<IntArray <ND, IntT>> patternBoxes(count);
  for (size_t i = 0; i < count; ++i) {
    auto& point = patternBoxes[i];
    size_t reminder = i;
    for (size_t j = 0; j < ND; ++j) {
      point[j] = reminder / split;
      reminder = reminder % split;
    }
  }

  std::vector<MortonT> indexes;
  indexes.reserve(count);
  for (auto&& coord: patternBoxes)
    indexes.emplace_back(Interleaver<ND, IntT, MortonT>::interleave(coord));

  std::vector<size_t> res(count);
  std::iota(res.begin(), res.end(), 0);
  std::sort(res.begin(), res.end(),
      [&indexes](const size_t& a, const size_t& b) {
    return indexes[a] < indexes[b];
  });

  return res;
}

template <size_t ND, template <size_t> class MDEventType>
std::vector<API::IMDNode*> MergeMD::zOrderedLeafes(API::IMDNode* top) {
  using Event = MDEventType<ND>;
  auto order = zPermutation<ND, Event::IntT, Event::MortonT>(top->getNumChildren());
  std::vector<API::IMDNode*> res;
  top->getBoxes(res, [](API::IMDNode*) { return true; }, order());
  return res;
}

inline bool equalBcTreeParams(const API::BoxController& bc1, const API::BoxController& bc2) {
  if (bc1.getMaxDepth() != bc2.getMaxDepth()) return false;
  if (bc1.getSplitThreshold() != bc2.getSplitThreshold()) return false;
  if (bc1.getNDims() != bc2.getNDims()) return false;
  for (size_t d = 0; d < bc1.getNDims(); ++d)
    if (bc1.getSplitInto(d) != bc2.getSplitInto(d)) return false;
  return true;
}

template <size_t ND, template <size_t> class MDEventType>
void MergeMD::doMergeIndexed(const std::vector<size_t>& wsIndexes,
                    std::vector<size_t>::const_iterator lastIter) {
  auto firstIter =  wsIndexes.begin();
  if (lastIter != firstIter) {
    auto pr = mdo::getMDEventWSTypeND(m_workspaces[*firstIter]);
    if (equalBcTreeParams(*m_workspaces[*firstIter]->getBoxController().get(), *out->getBoxController().get()))
      out->setBox(m_workspaces[*(firstIter++)].get()->cloneBoxes());
    if (pr.first) { // full events - update runIndex with offset
    }
  }

  for(auto it = firstIter; it < lastIter; ++it) {
    // process workspaces with the same boundaries here
    // the events TODO
  }

  for(auto it = lastIter; it != wsIndexes.end(); ++it) {
    // process workspaces with different boundaries TODO
  }
}

template <size_t ND>
void MergeMD::doMergeIndexed(const std::vector<size_t>& wsIndexes,
                             std::vector<size_t>::const_iterator lastIter) {
  auto pr = mdo::getMDEventWSTypeND(out);
  if (pr.first) //full event
    doMergeIndexed<ND, mdo::MDEvent>(wsIndexes, lastIter);
  else // lean event
    doMergeIndexed<ND, mdo::MDLeanEvent>(wsIndexes, lastIter);
}

template <size_t maxDim>
void MergeMD::doMergeIndexedLoop(const std::vector<size_t>& wsIndexes,
                                 std::vector<size_t>::const_iterator lastIter) {
  auto pr = mdo::getMDEventWSTypeND(out);
  auto ndim = pr.second;
  if (ndim < 2)
    throw std::runtime_error("Can't merge MD workspaces with dims " +
                             std::to_string(ndim) + "less than 2");
  if (ndim > maxDim)
    return;
  if (ndim == maxDim) {
    doMergeIndexed<maxDim>(wsIndexes, lastIter);
    return;
  } else
    doMergeIndexedLoop<maxDim - 1>(wsIndexes, lastIter);
}

} // namespace MDAlgorithms
} // namespace Mantid

#endif /* MANTID_MDALGORITHMS_MERGEMD_H_ */
