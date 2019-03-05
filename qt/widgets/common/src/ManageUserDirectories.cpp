// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidQtWidgets/Common/ManageUserDirectories.h"
#include "MantidKernel/ConfigService.h"
#include "MantidQtWidgets/Common/HelpWindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QUrl>

using namespace MantidQt::API;

static constexpr auto DATASEARCH_DIRECTORIES = "datasearch.directories";
static constexpr auto DATASEARCH_SEARCHARCHIVE = "datasearch.searcharchive";
static constexpr auto DEFAULTSAVE_DIRECTORY = "defaultsave.directory";
static constexpr auto PYTHONSCRIPTS_DIRECTORIES = "pythonscripts.directories";
static constexpr auto USER_PYTHON_PLUGINS_DIRECTORIES =
    "user.python.plugins.directories";

ManageUserDirectories::ManageUserDirectories(QWidget *parent,
                                             bool keepPythonExtensions)
    : QDialog(parent), keepPythonExtensions(keepPythonExtensions) {
  setAttribute(Qt::WA_DeleteOnClose);
  m_uiForm.setupUi(this);
  initLayout();

  if (!keepPythonExtensions) {
    // if the python extensions tab is being removed, just remove the widget
    // from the tab widget
    m_uiForm.tabWidget->removeTab(m_uiForm.tabWidget->count() - 1);
  } else {
    // if the tab is being shown, then initialise all the connections so that it
    // works as expected
    initConnectionsForPythonExtensions();
  }
}

