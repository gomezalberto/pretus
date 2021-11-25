#include "QtPluginWidgetBase.h"
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

static const std::string sDefaultStreamTypesStr("Input");
const QString QtPluginWidgetBase::sQSliderStyle = "QSlider::groove:horizontal { background-color: none; border: 1px solid #828282;  height: 2px;  border-radius: 2px; }"
    "QSlider::handle:horizontal { background-color: white; border: 2px solid white; width: 10px; height: 10px; line-height: 12px; margin-top: -6px; margin-bottom: -6px; border-radius: 2px; }"
    "QSlider:sub-page:horizontal { background-color: rgb(50, 150, 255)}";

const QString QtPluginWidgetBase::sQCheckBoxStyle = "QCheckBox { background-color : black; color : white} "
    "QCheckBox::indicator {border: 2px solid white; background : none; color: white; border-radius: 3px;}"
    "QCheckBox::indicator:checked {border: 2px solid white; background :  rgb(50, 150, 255); color: white;}";

const QString QtPluginWidgetBase::sQComboBoxStyle = "QComboBox { background-color : black; color : white} "
    "QComboBox::indicator {border: 2px solid white; background : none; color: white; border-radius: 3px;}"
    "QComboBox::indicator:checked {border: 2px solid white; background :  rgb(50, 150, 255); color: white;}";

const QString QtPluginWidgetBase::sQLabelStyle = "QLabel { background-color : black; color : white; }";

const QString QtPluginWidgetBase::sQPushButtonStyle = "QPushButton { border: 2px solid white; background-color : black; color : white; } "
        "QPushButton::pressed {border: 2px solid white; background :  rgb(50, 150, 255); color: white;}"
        "QPushButton::checked {border: 2px solid white; background :  rgb(50, 150, 255); color: white;}";

const QString QtPluginWidgetBase::sQGroupBoxStyle = "QGroupBox { border: 1px solid white; background-color : black; color : white; } "
        "QGroupBox::title {subcontrol-origin: margin; subcontrol-position: top center; top: -4px; color: #000000; background-color: rgb(255, 255, 255);}";


const QString QtPluginWidgetBase::sQLineEditStyle  = "QLineEdit { background-color : black; color : white; }";

QtPluginWidgetBase::QtPluginWidgetBase(
    QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
    , mStreamTypes(ifind::InitialiseStreamTypeSetFromString(sDefaultStreamTypesStr))
    , mPluginName("N/A")
    , mViewImageCheckbox(nullptr)
    , mInputStreamComboBox(nullptr)
    , mInputLayerComboBox(nullptr)
{
    this->mWidgetLocation = WidgetLocation::visible;
}

const ifind::StreamTypeSet &QtPluginWidgetBase::StreamTypes() const
{
    return mStreamTypes;
}

void QtPluginWidgetBase::SetStreamTypes(const ifind::StreamTypeSet &streamTypesIn)
{
    mStreamTypes = streamTypesIn;
}

const ifind::StreamTypeSet &QtPluginWidgetBase::InputStreamTypes() const
{
    return mInputStreamTypes;
}

void QtPluginWidgetBase::SetInputStreamTypes(const ifind::StreamTypeSet &streamTypesIn)
{
    mInputStreamTypes = streamTypesIn;
}

void QtPluginWidgetBase::AddInputStreamComboboxToLayout(QBoxLayout *vLayout){

    QWidget *placeholder = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout();

    QLabel *label = new QLabel("Input: ");
    label->setStyleSheet(sQLabelStyle);
    mInputStreamComboBox = new QComboBox(this);
    mInputStreamComboBox->setStyleSheet(sQComboBoxStyle);
    mInputLayerComboBox = new QComboBox(this);
    mInputLayerComboBox->setStyleSheet(sQComboBoxStyle);
    mInputLayerComboBox->addItem("Layer 0");
    layout->addWidget(label);
    layout->addWidget(mInputStreamComboBox);
    layout->addWidget(mInputLayerComboBox);
    placeholder->setLayout(layout);
    vLayout->addWidget(placeholder);
}

void QtPluginWidgetBase::AddImageViewCheckboxToLayout(QBoxLayout *vLayout){

    mViewImageCheckbox = new QCheckBox("View image",this);
    mViewImageCheckbox->setChecked(true);
    mViewImageCheckbox->setStyleSheet(sQCheckBoxStyle);
    vLayout->addWidget(mViewImageCheckbox);
}

void QtPluginWidgetBase::SetStreamTypesFromStr(const std::string &streamTypesStr)
{
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString(streamTypesStr);
}

void QtPluginWidgetBase::SetInputStreamTypesFromStr(const std::string &streamTypesStr)
{
    mInputStreamTypes = ifind::InitialiseStreamTypeSetFromString(streamTypesStr);
}


void QtPluginWidgetBase::SendImageToWidget(ifind::Image::Pointer image)
{
    if (ifind::IsImageOfStreamTypeSet(image, mStreamTypes)) {
        this->SendImageToWidgetImpl(image);
    }
 }

void QtPluginWidgetBase::SetImageWidgetVisibility(int i){
    //std::cout << "QtPluginWidgetBase::SetImageWidgetVisibility changed visibility to "<< i <<std::endl;
    if (i!=0){
        this->mWidgetLocation = WidgetLocation::visible;
        this->show();
    } else {
        this->mWidgetLocation = WidgetLocation::hidden;
        this->hide();
    }
}

QtPluginWidgetBase::WidgetLocation QtPluginWidgetBase::GetWidgetLocation(){
    return this->mWidgetLocation;
}

void QtPluginWidgetBase::SetWidgetLocation(WidgetLocation location){
    this->mWidgetLocation = location;
}

QString QtPluginWidgetBase::pluginName() const
{
    return mPluginName;
}

void QtPluginWidgetBase::setPluginName(const QString &pluginName)
{
    mPluginName = pluginName;
}

