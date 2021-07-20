#include "QtFixedAspectRatio.h"

#include <QBoxLayout>
#include <QResizeEvent>

QtFixedAspectRatio::QtFixedAspectRatio(
    QWidget *widget, double width, double height, QWidget *parent)
    :
    QWidget(parent),
    mWidget(widget),
    mWidthToHeightRatio(width / height),
    mHeightToWidthRatio(1.0 / mWidthToHeightRatio)
{
    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(widget, 0, 0);
}

void QtFixedAspectRatio::resizeEvent(QResizeEvent *event)
{
    double thisAspectRatio = (double)event->size().width() / event->size().height();
    int widgetWidth, widgetHeight;

    if (thisAspectRatio > mWidthToHeightRatio) // too wide
    {
        widgetWidth = height() * mWidthToHeightRatio; // i.e., my width
        widgetHeight = height();
    }
    else // too tall
    {
        widgetWidth = width(); // i.e., my width
        widgetHeight = width() * mHeightToWidthRatio;
    }

    mWidget->resize(widgetWidth, widgetHeight);
}

