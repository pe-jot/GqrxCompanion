#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QScopedPointer>
#include <QScreen>
#include <QWindow>


const QString MainWindow::serverAddressKey = QString("ServerAddress");
const QString MainWindow::serverPortKey = QString("ServerPort");
const QString MainWindow::lowThresholdKey = QString("LowThreshold");
const QString MainWindow::highThresholdKey = QString("HighThreshold");
const QString MainWindow::savePathKey = QString("SavePath");


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings(new QSettings())
{
    ui->setupUi(this);

    restoreSettings();

    connect(ui->btnConnect, SIGNAL(clicked(bool)), this, SLOT(onBtnConnectClicked(bool)));
    connect(ui->btnSavePath, SIGNAL(clicked(bool)), this, SLOT(onBtnSavePathClicked(bool)));
    connect(&socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    // connect(&socket, SIGNAL(readyRead()), this, SLOT(onDataReceived())); // Debug only
    connect(&pollTimer, SIGNAL(timeout()), this, SLOT(onPollInterval()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(&socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#else
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#endif
}


MainWindow::~MainWindow()
{
    disconnect(ui->btnConnect, SIGNAL(clicked(bool)), this, SLOT(onBtnConnectClicked(bool)));
    disconnect(ui->btnSavePath, SIGNAL(clicked(bool)), this, SLOT(onBtnSavePathClicked(bool)));
    disconnect(&socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    // disconnect(&socket, SIGNAL(readyRead()), this, SLOT(onDataReceived())); // Debug only
    disconnect(&pollTimer, SIGNAL(timeout()), this, SLOT(onPollInterval()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    disconnect(&socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#else
    disconnect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#endif

    if (pollTimer.isActive())
    {
        pollTimer.stop();
    }

    if (socket.isOpen())
    {
        socket.close();
    }

    saveSettings();

    delete ui;
}


void MainWindow::onBtnConnectClicked(bool checked)
{
    Q_UNUSED(checked);

    ui->host->setEnabled(false);
    ui->portNumber->setEnabled(false);
    ui->btnConnect->setEnabled(false);

    socket.connectToHost(ui->host->text(), ui->portNumber->value());
}


void MainWindow::onBtnSavePathClicked(bool checked)
{
    Q_UNUSED(checked);

    auto savePath = QFileDialog::getExistingDirectory();
    ui->screenshotPath->setText(savePath);
}


void MainWindow::onSocketConnected()
{
    pollTimer.start(100);
}


void MainWindow::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    pollTimer.stop();

    if (socket.isOpen())
    {
        socket.close();
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Socket error:");
    msgBox.setText(socket.errorString());
    msgBox.exec();

    ui->host->setEnabled(true);
    ui->portNumber->setEnabled(true);
    ui->btnConnect->setEnabled(true);
}


void MainWindow::onDataReceived()
{
    auto bytesAvailable = socket.bytesAvailable();
    qDebug() << bytesAvailable << socket.read(bytesAvailable);
}


void MainWindow::onPollInterval()
{
    socket.write("l\n");
    socket.waitForBytesWritten(defaultTimeout);

    if (!socket.waitForReadyRead(defaultTimeout) || !socket.canReadLine())
    {
        return;
    }

    auto line = socket.readLine();
    auto ok = false;
    auto level = line.toDouble(&ok);

    if (!ok)
    {
        return;
    }

    handleLevelUpdate(level);
}


void MainWindow::handleLevelUpdate(const double& level)
{
    static double oldLevel = -999.9;
    static bool thresholdCrossed = false;

    const double hysteresis = 0.3; // Add a small hysteresis on the delta
    if ((level <= (oldLevel + hysteresis)) && (level >= oldLevel - hysteresis))
    {
        return;
    }
    oldLevel = level;

    // Update the current value display
    QString levelString;
    levelString.setNum(level, 'f', 1);
    ui->currentLevel->setText(levelString + " dB");

    // Filter multiple level changes
    if (level > ui->thresholdLevel->value() && !thresholdCrossed)
    {
        thresholdCrossed = true;
        takeScreenshot(thresholdCrossed, levelString);
        ui->rbThreshold->setChecked(thresholdCrossed);
    }
    else if (level < ui->thresholdLowLevel->value() && thresholdCrossed)
    {
        thresholdCrossed = false;
        takeScreenshot(thresholdCrossed, levelString);
        ui->rbThreshold->setChecked(thresholdCrossed);
    }
}


void MainWindow::takeScreenshot(const bool& thresholdState, const QString& level)
{
    // Throttle: max. 1 screenshot per second
    static QDateTime lastScreenshot = QDateTime::fromSecsSinceEpoch(0);
    auto timestamp = QDateTime::currentDateTimeUtc();

    if (lastScreenshot.secsTo(timestamp) < 1)
    {
        return;
    }
    lastScreenshot = timestamp;

    // Sanitize file path
    auto savePath = QDir(QDir::cleanPath(ui->screenshotPath->text()));
    if (!savePath.exists() || ui->screenshotPath->text().isEmpty())
    {
        savePath.setPath(QDir::currentPath());
        ui->screenshotPath->setText(savePath.path());
    }

    auto screen = QGuiApplication::primaryScreen();
    if (const QWindow *window = windowHandle())
    {
        screen = window->screen();
    }
    if (!screen)
    {
        return;
    }

    auto pixmap = screen->grabWindow(0);

    auto filename = QString("%1/%2 %3 %4.png").arg(
                savePath.path(),
                timestamp.toString("yyyy-MM-dd hh-mm-ss"),
                level,
                thresholdState ? "high" : "low");

    pixmap.save(filename);
}


void MainWindow::saveSettings()
{
    settings->setValue(serverAddressKey, ui->host->text());
    settings->setValue(serverPortKey, ui->portNumber->value());
    settings->setValue(lowThresholdKey, ui->thresholdLowLevel->value());
    settings->setValue(highThresholdKey, ui->thresholdLevel->value());
    settings->setValue(savePathKey, ui->screenshotPath->text());
    settings->sync();
}


void MainWindow::restoreSettings()
{
    const auto defaultServerAddress = ui->host->text();
    const auto storedServerAddress = settings->value(serverAddressKey, defaultServerAddress).toString();
    ui->host->setText(storedServerAddress);

    const auto defaultServerPort = ui->portNumber->value();
    const auto storedServerPort = settings->value(serverPortKey, defaultServerPort).toInt();
    ui->portNumber->setValue(storedServerPort);

    const auto defaultThresholdLowLevel = ui->thresholdLowLevel->value();
    const auto storedThresholdLowLevel = settings->value(lowThresholdKey, defaultThresholdLowLevel).toDouble();
    ui->thresholdLowLevel->setValue(storedThresholdLowLevel);

    const auto defaultThresholdLevel = ui->thresholdLevel->value();
    const auto storedThresholdLevel = settings->value(highThresholdKey, defaultThresholdLevel).toDouble();
    ui->thresholdLevel->setValue(storedThresholdLevel);

    const auto defaultScreenshotPath = ui->screenshotPath->text();
    const auto storedScreenshotPath = settings->value(savePathKey, defaultScreenshotPath).toString();
    ui->screenshotPath->setText(storedScreenshotPath);
}
