#pragma once

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBtnConnectClicked(bool checked = false);
    void onBtnSavePathClicked(bool checked = false);
    void onSocketConnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onPollInterval();
    void onDataReceived();

private:
    Ui::MainWindow *ui;
    QTcpSocket socket;
    QTimer pollTimer;
    QSettings *settings;

    const int defaultTimeout = 1000;

    static const QString serverAddressKey;
    static const QString serverPortKey;
    static const QString lowThresholdKey;
    static const QString highThresholdKey;
    static const QString savePathKey;

    void handleLevelUpdate(const double& level);
    void takeScreenshot(const bool& thresholdState, const QString& level);

    void saveSettings();
    void restoreSettings();
};
