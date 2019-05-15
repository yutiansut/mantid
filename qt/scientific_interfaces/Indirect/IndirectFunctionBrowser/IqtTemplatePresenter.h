// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef INDIRECT_IQTTEMPLATEPRESENTER_H_
#define INDIRECT_IQTTEMPLATEPRESENTER_H_

#include "DllConfig.h"
#include "IqtFunctionModel.h"

#include <QMap>
#include <QWidget>

class QtProperty;

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

class IqtTemplateBrowser;

/**
 * Class FunctionTemplateBrowser implements QtPropertyBrowser to display
 * and set properties that can be used to generate a fit function.
 *
 */
class MANTIDQT_INDIRECT_DLL IqtTemplatePresenter : public QObject {
  Q_OBJECT
public:
  IqtTemplatePresenter(IqtTemplateBrowser *view);
  void setNumberOfExponentials(int);
  void setStretchExponential(bool);
  void setBackground(const QString &name);
  void setNumberOfDatasets(int);
  int getNumberOfDatasets() const;
  void setFunction(const QString &funStr);
  IFunction_sptr getGlobalFunction() const;
  IFunction_sptr getFunction() const;
  QStringList getGlobalParameters() const;
  QStringList getLocalParameters() const;
  void setStretchingGlobal(bool on);
  void updateMultiDatasetParameters(const IFunction & fun);

signals:
  void functionStructureChanged();

private:
  void updateViewParameters();
  IqtTemplateBrowser *m_view;
  IqtFunctionModel m_model;
};

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /*INDIRECT_IQTTEMPLATEPRESENTER_H_*/
