// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "IqtFunctionModel.h"
#include "MantidAPI/IFunction.h"
#include "MantidAPI/FunctionFactory.h"

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

using namespace MantidWidgets;
using namespace Mantid::API;

IqtFunctionModel::IqtFunctionModel() {}

void IqtFunctionModel::setNumberOfExponentials(int n) {
  m_numberOfExponentials = n;
  m_model.setFunctionString(buildFunctionString());
  if (n < 2) {
    m_exp2Lifetime = 1.0;
    m_exp2Height = 1.0;
  }
  if (n < 1) {
    m_exp1Lifetime = 1.0;
    m_exp1Height = 1.0;
  }
}

int IqtFunctionModel::getNumberOfExponentials() const
{
  return m_numberOfExponentials;
}

void IqtFunctionModel::setStretchExponential(bool on)
{
  m_hasStretchExponential = on;
  m_model.setFunctionString(buildFunctionString());
  if (!on) {
    m_stretchHeight = 1.0;
    m_stretchLifetime = 1.0;
    m_stretchStretching = 1.0;
  }
}

bool IqtFunctionModel::hasStretchExponential() const
{
  return m_hasStretchExponential;
}

void IqtFunctionModel::setBackground(const QString & name)
{
  m_background = name;
  m_model.setFunctionString(buildFunctionString());
}

void IqtFunctionModel::removeBackground()
{
  m_background.clear();
  m_A0 = 0.0;
  m_model.setFunctionString(buildFunctionString());
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
      setExponentialOne(*fun);
    } else if (name == "StretchExp") {
      setStretchExponential(*fun);
    } else if (name == "FlatBackground") {
      setBackground(*fun);
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
        setExponentialOne(*fun);
      } else {
        setExponentialTwo(*fun);
        areExponentialsSet = true;
      }
    } else if (name == "StretchExp") {
      if (isStretchSet) {
        throw std::runtime_error("Function has wrong structure.");
      }
      setStretchExponential(*fun);
      areExponentialsSet = true;
      isStretchSet = true;
    } else if (name == "FlatBackground") {
      if (isBackgroundSet) {
        throw std::runtime_error("Function has wrong structure.");
      }
      setBackground(*fun);
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
    globals << getStretchPrefix() + "Stretching";
  }
  m_model.setGlobalParameters(globals);
  std::cerr << m_model.getFitFunctionString().toStdString() << std::endl;
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
    functions << QString("name=ExpDecay,Height=%1,Lifetime=%2").arg(m_exp1Height).arg(m_exp1Lifetime);
  }
  if (m_numberOfExponentials > 1) {
    functions << QString("name=ExpDecay,Height=%1,Lifetime=%2").arg(m_exp2Height).arg(m_exp2Lifetime);
  }
  if (m_hasStretchExponential) {
    functions << QString("name=StretchExp,Height=%1,Lifetime=%2,Stretching=%3").arg(m_stretchHeight).arg(m_stretchLifetime).arg(m_stretchStretching);
  }
  if (!m_background.isEmpty()) {
    functions << QString("name=%1,A0=%2").arg(m_background).arg(m_A0);
  }
  return functions.join(";");
}

void IqtFunctionModel::setExponentialOne(const IFunction &fun)
{
  m_numberOfExponentials = 1;
  m_exp1Height = fun.getParameter("Height");
  m_exp1Lifetime = fun.getParameter("Lifetime");
}

void IqtFunctionModel::setExponentialTwo(const IFunction &fun)
{
  m_numberOfExponentials = 2;
  m_exp2Height = fun.getParameter("Height");
  m_exp2Lifetime = fun.getParameter("Lifetime");
}

void IqtFunctionModel::setStretchExponential(const IFunction &fun)
{
  m_hasStretchExponential = true;
  m_stretchHeight = fun.getParameter("Height");
  m_stretchLifetime = fun.getParameter("Lifetime");
  m_stretchStretching = fun.getParameter("Stretching");
}

void IqtFunctionModel::setBackground(const IFunction &fun)
{
  m_background = QString::fromStdString(fun.name());
  m_A0 = fun.getParameter("A0");
}

QString IqtFunctionModel::getStretchPrefix() const
{
  if (m_numberOfExponentials == 0 && m_background.isEmpty()) return "";
  return QString("f%1.").arg(m_numberOfExponentials);
}

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
