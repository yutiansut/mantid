// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2013 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_CUSTOMINTERFACES_ALFTEST_H_
#define MANTID_CUSTOMINTERFACES_ALFTEST_H_

#include "DllConfig.h"
#include "MantidQtWidgets/Common/FitPropertyBrowser.h"
#include "MantidQtWidgets/Common/FunctionBrowser.h"
#include "MantidQtWidgets/Common/UserSubWindow.h"
#include "MantidQtWidgets/InstrumentView/InstrumentWidget.h" // can only get to common or plotting -> need to find out how to get around it
#include "MantidQtWidgets/Plotting/PreviewPlot.h"

#include <QAction>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

typedef bool (*FunctionPointer)(bool, bool); // Typedef for a function pointer
namespace MantidQt {
namespace CustomInterfaces {


/** ALCInterface : Custom interface for Avoided Level Crossing analysis
 */
class MANTIDQT_DIRECT_DLL ALFTest : public API::UserSubWindow {
  Q_OBJECT

public:
  ALFTest(QWidget *parent = nullptr);

  static std::string name() { return "ALF test"; }
  static QString categoryInfo() { return "Direct"; }


protected:
  void initLayout() override;

public slots:
  void change();
  void updatePlot();
  void doFit();

private:
  MantidQt::MantidWidgets::InstrumentWidget *m_instrument;
  QPushButton *m_button;
  QLineEdit *m_run;
  MantidQt::MantidWidgets::PreviewPlot *m_plot;
  MantidQt::MantidWidgets::FunctionBrowser *m_funcBrowser;
  QLineEdit *m_start;
  QLineEdit *m_end;
  QPushButton *m_fit;
  QAction *m_extractTube;
};
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTID_CUSTOMINTERFACES_ALFTEST_H_ */
