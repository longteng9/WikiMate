#include "MessageForm.h"
#include "ui_MessageForm.h"
#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QLabel>
#include <QMovie>
#include "DictEngine.h"
#include "MainWindow.h"

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

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->transMemEdit->installEventFilter(this);

    playAnimation();
}

void MessageForm::setTitle(const QString &text){
    ui->labTitle->setText(text);
}

void MessageForm::playAnimation(){
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

void MessageForm::createAndShowAs(Role role, const QString &word){
    if(role == Role::AddTransMem){
        static MessageForm *form = new MessageForm;

        form->ui->tabWidget->setCurrentIndex(1);
        form->ui->labTitle->setText("Add Translation Memory");
        form->ui->labTransMemForWord->setText(word);
        form->ui->transMemEdit->clear();

        form->show();
    }
}

void MessageForm::createAndShowAs(Role role, QObject *caller){
    if(role == Role::LoadingForm){
        static MessageForm *form = new MessageForm;
        form->ui->tabWidget->setCurrentIndex(0);
        QMovie *movie = new QMovie(":/static/loading_line.gif");
        movie->setScaledSize(QSize(418,216));
        movie->start();
        form->ui->label->setMovie(movie);
        form->ui->labTitle->setText("Processing, just a moment...");
        MessageForm *form_dup = form; //否者在macos上编译时，会报'form' cannot be captured because it does not have automatic storage duration
        connect((MainWindow*)caller, &MainWindow::closeLoadingForm, [form_dup](){
            form_dup->hide();
        });
        form->show();
    }
}

void MessageForm::createAndShowAs(Role role,
                    const QString &title,
                    const QString &message,
                    std::function<void(bool)> callback){
    if(role == Role::QueryDialogForm){
        static MessageForm *form = new MessageForm;
        form->ui->tabWidget->setCurrentIndex(2);
        form->setTitle(title);
        form->ui->labDialogMain->setText(message);
        QMovie *movie = new QMovie(":/static/loading_cube.gif");
        movie->setScaledSize(QSize(71, 71));
        movie->start();
        form->ui->labDialogLeft->setMovie(movie);

        MessageForm *form_dup = form; //否者在macos上编译时，会报'form' cannot be captured because it does not have automatic storage duration
        connect(form->ui->btnDialogNO, &QPushButton::clicked, [form_dup, callback](){
            form_dup->hide();
            callback(false);
        });
        connect(form->ui->btnDialogOK, &QPushButton::clicked, [form_dup, callback](){
            form_dup->hide();
            callback(true);
        });
        form->show();
    }
}

void MessageForm::createAndShowAs(Role role,
                                  const QString& word,
                                  std::function<void(QString, QStringList)> callback){
    if(role == Role::SplitWordForm){
        static MessageForm *form = new MessageForm;

        form->ui->tabWidget->setCurrentIndex(3);
        form->ui->labTitle->setText("Split Entry Dialog");
        form->ui->labWordForSplitting->setText(word);
        form->ui->SplitWordEdit->clear();

        MessageForm *form_dup = form; //否者在macos上编译时，会报'form' cannot be captured because it does not have automatic storage duration
        connect(form->ui->btnCommitWordSplit, &QPushButton::clicked, [form_dup, callback](){
            if(!form_dup->ui->SplitWordEdit->toPlainText().isEmpty()){
                form_dup->hide();
                QStringList newWords = form_dup->ui->SplitWordEdit->toPlainText().split("\n");
                callback(form_dup->ui->labWordForSplitting->text(), newWords);
            }
        });

        form->show();
    }
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

void MessageForm::on_btnClose_clicked(){
    this->hide();
}

