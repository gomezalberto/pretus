#pragma once
#include <QWidget>

class QtFixedAspectRatio : public QWidget
{
public:
    QtFixedAspectRatio(QWidget *widget, double width, double height, QWidget *parent = 0);
    virtual void resizeEvent(QResizeEvent *event);

private:
    QWidget *mWidget;

    double mWidthToHeightRatio;
    double mHeightToWidthRatio;
};