void ManageUserDirectories::initLayout() {
  loadProperties();

  // Make Connections
  connect(m_uiForm.pbHelp, SIGNAL(clicked()), this, SLOT(helpClicked()));
  connect(m_uiForm.pbCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(m_uiForm.pbConfirm, SIGNAL(clicked()), this, SLOT(confirmClicked()));

  connect(m_uiForm.pbAddDirectory, SIGNAL(clicked()), this,
          SLOT(addDirectory()));
  connect(m_uiForm.pbAddDirectoryPython, SIGNAL(clicked()), this,
          SLOT(addDirectory()));

  connect(m_uiForm.pbBrowseToDir, SIGNAL(clicked()), this,
          SLOT(browseToDirectory()));
  connect(m_uiForm.pbBrowseToDirPython, SIGNAL(clicked()), this,
          SLOT(browseToDirectory()));
  connect(m_uiForm.pbRemDir, SIGNAL(clicked()), this, SLOT(remDir()));
  connect(m_uiForm.pbRemDirPython, SIGNAL(clicked()), this, SLOT(remDir()));

  connect(m_uiForm.pbMoveUp, SIGNAL(clicked()), this, SLOT(moveUp()));
  connect(m_uiForm.pbMoveUpPython, SIGNAL(clicked()), this, SLOT(moveUp()));

  connect(m_uiForm.pbMoveDown, SIGNAL(clicked()), this, SLOT(moveDown()));
  connect(m_uiForm.pbMoveDownPython, SIGNAL(clicked()), this, SLOT(moveDown()));

  connect(m_uiForm.pbSaveBrowse, SIGNAL(clicked()), this,
          SLOT(selectSaveDir()));
}

void ManageUserDirectories::loadFromStringIntoList(const std::string &keyName,
                                                   QListWidget *listWidget) {
  auto directories =
      QString::fromStdString(
          Mantid::Kernel::ConfigService::Instance().getString(keyName))
          .trimmed();
  const auto list = directories.split(";", QString::SkipEmptyParts);
  listWidget->clear();
  listWidget->addItems(list);
}

void ManageUserDirectories::loadExtensionProperties() {
  // get data search directories and populate the list widget (lwDataSearchDirs)
  loadFromStringIntoList(USER_PYTHON_PLUGINS_DIRECTORIES,
                         m_uiForm.lwDataSearchDirs);
}

void ManageUserDirectories::initConnectionsForPythonExtensions() {
  loadExtensionProperties();
  connect(m_uiForm.pbAddDirectoryExtensions, SIGNAL(clicked()), this,
          SLOT(addDirectory()));
  connect(m_uiForm.pbBrowseToDirExtensions, SIGNAL(clicked()), this,
          SLOT(browseToDirectory()));
  connect(m_uiForm.pbRemDirExtensions, SIGNAL(clicked()), this, SLOT(remDir()));
  connect(m_uiForm.pbMoveUpExtensions, SIGNAL(clicked()), this, SLOT(moveUp()));
  connect(m_uiForm.pbMoveDownExtensions, SIGNAL(clicked()), this,
          SLOT(moveDown()));
}

void ManageUserDirectories::loadProperties() {
  m_userPropFile =
      QString::fromStdString(
          Mantid::Kernel::ConfigService::Instance().getUserFilename())
          .trimmed();

  // get data search directories and populate the list widget (lwDataSearchDirs)
  loadFromStringIntoList(DATASEARCH_DIRECTORIES, m_uiForm.lwDataSearchDirs);

  // Do the same thing for the "pythonscripts.directories" property.
  loadFromStringIntoList(PYTHONSCRIPTS_DIRECTORIES, m_uiForm.lwUserSearchDirs);

  // set flag of whether to search the data archive
  auto archive = QString::fromStdString(
                     Mantid::Kernel::ConfigService::Instance().getString(
                         DATASEARCH_SEARCHARCHIVE))
                     .trimmed()
                     .toLower();
  const auto defaultFacility =
      QString::fromStdString(
          Mantid::Kernel::ConfigService::Instance().getString(
              "default.facility"))
          .trimmed()
          .toUpper();
  m_uiForm.cbSearchArchive->addItem(QString("default facility only - ") +
                                    defaultFacility);
  m_uiForm.cbSearchArchive->addItem("all");
  m_uiForm.cbSearchArchive->addItem("off");
  if (archive == "on") {
    m_uiForm.cbSearchArchive->setCurrentIndex(0);
  } else if (archive == "all") {
    m_uiForm.cbSearchArchive->setCurrentIndex(1);
  } else if (archive == "off") {
    m_uiForm.cbSearchArchive->setCurrentIndex(2);
  } else { // only add custom if it has been set
    m_uiForm.cbSearchArchive->addItem("custom - " + archive.toUpper());
    m_uiForm.cbSearchArchive->setCurrentIndex(3);
  }

  // default save directory
  const auto &saveDir = QString::fromStdString(
                            Mantid::Kernel::ConfigService::Instance().getString(
                                DEFAULTSAVE_DIRECTORY))
                            .trimmed();
  m_uiForm.leDefaultSave->setText(saveDir);
}

void ManageUserDirectories::savePropertiesForPythonExtensions(
    Mantid::Kernel::ConfigServiceImpl &config) const {
  QStringList extensionDirs;
  extensionDirs.reserve(m_uiForm.lwExtensionsSearchDirs->count());

  for (int i{0}; i < m_uiForm.lwExtensionsSearchDirs->count(); i++) {
    auto dir = m_uiForm.lwExtensionsSearchDirs->item(i)->text();
    appendSlashIfNone(dir);
    extensionDirs.append(dir);
  }
  const auto &newExtensionDirs = extensionDirs.join(";").replace('\\', '/');
  config.setString(USER_PYTHON_PLUGINS_DIRECTORIES,
                   newExtensionDirs.toStdString());
}

void ManageUserDirectories::saveProperties() const {
  QString newSearchArchive = m_uiForm.cbSearchArchive->currentText().toLower();
  if (newSearchArchive == "all" || newSearchArchive == "off") {
    // do nothing
  } else if (newSearchArchive.startsWith("default facility only")) {
    newSearchArchive = "on";
  } else {
    // the only way "custom" gets set is by using the value in ConfigService
    // already, so just copy it
    newSearchArchive = QString::fromStdString(
                           Mantid::Kernel::ConfigService::Instance().getString(
                               DATASEARCH_SEARCHARCHIVE))
                           .trimmed()
                           .toLower();
  }

  QStringList dataDirs;
  dataDirs.reserve(m_uiForm.lwDataSearchDirs->count());

  for (int i = 0; i < m_uiForm.lwDataSearchDirs->count(); i++) {
    auto dir = m_uiForm.lwDataSearchDirs->item(i)->text();
    appendSlashIfNone(dir);
    dataDirs.append(dir);
  }

  QStringList userDirs;
  userDirs.reserve(m_uiForm.lwUserSearchDirs->count());
  for (int i = 0; i < m_uiForm.lwUserSearchDirs->count(); i++) {
    auto dir = m_uiForm.lwUserSearchDirs->item(i)->text();
    appendSlashIfNone(dir);
    userDirs.append(dir);
  }

  const auto &newDataDirs = dataDirs.join(";").replace('\\', '/');
  const auto &newUserDirs = userDirs.join(";").replace('\\', '/');
  auto newSaveDir = m_uiForm.leDefaultSave->text().replace('\\', '/');
  appendSlashIfNone(newSaveDir);

  auto &config = Mantid::Kernel::ConfigService::Instance();
  config.setString(DATASEARCH_SEARCHARCHIVE, newSearchArchive.toStdString());
  config.setString(DATASEARCH_DIRECTORIES, newDataDirs.toStdString());
  config.setString(DEFAULTSAVE_DIRECTORY, newSaveDir.toStdString());
  config.setString(PYTHONSCRIPTS_DIRECTORIES, newUserDirs.toStdString());

  if (this->keepPythonExtensions) {
    savePropertiesForPythonExtensions(config);
  }

  config.saveConfig(m_userPropFile.toStdString());
}

/**
 * Appends a forward slash to the end of a path if there is no slash (forward or
 * back) there already, and strip whitespace from the path.
 *
 * @param path :: A reference to the path
 */
void ManageUserDirectories::appendSlashIfNone(QString &path) const {
  path = path.trimmed();
  if (!(path.endsWith("/") || path.endsWith("\\") || path.isEmpty())) {
    // Don't need to add to a \\, as it would just get changed to a /
    // immediately after
    path.append("/");
  }
}

QLineEdit *ManageUserDirectories::lineEdit() const {
  if (m_uiForm.tabWidget->currentWidget() == m_uiForm.tabDataSearch) {
    return m_uiForm.leDirectoryPath;
  } else if (m_uiForm.tabWidget->currentWidget() ==
             m_uiForm.tabPythonDirectories) {
    return m_uiForm.leDirectoryPathPython;
  } else if (m_uiForm.tabWidget->currentWidget() ==
             m_uiForm.tabPythonExtensions) {
    return m_uiForm.leDirectoryPathExtensions;
  } else {
    throw std::runtime_error("Unknown tab, cannot return the correct widget!.");
  }
}

QListWidget *ManageUserDirectories::listWidget() const {
  if (m_uiForm.tabWidget->currentWidget() == m_uiForm.tabDataSearch) {
    return m_uiForm.lwDataSearchDirs;
  } else if (m_uiForm.tabWidget->currentWidget() ==
             m_uiForm.tabPythonDirectories) {
    return m_uiForm.lwUserSearchDirs;
  } else if (m_uiForm.tabWidget->currentWidget() ==
             m_uiForm.tabPythonExtensions) {
    return m_uiForm.lwExtensionsSearchDirs;
  } else {
    throw std::runtime_error("Unknown tab, cannot return the correct widget!.");
  }
}

// SLOTS
void ManageUserDirectories::helpClicked() {
  HelpWindow::showConcept(this, QString("ManageUserDirectories"));
}
void ManageUserDirectories::cancelClicked() { this->close(); }
void ManageUserDirectories::confirmClicked() {
  saveProperties();
  this->close();
}

void ManageUserDirectories::addDirectory() const {
  auto *input = lineEdit();

  if (input->text() != "") {
    listWidget()->addItem(input->text());
    input->clear();
  }
}

void ManageUserDirectories::browseToDirectory() {
  QSettings settings;
  const auto lastDirectory =
      settings.value("ManageUserSettings/last_directory", "").toString();

  const auto newDir = QFileDialog::getExistingDirectory(
      this, tr("Select New Data Directory"), lastDirectory,
      QFileDialog::ShowDirsOnly);

  if (newDir != "") {
    settings.setValue("ManageUserSettings/last_directory", newDir);
    listWidget()->addItem(newDir);
  }
}
void ManageUserDirectories::remDir() const {
  auto selected{listWidget()->selectedItems()};
  for (auto &i : selected) {
    delete i;
  }
}
void ManageUserDirectories::moveUp() const {
  auto *list{listWidget()};
  auto selected{list->selectedItems()};
  for (auto &i : selected) {
    const int index = list->row(i);
    if (index != 0) {
      const auto move = list->takeItem(index);
      list->insertItem(index - 1, move);
    }
    list->setCurrentItem(i);
  }
}
void ManageUserDirectories::moveDown() const {
  auto *list{listWidget()};
  const int count = list->count();
  auto selected{list->selectedItems()};
  for (auto &i : selected) {
    const int index = list->row(i);
    if (index != (count - 1)) {
      auto *move = list->takeItem(index);
      list->insertItem(index + 1, move);
    }
    list->setCurrentItem(i);
  }
}
void ManageUserDirectories::selectSaveDir() {
  QSettings settings;
  QString lastDirectory = m_uiForm.leDefaultSave->text();
  if (lastDirectory.trimmed() == "")
    lastDirectory =
        settings.value("ManageUserSettings/last_directory", "").toString();

  const QString newDir = QFileDialog::getExistingDirectory(
      this, tr("Select New Default Save Directory"), lastDirectory,
      QFileDialog::ShowDirsOnly);

  if (newDir != "") {
    QString path = newDir + QDir::separator();
    path.replace('\\', '/');
    settings.setValue("ManageUserSettings/last_directory", path);
    m_uiForm.leDefaultSave->setText(path);
  }
}
/** Opens a manage directories dialog and gives it focus
 *  @param parent :: the parent window, probably the window that called it
 */
void ManageUserDirectories::openUserDirsDialog(QWidget *parent) {
  auto *ad = new ManageUserDirectories(parent);
  ad->show();
  ad->setFocus();
}
