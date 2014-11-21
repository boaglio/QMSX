#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    readSettings();
    checkOpenMSXInfo();
    ui->setupUi(this);
    populateGamesTreeWidget();

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateGamesTreeWidget()
{
    QDir jsonDir(QDir::currentPath());
    qDebug() << " JSON directory files: ";
    qDebug() << jsonDir.absolutePath();
    jsonDir.cd("json-files");
    foreach(const QString &json, jsonDir.entryList(QStringList() << "*.json")) {
        QFile jsonFile(jsonDir.absoluteFilePath(json));
        if (!jsonFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Error loading" << jsonFile.fileName() << "file !";
            return;
        }
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
        QJsonObject jsonObject = jsonDoc.object();
        QString arch = jsonObject.value("arch").toString();
        QTreeWidgetItem *archItem = ui->gamesTreeWidget->topLevelItem(arch == "MSX1" ? 0:1);
        QTreeWidgetItem *gameItem = new QTreeWidgetItem(archItem, QStringList() << jsonObject.value("name").toString());
        qDebug() << " loading " << jsonFile.fileName() <<  jsonObject.value("name").toString();
        gameItem->setData(0, Qt::UserRole, QVariant::fromValue(jsonObject));
    }
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About QMSX"),
            tr("<br><b>QMSX</b> is a graphic user interface game launcher for <a href='http://www.openmsx.org'>openMSX</a>.<br><br><b>openMSX</b> is the best emulator for the MSX 8-bit computer.<br><br><b>QMSX</b> is also <a href='https://github.com/boaglio/QMSX'>an Open Source project</a>!   \\o/  "));
}



void MainWindow::on_runButton_clicked()
{
    this->hide();
    QString openMsx = "openmsx";
    QStringList args;

    QJsonObject jsonObject = ui->gamesTreeWidget->currentItem()->data(0, Qt::UserRole).value<QJsonObject>();

    args << "-machine" << jsonObject.value("arch").toString().toLower();
    args << QDir::currentPath() + '/' + jsonObject.value("local").toString() + '/' + jsonObject.value("game").toString();

    qDebug() << "Running:" << openMsx + args.join(' ');
    QProcess *openMsxProcess = new QProcess;
    openMsxProcess->start(openMsx, args);
    openMsxProcess->waitForFinished();

    qDebug() << "openMSX hide" ;
    this->show();
}


void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "QMSX",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
        writeSettings();
    }
}



void MainWindow::on_gamesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    QJsonObject jsonObject = current->data(0, Qt::UserRole).value<QJsonObject>();
    ui->gameLabel->setPixmap(QPixmap(QDir::currentPath() + '/' + jsonObject.value("local").toString() + '/' + jsonObject.value("thumbnail").toString()));
}



void MainWindow::writeSettings()
{
    qDebug() << "writing settings..." ;
    QSettings settings("QMSX", "QMSX");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    qDebug() << "reading settings..." ;
    QSettings settings("QMSX", "QMSX");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
}

void  MainWindow::checkOpenMSXInfo()
{

    qDebug() << "get OpenMSX info..." ;
    QProcess process;
    process.start("oapenmsx -v");
    process.waitForFinished(-1);

    QString stdout = process.readAllStandardOutput();
    qDebug() << "stdout = " + stdout;
    QString stderr = process.readAllStandardError();
    qDebug() << "stderr = " + stderr;

    QString openMSXstr = "openMSX";

    if (stdout.indexOf(openMSXstr) ==-1) {
        QMessageBox::warning(this, tr("OpenMSX executable was not found"),tr("OpenMSX executable was not found ! <br/><br/>Please install it before running any game. <br/><br/>Install running <i>apt-get install openmsx</i> or <a href='http://sourceforge.net/projects/openmsx/files/openmsx/'>download the files from the latest version</a>."));

    }


}

