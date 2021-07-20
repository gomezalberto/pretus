#include "QtInfoPanelBase.h"

static const std::string sDefaultStreamTypesStr("Input");

QtInfoPanelBase::QtInfoPanelBase(
    QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , mStreamTypes(ifind::InitialiseStreamTypeSetFromString(sDefaultStreamTypesStr))
{
}

const ifind::StreamTypeSet &QtInfoPanelBase::StreamTypes() const
{
    return mStreamTypes;
}

void QtInfoPanelBase::SetStreamTypes(const ifind::StreamTypeSet &streamTypesIn)
{
    mStreamTypes = streamTypesIn;
}

void QtInfoPanelBase::SetStreamTypesFromStr(const std::string &streamTypesStr)
{
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString(streamTypesStr);
}

void QtInfoPanelBase::SendImageToWidget(ifind::Image::Pointer image)
{
    if (ifind::IsImageOfStreamTypeSet(image, mStreamTypes)) {
        SendImageToWidgetImpl(image);
    }
}

