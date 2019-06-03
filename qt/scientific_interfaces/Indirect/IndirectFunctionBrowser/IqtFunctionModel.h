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
#include "MantidAPI/ITableWorkspace_fwd.h"

#include <QMap>
#include <boost/optional.hpp>

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
  bool hasBackground() const;
  void setNumberOfDatasets(int);
  int getNumberOfDatasets() const;
  void setFunction(const QString &funStr);
  IFunction_sptr getGlobalFunction() const;
  IFunction_sptr getFunction() const;
  QStringList getGlobalParameters() const;
  QStringList getLocalParameters() const;
  bool isGlobal(const QString &parName) const;
  void setStretchingGlobal(bool on);
  void updateMultiDatasetParameters(const IFunction & fun);
  void updateMultiDatasetParameters(const ITableWorkspace & paramTable);
  void updateParameters(const IFunction &fun);
  void setCurrentDataset(int i);
  int getCurrentDataset() const;
  void setDatasetNames(const QStringList &names);
  QStringList getDatasetNames() const;
  double getLocalParameterValue(const QString &parName, int i) const;
  bool isLocalParameterFixed(const QString &parName, int i) const;
  QString getLocalParameterTie(const QString &parName, int i) const;
  void setLocalParameterValue(const QString &parName, int i, double value);
  void setLocalParameterFixed(const QString &parName, int i, bool fixed);
  void setLocalParameterTie(const QString &parName, int i, const QString &tie);

  enum class ParamNames {
    EXP1_HEIGHT,
    EXP1_LIFETIME,
    EXP2_HEIGHT,
    EXP2_LIFETIME,
    STRETCH_HEIGHT,
    STRETCH_LIFETIME,
    STRETCH_STRETCHING,
    BG_A0
  };
  QMap<ParamNames, double> getCurrentValues() const;
  QMap<ParamNames, double> getCurrentErrors() const;
  QMap<int, QString> getParameterNameMap() const;
  QMap<int, std::string> getParameterDescriptionMap() const;

private:
  QString buildFunctionString() const;
  boost::optional<QString> getExp1Prefix() const;
  boost::optional<QString> getExp2Prefix() const;
  boost::optional<QString> getStretchPrefix() const;
  boost::optional<QString> getBackgroundPrefix() const;
  void setParameter(ParamNames name, double value);
  boost::optional<double> getParameter(ParamNames name) const;
  boost::optional<double> getParameterError(ParamNames name) const;
  boost::optional<QString> getParameterName(ParamNames name) const;
  boost::optional<QString> getParameterDescription(ParamNames name) const;
  boost::optional<QString> getPrefix(ParamNames name) const;
  void setCurrentValues(const QMap<ParamNames, double> &);
  void applyParameterFunction(std::function<void(ParamNames)> paramFun) const;
  std::string buildExpDecayFunctionString() const;
  std::string buildStretchExpFunctionString() const;
  std::string buildBackgroundFunctionString() const;

  MultiDomainFunctionModel m_model;
  int m_numberOfExponentials = 0;
  bool m_hasStretchExponential = false;
  QString m_background;
  bool m_isStretchGlobal = false;
};

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQT_INDIRECT_IQTFUNCTIONMODEL_H_ */
