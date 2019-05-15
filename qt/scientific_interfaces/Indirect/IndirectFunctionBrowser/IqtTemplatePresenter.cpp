// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "IqtTemplatePresenter.h"
#include "IqtTemplateBrowser.h"

#include <iostream>

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

/**
 * Constructor
 * @param parent :: The parent widget.
 */
IqtTemplatePresenter::IqtTemplatePresenter(IqtTemplateBrowser *view)
    : QObject(view), m_view(view) {
}

void IqtTemplatePresenter::setNumberOfExponentials(int n) {
  if (n < 0) {
    throw std::logic_error("The number of exponents cannot be a negative number.");
  }
  if (n > 2) {
    throw std::logic_error("The number of exponents is limited to 2.");
  }
  auto nCurrent = m_model.getNumberOfExponentials();
  if (n == 0) {
    if (nCurrent == 2) {
      m_view->removeExponentialTwo();
      --nCurrent;
    }
    if (nCurrent == 1) {
      m_view->removeExponentialOne();
      --nCurrent;
    }
  } else if (n == 1) {
    if (nCurrent == 0) {
      m_view->addExponentialOne();
      ++nCurrent;
    } else {
      m_view->removeExponentialTwo();
      --nCurrent;
    }
  } else /*n == 2*/ {
    if (nCurrent == 0) {
      m_view->addExponentialOne();
      ++nCurrent;
    }
    if (nCurrent == 1) {
      m_view->addExponentialTwo();
      ++nCurrent;
    }
  }
  assert(nCurrent == n);
  m_model.setNumberOfExponentials(n);
  updateViewParameters();
  emit functionStructureChanged();
}

void IqtTemplatePresenter::setStretchExponential(bool on)
{
  if (on == m_model.hasStretchExponential()) return;
  if (on) {
    m_view->addStretchExponential();
  } else {
    m_view->removeStretchExponential();
  }
  m_model.setStretchExponential(on);
  emit functionStructureChanged();
}

void IqtTemplatePresenter::setBackground(const QString & name)
{
  if (name == "None") {
    m_view->removeBackground();
    m_model.removeBackground();
  } else if (name == "FlatBackground") {
    m_view->addFlatBackground();
    m_model.setBackground(name);
  } else {
    throw std::logic_error("Browser doesn't support background " + name.toStdString());
  }
  emit functionStructureChanged();
}

void IqtTemplatePresenter::setNumberOfDatasets(int n)
{
  m_model.setNumberOfDatasets(n);
}

int IqtTemplatePresenter::getNumberOfDatasets() const
{
  return m_model.getNumberOfDatasets();
}

void IqtTemplatePresenter::setFunction(const QString & funStr)
{
  m_model.setFunction(funStr);
  emit functionStructureChanged();
}

IFunction_sptr IqtTemplatePresenter::getGlobalFunction() const
{
  return m_model.getGlobalFunction();
}

IFunction_sptr IqtTemplatePresenter::getFunction() const
{
  return m_model.getFunction();
}

QStringList IqtTemplatePresenter::getGlobalParameters() const
{
  return m_model.getGlobalParameters();
}

QStringList IqtTemplatePresenter::getLocalParameters() const
{
  return m_model.getLocalParameters();
}

void IqtTemplatePresenter::setStretchingGlobal(bool on)
{
  m_model.setStretchingGlobal(on);
}

void IqtTemplatePresenter::updateMultiDatasetParameters(const IFunction & fun)
{
  m_model.updateMultiDatasetParameters(fun);
}

void IqtTemplatePresenter::updateViewParameters()
{
}

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
