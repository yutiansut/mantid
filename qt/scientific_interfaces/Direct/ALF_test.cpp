// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "ALF_test.h"
#include "MantidQtWidgets/InstrumentView/InstrumentWidget.h"


#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QSplitter>
#include <QSpinBox>



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
    : UserSubWindow(parent) { 
	  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("LoadEmptyInstrument");

  alg->initialize();
  alg->setProperty("OutputWorkspace", "ALF");

  alg->setProperty("InstrumentName", "ALF");
  alg->execute();

}

void ALFTest::initLayout() {

  m_instrument = new MantidQt::MantidWidgets::InstrumentWidget("ALF");
  m_button = new QPushButton("Load");
  m_SpinBox = new QSpinBox();
  m_SpinBox->setMaximum(1000000);
 QWidget *loadBar = new QWidget();
  QHBoxLayout *mainLayout = new QHBoxLayout(loadBar);
  mainLayout->addWidget(m_SpinBox);
  mainLayout->addWidget(m_button);
      QSplitter *controlPanelLayout = new QSplitter(Qt::Vertical);
  controlPanelLayout->addWidget(loadBar);
  controlPanelLayout->addWidget(m_instrument);

  this->setCentralWidget(controlPanelLayout);

    connect(m_button, SIGNAL(clicked()), this, SLOT(change()));


	}

void ALFTest::change() {
  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("Load");
  alg->initialize();

  alg->setProperty("Filename", "ALF" + std::to_string(int(m_SpinBox->value())));
  alg->setProperty("OutputWorkspace", "ALF");
  alg->execute();
        }
  /**
 * Destructor
 */


  
} // namespace CustomInterfaces
} // namespace MantidQt
