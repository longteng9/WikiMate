#include "MessageForm.h"
#include "ui_MessageForm.h"
#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QLabel>
#include <QMovie>
#include "DictEngine.h"

MessageForm::MessageForm(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MessageForm)
{
    ui->setupUi(this);
    ui->tabWidget->tabBar()->hide();
    MoveEventFilter *filter = new MoveEventFilter;
    ui->labTitle->installEventFilter(filter);
    connect(filter, &MoveEventFilter::doubleClicked, [](QPoint clickPos){
        qDebug() << "clicked " << clickPos;
    });
    connect(filter, &MoveEventFilter::moveVector, [this](QPoint vec){
        this->move(this->pos() + vec);
    });

    this->setWindowOpacity(0.85);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    QDesktopWidget *desktopWidget = QApplication::desktop();
    int x = (desktopWidget->availableGeometry().width() - this->width()) / 2;
    int y = (desktopWidget->availableGeometry().height() - this->height()) / 2;
    animation->setDuration(1000);
    animation->setStartValue(QRect(x, 0, this->width(), this->height()));
    animation->setEndValue(QRect(x, y, this->width(), this->height()));
    animation->setEasingCurve(QEasingCurve::OutElastic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    ui->transMemEdit->installEventFilter(this);

    initWaitingForm();
}

void MessageForm::setTitle(const QString &text){
    ui->labTitle->setText(text);
}

void MessageForm::showAs(Role role, const QString &word){
    if(role == Role::AddTransMem){
        ui->tabWidget->setCurrentIndex(1);
        this->setWindowOpacity(1);
        ui->labTitle->setText("Add Translation Memory");
        ui->labTransMemForWord->setText(word);
        ui->transMemEdit->clear();
        this->show();
    }
}

void MessageForm::initWaitingForm(){
    ui->tabWidget->setCurrentIndex(0);
    QMovie *movie = new QMovie(":/static/loading_line.gif");
    movie->setScaledSize(QSize(418,216));
    movie->start();
    ui->label->setMovie(movie);
    ui->labTitle->setText("Processing, just a moment...");
}

bool MessageForm::eventFilter(QObject *watched, QEvent *event){
    int key;
    if(watched == ui->transMemEdit){
        switch(event->type())  {
            case QEvent::KeyPress:
                key = (static_cast<QKeyEvent *>(event))->key();
                if(key == Qt::Key_S && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                    on_btnExport_clicked();
                    return true;
                }
            default:
                return false;
        }
        return false;
    }
    return false;
}

void MessageForm::on_btnExport_clicked()
{
    if(!ui->transMemEdit->toPlainText().isEmpty()){
        QStringList transList = ui->transMemEdit->toPlainText().split("\n", QString::SkipEmptyParts);
        for(QString line : transList){
            DictEngine::instance()->insertTransMem(ui->labTransMemForWord->text(), line.trimmed());
        }
    }
    this->hide();
}

void MessageForm::on_btnClose_clicked()
{
    this->hide();
}
