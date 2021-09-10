#include "Widget_planeDetection.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QtInfoPanelTrafficLightBase.h>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

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

    mSlider->setMaximum(100);
    mSlider->setMinimum(0);
    mSlider->setValue(100);
    mSlider->setAutoFillBackground(true);

    //
    mSliderTA = new QSlider(Qt::Orientation::Horizontal);
    mSliderTA->setStyleSheet(QtPluginWidgetBase::sQSliderStyle);

    mSliderTA->setMaximum(100);
    mSliderTA->setMinimum(0);
    mSliderTA->setAutoFillBackground(true);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
    this->AddInputStreamComboboxToLayout(vLayout);
    vLayout->addWidget(mSlider);
    vLayout->addWidget(mSliderTA);


}

void Widget_planeDetection::Build(std::vector<std::string> &labelnames){

    //QVBoxLayout * outmost_layout = new QVBoxLayout(this);
    QVBoxLayout * outmost_layout = reinterpret_cast<QVBoxLayout*>(this->layout());
    /*outmost_layout->addWidget(mLabel, 1, Qt::AlignTop);
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
    this->AddInputStreamComboboxToLayout(outmost_layout);
    */
    if (mWidgetOptions.show_bars == true) {
        //QVBoxLayout *vLayout = dynamic_cast<QVBoxLayout*>(outmost_layout);
        /// This will have bar graphs of the live scan plane values
        {
            QtInfoPanelTrafficLightBase::Configuration standardPlaneTrafficLightConfig;

            standardPlaneTrafficLightConfig.Mode =
                    QtInfoPanelTrafficLightBase::Modes::ImmediateBarNormalised;
            standardPlaneTrafficLightConfig.LabelNames = labelnames;
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
        std::string labels = image->GetMetaData<std::string>((this->pluginName() +"_labels").toStdString().c_str());

        boost::char_separator<char> sep(",");
        boost::tokenizer< boost::char_separator<char> > tokens(labels, sep);
        std::vector<std::string> labelnames;
        BOOST_FOREACH (const std::string& t, tokens) {
                labelnames.push_back(t);
            }
//        std::vector<std::string> labelnames = { "3VV","4CH","Abdominal",
//                                                "Background","Brain (Cb.)","Brain (Tv.)", "Femur","Kidneys","Lips","LVOT",
//                                                "Profile","RVOT","Spine (cor.)","Spine (sag.)" };
        this->Build(labelnames);
    }

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<<std::endl;
    if (image->HasKey("Standardplanedetection_bckth")){
        stream << "(1) Background threshold: "<< image->GetMetaData<std::string>("Standardplanedetection_bckth")<< std::endl;
        stream << "(2) Averaged frames: "<< image->GetMetaData<std::string>("Standardplanedetection_tempAvg")<< std::endl;
    }
    //stream << "Receiving " << ifind::StreamTypeSetToString(this->mInputStreamTypes) << std::endl;
    stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes);

    mLabel->setText(stream.str().c_str());

    Q_EMIT this->ImageAvailable(image);
}
