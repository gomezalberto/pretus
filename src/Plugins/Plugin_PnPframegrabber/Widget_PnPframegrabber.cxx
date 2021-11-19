#include "Widget_PnPframegrabber.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
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

    {
        // create a miniwidget for the play/pause/slider
        mPausePlayButton = new QPushButton("⏸︎");
        mPausePlayButton->setStyleSheet(QtPluginWidgetBase::sQPushButtonStyle);
        mPausePlayButton->setCheckable(true);

        QWidget *placeholder = new QWidget();
        QHBoxLayout *ph_layout = new QHBoxLayout();
        ph_layout->addWidget(mPausePlayButton);

        // create a miniwidget for selecting framerates
        mFrameRateList = new QComboBox(this);
        mFrameRateList->setStyleSheet(QtPluginWidgetBase::sQComboBoxStyle);
        ph_layout->addWidget(mFrameRateList);

        mResolutionList = new QComboBox(this);
        mResolutionList->setStyleSheet(QtPluginWidgetBase::sQComboBoxStyle);
        ph_layout->addWidget(mResolutionList);

        mEncodingList = new QComboBox(this);
        mEncodingList->setStyleSheet(QtPluginWidgetBase::sQComboBoxStyle);
        ph_layout->addWidget(mEncodingList);

        ph_layout->addStretch();

        placeholder->setLayout(ph_layout);
        vLayout->addWidget(placeholder);
    }

    QObject::connect(this->mFrameRateList,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &Widget_PnPframegrabber::slot_updateFrameRate);

    QObject::connect(this->mResolutionList,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &Widget_PnPframegrabber::slot_updateResolution);

    QObject::connect(this->mEncodingList,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &Widget_PnPframegrabber::slot_updateEncoding);


    this->AddImageViewCheckboxToLayout(vLayout);
}

void Widget_PnPframegrabber::setFrameRates(std::vector<int> &framerates){
    for (int i=0; i<framerates.size(); i++){
        mFrameRateList->addItem(QString::number(framerates[i]) + " fps");
    }
}

void Widget_PnPframegrabber::setResolutions(std::vector<std::string> &resolutions){
    for (int i=0; i<resolutions.size(); i++){
        mResolutionList->addItem(QString::fromStdString(resolutions[i]));
    }
}

void Widget_PnPframegrabber::setEncodings(std::vector<std::string> &encs){
    for (int i=0; i<encs.size(); i++){
        mEncodingList->addItem(QString::fromStdString(encs[i]));
    }
}

void Widget_PnPframegrabber::slot_togglePlayPause(bool v){

    if (v == true){
        this->mPausePlayButton->setText("▶");
    } else {
        this->mPausePlayButton->setText("⏸︎");
    }

}

void Widget_PnPframegrabber::slot_updateFrameRate(int idx){
    QString newframerate = mFrameRateList->itemText(idx);
    Q_EMIT signal_newFrameRate(newframerate);

}

void Widget_PnPframegrabber::slot_updateResolution(int idx){
    QString newres = mResolutionList->itemText(idx);
    Q_EMIT signal_newResolution(newres);
}

void Widget_PnPframegrabber::slot_updateEncoding(int idx){
    QString newenc = mEncodingList->itemText(idx);
    Q_EMIT signal_newEncoding(newenc);
}

void Widget_PnPframegrabber::setSelectedFramerate(double r_fr){

    int idx = -1;

    for (int i=0; i<mFrameRateList->count(); i++){
        QString item_name = mFrameRateList->itemText(i);
        QStringList pieces0 = item_name.split( " " );

        double item_fr= pieces0.value(0).toDouble();

        if (r_fr == item_fr){
            idx = i;
            break;
        }
    }

    mFrameRateList->setCurrentIndex(idx);
}

void Widget_PnPframegrabber::setSelectedResolution(QString res){

    QStringList pieces = res.split( "." );
    int r_w = pieces.value(0).toInt();
    int r_h = pieces.value(1).toInt();

    int idx = -1;

    for (int i=0; i<mResolutionList->count(); i++){
        QString item_name = mResolutionList->itemText(i);
        QStringList pieces0 = item_name.split( " " );

        QStringList pieces = pieces0.value(0).split( "x" );
        int item_w = pieces.value(0).toInt();
        int item_h = pieces.value(1).toInt();

        if (r_w == item_w && r_h == item_h){
            idx = i;
            break;
        }
    }

    mResolutionList->setCurrentIndex(idx);

}

void Widget_PnPframegrabber::setSelectedEncoding(QString enc){

    int idx = -1;

    for (int i=0; i<mEncodingList->count(); i++){
        QString item_name = mEncodingList->itemText(i);
        if (enc.toLower()==item_name.toLower()){
            idx = i;
            break;
        }
    }

    mEncodingList->setCurrentIndex(idx);

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
