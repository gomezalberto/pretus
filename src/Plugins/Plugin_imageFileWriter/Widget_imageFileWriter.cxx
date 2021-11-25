#include "Widget_imageFileWriter.h"

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

Widget_imageFileWriter::Widget_imageFileWriter(
    QWidget *parent, Qt::WindowFlags f)
    : QtPluginWidgetBase(parent, f)
{
    this->mWidgetLocation = WidgetLocation::top_left;
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");
    this->n_images_written = 0;
    this->mOutputFolder = "";

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

    mCheckBoxSaveFiles = new QCheckBox("Save to disk", this);
    mCheckBoxSaveFiles->setStyleSheet(QtPluginWidgetBase::sQCheckBoxStyle);
    mCheckBoxSaveFiles->setChecked(true);
    mPushButtonSaveOne = new QPushButton("Save one");
    mPushButtonSaveOne->setStyleSheet(QtPluginWidgetBase::sQPushButtonStyle);

    auto hlayout = new QHBoxLayout();
    hlayout->addWidget(mCheckBoxSaveFiles);
    hlayout->addWidget(mPushButtonSaveOne);

    mSubfolderText = new QLineEdit("");
    mSubfolderText->setStyleSheet(QtPluginWidgetBase::sQLineEditStyle);
    QLabel *labeltext = new QLabel("Subfolder: ");
    labeltext->setStyleSheet(QtPluginWidgetBase::sQLabelStyle);
    auto hlayout2 = new QHBoxLayout();
    hlayout2->addWidget(labeltext );
    hlayout2->addWidget(mSubfolderText);

    vLayout->addWidget(mLabel);
    vLayout->addLayout(hlayout);
    vLayout->addLayout(hlayout2);
}

void Widget_imageFileWriter::slot_imageWritten(ifind::Image::Pointer image){
        this->n_images_written++;
}

void Widget_imageFileWriter::setOutputFolder(const QString &outputFolder)
{
    mOutputFolder = outputFolder;
}

void Widget_imageFileWriter::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;
    //stream << "Receiving " << ifind::StreamTypeSetToString(this->mInputStreamTypes) << std::endl;
    //stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes) << std::endl;

    std::stringstream stream_phrase;
    if (ifind::StreamTypeSetToString(mStreamTypes).size()==0 ){
        stream_phrase << "all streams";
    } else {
        stream_phrase << "the\""<< ifind::StreamTypeSetToString(mStreamTypes)<< "\"  stream(s)";
    }

    stream << "Save "<< stream_phrase.str() <<" ["<< this->n_images_written<< "]"<< std::endl;
    stream << "Output folder: "<< std::endl<< this->mOutputFolder.toStdString()<< "*/"<< this->mSubfolderText->text().toStdString()<< std::endl;

    mLabel->setText(stream.str().c_str());
}

