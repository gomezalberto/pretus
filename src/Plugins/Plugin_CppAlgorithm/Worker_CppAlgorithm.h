#pragma once

#include <Worker.h>
#include <memory>

/// For image data. Change if image data is different
#include <ifindImage.h>

class Worker_CppAlgorithm : public Worker{
    Q_OBJECT

public:

    typedef Worker_CppAlgorithm            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new Self(parent));
    }

    unsigned char mThreshold;

public Q_SLOTS:

    virtual void slot_thresholdValueChanged(int v);

protected:
    Worker_CppAlgorithm(QObject* parent = 0);
    void doWork(ifind::Image::Pointer image);
};
