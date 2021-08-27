#include "Widget_videomanager.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>

Widget_videomanager::Widget_videomanager(
    QWidget *parent, Qt::WindowFlags f)
    : QtPluginWidgetBase(parent, f)
{
    this->mWidgetLocation = WidgetLocation::bottom_left;

    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");

    mLabel = new QLabel("Text not set", this);
    mLabel->setStyleSheet("QLabel { background-color : black; color : white; }");

    auto labelFont = mLabel->font();
    labelFont.setPixelSize(15);
    labelFont.setBold(true);
    mLabel->setFont(labelFont);
    //
    mSlider = new QSlider(Qt::Orientation::Horizontal);
    mSlider->setStyleSheet(QtPluginWidgetBase::sQSliderStyle);

    mSlider->setMaximum(1000);
    mSlider->setMinimum(0);
    mSlider->setAutoFillBackground(true);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
    vLayout->addWidget(mSlider);
    this->AddImageViewCheckboxToLayout(vLayout);
}

void Widget_videomanager::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;
    stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes) << std::endl;

    if (image->HasKey("VideoTime")){
        stream << "Play time: "<<image->GetMetaData<std::string>("VideoTime") << std::endl;
        stream << "Acq. FR / Eff. FR: "<<image->GetMetaData<std::string>("AcquisitionFrameRate") << "/"
        << std::fixed << std::setprecision(1)  << atof(image->GetMetaData<std::string>("TransmissionFrameRate").c_str())
        << " Hz"<< std::endl;

    }

    // update the slider
    if (image->HasKey("CurrentVideoFrame")){
        int current_frame = stoi(image->GetMetaData<std::string>("CurrentVideoFrame"));
        int total_frames = stoi(image->GetMetaData<std::string>("TotalVideoFrames"));

        int current_slider_pos = (current_frame*100/total_frames)*10; // this convoluted *1000 in two steps is to only update every 1/100.
        this->mSlider->setValue(current_slider_pos);

    }

    //uint64_t t_dnl = std::atol(image->GetMetaData<std::string>("DNLTimestamp").c_str());
    //stream << "Timestamp DNL: " << t_dnl;



    mLabel->setText(stream.str().c_str());
}

