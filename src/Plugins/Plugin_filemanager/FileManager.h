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
            extension = "mhd";
        }
        int FrameRate;
        bool LoopAround;
        bool AsRaw;
        bool verbose;
        bool checkMhdConsistency;
        std::string extension;
    };

    Parameters params;

    void SetInputFolder(const QString &inputFolder);
    void SetExtension(const QString &extension);
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
    virtual void slot_frameValueChanged(int v);
    virtual void slot_togglePlayPause(bool v);
    virtual void slot_next();
    virtual void slot_previous();

protected:
    FileManager(QObject *parent = 0);

private:
    bool mIsPaused;




};
