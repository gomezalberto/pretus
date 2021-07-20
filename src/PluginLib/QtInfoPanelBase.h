#pragma once
#include <QWidget>

#include <ifindImage.h>

#include <ifindStreamTypeHelper.h>

class QLabel;

class QtInfoPanelBase : public QWidget
{
    Q_OBJECT

public:
    QtInfoPanelBase(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    const ifind::StreamTypeSet &StreamTypes() const;
    void SetStreamTypes(const ifind::StreamTypeSet &streamTypesIn);
    void SetStreamTypesFromStr(const std::string &streamTypesStr);

public Q_SLOTS:
    void SendImageToWidget(ifind::Image::Pointer image);

protected:

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image) = 0;

    ifind::StreamTypeSet mStreamTypes;

private:

};
