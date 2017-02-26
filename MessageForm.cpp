#include "MessageForm.h"
#include "ui_MessageForm.h"
#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QLabel>
#include <QMovie>

MessageForm::MessageForm(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MessageForm)
{
    ui->setupUi(this);
    initWaitingForm();
}

void MessageForm::setText(const QString &text){
    ui->labTitle->setText(text);
}

void MessageForm::initWaitingForm(){
    QMovie *movie = new QMovie(":/static/loading_line.gif");
    movie->setScaledSize(QSize(418,216));
    movie->start();
    ui->label->setMovie(movie);
    ui->labTitle->setText("Processing, just a moment...");
    connect(ui->btnClose, &QPushButton::clicked, [this](){
        this->deleteLater();
    });

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
}
