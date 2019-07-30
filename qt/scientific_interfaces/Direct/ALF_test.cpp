// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "ALF_test.h"
#include "MantidQtWidgets/Common/FunctionBrowser.h"

#include "MantidQtWidgets/InstrumentView/InstrumentWidget.h"
#include "MantidQtWidgets/Plotting/PreviewPlot.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>

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

ALFTest::ALFTest(QWidget *parent) : UserSubWindow(parent) {
  IAlgorithm_sptr alg =
      AlgorithmManager::Instance().create("LoadEmptyInstrument");

  alg->initialize();
  alg->setProperty("OutputWorkspace", "ALF");

  alg->setProperty("InstrumentName", "ALF");
  alg->execute();
}

void ALFTest::initLayout() {

  m_instrument = new MantidQt::MantidWidgets::InstrumentWidget("ALF");

  // create load widget
  m_button = new QPushButton("Load");
  m_SpinBox = new QSpinBox();
  m_SpinBox->setMaximum(1000000);
  QWidget *loadBar = new QWidget();
  QHBoxLayout *loadLayout = new QHBoxLayout(loadBar);
  loadLayout->addWidget(m_SpinBox);
  loadLayout->addWidget(m_button);
  //


  QSplitter *MainLayout = new QSplitter(Qt::Vertical);
  MainLayout->addWidget(loadBar);


  QSplitter *widgetSplitter = new QSplitter(Qt::Horizontal);
  widgetSplitter->addWidget(m_instrument);
  //m_instrument->connectToStoreCurve(plotCurve);

  // plot
  m_plot = new MantidQt::MantidWidgets::PreviewPlot();
  m_plot->setCanvasColour(Qt::white);
   // test adding a plot
  std::vector<double> vals{0.0, 1.0, 2.0, 3.0, 4.0, 4.0};
  std::vector<double> yvals{0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("CreateWorkspace");
  alg->initialize();
  alg->setProperty("OutputWorkspace", "test");
  alg->setProperty("DataX", vals);
  alg->setProperty("DataY", yvals);
  alg->execute();
  m_plot->addSpectrum("Data", "test", 0, Qt::black);
  // end test
  //

  // fit - plot widget
  QSplitter *fitPlotLayout = new QSplitter(Qt::Vertical);
  fitPlotLayout->addWidget(m_plot);


	// function browser

	 m_fitProp = new MantidQt::MantidWidgets::FitPropertyBrowser(this);
	 QSplitter *fitLayout = new QSplitter(Qt::Vertical);

         m_fitProp->init();
	fitLayout->addWidget(m_fitProp);
   //

   fitPlotLayout->addWidget(fitLayout);

  widgetSplitter->addWidget(fitPlotLayout);
  MainLayout->addWidget(widgetSplitter);
  this->setCentralWidget(MainLayout);

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
