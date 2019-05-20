// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "IqtTemplateBrowser.h"

#include "MantidAPI/CostFunctionFactory.h"
#include "MantidAPI/FuncMinimizerFactory.h"
#include "MantidAPI/IAlgorithm.h"
#include "MantidAPI/IFuncMinimizer.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/IWorkspaceProperty.h"
#include "MantidKernel/PropertyWithValue.h"

#include "MantidQtWidgets/Common/QtPropertyBrowser/qtpropertymanager.h"
#include "MantidQtWidgets/Common/QtPropertyBrowser/qttreepropertybrowser.h"
#include "MantidQtWidgets/Common/QtPropertyBrowser/ButtonEditorFactory.h"
#include "MantidQtWidgets/Common/QtPropertyBrowser/CompositeEditorFactory.h"
#include "MantidQtWidgets/Common/QtPropertyBrowser/DoubleEditorFactory.h"
#include "MantidQtWidgets/Common/QtPropertyBrowser/qteditorfactory.h"

#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

#include <limits>
#include <iostream>

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

/**
 * Constructor
 * @param parent :: The parent widget.
 */
IqtTemplateBrowser::IqtTemplateBrowser(QWidget *parent)
    : FunctionTemplateBrowser(parent), m_presenter(this) {
  connect(&m_presenter, SIGNAL(functionStructureChanged()), this, SIGNAL(functionStructureChanged()));
  m_parameterMap[m_exp1Height] = 0;
  m_parameterMap[m_exp1Lifetime] = 1;
  m_parameterMap[m_exp2Height] = 2;
  m_parameterMap[m_exp2Lifetime] = 3;
  m_parameterMap[m_stretchExpHeight] = 4;
  m_parameterMap[m_stretchExpLifetime] = 5;
  m_parameterMap[m_stretchExpStretching] = 6;
  m_parameterMap[m_A0] = 7;
}

void IqtTemplateBrowser::addExponentialOne() {
  m_exp1Height = m_parameterManager->addProperty("f0.Height");
  m_parameterManager->setDecimals(m_exp1Height, 6);
  m_exp1Lifetime = m_parameterManager->addProperty("f0.Lifetime");
  m_parameterManager->setDecimals(m_exp1Lifetime, 6);
  m_numberOfExponentials->addSubProperty(m_exp1Height);
  m_numberOfExponentials->addSubProperty(m_exp1Lifetime);
}

void IqtTemplateBrowser::removeExponentialOne() {
  m_numberOfExponentials->removeSubProperty(m_exp1Height);
  m_numberOfExponentials->removeSubProperty(m_exp1Lifetime);
  m_exp1Height = m_exp1Lifetime = nullptr;
}

void IqtTemplateBrowser::addExponentialTwo()
{
  m_exp2Height = m_parameterManager->addProperty("f1.Height");
  m_parameterManager->setDecimals(m_exp2Height, 6);
  m_exp2Lifetime = m_parameterManager->addProperty("f1.Lifetime");
  m_parameterManager->setDecimals(m_exp2Lifetime, 6);
  m_numberOfExponentials->addSubProperty(m_exp2Height);
  m_numberOfExponentials->addSubProperty(m_exp2Lifetime);
}

void IqtTemplateBrowser::removeExponentialTwo()
{
  m_numberOfExponentials->removeSubProperty(m_exp2Height);
  m_numberOfExponentials->removeSubProperty(m_exp2Lifetime);
  m_exp2Height = m_exp2Lifetime = nullptr;
}

void IqtTemplateBrowser::addStretchExponential()
{
  if (m_stretchExpHeight) return;
  m_stretchExpHeight = m_parameterManager->addProperty("Height");
  m_parameterManager->setDecimals(m_stretchExpHeight, 6);
  m_stretchExpLifetime = m_parameterManager->addProperty("Lifetime");
  m_parameterManager->setDecimals(m_stretchExpLifetime, 6);
  m_stretchExpStretching = m_parameterManager->addProperty("Stretching");
  m_parameterManager->setDecimals(m_stretchExpStretching, 6);
  m_stretchExponential->addSubProperty(m_stretchExpHeight);
  m_stretchExponential->addSubProperty(m_stretchExpLifetime);
  m_stretchExponential->addSubProperty(m_stretchExpStretching);
}

void IqtTemplateBrowser::removeStretchExponential()
{
  if (!m_stretchExpHeight) return;
  m_stretchExponential->removeSubProperty(m_stretchExpHeight);
  m_stretchExponential->removeSubProperty(m_stretchExpLifetime);
  m_stretchExponential->removeSubProperty(m_stretchExpStretching);
  m_stretchExpHeight = m_stretchExpLifetime = m_stretchExpStretching = nullptr;
}

void IqtTemplateBrowser::addFlatBackground()
{
  if (m_A0) return;
  m_A0 = m_parameterManager->addProperty("A0");
  m_parameterManager->setDecimals(m_A0, 6);
  m_background->addSubProperty(m_A0);
}

void IqtTemplateBrowser::removeBackground()
{
  if (!m_A0) return;
  m_background->removeSubProperty(m_A0);
  m_A0 = nullptr;
}

void IqtTemplateBrowser::setExp1Height(double value)
{
  setParameterPropertyValue(m_exp1Height, value);
}

