#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QPushButton;
class QComboBox;

class Widget_PnPframegrabber : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_PnPframegrabber(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    void setFrameRates(std::vector<int> &framerates);
    void setResolutions(std::vector<std::string> &resolutions);
    void setEncodings(std::vector<std::string> &encs);
    void setSelectedResolution(QString res);
    void setSelectedFramerate(double r_fr);
    void setSelectedEncoding(QString enc);

    QPushButton *mPausePlayButton;
    QComboBox *mFrameRateList;
    QComboBox *mResolutionList;
    QComboBox *mEncodingList;

public Q_SLOTS:

    virtual void slot_togglePlayPause(bool v);
    virtual void slot_updateFrameRate(int idx);
    virtual void slot_updateResolution(int idx);
    virtual void slot_updateEncoding(int idx);

Q_SIGNALS:
    void signal_newFrameRate(QString f);
    void signal_newResolution(QString r);
    void signal_newEncoding(QString r);

private:
    // raw pointer to new object which will be deleted by QT hierarchy
    QLabel *mLabel;

};
