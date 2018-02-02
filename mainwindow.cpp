#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QResizeEvent>
#include <QSvgRenderer>
#include <QMimeData>

CMainWindow::CMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CMainWindow),
    mpImageLoader(nullptr),
    mInternalImage{}
{
    ui->setupUi(this);

    // Setup the image loader.
    mpImageLoader = new CImageLoader{};
    if (!mpImageLoader->InitLoader())
    {
        LogMessage("Was unable to initialize the image loader!", 0, __FILE__, __LINE__);
    }
    else
    {
        connect(ui->actionOpen_Image, &QAction::triggered, this, &CMainWindow::OpenImageDialog);
        connect(ui->infoBtn, &QPushButton::released, this, &CMainWindow::ShowImageInfo);
        connect(ui->actionQuit, &QAction::triggered, this, &CMainWindow::close);

        ui->actionOpen_Image->setShortcut(QKeySequence(tr("Ctrl+O")));
        ui->actionQuit->setShortcut(QKeySequence(tr("Ctrl+Q")));

        resize(800, 600);

        setAcceptDrops(true);

        setWindowTitle(tr("PhotoInsight: None"));
        setWindowIcon(QIcon(":/icon/icon.png"));
    }
}

CMainWindow::~CMainWindow()
{
    if (nullptr != mpImageLoader) { delete mpImageLoader; }
    if (nullptr != ui) { delete ui; }
}

void CMainWindow::ShowImageInfo()
{
    if (nullptr != mpImageLoader)
    {
        const size_t ciMaxBuffSz = 4096;
        char* pTmpBuff = new char[ciMaxBuffSz];
        memset(pTmpBuff, 0, ciMaxBuffSz);

        if (!mbUseInternal)
        {
            snprintf(pTmpBuff, ciMaxBuffSz, "Width: %u<br />Height: %u<br />Depth: %u<br />MIME: %s", mpImageLoader->GetWidth(), mpImageLoader->GetHeight(), mpImageLoader->GetBitDepth(), mpImageLoader->GetMime().c_str());
        }
        else
        {
            snprintf(pTmpBuff, ciMaxBuffSz, "Width: %u<br />Height: %u<br />Depth: %u<br />MIME: N/A", mInternalImage.width(), mInternalImage.height(), mInternalImage.depth());
        }

        QMessageBox::information(this, tr("Image Info"), tr(pTmpBuff), QMessageBox::Ok);

        if (nullptr != pTmpBuff)
        {
            delete[] pTmpBuff;
        }
    }
}

void CMainWindow::OpenImageDialog()
{
    if (nullptr != mpImageLoader)
    {
        // File filters. (There MUST be a better way!)
        QString lFilters = "";
        lFilters.append("All Images (*.bmp *.cut *.dds *.exr *.g3 *.gif *.hdr *.ico *.iff *.lbm *.j2k *.j2c *.jng *.jp2 *.jpg *.jif *.jpeg *.jpe *.jxr *.wdp *.hdp *.koa *.mng *.pbm *.pcd *.pcx *.pfm *.pgm *.pct *.pict *.pic *.png *.ppm *.psd *.ras *.raw *.sgi *.svg *.tga *.targa *.tif *.tiff *.wbmp *.webp *.xbm);;");
        lFilters.append("Bitmap (*.bmp);;");
        lFilters.append("Dr. Halo (*.cut);;");
        lFilters.append("DirectDraw Surface(*.dds);;");
        lFilters.append("OpenEXR (*.exr);;");
        lFilters.append("Raw Fax CCITT G3 (*.g3);;");
        lFilters.append("Graphics Interchangable Format(*.gif);;");
        lFilters.append("High Dynamic Range (*.hdr);;");
        lFilters.append("Windows Icon (*.ico);;");
        lFilters.append("Amiga IFF (*.iff, *.lbm);;");
        lFilters.append("JPEG-2000 Codestream (*.j2k *.j2c);;");
        lFilters.append("JPEG Network Graphics (*.jng);;");
        lFilters.append("JPEG-2000 (*.jp2);;");
        lFilters.append("JPEG (*.jpg *.jif *.jpeg *.jpe);;");
        lFilters.append("JPEG XR (*.jxr *.wdp *.hdp);;");
        lFilters.append("Commodore 64 Koala (*.koa);;");
        lFilters.append("Multiple Network Graphics (*.mng);;");
        lFilters.append("Portable Bitmap (*.pbm);;");
        lFilters.append("Kodak PhotoCD (*.pcd);;");
        lFilters.append("Zsoft Paintbrush (*.pcx);;");
        lFilters.append("Portable Floatmap (*.pfm);;");
        lFilters.append("Portable Graymap (*.pgm);;");
        lFilters.append("Macintosh PICT (*.pct *.pict *.pic);;");
        lFilters.append("Portable Network Graphics (*.png);;");
        lFilters.append("Portable Pixelmap (*.ppm);;");
        lFilters.append("Adobe Photoshop (*.psd);;");
        lFilters.append("Sun Rasterfile (*.ras);;");
        lFilters.append("RAW Camera Image (*.raw);;");
        lFilters.append("Silicon Graphics (*.sgi);;");
        lFilters.append("Scalable Vector Graphics (*.svg);;");
        lFilters.append("Truevision TARGA (*.tga *.targa);;");
        lFilters.append("Tagged Image Format (*.tif *.tiff);;");
        lFilters.append("Wireless Bitmap (*.wbmp);;");
        lFilters.append("Google WebP (*.webp);;");
        lFilters.append("X11 Bitmap (*.xbm);;");

        QString lFile = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), lFilters);

        if (!lFile.isEmpty())
        {
            OpenImage(lFile);
        }
    }
}

