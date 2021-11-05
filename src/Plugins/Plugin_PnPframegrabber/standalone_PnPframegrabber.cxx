#include <PnPFrameGrabberManager.h>
//#include <Plugin_framegrabber.h>
#include <QThread>
#include <ifindImage.h>
#include <QObject>
#include <PnPFGMiniworker.h>


std::string filename("/tmp/image.mhd");

void Usage(void){
    std::cout << std::endl;
    std::cout << "# Standalone frame grabber "<<std::endl;
    std::cout <<"\t"<< "Reads images from a Epiphan DVI2USB 3.0 framegrabber."<<std::endl;
    std::cout <<"\t"<< "Additional arguments:"<<std::endl;
    //std::cout <<"\t"<<"\t-fg_resolution <multiplicative factor >\tFactor to multiply the default capture resolution of  1920×1080  (HD). Default: 1"<<std::endl;
    std::cout <<"\t"<<"\t-fg_framerate <capture frame rate >\tFrame rate of the capture for the frame grabber. If <=0, then maximum framerate is used.  "<<std::endl;
    std::cout <<"\t"<<"\t-fg_pixelsize <px py>\tPixel size, in mm "<<std::endl;
    std::cout <<"\t"<<"\t-fg_filename <fn>\tName of the image to be saved, default "<< filename <<std::endl;
    std::cout <<"\t"<<"\t-fg_ncomponents <fn>\t1 (grayscale) or 3 (YUV)"<<std::endl;
}


int main(int argc, char *argv[])
{

    PnPFrameGrabberManager::Pointer fgm = PnPFrameGrabberManager::New();

    /// Read arguments
    /*
    int i = 1;
    while (i < argc)
    {
        if (!strcmp(argv[i], "-fg_resolution"))
        {
            fgm->params.Resolution_factor =atof(argv[++i]);
        } else  if (!strcmp(argv[i], "-fg_framerate"))
        {
            fgm->params.CaptureFrameRate = atof(argv[++i]);
        }
        else  if (!strcmp(argv[i], "-fg_pixelsize"))
        {
            fgm->params.pixel_size[0] = atof(argv[++i]);
            fgm->params.pixel_size[1] = atof(argv[++i]);
            fgm->params.pixel_size[2] = 1;
        }   else  if (!strcmp(argv[i], "-fg_filename"))
        {
            filename = argv[++i];
        }   else  if (!strcmp(argv[i], "-fg_ncomponents"))
        {
            fgm->params.n_components = atoi(argv[++i]);
        }
        i++;
    }
    */

    /// Set up a miniworker
    PnPFGMiniworker* miniworker = new PnPFGMiniworker();
    miniworker->filename = filename;
    QObject::connect(fgm.get(),SIGNAL(ImageGenerated(ifind::Image::Pointer)), miniworker, SLOT(doWork(ifind::Image::Pointer)));
    /// Set up the video grabber

    fgm->params.verbose = true;
    fgm->Initialize();
    fgm->SetActivate(true); // will trigger 1 frame

    delete(miniworker);


    return 0;
}
