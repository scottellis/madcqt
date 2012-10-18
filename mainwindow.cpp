/*
 * Copyright (C) 2012, Pansenti, LLC
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStop()));

    m_adcReader = new MadcReader();

    if (m_adcReader) {
        connect(m_adcReader, SIGNAL(dataEvent(QList<int>)), this, SLOT(adcDataEvent(QList<int>)), Qt::DirectConnection);
        connect(m_adcReader, SIGNAL(stopEvent()), this, SLOT(adcStopEvent()), Qt::DirectConnection);
        m_timer = startTimer(50);
    }
    else {
        ui->actionStart->setEnabled(false);
    }

    ui->actionStop->setEnabled(false);

#ifdef Q_WS_QWS
    // remove title bar
    setWindowFlags(Qt::CustomizeWindowHint);
#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (m_adcReader) {
        disconnect(m_adcReader, SIGNAL(dataEvent(QList<int>)), this, SLOT(adcDataEvent(QList<int>)));
        disconnect(m_adcReader, SIGNAL(stopEvent()), this, SLOT(adcStopEvent()));
        m_adcReader->stopLoop();
        delete m_adcReader;
        m_adcReader = NULL;
    }
}

void MainWindow::timerEvent(QTimerEvent *)
{
    int divisor = NUM_SAMPLES;

    m_dataMutex.lock();

    if (m_sampleCount < NUM_SAMPLES)
        divisor = m_sampleCount;

    if (ui->check_2->isChecked())
        ui->check_2->setText(QString::number(m_sum[0] / divisor));

    if (ui->check_3->isChecked())
        ui->check_2->setText(QString::number(m_sum[1] / divisor));

    if (ui->check_4->isChecked())
        ui->check_2->setText(QString::number(m_sum[2] / divisor));

    if (ui->check_5->isChecked())
        ui->check_2->setText(QString::number(m_sum[3] / divisor));

    if (ui->check_6->isChecked())
        ui->check_2->setText(QString::number(m_sum[4] / divisor));

    if (ui->check_7->isChecked())
        ui->check_2->setText(QString::number(m_sum[5] / divisor));

    m_dataMutex.unlock();
}

/*
int m_samples[NUM_ADC][NUM_SAMPLES];
unsigned int m_sampleIndex;
int m_sums[NUM_ADC];
*/
void MainWindow::adcDataEvent(QList<int> values)
{
    m_dataMutex.lock();

    for (int i = 0; i < values.length(); i++) {
        int val = values.at(i);
        m_sum[i] = val - m_samples[i][m_sampleIndex];
        m_samples[i][m_sampleIndex] = val;
    }

    m_sampleIndex = (m_sampleIndex + 1) % NUM_SAMPLES;
    m_sampleCount++;

    m_dataMutex.unlock();
}

void MainWindow::adcStopEvent()
{
    if (m_adcReader) {
        ui->actionStart->setEnabled(true);
        ui->actionStop->setEnabled(false);
    }
}

void MainWindow::onStart()
{
    QList<int> adcList;

    if (!m_adcReader)
        return;

    if (ui->check_2->isChecked())
        adcList.append(2);

    if (ui->check_3->isChecked())
        adcList.append(3);

    if (ui->check_4->isChecked())
        adcList.append(4);

    if (ui->check_5->isChecked())
        adcList.append(5);

    if (ui->check_6->isChecked())
        adcList.append(6);

    if (ui->check_7->isChecked())
        adcList.append(7);

    if (adcList.length() < 1)
        return;

    for (int i = 0; i < NUM_ADC; i++) {
        m_sum[i] = 0;

        for (int j = 0; j < NUM_SAMPLES; j++)
            m_samples[i][j] = 0;
    }

    m_sampleIndex = 0;
    m_sampleCount = 0;

    if (m_adcReader->startLoop(50, adcList)) {
        ui->actionStart->setEnabled(false);
        ui->actionStop->setEnabled(true);
        m_timer = startTimer(100);
    }
}

void MainWindow::onStop()
{
    if (m_adcReader) {
        m_adcReader->stopLoop();
        ui->actionStart->setEnabled(true);
        ui->actionStop->setEnabled(false);
        killTimer(m_timer);
        m_timer = 0;
    }
}
