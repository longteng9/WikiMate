#ifndef MESSAGEFORM_H
#define MESSAGEFORM_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
#include <QThread>
#include <functional>
#include <QStringList>

namespace Ui {
class MessageForm;
}

class MoveEventFilter : public QObject{
    Q_OBJECT
signals:
    void moveVector(QPoint vec);
    void doubleClicked(QPoint clickPos);

protected:
    bool eventFilter(QObject *watched, QEvent *event){
        switch(event->type()){
            case QEvent::MouseButtonPress:{
                if(((QMouseEvent*)event)->button() == Qt::LeftButton){
                    isPressed = true;
                    prevPoint = ((QMouseEvent*)event)->globalPos();
                    return true;
                }
            }
            break;
            case QEvent::MouseMove:{
                if(isPressed){
                    QPoint curPoint = ((QMouseEvent*)event)->globalPos();
                    emit moveVector(curPoint - prevPoint);
                    prevPoint = curPoint;
                    return true;
                }
            }
            break;
            case QEvent::MouseButtonRelease:{
                isPressed = false;
                return true;
            }
            break;
            case QEvent::MouseButtonDblClick:{
                emit doubleClicked(((QMouseEvent*)event)->pos());
                return true;
            }
            break;
        }
        return false;
    }
private:
    bool isPressed = false;
    QPoint prevPoint;
};


class MessageForm : public QWidget
{
    Q_OBJECT
public:
    enum class Role{
        LoadingForm,
        AddTransMem,
        QueryDialogForm,
        SplitWordForm
    };
    explicit MessageForm(QWidget *parent = 0);
    void setTitle(const QString &text);
    static void createAndShowAs(Role role, const QString &word);
    static void createAndShowAs(Role role, QObject* caller);
    static void createAndShowAs(Role role,
                                const QString &title,
                                const QString &message,
                                std::function<void(bool)> callback);
    static void createAndShowAs(Role role,
                                int col,
                                const QString& word,
                                std::function<void(int, QStringList)> callback);

    bool eventFilter(QObject *watched, QEvent *event);

signals:

public slots:

protected:
    void playAnimation();

private slots:
    void on_btnExportTM_clicked();

    void on_btnClose_clicked();

    void on_btnCommitWordSplit_clicked();

private:
    Ui::MessageForm *ui;
    std::function<void(int, QStringList)> cb_splitWord;
    static MessageForm *sharedForm;
};

#endif // MESSAGEFORM_H
