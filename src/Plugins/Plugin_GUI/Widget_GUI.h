#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QSlider;
class QPushButton;

class Widget_GUI : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_GUI(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    QSlider *mSlider;
    QPushButton *mResetButton;

public Q_SLOTS:
    virtual void slot_updateSliderLabel();

private:
    QLabel *mLabel;    
    QLabel *mSlLabel;
};
