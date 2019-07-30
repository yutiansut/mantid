// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "ALF_test.h"
//#include "MantidQtWidgets/InstrumentView/InstrumentWidget.h"


#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>


namespace MantidQt {
namespace CustomInterfaces {
using namespace Mantid::Kernel;
using namespace Mantid::API;



DECLARE_SUBWINDOW(ALFTest)

using namespace Mantid;
using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace MantidQt::MantidWidgets;
using namespace MantidQt::CustomInterfaces;

using Mantid::API::Workspace_sptr;

/// static logger
Mantid::Kernel::Logger g_log("ALFTest");

ALFTest::ALFTest(QWidget *parent)
    : UserSubWindow(parent) {}

void ALFTest::initLayout() {
  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("Load");
  alg->initialize();

  alg->setProperty("Filename", "ALF80511");
  alg->execute();
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    //m_instrument = new InstrumentWidget("ALF80511");
    //mainLayout->addWidget(m_instrument);

	}
  /**
 * Destructor
 */


  
} // namespace CustomInterfaces
} // namespace MantidQt
