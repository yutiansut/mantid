#include "MantidAPI/AlgoTimeRegister.h"
#include "MantidKernel/MultiThreaded.h"
#include <fstream>
#include <time.h>

namespace Mantid {
namespace Instrumentation {

AlgoTimeRegister::Dump::Dump(AlgoTimeRegister &atr, const std::string &nm)
    : m_algoTimeRegister(atr), m_name(nm), m_regStart(atr.m_timer.elapsed(false)) {}

AlgoTimeRegister::Dump::~Dump() {
  {
    std::lock_guard<std::mutex> lock(m_algoTimeRegister.m_mutex);
    m_algoTimeRegister.m_info.emplace_back(m_name, std::this_thread::get_id(),
                                           m_regStart, m_algoTimeRegister.m_timer.elapsed(false));
  }
}

AlgoTimeRegister::AlgoTimeRegister()
    : m_start(std::chrono::high_resolution_clock::now()) {}

AlgoTimeRegister::~AlgoTimeRegister() {
  std::fstream fs;
  fs.open("./algotimeregister.out", std::ios::out);
  fs << "START_POINT: "
     << std::chrono::duration_cast<std::chrono::nanoseconds>(
            m_start.time_since_epoch())
            .count()
     << " MAX_THREAD: " << PARALLEL_GET_MAX_THREADS << "\n";
  for (auto &elem : m_info) {
    auto st = elem.m_begin - m_timer.getStart();
    auto fi = elem.m_end - m_timer.getStart();
    fs << "ThreadID=" << elem.m_threadId << ", AlgorithmName=" << elem.m_name
       << ", StartTime=" << st << ", EndTime=" << fi << "\n";
  }
}

} // namespace Instrumentation
} // namespace Mantid
