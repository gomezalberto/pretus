/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

// https://doc-snapshots.qt.io/qt5-5.9/qtmultimedia-multimedia-spectrum-app-levelmeter-cpp.html

#include "QtLevelMeter.h"
#include <iostream>
#include <math.h>

#include <QPainter>
#include <QTimer>
#include <QDebug>

// Constants
const int RedrawInterval = 100; // ms

QtLevelMeter::QtLevelMeter(QWidget *parent)
    : QWidget(parent)
    , mLevel(0.0)
    , mColorLevel(-1.0)
    , m_redrawTimer(new QTimer(this))
{
    mLevelColor = Qt::green;

    connect(m_redrawTimer, &QTimer::timeout,
            this, &QtLevelMeter::redrawTimerExpired);
    m_redrawTimer->start(RedrawInterval);
    mColorWithLevel = true;
}

QtLevelMeter::~QtLevelMeter()
{

}

void QtLevelMeter::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), Qt::darkGray);
    //painter.fillRect(rect(), Qt::black);

    QRect bar = rect();

    bar.setRight(rect().left() + (mLevel * rect().width()));
    QColor currentColor = mLevelColor;
    if (mColorWithLevel == true){
        double H, S, V;
        currentColor.getHsvF(&H, &S, &V);
        if (mColorLevel>=0){
            //double N = 0;
            //H = H*(mColorLevel*mColorLevel+N)/(N+1);
            S = mColorLevel;
            V = mColorLevel;
        } else {
            S = mLevel;
            V = mLevel;
        }
        currentColor.setHsvF(H, S, V);
    }
    painter.fillRect(bar, currentColor);
}

void QtLevelMeter::LevelChanged(double level, double colorLevel)
{
    mLevel = level;
    mColorLevel = colorLevel;
    update();
}

void QtLevelMeter::LevelChanged(double level)
{
    mLevel = level;
    update();
}

bool QtLevelMeter::colorWithLevel() const
{
    return mColorWithLevel;
}

void QtLevelMeter::setColorWithLevel(bool colorWithLevel)
{
    mColorWithLevel = colorWithLevel;
}

void QtLevelMeter::setLevelColor(const Qt::GlobalColor &levelColor)
{
    mLevelColor = levelColor;
}

void QtLevelMeter::redrawTimerExpired()
{
    // Decay the peak signal
    /*
    const int elapsedMs = m_peakLevelChanged.elapsed();
    const qreal decayAmount = m_peakDecayRate * elapsedMs;
    if (decayAmount < m_peakLevel)
        m_decayedPeakLevel = m_peakLevel - decayAmount;
    else
        m_decayedPeakLevel = 0.0;

    // Check whether to clear the peak hold level
    if (m_peakHoldLevelChanged.elapsed() > PeakHoldLevelDuration)
        m_peakHoldLevel = 0.0;
    */
    update();
}
