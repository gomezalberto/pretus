#include "Widget_framegrabber.h"

#include <QLabel>
#include <QVBoxLayout>

Widget_framegrabber::Widget_framegrabber(
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

void Widget_framegrabber::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;

    if (image->HasKey("StreamTime")){
        stream << "Stream time: "<<image->GetMetaData<std::string>("StreamTime") << std::endl;
    }
    //uint64_t t_dnl = std::atol(image->GetMetaData<std::string>("DNLTimestamp").c_str());
    //stream << "Timestamp DNL: " << t_dnl;



    mLabel->setText(stream.str().c_str());
}
