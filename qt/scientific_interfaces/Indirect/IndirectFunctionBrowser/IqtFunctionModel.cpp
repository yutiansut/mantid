// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "IqtFunctionModel.h"
#include "MantidAPI/IFunction.h"
#include "MantidAPI/FunctionFactory.h"
#include <map>

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

using namespace MantidWidgets;
using namespace Mantid::API;

namespace {
  std::map<IqtFunctionModel::ParamNames, QString> g_paramName{
    {IqtFunctionModel::ParamNames::EXP1_HEIGHT, "Height"},
    {IqtFunctionModel::ParamNames::EXP1_LIFETIME, "Lifetime"},
    {IqtFunctionModel::ParamNames::EXP2_HEIGHT, "Height"},
    {IqtFunctionModel::ParamNames::EXP2_LIFETIME, "Lifetime"},
    {IqtFunctionModel::ParamNames::STRETCH_HEIGHT, "Height"},
    {IqtFunctionModel::ParamNames::STRETCH_LIFETIME, "Lifetime"},
    {IqtFunctionModel::ParamNames::STRETCH_STRETCHING, "Stretching"},
    {IqtFunctionModel::ParamNames::BG_A0, "A0"}
  };
}

IqtFunctionModel::IqtFunctionModel() {}

void IqtFunctionModel::setNumberOfExponentials(int n) {
  auto oldValues = getCurrentValues();
  m_numberOfExponentials = n;
  m_model.setFunctionString(buildFunctionString());
  setCurrentValues(oldValues);
}

int IqtFunctionModel::getNumberOfExponentials() const
{
  return m_numberOfExponentials;
}

void IqtFunctionModel::setStretchExponential(bool on)
{
  auto oldValues = getCurrentValues();
  m_hasStretchExponential = on;
  m_model.setFunctionString(buildFunctionString());
  setCurrentValues(oldValues);
}

bool IqtFunctionModel::hasStretchExponential() const
{
  return m_hasStretchExponential;
}

void IqtFunctionModel::setBackground(const QString & name)
{
  auto oldValues = getCurrentValues();
  m_background = name;
  m_model.setFunctionString(buildFunctionString());
  setCurrentValues(oldValues);
}

void IqtFunctionModel::removeBackground()
{
  auto oldValues = getCurrentValues();
  m_background.clear();
  m_model.setFunctionString(buildFunctionString());
  setCurrentValues(oldValues);
}

void IqtFunctionModel::setNumberOfDatasets(int n)
{
  m_model.setNumberDomains(n);
}

int IqtFunctionModel::getNumberOfDatasets() const
{
  return m_model.getNumberDomains();
}

void IqtFunctionModel::setFunction(const QString & funStr)
{
  clear();
  if (funStr.isEmpty()) {
    m_model.setFunctionString(funStr);
    return;
  }
  auto fun = FunctionFactory::Instance().createInitialized(funStr.toStdString());
  if (fun->nFunctions() == 0) {
    auto const name = fun->name();
    if (name == "ExpDecay") {
      m_numberOfExponentials = 1;
    } else if (name == "StretchExp") {
      m_hasStretchExponential = true;
    } else if (name == "FlatBackground") {
      m_background = QString::fromStdString(name);
    } else {
      throw std::runtime_error("Cannot set function " + name);
    }
    m_model.setFunctionString(funStr);
    return;
  }
  bool areExponentialsSet = false;
  bool isStretchSet = false;
  bool isBackgroundSet = false;
  for (size_t i = 0; i < fun->nFunctions(); ++i) {
    auto f = fun->getFunction(i);
    auto const name = f->name();
    if (name == "ExpDecay") {
      if (areExponentialsSet) {
        throw std::runtime_error("Function has wrong structure.");
      }
      if (m_numberOfExponentials == 0) {
        m_numberOfExponentials = 1;
      } else {
        m_numberOfExponentials = 2;
        areExponentialsSet = true;
      }
    } else if (name == "StretchExp") {
      if (isStretchSet) {
        throw std::runtime_error("Function has wrong structure.");
      }
      m_hasStretchExponential = true;
      areExponentialsSet = true;
      isStretchSet = true;
    } else if (name == "FlatBackground") {
      if (isBackgroundSet) {
        throw std::runtime_error("Function has wrong structure.");
      }
      m_background = QString::fromStdString(name);
      areExponentialsSet = true;
      isStretchSet = true;
      isBackgroundSet = true;
    } else {
      throw std::runtime_error("Function has wrong structure.");
    }
  }
  m_model.setFunctionString(funStr);
}

IFunction_sptr IqtFunctionModel::getGlobalFunction() const
{
  return m_model.getFitFunction();
}

