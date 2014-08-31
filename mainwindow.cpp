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
    qDebug() << " diretorio de arquivos JSON: ";
    qDebug() << jsonDir.absolutePath();
    jsonDir.cd("json-files");
    foreach(const QString &json, jsonDir.entryList(QStringList() << "*.json")) {
        QFile jsonFile(jsonDir.absoluteFilePath(json));
        if (!jsonFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Error when loading" << jsonFile.fileName() << "file !";
            return;
        }
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
        QJsonObject jsonObject = jsonDoc.object();
        QString arch = jsonObject.value("arch").toString();
        QTreeWidgetItem *archItem = ui->gamesTreeWidget->topLevelItem(arch == "MSX1" ? 0:1);
        QTreeWidgetItem *gameItem = new QTreeWidgetItem(archItem, QStringList() << jsonObject.value("name").toString());
        qDebug() << " carregando " << jsonFile.fileName() <<  jsonObject.value("name").toString();
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

    qDebug() << "Executando:" << openMsx + args.join(' ');
    QProcess *openMsxProcess = new QProcess;
    openMsxProcess->start(openMsx, args);
    openMsxProcess->waitForFinished();

    qDebug() << "openMSX fechado" ;
    this->show();
}


void MainWindow::on_gamesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    QJsonObject jsonObject = current->data(0, Qt::UserRole).value<QJsonObject>();
    ui->gameLabel->setPixmap(QPixmap(QDir::currentPath() + '/' + jsonObject.value("local").toString() + '/' + jsonObject.value("thumbnail").toString()));
}
