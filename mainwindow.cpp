/*
 * Copyright (C) 2012, Pansenti, LLC
 *
 */

#include <qdebug.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    layoutControls();

    m_timer = 0;

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStop()));

    m_madcReader = new MadcReader();

    if (m_madcReader) {
        connect(m_madcReader, SIGNAL(dataEvent(QList<int>)), this, SLOT(adcDataEvent(QList<int>)), Qt::DirectConnection);
        connect(m_madcReader, SIGNAL(stopEvent()), this, SLOT(adcStopEvent()));
        connect(m_madcReader, SIGNAL(errorEvent(QString)), this, SLOT(adcErrorEvent(QString)));
    }
    else {
        ui->actionStart->setEnabled(false);
    }

    ui->actionStop->setEnabled(false);

#ifdef Q_WS_QWS
    setWindowFlags(Qt::CustomizeWindowHint);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (m_madcReader) {
        disconnect(m_madcReader, SIGNAL(dataEvent(QList<int>)), this, SLOT(adcDataEvent(QList<int>)));
        disconnect(m_madcReader, SIGNAL(stopEvent()), this, SLOT(adcStopEvent()));
        connect(m_madcReader, SIGNAL(errorEvent(QString)), this, SLOT(adcErrorEvent(QString)));
        m_madcReader->stopLoop();
        delete m_madcReader;
        m_madcReader = NULL;
    }
}

void MainWindow::timerEvent(QTimerEvent *)
{
    int divisor = NUM_SAMPLES;

    m_dataMutex.lock();

    if (m_sampleCount > 0) {
        if (m_sampleCount < NUM_SAMPLES)
            divisor = m_sampleCount;

        for (int i = 0; i < NUM_ADC; i++) {
            if (m_check[i]->isChecked())
                m_data[i]->setText(QString::number(m_sum[i] / divisor));
        }
    }

    m_dataMutex.unlock();
}

void MainWindow::adcDataEvent(QList<int> values)
{
    m_dataMutex.lock();

    for (int i = 0; i < values.length(); i++) {
        int val = values.at(i);
        m_sum[i] += val - m_samples[i][m_sampleIndex];
        m_samples[i][m_sampleIndex] = val;
    }

    m_sampleIndex = (m_sampleIndex + 1) % NUM_SAMPLES;
    m_sampleCount++;

    m_dataMutex.unlock();
}

void MainWindow::adcStopEvent()
{
    if (m_madcReader) {
        ui->actionStart->setEnabled(true);
        ui->actionStop->setEnabled(false);
        killTimer(m_timer);
        m_timer = 0;

        for (int i = 0; i < NUM_ADC; i++) {
            m_check[i]->setEnabled(true);
        }
    }
}

void MainWindow::adcErrorEvent(QString errMsg)
{
    qDebug() << errMsg;
}

void MainWindow::onStart()
{
    QList<int> adcList;

    if (!m_madcReader)
        return;

    for (int i = 0; i < NUM_ADC; i++) {
        if (m_check[i]->isChecked())
            adcList.append(i + 2);
    }

    if (adcList.length() < 1)
        return;

    for (int i = 0; i < NUM_ADC; i++) {
        m_data[i]->setText("0");
        m_sum[i] = 0;

        for (int j = 0; j < NUM_SAMPLES; j++)
            m_samples[i][j] = 0;
    }

    m_sampleIndex = 0;
    m_sampleCount = 0;

    if (m_madcReader->startLoop(10, adcList)) {
        ui->actionStart->setEnabled(false);
        ui->actionStop->setEnabled(true);

        for (int i = 0; i < NUM_ADC; i++) {
            m_check[i]->setEnabled(false);
        }

        m_timer = startTimer(50);
    }
}

void MainWindow::onStop()
{
    if (m_madcReader) {
        m_madcReader->stopLoop();
        ui->actionStart->setEnabled(true);
    }

    ui->actionStop->setEnabled(false);

    if (m_timer) {
        killTimer(m_timer);
        m_timer = 0;
    }

    for (int i = 0; i < NUM_ADC; i++) {
        m_check[i]->setEnabled(true);
    }
}

void MainWindow::layoutControls()
{
    QHBoxLayout *hLayout;

    QVBoxLayout *vLayout = new QVBoxLayout();

    for (int i = 0; i < NUM_ADC; i++) {
        m_check[i] = new QCheckBox(QString::number(i + 2), this);

        m_data[i] = new QLabel("0", this);
        m_data[i]->setFrameShape(QFrame::Panel);
        m_data[i]->setFrameShadow(QFrame::Sunken);
        m_data[i]->setMinimumWidth(100);
        m_data[i]->setMaximumWidth(100);
        m_data[i]->setMaximumHeight(24);

        hLayout = new QHBoxLayout();
        hLayout->addWidget(m_check[i]);
        hLayout->addWidget(m_data[i]);
        hLayout->addStretch();

        vLayout->addItem(hLayout);
    }

    centralWidget()->setLayout(vLayout);
}
