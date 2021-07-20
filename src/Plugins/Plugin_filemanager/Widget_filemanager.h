#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;

class Widget_filemanager : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_filemanager(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);


private:
    // raw pointer to new object which will be deleted by QT hierarchy
    QLabel *mLabel;

};
