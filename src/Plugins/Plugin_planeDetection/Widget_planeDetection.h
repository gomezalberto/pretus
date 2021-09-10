#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QSlider;

class Widget_planeDetection : public QtPluginWidgetBase
{
    Q_OBJECT

public:

    struct WidgetOptions
    {
        WidgetOptions() {

            show_bars = true;
            show_checklist = false;
        }

        bool show_bars;
        bool show_checklist;
    };

    WidgetOptions mWidgetOptions;

    Widget_planeDetection(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    QSlider *mSlider;
    QSlider *mSliderTA;
    bool colorWithLevel() const;
    void setColorWithLevel(bool colorWithLevel);

private:
    // raw pointer to new object which will be deleted by QT hierarchy
    QLabel *mLabel;
    bool mIsBuilt;
    bool mColorWithLevel;

    /**
     * @brief Build the widget
     */
    void Build(std::vector<std::string> &labelnames);

};
