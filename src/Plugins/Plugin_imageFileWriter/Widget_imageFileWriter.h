#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>
#include <ifindStreamTypeHelper.h>

class QCheckBox;
class QLabel;

class Widget_imageFileWriter : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_imageFileWriter(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    QCheckBox *mCheckBoxSaveFiles;

public Q_SLOTS:

    virtual void slot_imageWritten(ifind::Image::Pointer image);


private:
    // raw pointer to new object which will be deleted by QT hierarchy
    QLabel *mLabel;

    int n_images_written;
};
