#pragma once

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>

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
    const int defaultTimeout = 1000;

    void handleLevelUpdate(const double& level);
    void takeScreenshot(const bool& thresholdState, const QString& level);
};
