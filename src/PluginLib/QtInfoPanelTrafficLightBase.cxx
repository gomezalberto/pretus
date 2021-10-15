#include "QtInfoPanelTrafficLightBase.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <boost/range/combine.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <QtLevelMeter.h>
#include "VisualizationConsts.h"


QtInfoPanelTrafficLightBase::QtInfoPanelTrafficLightBase(
        Configuration configuration,
        QWidget *parent,
        Qt::WindowFlags f)
    :
      QtInfoPanelBase(parent, f),
      mConfiguration(configuration)
{
    // Initialise the labels and coloursd to a test set

    mColorWithLevel = true;
    // Create the panel of traffic lights based upon the label names
    auto labelsGrid = new QGridLayout(this);
    this->setLayout(labelsGrid);

    int gridRow(0);
    int gridColumn(0);

    for (auto labelName : mConfiguration.LabelNames)
    {
        // create the label widget and add it to the grid
        {
            auto labelWidget = new QLabel(labelName.c_str(), this);
            labelWidget->setAutoFillBackground(true);
            SetLabelColors(labelWidget, mConfiguration.ValueColorsVector[0]);

            auto labelFont = labelWidget->font();
            labelFont.setPixelSize(VisualizationConsts::NormalTextPixelSize());
            labelFont.setBold(true);
            labelWidget->setFont(labelFont);

            labelsGrid->addWidget(labelWidget, gridRow, gridColumn);

            // then add the widget to the list for later updating
            mTrafficLights.insert(std::make_pair(labelName, labelWidget));

            // generate the next grid position
            ++gridColumn;
            if (gridColumn >= mConfiguration.NGridColumns)
            {
                gridColumn = 0;
                ++gridRow;
            }
        }

        // add the level meters if we are in that mode
        if (Modes::ImmediateBarNormalised == mConfiguration.Mode ||
             Modes::ImmediateBarAbsolute == mConfiguration.Mode )
        {
            auto levelWidget = new QtLevelMeter(this);
            if (mColorWithLevel==true){
                levelWidget->setColorWithLevel(true);
            }
            labelsGrid->addWidget(levelWidget, gridRow, gridColumn);

            mLevelMeters.insert(std::make_pair(labelName, levelWidget));

            // generate the next grid position
            ++gridColumn;
            if (gridColumn >= mConfiguration.NGridColumns)
            {
                gridColumn = 0;
                ++gridRow;
            }
        }
    }

    // And add a reset button at the bottom
    {
        auto resetButton = new QPushButton("Reset", this);

        auto labelFont = resetButton->font();
        labelFont.setPixelSize(VisualizationConsts::NormalTextPixelSize());
        labelFont.setBold(true);
        resetButton->setFont(labelFont);

        QObject::connect(
                    resetButton, &QPushButton::pressed,
                    this, &QtInfoPanelTrafficLightBase::ResetTrafficLights);

        labelsGrid->addWidget(resetButton, ++gridRow, 0, 1, mConfiguration.NGridColumns);
    }

}

void QtInfoPanelTrafficLightBase::ResetTrafficLights()
{
    for (auto trafficLight : mTrafficLights)
    {
        UpdateLabelColors(trafficLight.second, std::numeric_limits<double>::lowest() + 1.0);
    }
}


