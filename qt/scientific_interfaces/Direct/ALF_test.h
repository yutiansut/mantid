// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2013 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_CUSTOMINTERFACES_ALFTEST_H_
#define MANTID_CUSTOMINTERFACES_ALFTEST_H_

#include "MantidQtWidgets/Common/UserSubWindow.h"
#include "MantidQtWidgets/InstrumentView/InstrumentWidget.h"// can only get to common or plotting -> need to find out how to get around it
#include "DllConfig.h"
#include <QDialog>
#include <QPushButton>
#include <QSpinBox>

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

private:
  MantidQt::MantidWidgets::InstrumentWidget *m_instrument;
  QPushButton *m_button;
  QSpinBox *m_SpinBox;
};
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTID_CUSTOMINTERFACES_ALFTEST_H_ */
