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
#include "MantidQtWidgets/InstrumentView/InstrumentWidgetPickTab.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <math.h>
	  bool extractTubeCondition(bool ifPlot, bool ifStored) {
  return (ifPlot || ifStored);
}

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
  m_instrument->connectToStoreCurve();
  m_extractTube = new QAction("Extract Tube", this);
  connect(m_extractTube, SIGNAL(triggered()),this, SLOT(updatePlot())),

	  m_instrument->addAction(m_extractTube, &extractTubeCondition );


  // create load widget
  m_button = new QPushButton("Browse");

  // connect(m_button, SIGNAL(clicked()), this, SLOT(change()));
  m_run = new QLineEdit();
  connect(m_run, SIGNAL(editingFinished()), this, SLOT(change()));
  QWidget *loadBar = new QWidget();
  QHBoxLayout *loadLayout = new QHBoxLayout(loadBar);
  loadLayout->addWidget(m_run);
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

  m_funcBrowser = new MantidQt::MantidWidgets::FunctionBrowser(this);
  m_fit = new QPushButton("Fit");
  QLabel *start = new QLabel("Fit from:");
  m_start = new QLineEdit("-15.0");
  QLabel *end = new QLabel("to:");
  m_end = new QLineEdit("15.0");
  
  QWidget *range = new QWidget();
  QHBoxLayout *rangeLayout = new QHBoxLayout(range);
  
  rangeLayout->addWidget(start);
  rangeLayout->addWidget(m_start);
  rangeLayout->addWidget(end);
  rangeLayout->addWidget(m_end);
  
	 QSplitter *fitLayout = new QSplitter(Qt::Vertical);

         fitLayout->addWidget(m_fit);
         fitLayout->addWidget(m_funcBrowser);
         fitLayout->addWidget(range);
   //
         connect(m_fit, SIGNAL(clicked()), this, SLOT(doFit()));

   fitPlotLayout->addWidget(fitLayout);

  widgetSplitter->addWidget(fitPlotLayout);
  MainLayout->addWidget(widgetSplitter);
  this->setCentralWidget(MainLayout);

}

void ALFTest::doFit() {
  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("Fit");
  alg->initialize();
  alg->setProperty("Function", m_funcBrowser->getFunction());
  alg->setProperty("InputWorkspace", "Curves");
  alg->setProperty("Output", "ALF_fits");
  alg->setProperty("StartX", m_start->text().toDouble());
  alg->setProperty("EndX", m_end->text().toDouble());
  alg->execute();
  m_plot->clear();
  m_plot->addSpectrum("new Data", "ALF_fits_Workspace", 0, Qt::black);
  m_plot->addSpectrum("Fit", "ALF_fits_Workspace", 1, Qt::red);
  //need to update function browser
  Mantid::API::IFunction_sptr function = alg->getProperty("Function");
  m_funcBrowser->updateMultiDatasetParameters(*function);
}

void ALFTest::change() {
  const std::string name = "ALF" + std::to_string(m_run->text().toDouble());

  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("Load");
  alg->initialize();

  alg->setProperty("Filename",name );
  alg->setProperty("OutputWorkspace", "ALF");
  alg->execute();

  IAlgorithm_sptr normAlg =
      AlgorithmManager::Instance().create("NormaliseByCurrent");
  normAlg->initialize();
  normAlg->setProperty("InputWorkspace", "ALF");
  normAlg->setProperty("OutputWorkspace", "ALF");
  normAlg->execute();

  IAlgorithm_sptr dSpacingAlg = AlgorithmManager::Instance().create("ConvertUnits");
  dSpacingAlg->initialize();

  dSpacingAlg->setProperty("InputWorkspace", "ALF");
  dSpacingAlg->setProperty("Target", "dSpacing");
  dSpacingAlg->setProperty("OutputWorkspace", "ALF");
  dSpacingAlg->execute();


}
void ALFTest::updatePlot() { 
	// will need to rescale to degrees
m_instrument->getPickTab()->savePlotToWorkspace();

        IAlgorithm_sptr alg = AlgorithmManager::Instance().create("ScaleX");
        alg->initialize();

        alg->setProperty("InputWorkspace", "Curves");
        alg->setProperty("OutputWorkspace", "Curves");
        alg->setProperty("Factor",180./M_PI);

        alg->execute();

	  m_plot->clear();
  m_plot->addSpectrum("new Data", "Curves", 0, Qt::black);

}
/**
 * Destructor
 */

} // namespace CustomInterfaces
} // namespace MantidQt
