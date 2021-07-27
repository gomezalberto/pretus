#include "Widget_planeDetection.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QtInfoPanelTrafficLightBase.h>

Widget_planeDetection::Widget_planeDetection(
        QWidget *parent, Qt::WindowFlags f)
    : QtPluginWidgetBase(parent, f)
{

    this->mWidgetLocation = WidgetLocation::top_right;
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Standardplanedetection");
    mIsBuilt = false;

    mLabel = new QLabel("Text not set", this);
    mLabel->setStyleSheet(sQLabelStyle);
    //
    mSlider = new QSlider(Qt::Orientation::Horizontal);
    mSlider->setStyleSheet(QtPluginWidgetBase::sQSliderStyle);

    mSlider->setMaximum(101);
    mSlider->setMinimum(0);
    mSlider->setAutoFillBackground(true);

    //
    mSliderTA = new QSlider(Qt::Orientation::Horizontal);
    mSliderTA->setStyleSheet(QtPluginWidgetBase::sQSliderStyle);

    mSliderTA->setMaximum(100);
    mSliderTA->setMinimum(0);
    mSliderTA->setAutoFillBackground(true);
}

void Widget_planeDetection::Build(){

    auto labelFont = mLabel->font();
    labelFont.setPixelSize(15);
    labelFont.setBold(true);
    mLabel->setFont(labelFont);

    QVBoxLayout * outmost_layout = new QVBoxLayout(this);
    outmost_layout->addWidget(mLabel, 1, Qt::AlignTop);
    {
        QHBoxLayout * slider_layout = new QHBoxLayout();
        QLabel *sliderLabel = new QLabel("(1)",this);
        sliderLabel->setStyleSheet(sQLabelStyle);
        slider_layout->addWidget(sliderLabel);
        slider_layout->addWidget(mSlider);
        outmost_layout->addLayout(slider_layout);
    }
    {
        QHBoxLayout * slider_layout = new QHBoxLayout();
        QLabel *sliderLabel = new QLabel("(2)",this);
        sliderLabel->setStyleSheet(sQLabelStyle);
        slider_layout->addWidget(sliderLabel);
        slider_layout->addWidget(mSliderTA);
        outmost_layout->addLayout(slider_layout);
    }

    if (mWidgetOptions.show_bars == true) {
        //QVBoxLayout *vLayout = dynamic_cast<QVBoxLayout*>(outmost_layout);
        /// This will have bar graphs of the live scan plane values
        {
            QtInfoPanelTrafficLightBase::Configuration standardPlaneTrafficLightConfig;

            standardPlaneTrafficLightConfig.Mode =
                    QtInfoPanelTrafficLightBase::Modes::ImmediateBarNormalised;
            standardPlaneTrafficLightConfig.LabelNames = { "3VV","4CH","Abdominal",
                                                           "Background","Brain (Cb.)","Brain (Tv.)", "Femur","Kidneys","Lips","LVOT",
                                                           "Profile","RVOT","Spine (cor.)","Spine (sag.)" };
            standardPlaneTrafficLightConfig.NGridColumns = 2;

            standardPlaneTrafficLightConfig.ValueColorsVector.push_back(
                        QtInfoPanelTrafficLightBase::ValueColors(
                            std::numeric_limits<double>::lowest(), // value
                            QColor("black"), // background colour
                            QColor("silver"))); // text colour

            standardPlaneTrafficLightConfig.MetadataLabelsKey = "Standardplanedetection_labels";
            standardPlaneTrafficLightConfig.MetadataValuesKey = "Standardplanedetection_confidences";
            standardPlaneTrafficLightConfig.MetadataSplitCharacter = ',';

            auto infoPanel = new QtInfoPanelTrafficLightBase(standardPlaneTrafficLightConfig, this);
            infoPanel->SetStreamTypesFromStr("Standardplanedetection");
            outmost_layout->addWidget(infoPanel, 1, Qt::AlignTop);

            QObject::connect(this, &QtPluginWidgetBase::ImageAvailable,
                             infoPanel, &QtInfoPanelBase::SendImageToWidget);
        }
        /// This will 'light up' as each scan plane is found
        if (mWidgetOptions.show_checklist == true) {
            QtInfoPanelTrafficLightBase::Configuration standardPlaneTrafficLightConfig;

            standardPlaneTrafficLightConfig.Mode =
                    QtInfoPanelTrafficLightBase::Modes::ImmediateLabelAbsolute;
            standardPlaneTrafficLightConfig.LabelNames ={ "3VV","4CH","Abdominal",
                                                          "Background","Brain (Cb.)","Brain (Tv.)", "Femur","Kidneys","Lips","LVOT",
                                                          "Profile","RVOT","Spine (cor.)","Spine (sag.)" };
            standardPlaneTrafficLightConfig.NGridColumns = 2;

            standardPlaneTrafficLightConfig.ValueColorsVector.push_back(
                        QtInfoPanelTrafficLightBase::ValueColors(
                            std::numeric_limits<double>::lowest(), // value
                            QColor("black"), // background colour
                            QColor("silver"))); // text colour

            standardPlaneTrafficLightConfig.ValueColorsVector.push_back(
                        QtInfoPanelTrafficLightBase::ValueColors(
                            5.0, // value
                            QColor(214, 123, 1), // background colour - like coral but darker and less red
                            QColor("white"))); // black"))); // text colour

            standardPlaneTrafficLightConfig.ValueColorsVector.push_back(
                        QtInfoPanelTrafficLightBase::ValueColors(
                            10.0, // value
                            QColor("green"), // background colour
                            QColor("white"))); // black"))); // text colour

            standardPlaneTrafficLightConfig.MetadataLabelsKey = "Autoreport_spd_labels";
            standardPlaneTrafficLightConfig.MetadataValuesKey = "Autoreport_Standardplane_clusterR";
            standardPlaneTrafficLightConfig.MetadataSplitCharacter = ',';

            auto infoPanel = new QtInfoPanelTrafficLightBase(
                        standardPlaneTrafficLightConfig, this);
            infoPanel->SetStreamTypesFromStr("AutoReport");
            outmost_layout->addWidget(infoPanel, 1, Qt::AlignTop);

            QObject::connect(
                        this, &QtPluginWidgetBase::ImageAvailable,
                        infoPanel, &QtInfoPanelBase::SendImageToWidget);
        }
    }

}

void Widget_planeDetection::SendImageToWidgetImpl(ifind::Image::Pointer image){

    if (mIsBuilt == false){
        mIsBuilt = true;
        this->Build();
    }

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "==";

    if (image->HasKey("Standardplanedetection_bckth")){
        stream << std::endl<< "(1) Background threshold: "<< image->GetMetaData<std::string>("Standardplanedetection_bckth");
        stream << std::endl<< "(2) Averaged frames: "<< image->GetMetaData<std::string>("Standardplanedetection_tempAvg");
    }

    mLabel->setText(stream.str().c_str());

    Q_EMIT this->ImageAvailable(image);
}
