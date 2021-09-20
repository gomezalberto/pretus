#include "Widget_filemanager.h"

#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSignalBlocker>

Widget_filemanager::Widget_filemanager(
        QWidget *parent, Qt::WindowFlags f)
    : QtPluginWidgetBase(parent, f)
{

    this->mWidgetLocation = WidgetLocation::top_left;

    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");

    mLabel = new QLabel("Text not set", this);
    mLabel->setStyleSheet(QtPluginWidgetBase::sQLabelStyle);

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
    // create a miniwidget for the play/pause/slider
    {
        mPausePlayButton = new QPushButton("Pause");
        mPausePlayButton->setStyleSheet(QtPluginWidgetBase::sQPushButtonStyle);
        mPausePlayButton->setCheckable(true);
        mNextButton= new QPushButton(">");
        mNextButton->setStyleSheet(QtPluginWidgetBase::sQPushButtonStyle);
        mPreviousButton= new QPushButton("<");
        mPreviousButton->setStyleSheet(QtPluginWidgetBase::sQPushButtonStyle);
        QWidget *placeholder = new QWidget();
        QHBoxLayout *ph_layout = new QHBoxLayout();
        ph_layout->addWidget(mSlider);
        ph_layout->addWidget(mPreviousButton);
        ph_layout->addWidget(mPausePlayButton);
        ph_layout->addWidget(mNextButton);
        placeholder->setLayout(ph_layout);
        vLayout->addWidget(placeholder);
    }
    this->AddImageViewCheckboxToLayout(vLayout);
}

void Widget_filemanager::slot_togglePlayPause(bool v){

    if (v == true){
        this->mPausePlayButton->setText("Play ");
    } else {
        this->mPausePlayButton->setText("Pause");
    }

}

void Widget_filemanager::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;
    stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes) << std::endl;

    if (image->HasKey("TransmitedFrameCount")){
        stream << "Image "<<image->GetMetaData<std::string>("CurrentFrame") << "/" << image->GetMetaData<std::string>("FrameCountTotal")<< "("<<image->GetMetaData<std::string>("TransmitedFrameCount")<<")"<< std::endl;
    }
    if (image->HasKey("AcquisitionFrameRate")){
        stream << "Framerate: "<<image->GetMetaData<std::string>("AcquisitionFrameRate") << " fps"<< std::endl;
    }

    // update the slider
    if (image->HasKey("CurrentFrame")){
        int current_frame = stoi(image->GetMetaData<std::string>("CurrentFrame"));
        int total_frames = stoi(image->GetMetaData<std::string>("FrameCountTotal"));

        int current_slider_pos = (current_frame*100/total_frames)*10; // this convoluted *1000 in two steps is to only update every 1/100.
        // prevent the callback
        {
            const QSignalBlocker blocker(this->mSlider);
            this->mSlider->setValue(current_slider_pos);
        }

    }


    mLabel->setText(stream.str().c_str());
}