void CMainWindow::OpenImage(QString asPath)
{
    // Some cleaning in case the path is a URL.
    asPath.remove("file://");

    if (nullptr != mpImageLoader)
    {
        // Test to see if we can simply use just QImage instead of FreeImage.
        if (QRegExp(".*\.(bmp|gif|jpg|jpeg|png|pbm|pgm|ppm|xbm|xpm)$", Qt::CaseInsensitive).exactMatch(asPath))
        {
            // Load the image via QImage instead of FreeImage.
            mInternalImage.load(asPath);
            mbUseInternal = true;

            // Update the image canvas.
            UpdateImage();

            // Enable the "info" button.
            ui->infoBtn->setEnabled(true);

            // Set the window title to the image.
            QString lFilename = asPath.remove(0, asPath.lastIndexOf('/'));
            lFilename = lFilename.remove('/'); // Just to be safe.
            setWindowTitle(tr("PhotoInsight: %1").arg(lFilename));
        }
        else if (QRegExp(".*\.svg$", Qt::CaseInsensitive).exactMatch(asPath))
        {
            // Open the SVG and paint it to the image.
            QSvgRenderer lSvg{asPath};

            mInternalImage = QImage(lSvg.viewBox().size(), QImage::Format_ARGB32);
            mInternalImage.fill(Qt::white);

            QPainter *lPainter = new QPainter(&mInternalImage);
            if (nullptr != lPainter)
            {
                lPainter->begin(&mInternalImage);
                lSvg.render(lPainter);
                lPainter->end();
                delete lPainter;
            }

            mbUseInternal = true;

            // Update the image canvas.
            UpdateImage();

            // Enable the "info" button.
            ui->infoBtn->setEnabled(true);

            // Set the window title to the image.
            QString lFilename = asPath.remove(0, asPath.lastIndexOf('/'));
            lFilename = lFilename.remove('/'); // Just to be safe.
            setWindowTitle(tr("PhotoInsight: %1").arg(lFilename));
        }
        else
        {
            // Attempt to load the image.
            if (mpImageLoader->LoadImage(asPath.toStdString()))
            {
                mbUseInternal = false;

                // Update the image canvas.
                UpdateImage();

                // Enable the "info" button.
                ui->infoBtn->setEnabled(true);

                // Set the window title to the image.
                QString lFilename = asPath.remove(0, asPath.lastIndexOf('/'));
                lFilename = lFilename.remove('/'); // Just to be safe.
                setWindowTitle(tr("PhotoInsight: %1").arg(lFilename));
            }
            else
            {
                QString lErrStr = QString::fromStdString(mpImageLoader->GetErrorString(mpImageLoader->GetLastError()));
                QMessageBox::critical(this, tr("Unable to load image!"), lErrStr, QMessageBox::Ok);
            }
        }
    }
}

void CMainWindow::UpdateImage()
{
    if (nullptr != mpImageLoader)
    {
        QPixmap lPixmap;
        if (!mbUseInternal)
        {
            QImage lImage(mpImageLoader->GetRawData(), mpImageLoader->GetWidth(), mpImageLoader->GetHeight(), QImage::Format_RGB32);
            lPixmap = QPixmap::fromImage(lImage);
        }
        else
        {
            lPixmap = QPixmap::fromImage(mInternalImage);
        }

        ui->canvas->setPixmap(lPixmap.scaled(ui->canvas->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        repaint();
    }
    else
    {
        LogMessage("Unable to update image, loader is null!", 0, __FILE__, __LINE__);
    }
}

void CMainWindow::resizeEvent(QResizeEvent*)
{
    UpdateImage();
}

void CMainWindow::dragEnterEvent(QDragEnterEvent* aEvent)
{
    aEvent->accept();
}

void CMainWindow::dropEvent(QDropEvent* aEvent)
{
    QRegExp lExpr{".*\.(bmp|cut|dds|exr|g3|gif|hdr|ico|iff|lbm|j2k|j2c|jng|jp2|jpg|jif|jpeg|jpe|jxr|wdp|hdp|koa|mng|pbm|pcd|pcx|pfm|pgm|pct|pict|pic|png|ppm|psd|ras|raw|sgi|svg|tga|targa|tif|tiff|wbmp|webp|xbm)$", Qt::CaseInsensitive};

    if (aEvent->mimeData()->hasUrls())
    {
        bool bIsImage = false;
        foreach (QUrl lUrl, aEvent->mimeData()->urls())
        {
            if (lExpr.exactMatch(lUrl.toString()))
            {
                OpenImage(lUrl.toString());
                aEvent->accept();
                bIsImage = true;
                break;
            }
        }

        if (!bIsImage)
        {
            QMessageBox::warning(this, tr("Unable to open"), tr("Unable to open file!<br />Format not supported or image!"), QMessageBox::Ok);
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Unable to open"), tr("Unable to open file!<br />No paths where found!"), QMessageBox::Ok);
    }
}
