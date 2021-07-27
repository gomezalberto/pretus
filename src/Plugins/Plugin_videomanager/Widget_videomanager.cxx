#include "Widget_videomanager.h"

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

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
    this->AddImageViewCheckboxToLayout(vLayout);
}

void Widget_videomanager::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;

    if (image->HasKey("VideoTime")){
        stream << "Play time: "<<image->GetMetaData<std::string>("VideoTime") << std::endl;
        stream << "Acq. FR / Eff. FR: "<<image->GetMetaData<std::string>("AcquisitionFrameRate") << "/"
        << std::fixed << std::setprecision(1)  << atof(image->GetMetaData<std::string>("TransmissionFrameRate").c_str())
        << " Hz"<< std::endl;

    }
    //uint64_t t_dnl = std::atol(image->GetMetaData<std::string>("DNLTimestamp").c_str());
    //stream << "Timestamp DNL: " << t_dnl;



    mLabel->setText(stream.str().c_str());
}

