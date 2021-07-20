#pragma once

#include <Worker.h>
#include <memory>

/// For image data. Change if image data is different
#include <ifindImage.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

class Worker_PythonAlgorithm : public Worker{
    Q_OBJECT

public:

    typedef Worker_PythonAlgorithm            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new Self(parent));
    }
    ~Worker_PythonAlgorithm();

    void Initialize();

    double mFsigma;
    double mDelay;
    std::string python_folder;

public Q_SLOTS:

    virtual void slot_sigmaValueChanged(int v);

protected:
    Worker_PythonAlgorithm(QObject* parent = 0);
    void doWork(ifind::Image::Pointer image);

private:

    /// Python Functions
    py::object PyImageProcessingFunction;
    py::object PyPythonInitializeFunction;

};
