/*
 * Copyright (C) 2012, Pansenti, LLC
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <getopt.h>
#include <errno.h>

#include <qdebug.h>

#include "madcReader.h"


#define ADC_READ_ERROR -100000

MadcReader::MadcReader()
{
    m_delay = 50;

    for (int i = 0; i < NUM_ADC; i++)
        m_activeList.append(false);
}

// The adcList should contain numbers 2-7
bool MadcReader::startLoop(int delay, QList<int> adcList)
{
    if (isRunning())
        return false;

    m_stop = false;

    m_delay = delay;

    if (delay < 1)
        m_delay = 1;
    else
        m_delay = delay;

    for (int i = 0; i < NUM_ADC; i++)
        m_activeList[i] = false;

    if (adcList.length() < 1) {
        qDebug() << "MadcReader::startLoop: adcList is empty";
        return false;
    }

    for (int i = 0; i < adcList.length(); i++) {
        int adc = adcList.at(i);

        if (adc < MIN_ADC || adc > MAX_ADC) {
            qDebug() << "MadcReader::startLoop: Bad ADC number: " << adc;
            return false;
        }

        m_activeList[adc-2] = true;
    }

    start();

    return true;
}

void MadcReader::stopLoop()
{
    m_stop = true;

    while (isRunning())
        wait(200);
}

void MadcReader::run()
{
    QList<int> values;
    int val;

    for (int i = 0; i < NUM_ADC; i++)
        values.append(0);

    while (!m_stop) {
        for (int i = 0; i < NUM_ADC; i++) {
            if (m_activeList.at(i)) {
                val = readADC(i + 2);

                if (val == ADC_READ_ERROR) {
                    m_stop = true;
                    break;
                }
                else {
                    values[i] = val;
                }
            }

            if (!m_stop)
                emit dataEvent(values);

            msleep(m_delay);
        }
    }

    emit stopEvent();
}

int MadcReader::readADC(int adc)
{
    int val;
    char buff[8];

    int fd = openADC(adc);
    if (fd < 0)
        return ADC_READ_ERROR;

    memset(buff, 0, sizeof(buff));

    if (read(fd, buff, 6) < 0) {
        qDebug() << "MadcReader::readADC: read error: " << strerror(errno);
        val = ADC_READ_ERROR;
    }
    else {
        val = atoi(buff);
    }

    close(fd);

    return val;
}

int MadcReader::openADC(int adc)
{
    char path[128];

    sprintf(path, "/sys/class/hwmon/hwmon0/device/in%d_input", adc);

     int fd = open(path, O_RDONLY);

     if (fd < 0)
         qDebug() << "MadcReader::openADC: open(" << path << "): " << strerror(errno);

     return fd;
}
