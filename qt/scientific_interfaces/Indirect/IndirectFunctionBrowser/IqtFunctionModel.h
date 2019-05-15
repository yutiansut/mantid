// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTIDQT_INDIRECT_IQTFUNCTIONMODEL_H_
#define MANTIDQT_INDIRECT_IQTFUNCTIONMODEL_H_

#include "DllConfig.h"
#include "MantidQtWidgets/Common/FunctionModel.h"
#include "MantidAPI/IFunction_fwd.h"

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

  using namespace Mantid::API;
using namespace MantidWidgets;

class MANTIDQT_INDIRECT_DLL IqtFunctionModel {
public:
  IqtFunctionModel();
  void clear();
  void setNumberOfExponentials(int);
  int getNumberOfExponentials() const;
  void setStretchExponential(bool);
  bool hasStretchExponential() const;
  void setBackground(const QString &name);
  void removeBackground();
  void setNumberOfDatasets(int);
  int getNumberOfDatasets() const;
  void setFunction(const QString &funStr);
  IFunction_sptr getGlobalFunction() const;
  IFunction_sptr getFunction() const;
  QStringList getGlobalParameters() const;
  QStringList getLocalParameters() const;
  void setStretchingGlobal(bool on);
  void updateMultiDatasetParameters(const IFunction & fun);
private:
  QString buildFunctionString() const;
  void setExponentialOne(const IFunction&);
  void setExponentialTwo(const IFunction&);
  void setStretchExponential(const IFunction&);
  void setBackground(const IFunction&);
  QString getStretchPrefix() const;

  MultiDomainFunctionModel m_model;
  int m_numberOfExponentials = 0;
  bool m_hasStretchExponential = false;
  QString m_background;
  double m_exp1Height = 1.0;
  double m_exp1Lifetime = 1.0;
  double m_exp2Lifetime = 1.0;
  double m_exp2Height = 1.0;
  double m_stretchHeight = 1.0;
  double m_stretchLifetime = 1.0;
  double m_stretchStretching = 1.0;
  double m_A0 = 0.0;
  bool m_isStretchGlobal = false;
};

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQT_INDIRECT_IQTFUNCTIONMODEL_H_ */
