#pragma once

#include <Worker.h>
#include <memory>
#include <mutex>
#include <QQueue>

/// For image data. Change if image data is different
#include <ifindImage.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

class Worker_planeDetection : public Worker{
    Q_OBJECT

public:

    typedef Worker_planeDetection            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new Self(parent));
    }
    ~Worker_planeDetection();

    void Initialize();

    QStringList getLabels() const;
    void setLabels(const QStringList &value);

    /// parameters must be only in the parent class

    std::string python_folder;
    /*
         * @brief number of frames to do temporal average
         */
    int temporalAverage;
    float background_threshold;
    std::string modelname;
    /**
     * @brief flag to enable or disable background file writing.
     */
    bool m_write_background;


protected:
    Worker_planeDetection(QObject* parent = 0);

    void doWork(ifind::Image::Pointer image);

private:

    /// Python Functions
    py::object PyImageProcessingFunction;
    py::object PyPythonInitializeFunction;
    QStringList labels;

    QQueue< QList<float> > confidences;

};
