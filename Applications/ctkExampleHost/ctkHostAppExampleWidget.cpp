#include "ctkHostAppExampleWidget.h"
#include "ui_ctkHostAppExampleWidget.h"
#include "ctkDicomExampleHost.h"
#include "ctkDicomAppService.h"
#include <ctkDicomWG23TypesHelper.h>

#include <QDebug>
#include <QFileDialog>
#include <QProcess>

ctkHostAppExampleWidget::ctkHostAppExampleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ctkHostAppExampleWidget)
{
    qDebug() << "setup ui";
    ui->setupUi(this);
    ui->crashLabel->setVisible(false);
    ui->messageOutput->setVisible(false);
    this->host = new ctkDicomExampleHost(ui->placeholderFrame);

    connect(&this->host->getAppProcess(),SIGNAL(error(QProcess::ProcessError)),SLOT(appProcessError(QProcess::ProcessError)));
    connect(&this->host->getAppProcess(),SIGNAL(stateChanged(QProcess::ProcessState)),SLOT(appProcessStateChanged(QProcess::ProcessState)));
    connect(ui->placeholderFrame,SIGNAL(resized()),SLOT(placeholderResized()));
    connect(this->host,SIGNAL( stateChangeReceived(ctkDicomWG23::State)),SLOT(appStateChanged(ctkDicomWG23::State)));

  }



ctkHostAppExampleWidget::~ctkHostAppExampleWidget()
{
  delete host;
  delete ui;
}

void ctkHostAppExampleWidget::startButtonClicked()
{
    qDebug() << "start button clicked";
    if (host)
    {
      host->StartApplication(appFileName);
    }
}

void ctkHostAppExampleWidget::runButtonClicked()
{
    qDebug() << "run button clicked";
    if (host)
    {
      bool reply = host->getDicomAppService()->setState(ctkDicomWG23::INPROGRESS);
      qDebug() << "  setState(INPROGRESS) returned: " << reply;
    }
}

void ctkHostAppExampleWidget::stopButtonClicked()
{
    qDebug() << "stop button clicked";
}

void ctkHostAppExampleWidget::loadButtonClicked()
{
    qDebug() << "load button clicked";
    this->setAppFileName(QFileDialog::getOpenFileName(this,"Choose hosted application",QApplication::applicationDirPath()));
}

void ctkHostAppExampleWidget::setAppFileName(QString name)
{
  this->appFileName = name;
  if (QFile(this->appFileName).permissions() & QFile::ExeUser )
  {
    this->ui->applicationPathLabel->setText(this->appFileName);
  }
  else
  {
    this->ui->applicationPathLabel->setText(QString("<font color='red'>Not executable:</font>").append(this->appFileName));
  }
}

void ctkHostAppExampleWidget::appProcessError(QProcess::ProcessError error)
{
  if (error == QProcess::Crashed)
  {
    qDebug() << "crash detected";
    ui->crashLabel->setVisible(true);
  }
  }

void ctkHostAppExampleWidget::appProcessStateChanged(QProcess::ProcessState state)
{
  QString labelText;
  switch (state){
  case QProcess::Running:
    ui->processStateLabel->setText("Running");
    break;
  case QProcess::NotRunning:
    if (host->getAppProcess().exitStatus() == QProcess::CrashExit )
    {
        labelText = "crashed";
    }
    else
    {
      labelText = "Not Running, last exit code ";
      labelText.append(QString::number(host->getAppProcess().exitCode()));
    }
    ui->processStateLabel->setText(labelText);
    break;
  case QProcess::Starting:
    ui->processStateLabel->setText("Starting");
    break;
  default:
    ;
  }
  }
void ctkHostAppExampleWidget::placeholderResized()
{
  qDebug() << "resized";
  //ui->placeholderFrame->printPosition();
}

void ctkHostAppExampleWidget::appStateChanged(ctkDicomWG23::State state)
{
   ui->statusLabel->setText(ctkDicomSoapState::toStringValue(state));
}