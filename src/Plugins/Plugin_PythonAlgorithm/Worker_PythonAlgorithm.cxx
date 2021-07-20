#include "Worker_PythonAlgorithm.h"
#include <iostream>
#include <QDebug>

#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include <itkImportImageFilter.h>

Worker_PythonAlgorithm::Worker_PythonAlgorithm(QObject *parent) : Worker(parent){
    this->mFsigma = 3;
    this->mDelay = 0;
}

Worker_PythonAlgorithm::~Worker_PythonAlgorithm(){
    /// Finalize python stuff
    /// @todo: this is causing segfault.
    py::finalize_interpreter();
}

void  Worker_PythonAlgorithm::slot_sigmaValueChanged(int v){
    /// somehow communicate with the worker.
    this->mFsigma = v;
}


void Worker_PythonAlgorithm::Initialize(){

    if (!this->PythonInitialized){
        try {
            py::initialize_interpreter(); // this will call Py_Initialize();
        }
        catch (py::error_already_set const &pythonErr) {
            std::cout << pythonErr.what();
        }
    }

    PyGILState_STATE gstate = PyGILState_Ensure();
    {

        py::exec("import sys");
        std::string command = "sys.path.append('" + this->python_folder + "')";
        py::exec(command.c_str());

        py::object processing = py::module::import("worker");
        /// Check for errors
        if (PyErr_Occurred())
        {
            PyErr_Print();
            return;
        }

        /// grabbing the functions from module
        this->PyImageProcessingFunction = processing.attr("dowork");
        this->PyPythonInitializeFunction = processing.attr("initialize");
        this->PyPythonInitializeFunction(this->python_folder);
        this->PythonInitialized = true;
    }
    PyGILState_Release(gstate);
}

void Worker_PythonAlgorithm::doWork(ifind::Image::Pointer image){

    if (!this->PythonInitialized){
        if (this->params.verbose){
            std::cout << "Worker_PythonAlgorithm::doWork() - python not initialised" <<std::endl;
        }
        return;
    }

    if (!Worker::gil_init) {
        if (this->params.verbose){
            std::cout << "Worker_PythonAlgorithm::doWork() - init GIL" <<std::endl;
        }
        Worker::gil_init = 1;
        PyEval_InitThreads();
        PyEval_SaveThread();

        ifind::Image::Pointer configuration = ifind::Image::New();
        configuration->SetMetaData<std::string>("Python_gil_init","True");
        Q_EMIT this->ConfigurationGenerated(configuration);
    }

    if (image == nullptr){
        if (this->params.verbose){
            std::cout << "Worker_PythonAlgorithm::doWork() - input image was null" <<std::endl;
        }
        return;
    }

    /// Extract central slice and crop
    GrayImageType2D::Pointer image_2d = this->get2dimage(image);
    GrayImageType2D::Pointer output_2d;

    if (this->params.verbose){
        std::cout << "Worker_PythonAlgorithm::doWork() - 2D image extracted" <<std::endl;
    }
    std::vector <unsigned long> dims = {image_2d->GetLargestPossibleRegion().GetSize()[1],
                                        image_2d->GetLargestPossibleRegion().GetSize()[0]};

    PyGILState_STATE gstate = PyGILState_Ensure();
    {
        if (this->params.verbose){
            std::cout << "Worker_PythonAlgorithm::doWork() - within PyGILState_Ensure;" <<std::endl;
        }
        py::array numpyarray(dims, static_cast<GrayImageType2D::PixelType*>(image_2d->GetBufferPointer()));
        py::object _function = this->PyImageProcessingFunction;

        py::array result = py::array(_function(numpyarray, this->mFsigma, this->mDelay, this->params.verbose));

        /// convert the result to a itk image
        typedef itk::ImportImageFilter< GrayImageType::PixelType, 2 >   ImportFilterType;
        ImportFilterType::SizeType imagesize;

        imagesize[0] = result.shape(1);
        imagesize[1] = result.shape(0);

        ImportFilterType::IndexType start;
        start.Fill(0);
        ImportFilterType::RegionType region;
        region.SetIndex(start);
        region.SetSize(imagesize);

        /// Define import filter
        ImportFilterType::Pointer importer = ImportFilterType::New();
        importer->SetOrigin( image_2d->GetOrigin() );
        importer->SetSpacing( image_2d->GetSpacing() );
        importer->SetDirection( image_2d->GetDirection() );
        importer->SetRegion(region);
        /// Separate the regional scalar buffer
        /// @todo check if a memcpy is necessary here
        GrayImageType::PixelType* localbuffer = static_cast<GrayImageType::PixelType*>(result.mutable_data());
        /// Import the buffer
        importer->SetImportPointer(localbuffer, imagesize[0] * imagesize[1], false);
        importer->Update();

        /// Disconnect the output from the filter
        /// @todo Check if that is sufficient to release the numpy buffer, or if the buffer needs to obe memcpy'ed
        output_2d = importer->GetOutput();
        output_2d->DisconnectPipeline();

        output_2d->SetMetaDataDictionary(image_2d->GetMetaDataDictionary());

        /// Create a 3D image with the 2D slice
        GrayImageType::Pointer output = this->get3dimagefrom2d(output_2d);

        /// Finally add to the image before emitting it.
        image->GraftOverlay(output.GetPointer(), image->GetNumberOfLayers());
        image->SetMetaData<std::string>( this->mPluginName.toStdString() +"_output", QString::number(image->GetNumberOfLayers()).toStdString() );
        image->SetMetaData<std::string>(this->mPluginName.toStdString() + "_sigma",  QString::number(this->mFsigma).toStdString());
        image->SetMetaData<std::string>(this->mPluginName.toStdString() + "_delay",  QString::number(this->mDelay).toStdString());
    }
    PyGILState_Release(gstate);
    Q_EMIT this->ImageProcessed(image);
}
