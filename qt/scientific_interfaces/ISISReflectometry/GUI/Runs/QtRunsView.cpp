// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "QtRunsView.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidQtIcons/Icon.h"
#include "MantidQtWidgets/Common/AlgorithmRunner.h"
#include "MantidQtWidgets/Common/FileDialogHandler.h"
#include "MantidQtWidgets/Common/HelpWindow.h"
#include "MantidQtWidgets/Common/HintingLineEditFactory.h"
#include "MantidQtWidgets/Common/SlitCalculator.h"
#include <QMenu>
#include <QMessageBox>

namespace MantidQt {
namespace CustomInterfaces {
namespace ISISReflectometry {
using namespace Mantid::API;
using namespace MantidQt::MantidWidgets;
using namespace MantidQt::Icons;

//----------------------------------------------------------------------------------------------
/** Constructor
 * @param parent :: The parent of this view
 * @param makeRunsTableView :: The factory for the RunsTableView.
 */
QtRunsView::QtRunsView(QWidget *parent, RunsTableViewFactory makeRunsTableView)
    : MantidWidget(parent), m_notifyee(nullptr), m_timerNotifyee(nullptr),
      m_searchNotifyee(nullptr), m_searchModel(),
      m_calculator(new SlitCalculator(this)), m_tableView(makeRunsTableView()),
      m_timer() {
  initLayout();
  ui.tableSearchResults->setModel(&m_searchModel);
}

void QtRunsView::subscribe(RunsViewSubscriber *notifyee) {
  m_notifyee = notifyee;
}

void QtRunsView::subscribeTimer(RunsViewTimerSubscriber *notifyee) {
  m_timerNotifyee = notifyee;
}

void QtRunsView::subscribeSearch(RunsViewSearchSubscriber *notifyee) {
  m_searchNotifyee = notifyee;
}

IRunsTableView *QtRunsView::table() const { return m_tableView; }

/**
Initialise the Interface
*/
void QtRunsView::initLayout() {
  ui.setupUi(this);

  ui.buttonTransfer->setDefaultAction(ui.actionTransfer);

  // Expand the process runs column at the expense of the search column
  ui.splitterTables->setStretchFactor(0, 0);
  ui.splitterTables->setStretchFactor(1, 1);
  ui.tablePane->layout()->addWidget(m_tableView);

  // Add Icons to the buttons
  ui.actionAutoreducePause->setIcon(getIcon("mdi.pause", "red", 1.3));
  ui.buttonAutoreduce->setIcon(getIcon("mdi.play", "green", 1.3));
  ui.buttonAutoreducePause->setIcon(getIcon("mdi.pause", "red", 1.3));
  ui.buttonMonitor->setIcon(getIcon("mdi.play", "green", 1.3));
  ui.buttonStopMonitor->setIcon(getIcon("mdi.pause", "red", 1.3));
  ui.actionAutoreduce->setIcon(getIcon("mdi.play", "green", 1.3));
  ui.actionSearch->setIcon(getIcon("mdi.folder", "black", 1.3));
  ui.actionTransfer->setIcon(getIcon("mdi.file-move", "black", 1.3));

  m_algoRunner = boost::make_shared<MantidQt::API::AlgorithmRunner>(this);
  m_monitorAlgoRunner =
      boost::make_shared<MantidQt::API::AlgorithmRunner>(this);

  // Custom context menu for table
  connect(ui.searchPane, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(onShowSearchContextMenuRequested(const QPoint &)));
  // Synchronize the slit calculator
  connect(ui.comboSearchInstrument, SIGNAL(currentIndexChanged(int)), this,
          SLOT(onInstrumentChanged(int)));
  // Connect signal for when search algorithm completes
  connect(m_algoRunner.get(), SIGNAL(algorithmComplete(bool)), this,
          SLOT(onSearchComplete()), Qt::UniqueConnection);
}

/**
 * Updates actions in the menus to be enabled or disabled
 * according to whether processing is running or not.
 * @param isProcessing: Whether processing is running
 */
void QtRunsView::updateMenuEnabledState(bool isProcessing) {
  UNUSED_ARG(isProcessing)
}

/**
 * Sets the "Autoreduce" button enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setAutoreduceButtonEnabled(bool enabled) {

  ui.buttonAutoreduce->setEnabled(enabled);
}

/**
 * Sets the "Autoreduce" button enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setAutoreducePauseButtonEnabled(bool enabled) {

  ui.buttonAutoreducePause->setEnabled(enabled);
}

/**
 * Sets the "Transfer" button enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setTransferButtonEnabled(bool enabled) {

  ui.buttonTransfer->setEnabled(enabled);
}

/**
 * Sets the "Instrument" combo box enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setInstrumentComboEnabled(bool enabled) {

  ui.comboSearchInstrument->setEnabled(enabled);
}

/**
 * Sets the search text box enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setSearchTextEntryEnabled(bool enabled) {

  ui.textSearch->setEnabled(enabled);
}

/**
 * Sets the search button enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setSearchButtonEnabled(bool enabled) {

  ui.buttonSearch->setEnabled(enabled);
}

/**
 * Sets the start-monitor button enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setStartMonitorButtonEnabled(bool enabled) {

  ui.buttonMonitor->setEnabled(enabled);
}

/**
 * Sets the stop-monitor enabled or disabled
 * @param enabled : Whether to enable or disable the button
 */
void QtRunsView::setStopMonitorButtonEnabled(bool enabled) {

  ui.buttonStopMonitor->setEnabled(enabled);
}

/**
Set the list of available instruments to search for and updates the list of
available instruments in the table view
@param instruments : The list of instruments available
@param defaultInstrumentIndex : The index of the instrument to have selected by
default
*/
void QtRunsView::setInstrumentList(const std::vector<std::string> &instruments,
                                   int defaultInstrumentIndex) {
  ui.comboSearchInstrument->clear();
  for (auto &&instrument : instruments)
    ui.comboSearchInstrument->addItem(QString::fromStdString(instrument));
  ui.comboSearchInstrument->setCurrentIndex(defaultInstrumentIndex);
}

/**
Set the range of the progress bar
@param min : The minimum value of the bar
@param max : The maxmimum value of the bar
*/
void QtRunsView::setProgressRange(int min, int max) {
  ui.progressBar->setRange(min, max);
  ProgressableView::setProgressRange(min, max);
}

/**
Set the status of the progress bar
@param progress : The current value of the bar
*/
void QtRunsView::setProgress(int progress) {
  ui.progressBar->setValue(progress);
}

/**
 * Clear the progress
 */
void QtRunsView::clearProgress() { ui.progressBar->reset(); }

/**
 * Resize the search results table columns
 */
void QtRunsView::resizeSearchResultsColumnsToContents() {
  ui.tableSearchResults->resizeColumnsToContents();
}

/**
 * Get the model containing the search results
 */
ISearchModel const &QtRunsView::searchResults() { return m_searchModel; }

ISearchModel &QtRunsView::mutableSearchResults() { return m_searchModel; }

/**
This slot notifies the presenter that the ICAT search was completed
*/
void QtRunsView::onSearchComplete() {
  m_searchNotifyee->notifySearchComplete();
}

/**
This slot notifies the presenter that the "search" button has been pressed
*/
void QtRunsView::on_actionSearch_triggered() { m_notifyee->notifySearch(); }

/**
This slot conducts a search operation before notifying the presenter that the
"autoreduce" button has been pressed
*/
void QtRunsView::on_actionAutoreduce_triggered() {
  m_notifyee->notifyAutoreductionResumed();
}

/**
This slot conducts a search operation before notifying the presenter that the
"pause autoreduce" button has been pressed
*/
void QtRunsView::on_actionAutoreducePause_triggered() {
  m_notifyee->notifyAutoreductionPaused();
}

/**
This slot notifies the presenter that the "transfer" button has been pressed
*/
void QtRunsView::on_actionTransfer_triggered() { m_notifyee->notifyTransfer(); }

/**
This slot shows the slit calculator
*/
void QtRunsView::onShowSlitCalculatorRequested() {
  m_calculator->setCurrentInstrumentName(
      ui.comboSearchInstrument->currentText().toStdString());
  m_calculator->show();
}

/**
This slot is triggered when the user right clicks on the search results table
@param pos : The position of the right click within the table
*/
void QtRunsView::onShowSearchContextMenuRequested(const QPoint &pos) {
  if (!ui.tableSearchResults->indexAt(pos).isValid())
    return;

  // parent widget takes ownership of QMenu
  QMenu *menu = new QMenu(this);
  menu->addAction(ui.actionTransfer);
  menu->popup(ui.tableSearchResults->viewport()->mapToGlobal(pos));
}

/** This is slot is triggered when any of the instrument combo boxes changes. It
 * notifies the main presenter and updates the Slit Calculator
 * @param index : The index of the combo box
 */
void QtRunsView::onInstrumentChanged(int index) {
  ui.textSearch->clear();
  m_calculator->setCurrentInstrumentName(
      ui.comboSearchInstrument->itemText(index).toStdString());
  m_calculator->processInstrumentHasBeenChanged();
  m_notifyee->notifyInstrumentChanged();
}

/**
Get the selected instrument for searching
@returns the selected instrument to search for
*/
std::string QtRunsView::getSearchInstrument() const {
  return ui.comboSearchInstrument->currentText().toStdString();
}

void QtRunsView::setSearchInstrument(std::string const &instrumentName) {
  setSelected(*ui.comboSearchInstrument, instrumentName);
}

/**
Get the indices of the highlighted search result rows
@returns a set of ints containing the selected row numbers
*/
std::set<int> QtRunsView::getSelectedSearchRows() const {
  std::set<int> rows;
  auto selectionModel = ui.tableSearchResults->selectionModel();
  if (selectionModel) {
    auto selectedRows = selectionModel->selectedRows();
    for (auto it = selectedRows.begin(); it != selectedRows.end(); ++it)
      rows.insert(it->row());
  }
  return rows;
}

/**
Get the indices of all search result rows
@returns a set of ints containing the row numbers
*/
std::set<int> QtRunsView::getAllSearchRows() const {
  std::set<int> rows;
  if (!ui.tableSearchResults || !ui.tableSearchResults->model())
    return rows;
  auto const rowCount = ui.tableSearchResults->model()->rowCount();
  for (auto row = 0; row < rowCount; ++row)
    rows.insert(row);
  return rows;
}

boost::shared_ptr<MantidQt::API::AlgorithmRunner>
QtRunsView::getAlgorithmRunner() const {
  return m_algoRunner;
}

boost::shared_ptr<MantidQt::API::AlgorithmRunner>
QtRunsView::getMonitorAlgorithmRunner() const {
  return m_monitorAlgoRunner;
}

/**
Get the string the user wants to search for.
@returns The search string
*/
std::string QtRunsView::getSearchString() const {
  return ui.textSearch->text().toStdString();
}

void QtRunsView::on_buttonMonitor_clicked() { startMonitor(); }

void QtRunsView::on_buttonStopMonitor_clicked() { stopMonitor(); }

/** Start live data monitoring
 */
void QtRunsView::startMonitor() {
  m_monitorAlgoRunner.get()->disconnect(); // disconnect any other connections
  m_notifyee->notifyStartMonitor();
  connect(m_monitorAlgoRunner.get(), SIGNAL(algorithmComplete(bool)), this,
          SLOT(onStartMonitorComplete()), Qt::UniqueConnection);
}

/**
This slot notifies the presenter that the monitoring algorithm finished
*/
void QtRunsView::onStartMonitorComplete() {
  m_notifyee->notifyStartMonitorComplete();
}

/** Stop live data monitoring
 */
void QtRunsView::stopMonitor() { m_notifyee->notifyStopMonitor(); }

/** Set a combo box to the given value
 */
void QtRunsView::setSelected(QComboBox &box, std::string const &str) {
  auto const index = box.findText(QString::fromStdString(str));
  if (index != -1)
    box.setCurrentIndex(index);
}

/**
   This slot is called each time the timer times out
*/
void QtRunsView::timerEvent(QTimerEvent *event) {
  if (event->timerId() == m_timer.timerId()) {
    if (m_timerNotifyee)
      m_timerNotifyee->notifyTimerEvent();
  } else {
    QWidget::timerEvent(event);
  }
}

/** start the timer
 */
void QtRunsView::startTimer(const int millisecs) {
  m_timer.start(millisecs, this);
}

/** stop
 */
void QtRunsView::stopTimer() { m_timer.stop(); }
} // namespace ISISReflectometry
} // namespace CustomInterfaces
} // namespace MantidQt
