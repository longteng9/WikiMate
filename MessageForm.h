#ifndef MESSAGEFORM_H
#define MESSAGEFORM_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
#include <QThread>
#include <functional>

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
        QueryDialogForm
    };
    explicit MessageForm(QWidget *parent = 0);
    void setTitle(const QString &text);
    static void createAndShowAs(Role role, const QString &word);
    static void createAndShowAs(Role role, QObject* caller);
    static void createAndShowAs(Role role,
                                const QString &title,
                                const QString &message,
                                std::function<void(bool)> callback);

    bool eventFilter(QObject *watched, QEvent *event);

signals:

public slots:

protected:
    void playAnimation();

private slots:
    void on_btnExport_clicked();

    void on_btnClose_clicked();

private:
    Ui::MessageForm *ui;
};

#endif // MESSAGEFORM_H