void QtInfoPanelTrafficLightBase::SendImageToWidgetImpl(ifind::Image::Pointer image)
{
    const std::string tempMetadataLabels(
                image->GetMetaData<std::string>(mConfiguration.MetadataLabelsKey.c_str()));
    const std::string tempMetadataValues(
                image->GetMetaData<std::string>(mConfiguration.MetadataValuesKey.c_str()));

    QStringList metadataLabels(
                QString(image->GetMetaData<std::string>(
                            mConfiguration.MetadataLabelsKey.c_str() ).c_str()).split(mConfiguration.MetadataSplitCharacter));

    QStringList metadataValues(
                QString(image->GetMetaData<std::string>(
                            mConfiguration.MetadataValuesKey.c_str() ).c_str()).split(mConfiguration.MetadataSplitCharacter));

    if (metadataLabels.size() != metadataValues.size())
    {
        return;
    }

    // we can just get on and display the hold absolute results and finish
    if (Modes::ImmediateLabelAbsolute == mConfiguration.Mode)
    {
        for (auto metadataLabelValue : boost::combine(metadataLabels, metadataValues))
        {
            QString metadataLabel, metadataValueStr;
            boost::tie(metadataLabel, metadataValueStr) = metadataLabelValue;

            bool metadataValueOk(false);
            const double metadataValue(metadataValueStr.toDouble(&metadataValueOk));

            if (metadataValueOk) {
                UpdateTrafficLight(metadataLabel.toStdString(), metadataValue);
            }
        }

        return;
    }

    // for other modes we can do some common min/max processing
    std::vector<std::pair<std::string, double> > pairedLabelsValues;
    auto minLabelValue( std::make_pair(std::string("Min"), std::numeric_limits<double>::max()) );
    auto maxLabelValue(std::make_pair(std::string("Max"), std::numeric_limits<double>::lowest()));

    // The combine niceness isn't in the stl, so using boost
    for (auto metadataLabelValue : boost::combine(metadataLabels, metadataValues))
    {
        QString metadataLabel, metadataValueStr;
        boost::tie(metadataLabel, metadataValueStr) = metadataLabelValue;

        bool metadataValueOk(false);
        const double metadataValue(metadataValueStr.toDouble(&metadataValueOk));

        if (false == metadataValueOk) {
            return;
        }

        pairedLabelsValues.push_back(std::make_pair(metadataLabel.toStdString(), metadataValue));
        
        // first one will be both the minimum and the maximum
        if (metadataValue > maxLabelValue.second)
        {
            maxLabelValue = pairedLabelsValues.back();
        }
        
        if (metadataValue < minLabelValue.second)
        {
            minLabelValue = pairedLabelsValues.back();
        }
    }


    if (Modes::HoldLabelLargest == mConfiguration.Mode)
    {
        UpdateTrafficLight(maxLabelValue.first, mConfiguration.ValueColorsVector.back().Value + 1.0);
    }
    else if (Modes::ImmediateLabelNormalised == mConfiguration.Mode)
    {
        for (auto pairLabelValue : pairedLabelsValues)
        {
            UpdateTrafficLight(
                        pairLabelValue.first,
                        (pairLabelValue.second - minLabelValue.second)/ (maxLabelValue.second - minLabelValue.second));
        }
    }
    else if (Modes::ImmediateBarNormalised == mConfiguration.Mode)
    {
        for (auto pairLabelValue : pairedLabelsValues)
        {
            if (mColorWithLevel==true){

                UpdateTrafficLight(
                            pairLabelValue.first,
                            (pairLabelValue.second - minLabelValue.second)/ (maxLabelValue.second - minLabelValue.second));
            }

            // find the traffic light which matches the metadata label
            auto levelMeterIter = mLevelMeters.find(pairLabelValue.first);

            if (levelMeterIter != mLevelMeters.end())
            {
                // then set its colours according to the metadata value
                levelMeterIter->second->LevelChanged(
                            (pairLabelValue.second - minLabelValue.second) / (maxLabelValue.second - minLabelValue.second));
            }
        }
    } else if (Modes::ImmediateBarAbsolute == mConfiguration.Mode)
    {
        for (auto pairLabelValue : pairedLabelsValues)
        {
            if (mColorWithLevel==true){

                UpdateTrafficLight(
                            pairLabelValue.first,
                            (pairLabelValue.second - minLabelValue.second)/ (maxLabelValue.second - minLabelValue.second));
            }

            // find the traffic light which matches the metadata label
            auto levelMeterIter = mLevelMeters.find(pairLabelValue.first);

            if (levelMeterIter != mLevelMeters.end())
            {
                // then set its colours according to the metadata value
                levelMeterIter->second->LevelChanged(pairLabelValue.second, (pairLabelValue.second - minLabelValue.second) / (maxLabelValue.second - minLabelValue.second));
            }
        }
    }


}

bool QtInfoPanelTrafficLightBase::colorWithLevel() const
{
    return mColorWithLevel;
}

void QtInfoPanelTrafficLightBase::setColorWithLevel(bool colorWithLevel)
{
    mColorWithLevel = colorWithLevel;
}

void QtInfoPanelTrafficLightBase::UpdateTrafficLight(
        const std::string &metadataLabel,
        const double metadataValue)
{
    // find the traffic light which matches the metadata label
    auto trafficLightIter = mTrafficLights.find(metadataLabel);

    if (trafficLightIter != mTrafficLights.end())
    {
        // then set its colours according to the metadata value
        UpdateLabelColors(trafficLightIter->second, metadataValue);
    }
}

void QtInfoPanelTrafficLightBase::UpdateLabelColors(QLabel *label, const double value)
{
    // need to check the largest value first - which is at the end
    // so 'auto for' won't work - boost to the rescue
    for (auto valueColor : boost::adaptors::reverse(mConfiguration.ValueColorsVector))
    {

        if (mColorWithLevel==true){
            ValueColors currentColor = mConfiguration.ValueColorsVector[0];

            double H, S, V;
            currentColor.TextColor.getHsvF(&H, &S, &V);
            S = value;
            V = value;
            currentColor.TextColor.setHsvF(H, S, V);
            SetLabelColors(label, currentColor);
        } else {
            if (value >= valueColor.Value)
            {
                SetLabelColors(label, valueColor);
                break;
            }
        }
    }
}

void QtInfoPanelTrafficLightBase::SetLabelColors(QLabel *label, const ValueColors &valuesColors)
{
    QPalette palette = label->palette();
    palette.setColor(label->backgroundRole(), valuesColors.BackgroundColor);
    palette.setColor(label->foregroundRole(), valuesColors.TextColor);
    label->setPalette(palette);
}