IFunction_sptr IqtFunctionModel::getFunction() const
{
  auto fun = m_model.getCurrentFunction();
  return m_model.getCurrentFunction();
}

QStringList IqtFunctionModel::getGlobalParameters() const
{
  return m_model.getGlobalParameters();
}

QStringList IqtFunctionModel::getLocalParameters() const
{
  return m_model.getLocalParameters();
}

void IqtFunctionModel::setStretchingGlobal(bool on)
{
  m_isStretchGlobal = on;
  QStringList globals;
  if (on) {
    globals << *getStretchPrefix() + "Stretching";
  }
  m_model.setGlobalParameters(globals);
}

void IqtFunctionModel::updateMultiDatasetParameters(const IFunction & fun)
{
  m_model.updateMultiDatasetParameters(fun);
  setStretchingGlobal(m_isStretchGlobal);
}

void IqtFunctionModel::setCurrentDataset(int i)
{
  m_model.setCurrentDomainIndex(i);
}

void IqtFunctionModel::setParameter(ParamNames name, double value)
{
  auto const prefix = getPrefix(name);
  if (prefix) {
    m_model.setParameter(*prefix + g_paramName.at(name), value);
  }
}

double IqtFunctionModel::getParameter(ParamNames name) const
{
  return m_model.getParameter(*getPrefix(name) + g_paramName.at(name));
}

boost::optional<QString> IqtFunctionModel::getPrefix(ParamNames name) const
{
  if (name <= ParamNames::EXP1_LIFETIME) {
    return getExp1Prefix();
  } else if (name <= ParamNames::EXP2_LIFETIME) {
    return getExp2Prefix();
  } else if (name <= ParamNames::STRETCH_STRETCHING) {
    return getStretchPrefix();
  } else {
    return getBackgroundPrefix();
  }
}

std::map<IqtFunctionModel::ParamNames, double> IqtFunctionModel::getCurrentValues() const
{
  std::map<ParamNames, double> values;
  auto store = [&values, this](ParamNames name) {values[name] = getParameter(name); };
  if (m_numberOfExponentials > 0) {
    store(ParamNames::EXP1_HEIGHT);
    store(ParamNames::EXP1_LIFETIME);
  }
  if (m_numberOfExponentials > 1) {
    store(ParamNames::EXP2_HEIGHT);
    store(ParamNames::EXP2_LIFETIME);
  }
  if (m_hasStretchExponential) {
    store(ParamNames::STRETCH_HEIGHT);
    store(ParamNames::STRETCH_LIFETIME);
    store(ParamNames::STRETCH_STRETCHING);
  }
  if (!m_background.isEmpty()) {
    store(ParamNames::BG_A0);
  }
  return values;
}

void IqtFunctionModel::setCurrentValues(const std::map<ParamNames, double>& values)
{
  for (auto const v : values) {
    setParameter(v.first, v.second);
  }
}

void IqtFunctionModel::clear() {
  setNumberOfExponentials(0);
  setStretchExponential(false);
  removeBackground();
}

QString IqtFunctionModel::buildFunctionString() const
{
  QStringList functions;
  if (m_numberOfExponentials > 0) {
    functions << "name=ExpDecay,Height=1,Lifetime=1";
  }
  if (m_numberOfExponentials > 1) {
    functions << "name=ExpDecay,Height=1,Lifetime=1";
  }
  if (m_hasStretchExponential) {
    functions << "name=StretchExp,Height=1,Lifetime=1,Stretching=1";
  }
  if (!m_background.isEmpty()) {
    functions << "name=FlatBackground,A0=0";
  }
  return functions.join(";");
}

boost::optional<QString> IqtFunctionModel::getExp1Prefix() const
{
  if (m_numberOfExponentials == 0) return boost::optional<QString>();
  if (m_numberOfExponentials == 1 && !m_hasStretchExponential && m_background.isEmpty())
    return "";
  return "f0.";
}

boost::optional<QString> IqtFunctionModel::getExp2Prefix() const
{
  if (m_numberOfExponentials < 2) return boost::optional<QString>();
  return "f1.";
}

boost::optional<QString> IqtFunctionModel::getStretchPrefix() const
{
  if (!m_hasStretchExponential) return boost::optional<QString>();
  if (m_numberOfExponentials == 0 && m_background.isEmpty()) return "";
  return QString("f%1.").arg(m_numberOfExponentials);
}

boost::optional<QString> IqtFunctionModel::getBackgroundPrefix() const
{
  if (m_background.isEmpty()) return boost::optional<QString>();
  if (m_numberOfExponentials == 0 && !m_hasStretchExponential)
    return "";
  return QString("f%1.").arg(m_numberOfExponentials + (m_hasStretchExponential ? 1 : 0));
}

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
