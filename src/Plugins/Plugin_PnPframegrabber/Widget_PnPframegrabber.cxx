#include "Widget_PnPframegrabber.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

Widget_PnPframegrabber::Widget_PnPframegrabber(
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

        QWidget *placeholder = new QWidget();
        QHBoxLayout *ph_layout = new QHBoxLayout();
        ph_layout->addWidget(mPausePlayButton);
        ph_layout->addStretch();

        placeholder->setLayout(ph_layout);
        vLayout->addWidget(placeholder);
    }
    this->AddImageViewCheckboxToLayout(vLayout);
}

void Widget_PnPframegrabber::slot_togglePlayPause(bool v){

    if (v == true){
        this->mPausePlayButton->setText("Play ");
    } else {
        this->mPausePlayButton->setText("Pause");
    }

}

void Widget_PnPframegrabber::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;
    stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes) << std::endl;

    if (image->HasKey("StreamTime")){
        stream << "Stream time: "<<image->GetMetaData<std::string>("StreamTime") << std::endl;
    }

    mLabel->setText(stream.str().c_str());
}
