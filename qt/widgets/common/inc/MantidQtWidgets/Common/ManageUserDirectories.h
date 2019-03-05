// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTIDQT_MANAGE_USER_DIRECTORIES_H
#define MANTIDQT_MANAGE_USER_DIRECTORIES_H

#include "DllOption.h"
#include "ui_ManageUserDirectories.h"
#include <QDialog>
#include "MantidKernel/ConfigService.h"

namespace MantidQt {
namespace API {

class EXPORT_OPT_MANTIDQT_COMMON ManageUserDirectories : public QDialog {
  Q_OBJECT

public:
  ManageUserDirectories(QWidget *parent = nullptr,
                        bool keepPythonExtensions = false);
  static void openUserDirsDialog(QWidget *parent);

private:
  virtual void initLayout();
  void loadFromStringIntoList(const std::string &keyName,
                              QListWidget *listWidget);
  void loadExtensionProperties();
  void initConnectionsForPythonExtensions() ;
  void loadProperties();
  void savePropertiesForPythonExtensions(Mantid::Kernel::ConfigServiceImpl &config) const;
  void saveProperties() const;
  void appendSlashIfNone(QString &path) const;
  QListWidget *listWidget() const;
  QLineEdit *lineEdit() const;

private slots:
  void helpClicked();
  void cancelClicked();
  void confirmClicked();
  void addDirectory() const;
  void browseToDirectory();
  void remDir() const;
  void moveUp() const;
  void moveDown() const;
  void selectSaveDir();

private:
  Ui::ManageUserDirectories m_uiForm;
  QString m_userPropFile;
  bool keepPythonExtensions = false;
};

} // namespace API
} // namespace MantidQt

#endif /* MANTIDQT_MANAGE_USER_DIRECTORIES_H */
