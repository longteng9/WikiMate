#include "Launcher.h"

Launcher::Launcher()
{

}

void Launcher::prepare(QObject* worker){
    worker->moveToThread(this);
    this->start();
}
