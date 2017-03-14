#include "MessageForm.h"
#include "ui_MessageForm.h"
#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QLabel>
#include <QMovie>
#include "DictEngine.h"
#include "MainWindow.h"

MessageForm *MessageForm::sharedForm = NULL;

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
    ui->SplitWordEdit->installEventFilter(this);

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
        if(sharedForm == NULL){
            sharedForm = new MessageForm;
        }

        sharedForm->ui->tabWidget->setCurrentIndex(1);
        sharedForm->ui->labTitle->setText("Add Translation Memory");
        sharedForm->ui->labTransMemForWord->setText(word);
        sharedForm->ui->transMemEdit->clear();

        sharedForm->show();
    }
}

void MessageForm::createAndShowAs(Role role, QObject *caller){
    if(role == Role::LoadingForm){
        if(sharedForm == NULL){
            sharedForm = new MessageForm;
        }
        sharedForm->ui->tabWidget->setCurrentIndex(0);
        QMovie *movie = new QMovie(":/static/loading_line.gif");
        movie->setScaledSize(QSize(418,216));
        movie->start();
        sharedForm->ui->label->setMovie(movie);
        sharedForm->ui->labTitle->setText("Processing, just a moment...");
        MessageForm *form_dup = sharedForm; //否者在macos上编译时，会报'form' cannot be captured because it does not have automatic storage duration
        connect((MainWindow*)caller, &MainWindow::closeLoadingForm, [form_dup](){
            form_dup->hide();
        });
        sharedForm->show();
    }
}

void MessageForm::createAndShowAs(Role role,
                    const QString &title,
                    const QString &message,
                    std::function<void(bool)> callback){
    if(role == Role::QueryDialogForm){
        if(sharedForm == NULL){
            sharedForm = new MessageForm;
        }
        sharedForm->ui->tabWidget->setCurrentIndex(2);
        sharedForm->setTitle(title);
        sharedForm->ui->labDialogMain->setText(message);
        QMovie *movie = new QMovie(":/static/loading_cube.gif");
        movie->setScaledSize(QSize(71, 71));
        movie->start();
        sharedForm->ui->labDialogLeft->setMovie(movie);

        MessageForm *form_dup = sharedForm; //否者在macos上编译时，会报'form' cannot be captured because it does not have automatic storage duration
        connect(sharedForm->ui->btnDialogNO, &QPushButton::clicked, [form_dup, callback](){
            form_dup->hide();
            callback(false);
        });
        connect(sharedForm->ui->btnDialogOK, &QPushButton::clicked, [form_dup, callback](){
            form_dup->hide();
            callback(true);
        });
        sharedForm->show();
    }
}

void MessageForm::createAndShowAs(Role role,
                                  int col,
                                  const QString& word,
                                  std::function<void(int, QStringList)> callback){
    if(role == Role::SplitWordForm){
        if(sharedForm == NULL){
            sharedForm = new MessageForm;
        }
        sharedForm->cb_splitWord = callback;
        sharedForm->ui->tabWidget->setCurrentIndex(3);
        sharedForm->ui->labTitle->setText("Split Word at [" + QString::number(col) + "]");
        sharedForm->ui->labWordForSplitting->setText(word);
        sharedForm->ui->SplitWordEdit->clear();

        sharedForm->show();
        sharedForm->ui->SplitWordEdit->setFocus();
    }
}

bool MessageForm::eventFilter(QObject *watched, QEvent *event){
    int key;
    if(watched == ui->transMemEdit){
        switch(event->type())  {
            case QEvent::KeyPress:
                key = (static_cast<QKeyEvent *>(event))->key();
                if(key == Qt::Key_S && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                    on_btnExportTM_clicked();
                    return true;
                }
            default:
                return false;
        }
        return false;
    }else if(watched == ui->SplitWordEdit){
        switch(event->type())  {
            case QEvent::KeyPress:
                key = (static_cast<QKeyEvent *>(event))->key();
                if(key == Qt::Key_S && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                    on_btnCommitWordSplit_clicked();
                    return true;
                }
            default:
                return false;
        }
        return false;
    }
    return false;
}

void MessageForm::on_btnExportTM_clicked()
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


void MessageForm::on_btnCommitWordSplit_clicked()
{
    if(!sharedForm){
        return;
    }
    if(!sharedForm->ui->SplitWordEdit->toPlainText().isEmpty()){
        sharedForm->hide();
        QString colStr= sharedForm->ui->labTitle->text().mid(sharedForm->ui->labTitle->text().lastIndexOf("["));
        colStr = colStr.mid(1, colStr.lastIndexOf("]")-1);
        int col = colStr.toInt();
        QStringList newWords = sharedForm->ui->SplitWordEdit->toPlainText().split("\n", QString::SkipEmptyParts);
        for(QString word : newWords){
            DictEngine::instance()->insertTransMem(word, "");
        }
        cb_splitWord(col, newWords);
    }
}
