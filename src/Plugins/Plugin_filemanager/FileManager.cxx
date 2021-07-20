#include "FileManager.h"
#include <ifindImageReader.h>
#include <QTimer>
#include <QDebug>
#include <QElapsedTimer>
#include <QApplication>
#include <QFile>

FileManager::FileManager(QObject *parent){

}

void FileManager::SetInputFolder(const QString &inputFolder){

    this->DataBase.clear();
    if (inputFolder.size()>0){
        this->FindFiles(inputFolder, "mhd");
    }
    /// Now that we have the files, sort them
    this->SetDataBase(this->DataBase);

    if (params.verbose){
        std::cout << "FileManager::SetInputFolder() - loaded database of "<< this->DataBase.size() << "images"<<std::endl;
    }
}

void FileManager::CheckMhdConsistency(){
    //this->DataBase; // QStringList
    QStringList cleanedDataBase;
    for (int i=0; i<this->DataBase.size(); i++){
        QString filename = this->DataBase[i];
        QString rawfilename = filename.chopped(3)+"raw";
        QFile myfile_mhd(filename);
        QFile myfile_raw(rawfilename);
        if (myfile_mhd.size() >0 &&
            myfile_raw.exists() && myfile_raw.size()>0){
            cleanedDataBase.push_back(filename);
        }
    }
    if (this->DataBase.size() != cleanedDataBase.size()){
        std::cout << "[WARNING] FileManager::CheckMhdConsistency() - cleaned out "<< this->DataBase.size() - cleanedDataBase.size() <<" files"<<std::endl;
    }
    this->SetDataBase(cleanedDataBase);

}

void FileManager::Send(void)
{
    if (!this->GetDataBase().size()){
        std::cout << "[ERROR] FileManager::Send() - the database is empty!"<<std::endl;
        QApplication::quit();
        return;
    }

    unsigned int fileid = this->GetCurrentId() % this->GetDataBase().size();
    std::string fname = this->GetDataBase()[fileid].toStdString();

    ifind::ImageReader::Pointer reader = ifind::ImageReader::New();
    reader->SetFileName(fname.c_str());
    reader->Read();

    params.framecount++;

    ifind::Image::Pointer image = reader->GetifindImage();

    bool not_ifind_image = true;
    if (image->HasKey("DNLTimestamp")){
        int timestamp = QString(image->GetMetaData<std::string>("DNLTimestamp").c_str()).toInt();
        if (timestamp  > 0){
            not_ifind_image = false;
        }
    }

    if (this->params.AsRaw || not_ifind_image){
        ifind::Image::Pointer imagenull = ifind::Image::New();
        image->SetMetaDataDictionary( imagenull->GetMetaDataDictionary() );
        /// we need at least the timestamp "DNLTimestamp" and
        /// the dimensions "NDimensions"
        std::string ndims = "3";
        if (image->GetLargestPossibleRegion().GetSize()[2]==1){
            ndims = "2";
        }
        std::stringstream ss;
        ss<< ifind::LocalTimeStamp();

        image->SetMetaData<std::string>("NDimensions", ndims);
        image->SetMetaData<std::string>("DNLTimestamp", ss.str());
    }
    image->SetMetaData<std::string>("OriginalFilename", fname);
    image->SetMetaData<>("FrameCount", QString::number(this->params.framecount).toStdString());
    image->SetMetaData<>("FrameCountTotal", QString::number(this->GetDataBase().size()).toStdString());


    double framerate = this->params.FrameRate;
    if (image->HasKey("AcquisitionFrameRate") && this->params.FrameRate <0){
        framerate = atof(image->GetMetaData<std::string>("AcquisitionFrameRate").c_str());
    }
    framerate = (framerate > 0) && (framerate < 200)? framerate : 20;
    image->SetMetaData<>("AcquisitionFrameRate", QString::number(framerate).toStdString() );

    Q_EMIT ImageGenerated(image);

    if (!this->params.LoopAround && (this->GetCurrentId() == this->GetDataBase().size()-1)){
        std::cout << "[WARNING] FileManager::Send() - All "<< this->GetDataBase().size() <<" files sent, exiting"<<std::endl;
        this->SetActivate(false);
    }

    this->SetCurrentId( (this->GetCurrentId() + 1) % this->GetDataBase().size() );

    if (this->IsActive())
    {

        unsigned int wait = (unsigned int)(1000.0/(float)(framerate));
        QTimer::singleShot(wait, this, SLOT(Send()));
    } else {
        QApplication::quit();
    }
}