void IqtTemplateBrowser::setExp1Lifetime(double value)
{
  setParameterPropertyValue(m_exp1Lifetime, value);
}

void IqtTemplateBrowser::setExp2Height(double value)
{
  setParameterPropertyValue(m_exp2Height, value);
}

void IqtTemplateBrowser::setExp2Lifetime(double value)
{
  setParameterPropertyValue(m_exp2Lifetime, value);
}

void IqtTemplateBrowser::setStretchHeight(double value)
{
  setParameterPropertyValue(m_stretchExpHeight, value);
}

void IqtTemplateBrowser::setStretchLifetime(double value)
{
  setParameterPropertyValue(m_stretchExpLifetime, value);
}

void IqtTemplateBrowser::setStretchStretching(double value)
{
  setParameterPropertyValue(m_stretchExpStretching, value);
}

void IqtTemplateBrowser::setA0(double value)
{
  setParameterPropertyValue(m_A0, value);
}

void IqtTemplateBrowser::setFunction(const QString & funStr)
{
  m_presenter.setFunction(funStr);
}

IFunction_sptr IqtTemplateBrowser::getGlobalFunction() const
{
  return m_presenter.getGlobalFunction();
}

IFunction_sptr IqtTemplateBrowser::getFunction() const
{
  return m_presenter.getFunction();
}

void IqtTemplateBrowser::setNumberOfDatasets(int n)
{
  m_presenter.setNumberOfDatasets(n);
}

int IqtTemplateBrowser::getNumberOfDatasets() const
{
  return m_presenter.getNumberOfDatasets();
}

QStringList IqtTemplateBrowser::getGlobalParameters() const
{
  return m_presenter.getGlobalParameters();
}

QStringList IqtTemplateBrowser::getLocalParameters() const
{
  return m_presenter.getLocalParameters();
}

void IqtTemplateBrowser::intChanged(QtProperty *prop) {
  if (prop == m_numberOfExponentials) {
    m_presenter.setNumberOfExponentials(m_intManager->value(prop));
  }
}

void IqtTemplateBrowser::boolChanged(QtProperty *prop)
{
  if (prop == m_stretchExponential) {
    m_presenter.setStretchExponential(m_boolManager->value(prop));
  }
}

void IqtTemplateBrowser::enumChanged(QtProperty *prop)
{
  if (prop == m_background) {
    auto background = m_enumManager->enumNames(prop)[m_enumManager->value(prop)];
    m_presenter.setBackground(background);
  }
}

void IqtTemplateBrowser::globalChanged(QtProperty *prop, const QString &name, bool on)
{
  std::cerr << "Global " << name.toStdString() << ' ' << on << std::endl;
}

void IqtTemplateBrowser::parameterChanged(QtProperty *prop)
{
  if (prop == m_stretchExpStretching) {
    auto isGlobal = m_parameterManager->isGlobal(prop);
    m_presenter.setStretchingGlobal(isGlobal);
    emit functionStructureChanged();
  }
}

void IqtTemplateBrowser::parameterButtonClicked(QtProperty *prop)
{
  //std::cerr << "Local " << prop->propertyName().toStdString() << ' ' << m_actualParameterNames[prop].toStdString() << std::endl;
  //emit localParameterButtonClicked(m_actualParameterNames[prop]);
}

void IqtTemplateBrowser::updateMultiDatasetParameters(const IFunction & fun)
{
  m_presenter.updateMultiDatasetParameters(fun);
}

void IqtTemplateBrowser::updateMultiDatasetParameters(const ITableWorkspace & paramTable)
{
  m_presenter.updateMultiDatasetParameters(paramTable);
}

void IqtTemplateBrowser::updateParameters(const IFunction & fun)
{
  m_presenter.updateParameters(fun);
}

void IqtTemplateBrowser::setCurrentDataset(int i)
{
  m_presenter.setCurrentDataset(i);
}

void IqtTemplateBrowser::updateParameterNames(const QMap<int, QString>& parameterNames)
{
  m_actualParameterNames.clear();
  for (auto const prop : m_parameterMap.keys()) {
    auto const i = m_parameterMap[prop];
    m_actualParameterNames[prop] = parameterNames[i];
  }
}

void IqtTemplateBrowser::createProperties()
{
  m_numberOfExponentials = m_intManager->addProperty("Exponentials");
  m_intManager->setMinimum(m_numberOfExponentials, 0);
  m_intManager->setMaximum(m_numberOfExponentials, 2);
  m_browser->addProperty(m_numberOfExponentials);

  m_stretchExponential = m_boolManager->addProperty("Stretch Exponential");
  m_browser->addProperty(m_stretchExponential);

  m_background = m_enumManager->addProperty("Background");
  QStringList backgrounds; backgrounds << "None" << "FlatBackground";
  m_enumManager->setEnumNames(m_background, backgrounds);
  m_browser->addProperty(m_background);
}

void IqtTemplateBrowser::popupMenu(const QPoint &) {
  std::cerr << "Popup" << std::endl;
}

void IqtTemplateBrowser::setParameterPropertyValue(QtProperty * prop, double value)
{
  if (prop) m_parameterManager->setValue(prop, value);
}

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
