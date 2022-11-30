#include "Widget_GUI.h"
#include <QSlider>
#include <QPushButton>
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
    mResetButton = new QPushButton("Reset");
    mResetButton->setStyleSheet(QtPluginWidgetBase::sQPushButtonStyle);


    mSlider = new QSlider(Qt::Orientation::Horizontal);
    mSlider->setStyleSheet(QtPluginWidgetBase::sQSliderStyle);

    mSlider->setMaximum(200);
    mSlider->setMinimum(30);
    mSlider->setValue(50);
    mSlider->setAutoFillBackground(true);

    auto sliderWidget = new QWidget();
    auto hLayout = new QHBoxLayout(this);
    mSlLabel = new QLabel("Scale: ");
    mSlLabel->setStyleSheet(QtPluginWidgetBase::sQLabelStyle);
    hLayout->addWidget(mSlLabel);
    hLayout->addWidget(mSlider);
    hLayout->addWidget(mResetButton);
    sliderWidget->setLayout(hLayout);
    this->slot_updateSliderLabel();


    QObject::connect(this->mSlider, &QSlider::valueChanged,
             this, &Widget_GUI::slot_updateSliderLabel);


    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
    vLayout->addWidget(sliderWidget);
}

void Widget_GUI::slot_updateSliderLabel(){

    int valueInt = mSlider->value();
    float valueFloat = valueInt / 100.0;

    QString s;
    s.sprintf("%.2f", valueFloat);

    QString text = "Scale: " + s.rightJustified(5, ' ');

    this->mSlLabel->setText(text);
}

void Widget_GUI::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "==" << std::endl;
    stream << "Receiving " << ifind::StreamTypeSetToString(this->mInputStreamTypes) << std::endl;
    stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes) << std::endl;

    mLabel->setText(stream.str().c_str());

    Q_EMIT this->ImageAvailable(image);
}
