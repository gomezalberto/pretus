#include "QtVisualizationMainWindow.h"
#include <QDebug>
#include "QtFixedAspectRatio.h"
#include "QtVTKVisualization.h"

#include "VisualizationConsts.h"
#include <cmath>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QSizePolicy>
#include <ifindStreamTypeHelper.h>
#include <QVTKWidget.h>

QtVisualizationMainWindow::QtVisualizationMainWindow( QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)  {
    // All of these autos will end up as raw pointers -
    // However, as they are ((great-)grand-)children of this widget,
    // this will take care of their destruction

    // create a widget to contain the left and right hand bars and the render
    // create the render widget

    // define the colors
    mWidgetColors = QStringList({"red", "green", "blue", "cyan", "magenta", "yellow",
                                 "gray", "darkRed", "darkGreen", "darkBlue", "darkCyan",
                                 "darkMagenta", "darkYellow", "darkGray", "lightGray",
                                 "color0", "color1", "white", "black"});

    // set up the left panel
    {
        auto panelWidget = new QWidget();
        panelWidget->move(0, 0);
        //panelWidget->show();

        auto vLayout = new QVBoxLayout();
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);
        panelWidget->setLayout(vLayout);
        mLeftPanelWidget = panelWidget;
    }

    // set the central panel
    {
        // there is something to render
        auto panelWidget = new QWidget();
        panelWidget->move(0, 0); // move within the parent (will be the horizontal layout)

        auto gLayout = new QGridLayout ();
        gLayout->setContentsMargins(1, 1, 1, 1); /// leave some margin to see the border color
        gLayout->setSpacing(0);
        panelWidget->setLayout(gLayout);
        mCentralPanelWidget = panelWidget;
    }

    // set up the right panel
    {
        auto panelWidget = new QWidget();
        panelWidget->move(0, 0);

        auto vLayout = new QVBoxLayout(panelWidget);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);
        panelWidget->setLayout(vLayout);
        mRightPanelWidget = panelWidget;
    }

    // create a widget to contain the left and right hand bars and the render
    auto horizontalWidget = new QWidget(this);
    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    //horizontalLayout->setSpacing(0);

    horizontalLayout->addWidget(mLeftPanelWidget);
    horizontalLayout->addWidget(mCentralPanelWidget);
    horizontalLayout->addWidget(mRightPanelWidget);
    horizontalWidget->setLayout(horizontalLayout);

    setCentralWidget(horizontalWidget);

    // this may not be needed - try removing it
#ifndef WIN32
    // at least some of thisdoesn't work on windows
    setFocusPolicy(Qt::FocusPolicy::StrongFocus); // enable the window to take keyboard input by either tabbing or clicking
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    showFullScreen();
#else
    show();
#endif
}

void QtVisualizationMainWindow::SetWidgets(QList<QtPluginWidgetBase *> &widgets){
    this->mWidgets = widgets;
}

void QtVisualizationMainWindow::SetImageWidgets(QList<QtPluginWidgetBase *> &imageWidgets){
    this->mImageWidgets = imageWidgets;
}

void QtVisualizationMainWindow::Usage()
{
    //QtVTKVisualization::Parameters::Usage();
}



void QtVisualizationMainWindow::SetCommandLineArguments(const std::vector<std::string> &args)
{
    /* if (mRenderWidget != nullptr) {
        QtVTKVisualization::Parameters visualisationParameters;
        visualisationParameters.SetCommandLineArguments(args);
        mRenderWidget->SetParams(visualisationParameters);
    }
    */
}

