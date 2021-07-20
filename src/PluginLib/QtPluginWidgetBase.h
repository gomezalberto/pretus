#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <ifindStreamTypeHelper.h>
#include <QCheckBox>
#include <QBoxLayout>

class QtPluginWidgetBase : public QWidget
{
    Q_OBJECT

public:

    enum class WidgetLocation  { visible, visible_overlay, hidden, // for images
                               top_left, top_right, bottom_left, bottom_right, overlaid}; // for other widgets

    QtPluginWidgetBase(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    const ifind::StreamTypeSet &StreamTypes() const;
    void SetStreamTypes(const ifind::StreamTypeSet &streamTypesIn);
    void SetStreamTypesFromStr(const std::string &streamTypesStr);

    QString pluginName() const;
    void setPluginName(const QString &pluginName);

    virtual WidgetLocation GetWidgetLocation();
    virtual void SetWidgetLocation(WidgetLocation location);

    QCheckBox *mViewImageCheckbox;

public Q_SLOTS:
    void SendImageToWidget(ifind::Image::Pointer image);
    void SetImageWidgetVisibility(int i);

Q_SIGNALS:
    /**
     * @brief an image can be sent to the different parts of the widget
     * @param image
     */
    void ImageAvailable(ifind::Image::Pointer image);

protected:

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image) = 0;
    virtual void AddImageViewCheckboxToLayout(QBoxLayout *vLayout);
    ifind::StreamTypeSet mStreamTypes;
    QString mPluginName;

    WidgetLocation mWidgetLocation;

};
