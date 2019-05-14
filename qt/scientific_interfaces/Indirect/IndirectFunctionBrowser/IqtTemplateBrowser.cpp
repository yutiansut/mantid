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

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
