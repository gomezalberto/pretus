#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QSlider;

class Widget_CppAlgorithm : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_CppAlgorithm(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    QSlider *mSlider;
private:
    QLabel *mLabel;
};
