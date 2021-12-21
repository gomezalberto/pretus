#include "Worker_CppAlgorithm.h"
#include <iostream>
#include <QDebug>

#include <itkBinaryThresholdImageFilter.h>

Worker_CppAlgorithm::Worker_CppAlgorithm(QObject *parent) : Worker(parent){
    this->mThreshold = 50;
}

void  Worker_CppAlgorithm::slot_thresholdValueChanged(int v){
    /// somehow communicate with the worker.
    this->mThreshold = v;
}


void Worker_CppAlgorithm::doWork(ifind::Image::Pointer image){

    if (image == nullptr){
        if (this->params.verbose){
            std::cout << "Worker_CppAlgorithm::doWork() - input image was null" <<std::endl;
        }
        return;
    }



    using FilterType = itk::BinaryThresholdImageFilter<ifind::Image, GrayImageType>;
    FilterType::Pointer filter = FilterType::New();

    std::vector<std::string> layernames = image->GetLayerNames();
    int layer_idx = this->params.inputLayer;
    if (this->params.inputLayer <0){
        /// counting from the end
        layer_idx = image->GetNumberOfLayers() + this->params.inputLayer;
    }
    ifind::Image::Pointer layerImage = ifind::Image::New();
    layerImage->Graft(image->GetOverlay(layer_idx), layernames[layer_idx]);
    filter->SetInput(layerImage);


    filter->SetLowerThreshold(this->mThreshold);
    filter->SetUpperThreshold(255);
    filter->SetOutsideValue(0);
    filter->SetInsideValue(255);
    filter->Update();

    if (this->params.verbose){
        std::cout << "Worker_CppAlgorithm::doWork() - thresholding carried out" <<std::endl;
    }
    GrayImageType::Pointer segmentation = filter->GetOutput();

    image->GraftOverlay(segmentation.GetPointer(), image->GetNumberOfLayers(), "Segmentation");
    image->SetMetaData<int>(this->mPluginName.toStdString() + "_segmentation", image->GetNumberOfLayers() );
    image->SetMetaData<std::string>(this->mPluginName.toStdString() + "_threshold",  QString::number(this->mThreshold).toStdString());

    Q_EMIT this->ImageProcessed(image);
}
