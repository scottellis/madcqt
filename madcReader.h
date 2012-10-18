/*
 * Copyright (C) 2012, Pansenti, LLC
 *
 */

#ifndef MADCREADER_H
#define MADCREADER_H

#include <qthread.h>
#include <qlist.h>

#define MIN_ADC 2
#define MAX_ADC 7

#define NUM_ADC 6

class MadcReader : public QThread
{
    Q_OBJECT

public:
    MadcReader();
    bool startLoop(int delay, QList<int> adcList);
    void stopLoop();

signals:
    void dataEvent(QList<int>);
    void stopEvent();

protected:
    void run();

private:
    int readADC(int adc);
    int openADC(int adc);

    bool m_stop;
    QList<bool> m_activeList;
    int m_delay;
};

#endif // MADCREADER_H
