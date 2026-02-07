#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <QDialog>
#include <QString>
#include <QList>
#include "../testsheet.h"
#include "../devicedriver.h"

namespace Ui {
class identifier;
}

class identifier : public QDialog
{
    Q_OBJECT

public:
    explicit identifier(QString port, QString pLUTPath, QString settingsPath, int baudRate, bool warn, QWidget *parent = nullptr);
    ~identifier();

private slots:
    void on_btnNew_clicked();

    void on_btnSheetPath_clicked();

    void on_cbPins_activated(const QString &arg1);

    void on_btnStart_clicked();

private:
    Ui::identifier *ui;
    QString settingsPath;
    QString port;
    QString pLUTPath;
    QString filesPath;
    int baudRate;
    void selektDir();
    void repaintList();
    bool powerDisplayer();
    bool test (DeviceDriver * tester, TestSheet * sheet);

    DeviceDriver * tester;
    QList<TestSheet> ics;
    QList<TestSheet> ics_filt;
};

#endif // IDENTIFIER_H