void QtVisualizationMainWindow::Initialize()
{

    // See hopw many widgets need to be visualized
    this->setStyleSheet("QWidget { background-color : black}");
    mCentralPanelWidget->setStyleSheet("QWidget { background-color : black}");
    mLeftPanelWidget->setStyleSheet("QWidget { background-color : black}");
    mRightPanelWidget->setStyleSheet("QWidget { background-color : black}");


    QVBoxLayout *vLayoutL = dynamic_cast<QVBoxLayout*>(mLeftPanelWidget->layout());
    QVBoxLayout *vLayoutR = dynamic_cast<QVBoxLayout*>(mRightPanelWidget->layout());
    for (int i=0; i< mWidgets.size(); i++){
        if (mWidgets[i] == nullptr){
            continue;
        }
        QtPluginWidgetBase *w = mWidgets[i];
        //w->setStyleSheet("QtPluginWidgetBase { border: 1px solid green }");
        w->setStyleSheet(QString("QtPluginWidgetBase {border: 1px solid ") + mWidgetColors[i]+ "}" );
        switch (mWidgets[i]->GetWidgetLocation()){
        case QtPluginWidgetBase::WidgetLocation::top_left:
            vLayoutL->addWidget(w, 1, Qt::AlignTop);
            break;
        case QtPluginWidgetBase::WidgetLocation::bottom_left:
            vLayoutL->addWidget(w, 1, Qt::AlignBottom);
            break;
        case QtPluginWidgetBase::WidgetLocation::top_right:
            vLayoutR->addWidget(w, 1, Qt::AlignTop);
            break;
        case QtPluginWidgetBase::WidgetLocation::bottom_right:
            vLayoutR->addWidget(w, 1, Qt::AlignBottom);
            break;
        case QtPluginWidgetBase::WidgetLocation::hidden:
            // do not display
            break;
        case QtPluginWidgetBase::WidgetLocation::overlaid:
            break;
        }
    }

    this->InitializeCentralPanel();

    //QObject::connect(mWidgets[i], &QtPluginWidgetBase::ImageVisibilityChanged,
    //                 this, [=]{qDebug() << "[QtVisualizationMainWindow::Initialize] Signal: Image visibility changed";});



}

void QtVisualizationMainWindow::SetViewScale(int viewScaleInt){

    float maxViewScale = 100;
    float viewScale = float(viewScaleInt)/maxViewScale;

    Q_EMIT SignalSetViewScale(viewScale);

}

void QtVisualizationMainWindow::InitializeCentralPanel(){



    //QGridLayout *centralLayout = static_cast<QGridLayout *>(this->centralWidget()->layout());
    QGridLayout *centralLayout = static_cast<QGridLayout *>(this->mCentralPanelWidget->layout());

    int N = 0;
    for (int i=0; i< mImageWidgets.size(); i++){
        if (mImageWidgets[i] == nullptr){
            continue;
        }
        if  (mImageWidgets[i]->GetWidgetLocation() == QtPluginWidgetBase::WidgetLocation::visible){
            N++;
        }
    }

    int mNr=1;
    int mNc=1;

    switch (N){
    case 2:
    {
        mNc=2;
        break;
    }
    case 3:
    case 4:
    {
        mNr=2;
        mNc=2;
        break;
    }
    case 5:
    case 6:
    {
        mNr=2;
        mNc=3;
        break;
    }
    }

    int nvisibles = 0;
    for (int i=0; i< mImageWidgets.size(); i++){
        if (mImageWidgets[i] == nullptr){
            continue;
        }
        switch (mImageWidgets[i]->GetWidgetLocation()){
        case QtPluginWidgetBase::WidgetLocation::visible:
        {
            QtPluginWidgetBase *w = mImageWidgets[i];
            int r = std::floor(nvisibles/mNc);
            int c = nvisibles % mNc;

            QtVTKVisualization *ww = reinterpret_cast<QtVTKVisualization*>(w);
            ww->setStyleSheet(QString("QFrame {border: 1px solid ") + mWidgetColors[i]+ "}" );

            QObject::connect(this, &QtVisualizationMainWindow::SignalSetViewScale, ww,
                             &QtVTKVisualization::SetViewScale);

            nvisibles++;
            centralLayout->addWidget(w,r,c);
            w->show();            
            break;
        }
        case QtPluginWidgetBase::WidgetLocation::hidden:
        {
            //std::cout << "\tQtVisualizationMainWindow::Initialize() hidden "<< std::endl;
            break;
        }
        case QtPluginWidgetBase::WidgetLocation::visible_overlay:
            /// @todo implement solution
            break;
        }
    }
}

void QtVisualizationMainWindow::slot_Terminate(void)
{
}

void QtVisualizationMainWindow::SendImageToWidget(ifind::Image::Pointer image)
{

    Q_EMIT SignalSendImageToWidget(image);
}

void QtVisualizationMainWindow::resizeEvent(QResizeEvent *event)
{

    int width = this->width();
    mLeftPanelWidget->setMaximumWidth(0.12*width);
    mRightPanelWidget->setMaximumWidth(0.12*width);
    mCentralPanelWidget->setMaximumWidth(0.75*width);

    const int widgetWidth(mCentralPanelWidget->width());
    if (widgetWidth < 1) {
        return;
    }
}

void QtVisualizationMainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape && !event->isAutoRepeat()){
        if(isFullScreen())
        {
            setWindowFlags(Qt::Window);
            showNormal();
        }
        else
        {
            setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            showFullScreen();
        }
    }
}
