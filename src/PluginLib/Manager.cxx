#include "Manager.h"

#include <exports/ifind2_common_Export.h>
#include <boost/filesystem.hpp>
#include <thread>
#include <QtConcurrent/QtConcurrent>

Manager::Manager(QObject* parent){

    this->Active = false;
    this->CurrentId = 0;
    this->verbose = false;
}



void Manager::SetActivate(bool arg)
{
    if (arg == this->IsActive())
        return;

    this->SetActive(arg);

    if (this->IsActive())
        this->StartAcquisition();
}

void Manager::StartAcquisition(void){

    this->Send();
    this->EnableCommandLineExitLoop();
}

void Manager::FindFiles(const QString &path, const QString &extension){

    boost::filesystem::path p(path.toStdString());
    boost::filesystem::directory_iterator end_itr;
    for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr) {
        // If it's not a directory, list it. If you want to list directories too, just remove this check.
        if (is_regular_file(itr->path())) {
            // assign current file name to current_file and echo it out to the console.
            QString current_file = itr->path().string().c_str();
            if (current_file.endsWith(extension)){
                this->DataBase.push_back(current_file);
            }
        } else {
            this->FindFiles(itr->path().string().c_str(), extension);
        }
    }
}

void Manager::SetDataBase(QStringList arg)
{
    this->DataBase = arg;
    //qSort(this->DataBase.begin(),this->DataBase.end());
    std::sort(this->DataBase.begin(),this->DataBase.end());
}

void Manager::EnableCommandLineExitLoop(){
    std::thread th(&Manager::exitLoop, this);
    th.detach();
    // QtConcurrent::run(this, &Manager::exitLoop); // this also works

}

int Manager::exitLoop(){
    std::string word;

    std::ios_base::sync_with_stdio(false);
    do {
        std::cout << "Manager::exitLoop() - Enter 'quit' to exit:"<< std::endl<<">> ";
        std::cin >> word;
        if (std::strcmp(word.c_str(),"quit")==0){
            break;
        }

    } while (true);
    std::ios_base::sync_with_stdio(true);
    this->SetActive(false);

    return 0;
}
