#pragma once
#include <QtInfoPanelBase.h>
#include <ifindImage.h>


class QLabel;
class QtLevelMeter;

class QtInfoPanelTrafficLightBase : public QtInfoPanelBase
{
public:
    enum Modes
    {
        ImmediateLabelAbsolute = 0,
        ImmediateLabelNormalised,
        ImmediateBarNormalised,
        HoldLabelLargest
    };

    struct ValueColors {

        ValueColors(double value, QColor backgroundColor, QColor textColor)
            :
            Value(value),
            BackgroundColor(backgroundColor),
            TextColor(textColor)
        {}

        double Value;
        QColor BackgroundColor;
        QColor TextColor;
    };

    struct Configuration {
        Modes Mode;

        std::vector<std::string> LabelNames;
        int NGridColumns;

        std::string MetadataLabelsKey;
        std::string MetadataValuesKey;
        char MetadataSplitCharacter;

        std::vector<ValueColors> ValueColorsVector;
    };

    QtInfoPanelTrafficLightBase(
        Configuration configuration,
        QWidget *parent = nullptr,
        Qt::WindowFlags f = Qt::WindowFlags() );

public Q_SLOTS:

    void ResetTrafficLights();

protected:
    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

private:

    void UpdateTrafficLight(
        const std::string &metadataLabel,
        const double metadataValue);

    void UpdateLabelColors(QLabel *label, const double value);

    // update the given label's background and text colors 
    void SetLabelColors(QLabel *label, const ValueColors &valuesColors);

    Configuration mConfiguration;
       
    std::map<std::string, QLabel *> mTrafficLights;
    std::map<std::string, QtLevelMeter *> mLevelMeters;
};
