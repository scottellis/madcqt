/*
 * Copyright (C) 2012, Pansenti, LLC
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qqueue.h>
#include <qmutex.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include "madcReader.h"

#define NUM_SAMPLES 32

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onStart();
    void onStop();
    void adcDataEvent(QList<int> values);
    void adcStopEvent();
    void adcErrorEvent(QString);

protected:
    void closeEvent(QCloseEvent *);
    void timerEvent(QTimerEvent *);

private:
    void layoutControls();

    Ui::MainWindow *ui;

    MadcReader *m_madcReader;
    int m_timer;

    QMutex m_dataMutex;
    unsigned int m_sampleCount;
    unsigned int m_sampleIndex;
    int m_samples[NUM_ADC][NUM_SAMPLES];
    int m_sum[NUM_ADC];

    QLabel *m_data[NUM_ADC];
    QCheckBox *m_check[NUM_ADC];
};

#endif // MAINWINDOW_H
