#include "Widget_imageFileWriter.h"

#include <QLabel>
#include <QVBoxLayout>

Widget_imageFileWriter::Widget_imageFileWriter(
    QWidget *parent, Qt::WindowFlags f)
    : QtPluginWidgetBase(parent, f)
{

    this->mWidgetLocation = WidgetLocation::bottom_left;
    //mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Videomanager");
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");
    this->n_images_written = 0;
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

void Widget_imageFileWriter::slot_imageWritten(ifind::Image::Pointer image){
    if (image->HasKey("DO_NOT_WRITE")){
        // image should not be written.
        //std::cout << "do not write"<<std::endl;
    } else {
        this->n_images_written++;

    }
}

void Widget_imageFileWriter::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "=="<< std::endl;
    stream << "Receiving " << ifind::StreamTypeSetToString(this->mInputStreamTypes) << std::endl;
    stream << "Sending " << ifind::StreamTypeSetToString(this->mStreamTypes) << std::endl;

    stream << "Saving the \""<< ifind::StreamTypeSetToString(mStreamTypes)<< "\" stream(s)"<< std::endl;
    stream << "Saved "<< this->n_images_written<< " images"<< std::endl;


    mLabel->setText(stream.str().c_str());
}

