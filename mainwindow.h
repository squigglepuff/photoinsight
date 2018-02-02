#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "imageloader.h"

namespace Ui {
    class CMainWindow;
}

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget *parent = 0);
    ~CMainWindow();

    void OpenImage(QString asPath);

protected slots:
    void resizeEvent(QResizeEvent* aEvent);
    void dragEnterEvent(QDragEnterEvent* aEvent);
    void dropEvent(QDropEvent* aEvent);

private slots:
    void ShowImageInfo();
    void OpenImageDialog();

private:
    void UpdateImage();

    Ui::CMainWindow *ui;
    CImageLoader* mpImageLoader;
    QImage mInternalImage;
    bool mbUseInternal;
};

#endif // MAINWINDOW_H
