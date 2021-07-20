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

#include <ifindStreamTypeHelper.h>
#include <QVTKWidget.h>

QtVisualizationMainWindow::QtVisualizationMainWindow( QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)  {
    // All of these autos will end up as raw pointers -
    // However, as they are ((great-)grand-)children of this widget,
    // this will take care of their destruction

    // create a widget to contain the left and right hand bars and the render
    // create the render widget

    // create a widget to contain the left and right hand bars and the render
    auto horizontalWidget = new QWidget(this);
    auto horizontalLayout = new QHBoxLayout(horizontalWidget);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setSpacing(0);
    horizontalWidget->setLayout(horizontalLayout);

    {
        // there is something to render
        auto panelWidget = new QWidget(this);
        panelWidget->move(0, 0); // move within the parent (will be the horizontal layout)
        panelWidget->show();
        mCentralPanelWidget = panelWidget;

        auto gLayout = new QGridLayout (panelWidget);
        gLayout->setContentsMargins(0, 0, 0, 0);
        gLayout->setSpacing(0);
        panelWidget->setLayout(gLayout);

    }

    // set up the left panel
    {
        auto panelWidget = new QWidget(this);
        panelWidget->move(0, 0);
        panelWidget->show();
        mLeftPanelWidget = panelWidget;

        auto vLayout = new QVBoxLayout(panelWidget);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);
        panelWidget->setLayout(vLayout);
    }

    // set up the right panel
    {
        auto panelWidget = new QWidget(this);
        panelWidget->move(0, 0);
        panelWidget->show();
        mRightPanelWidget = panelWidget;

        auto vLayout = new QVBoxLayout(panelWidget);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);
        panelWidget->setLayout(vLayout);
    }


    // *Seems to work well without this*
    // then create the central widget as one which will fix its aspect ratio
    //    auto centralFixedAspect = new QtFixedAspectRatio(
    //                mCentralPanelWidget,
    //                VisualizationConsts::TargetWindowWidth(),
    //                VisualizationConsts::TargetWindowHeight(),
    //                this);
    //    setCentralWidget(centralFixedAspect);

    setCentralWidget(mCentralPanelWidget);

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

    QVBoxLayout *vLayoutL = dynamic_cast<QVBoxLayout*>(mLeftPanelWidget->layout());
    QVBoxLayout *vLayoutR = dynamic_cast<QVBoxLayout*>(mRightPanelWidget->layout());
    for (int i=0; i< mWidgets.size(); i++){
        QtPluginWidgetBase *w = mWidgets[i];
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

    //QObject::connect(mWidgets[i], &QtPluginWidgetBase::ImageVisibilityChanged,
    //                 this, [=]{qDebug() << "[QtVisualizationMainWindow::Initialize] Signal: Image visibility changed";});


    // See hopw many widgets need to be visualized
    this->InitializeCentralPanel();
}

void QtVisualizationMainWindow::InitializeCentralPanel(){

    //std::cout << "QtVisualizationMainWindow::InitializeCentralPanel"<<std::endl;

    this->setStyleSheet("QWidget { background-color : black}");
    QGridLayout *centralLayout = static_cast<QGridLayout *>(this->centralWidget()->layout());


//        /// First remove all widgets, then add them in the right order.
//        if ( centralLayout != NULL )
//        {
//            QLayoutItem* item;
//            while ( ( item = centralLayout->takeAt( 0 ) ) != NULL )
//            {
//                centralLayout->removeItem(item);
//                //delete item->widget();
//                std::cout << "    QtVisualizationMainWindow::InitializeCentralPanel() - delete one widget"<< std::endl;
//                //delete item;
//            }
//            //delete m_view->layout();
//        }

//        std::cout << "QtVisualizationMainWindow::InitializeCentralPanel() - all widgets removed"<< std::endl;

    int N = 0;
    for (int i=0; i< mImageWidgets.size(); i++){
        if  (mImageWidgets[i]->GetWidgetLocation() == QtPluginWidgetBase::WidgetLocation::visible){
            N++;
        }
    }
    //std::cout << "QtVisualizationMainWindow::InitializeCentralPanel there are "<< N << " visible widgets"<<std::endl;

    int Nr=1, Nc=1;

    switch (N){
    case 2:
    {
        Nc=2;
        break;
    }
    case 3:
    case 4:
    {
        Nr=2;
        Nc=2;
        break;
    }
    case 5:
    case 6:
    {
        Nr=2;
        Nc=3;
        break;
    }
    }

    int nvisibles = 0;
    for (int i=0; i< mImageWidgets.size(); i++){

        switch (mImageWidgets[i]->GetWidgetLocation()){
        case QtPluginWidgetBase::WidgetLocation::visible:
        {
            QtPluginWidgetBase *w = mImageWidgets[i];
            //this->centralWidget()->layout()->addWidget(w, i, i);
            int r = std::floor(nvisibles/Nc);
            int c = nvisibles % Nc;

            //std::cout << "QtVisualizationMainWindow::Initialize() image widget ("<<Nr<<"x"<<Nc<<") "<< nvisibles<<" with image location to r: "<<r << " and c: "<< c << std::endl;
            nvisibles++;
            centralLayout->addWidget(w,r,c);
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

    const int widgetWidth(mCentralPanelWidget->width());
    if (widgetWidth < 1) {
        return;
    }

    const int panelWidth(VisualizationConsts::InformationPanelWidth() *
                         (static_cast<double>(widgetWidth) / VisualizationConsts::TargetWindowWidth()));

    const int panelHeight(mCentralPanelWidget->height());

    //std::cout << "[QtVisualizationMainWindow::resizeEvent] panelWidth: " << panelWidth << std::endl;

    const int rightPanelXPosition(mCentralPanelWidget->width() - panelWidth);

    mLeftPanelWidget->resize(panelWidth, panelHeight);

    mRightPanelWidget->move(rightPanelXPosition, 0);
    mRightPanelWidget->resize(panelWidth, panelHeight);

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
