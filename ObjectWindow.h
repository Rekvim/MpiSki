#ifndef OBJECT_WINDOW_H
#define OBJECT_WINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QScreen>
#include "Registry.h"

namespace Ui {
class ObjectWindow;
}

class ObjectWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectWindow(QWidget *parent = nullptr);
    void LoadFromReg(Registry *registry);
    ~ObjectWindow();

private:
    Registry *m_registry;
    ObjectInfo *m_objectInfo;
    Ui::ObjectWindow *ui;
    void SaveObjectInfo();
private slots:
    void ButtonClick();
};

#endif // OBJECT_WINDOW_H
