#include "Widget_PythonAlgorithm.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QtInfoPanelTrafficLightBase.h>

Widget_PythonAlgorithm::Widget_PythonAlgorithm(QWidget *parent, Qt::WindowFlags f): QtPluginWidgetBase(parent, f)
{
    this->SetWidgetLocation(WidgetLocation::top_right);
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("PythonAlgorithm");
    //------ define gui ----------

    mLabel = new QLabel("Text not set", this);
    mLabel->setStyleSheet("QLabel { background-color : black; color : white; }");

    auto labelFont = mLabel->font();
    labelFont.setPixelSize(15);
    labelFont.setBold(true);
    mLabel->setFont(labelFont);
    //
    mSlider = new QSlider(Qt::Orientation::Horizontal);
    mSlider ->setStyleSheet(QtPluginWidgetBase::sQSliderStyle);
    mSlider->setMaximum(50);
    mSlider->setMinimum(1);
    mSlider->setAutoFillBackground(true);


    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
    vLayout->addWidget(mSlider);

    this->AddImageViewCheckboxToLayout(vLayout);
}

void Widget_PythonAlgorithm::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "==" << std::endl;

    if (image->HasKey("PythonAlgorithm_sigma")){
        stream << "Sigma: "<< image->GetMetaData<std::string>("PythonAlgorithm_sigma") << std::endl;
        stream << "Delay: "<< image->GetMetaData<std::string>("PythonAlgorithm_delay") << "s"<<std::endl;
    }

    mLabel->setText(stream.str().c_str());

    Q_EMIT this->ImageAvailable(image);
}
