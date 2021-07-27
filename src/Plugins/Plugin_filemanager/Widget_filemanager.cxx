#include "Widget_filemanager.h"

#include <QLabel>
#include <QVBoxLayout>

Widget_filemanager::Widget_filemanager(
    QWidget *parent, Qt::WindowFlags f)
    : QtPluginWidgetBase(parent, f)
{

    this->mWidgetLocation = WidgetLocation::bottom_left;

    //mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Videomanager");
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");

    mLabel = new QLabel("Text not set", this);
    mLabel->setStyleSheet("QLabel { background-color : black; color : white; }");

    auto labelFont = mLabel->font();
    labelFont.setPixelSize(15);
    labelFont.setBold(true);
    mLabel->setFont(labelFont);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
}

void Widget_filemanager::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;

    if (image->HasKey("TransmitedFrameCount")){
        stream << "Image "<<image->GetMetaData<std::string>("TransmitedFrameCount") << "/" << image->GetMetaData<std::string>("FrameCountTotal")<< std::endl;
    }
    if (image->HasKey("AcquisitionFrameRate")){
        stream << "Framerate: "<<image->GetMetaData<std::string>("AcquisitionFrameRate") << " fps"<< std::endl;
    }


    mLabel->setText(stream.str().c_str());
}
