#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QSlider;


class Widget_PythonAlgorithm : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_PythonAlgorithm(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    QSlider *mSlider;

private:
    // raw pointer to new object which will be deleted by QT hierarchy
    QLabel *mLabel;    

};
