#pragma once

#include <Manager.h>
#include <memory>

class FileManager : public Manager {
    Q_OBJECT

public:
    typedef FileManager            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new FileManager(parent));
    }

    struct Parameters {
        Parameters() {
            FrameRate = -1;
            LoopAround = true;
            AsRaw = false;
            verbose = false;
            checkMhdConsistency = true;
        }
        int FrameRate;
        bool LoopAround;
        bool AsRaw;
        bool verbose;
        bool checkMhdConsistency;
    };

    Parameters params;

    void SetInputFolder(const QString &inputFolder);
    /**
     * @brief Check that files are not corrupt and that they contain images
     */
    void CheckMhdConsistency(void);

public Q_SLOTS:

    /**
   * @brief Send the latest dnl::Image signal
   *
   * This method has to be public so that it can be binded to the periodic callback
   */
    virtual void Send(void);

protected:
    FileManager(QObject *parent = 0);




};
