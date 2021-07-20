#include "QtPluginWidgetBase.h"

static const std::string sDefaultStreamTypesStr("Input");

QtPluginWidgetBase::QtPluginWidgetBase(
    QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , mStreamTypes(ifind::InitialiseStreamTypeSetFromString(sDefaultStreamTypesStr))
    , mPluginName("N/A")
    , mViewImageCheckbox(nullptr)
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

void QtPluginWidgetBase::AddImageViewCheckboxToLayout(QBoxLayout *vLayout){

    mViewImageCheckbox = new QCheckBox("View image",this);
    mViewImageCheckbox->setChecked(true);
    mViewImageCheckbox->setStyleSheet("QCheckBox { background-color : black; color : white; }");
    vLayout->addWidget(mViewImageCheckbox);
}

void QtPluginWidgetBase::SetStreamTypesFromStr(const std::string &streamTypesStr)
{
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString(streamTypesStr);
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

