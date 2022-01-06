#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QPushButton;

class Widget_framegrabber : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_framegrabber(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    void setEncodings(std::vector<std::string> &encs);
    void setSelectedEncoding(QString enc);

    QPushButton *mPausePlayButton;
    QComboBox *mEncodingList;

public Q_SLOTS:

    virtual void slot_togglePlayPause(bool v);
    virtual void slot_updateEncoding(int idx);

Q_SIGNALS:
    void signal_newEncoding(QString r);

private:
    // raw pointer to new object which will be deleted by QT hierarchy
    QLabel *mLabel;

};
