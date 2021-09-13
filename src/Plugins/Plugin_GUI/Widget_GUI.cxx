#include "Widget_GUI.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QtInfoPanelTrafficLightBase.h>

Widget_GUI::Widget_GUI(QWidget *parent, Qt::WindowFlags f): QtPluginWidgetBase(parent, f)
{
    this->SetWidgetLocation(WidgetLocation::top_left);
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("GUI");

    //------ define gui ----------
    mLabel = new QLabel("==GUI==", this);
    mLabel->setStyleSheet(QtPluginWidgetBase::sQLabelStyle);

    auto labelFont = mLabel->font();
    labelFont.setPixelSize(15);
    labelFont.setBold(true);
    mLabel->setFont(labelFont);
    //
    mSlider = new QSlider(Qt::Orientation::Horizontal);
    mSlider->setStyleSheet(QtPluginWidgetBase::sQSliderStyle);

    mSlider->setMaximum(200);
    mSlider->setMinimum(30);
    mSlider->setValue(50);
    mSlider->setAutoFillBackground(true);

    auto sliderWidget = new QWidget();
    auto hLayout = new QHBoxLayout(this);
    auto slLabel = new QLabel("Scale: ");
    slLabel->setStyleSheet(QtPluginWidgetBase::sQLabelStyle);
    hLayout->addWidget(slLabel);
    hLayout->addWidget(mSlider);
    sliderWidget->setLayout(hLayout);


    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
    vLayout->addWidget(sliderWidget);
}

void Widget_GUI::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "==" << std::endl;
    stream << "Receiving " << ifind::StreamTypeSetToString(this->mInputStreamTypes) << std::endl;
    stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes) << std::endl;

    mLabel->setText(stream.str().c_str());

    Q_EMIT this->ImageAvailable(image);
}
