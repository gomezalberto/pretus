#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QSlider;
class QPushButton;

class Widget_videomanager : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_videomanager(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);
    QSlider *mSlider;
    QPushButton *mPausePlayButton;
    QPushButton *mNextButton;
    QPushButton *mPreviousButton;

public Q_SLOTS:

    virtual void slot_togglePlayPause(bool v);

private:
    // raw pointer to new object which will be deleted by QT hierarchy
    QLabel *mLabel;

};
