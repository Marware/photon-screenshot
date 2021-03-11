#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRubberBand>
#include <QMouseEvent>
#include <QDesktopWidget.h>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include "localsocketipc.h"
#include <QJsonArray>
#include <QShortcut>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

bool CLOSED = true;
QRect go;
int dob = 0;
volatile int im_size;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ocrtxt = new QDialog();
    ocrtxtDialog = new Ui::Dialog();
    ocrtxtDialog->setupUi(ocrtxt);

    setw = new QWidget();
    sets = new Ui::Form();
    sets->setupUi(setw);

    abtw = new QWidget();
    abt = new Ui::About();
    abt->setupUi(abtw);

    abtw->setWindowFlags(Qt::WindowCloseButtonHint);
    setw->setWindowFlags(Qt::WindowCloseButtonHint);

    /* INIT SETTINGS */

    setsFile = QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0) + "/settings.ini";
    QString setsdir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0);
    if(!QDir(setsdir).exists())
    {
        restoreDefSettings();
    }

    //ui->gSize->setParent();

    ui->ghotspot->hide();
    ui->gVbtns->hide();
    ui->gHbtns->hide();
    ui->gSize->hide();
    ui->gShbtns->hide();
    ui->gSelect->hide();

    bgroup = new QButtonGroup();
    bgroup->addButton(ui->gPen);
    bgroup->addButton(ui->gLine);
    bgroup->addButton(ui->gArrow);
    bgroup->addButton(ui->gMarker);
    bgroup->addButton(ui->gRect);
    bgroup->addButton(ui->gText);

    bgroup->setExclusive(false);
    connect(bgroup,SIGNAL(buttonClicked(int)),this,SLOT(uncheck(int)));

//    ui->gOcr->hide();
//    ui->gRec->hide();
    {
        ui->gSize->setAutoFillBackground(true);
        QPalette palette;
        palette.setBrush(QPalette::Window, QColor("grey"));
        ui->gSize->setPalette(palette);
        ui->gSize->setStyleSheet("color: white; border-radius: 5px;font: \"Calibri\";background-color: black;");
    }

    curr_shape = 0;

    drawSizeLabel();
    ds = QApplication::desktop();

    //connect(ui->gLabCo)

    //tx = 0;
    /*DRAWING*/
    drawing = false;
    textclckd = false;
    tx = NULL;

    im = new QList<QByteArray>;

    /*EOF DRAWING*/
    centralWidget()->setMouseTracking(true);
    setMouseTracking(true);
    ui->gArrow->setEnabled(true);
    ui->gText->setEnabled(true);////////
    ///
    ///
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));

    /* COLORS */
    connect(ui->gBcol,SIGNAL(clicked()),ui->gColor,SLOT(click()));

    /* EOF COLORS */

    /* Key shortcuts */
    QShortcut *shUpload = new QShortcut(QKeySequence("U"), this);
    connect(shUpload, SIGNAL(activated()), this, SLOT(on_gUpload_clicked()));

    QShortcut *shShare = new QShortcut(QKeySequence("W"), this);
    connect(shShare, SIGNAL(activated()), this, SLOT(on_gShare_clicked()));

    QShortcut *shGoogle = new QShortcut(QKeySequence("G"), this);
    connect(shGoogle, SIGNAL(activated()), this, SLOT(on_gGoogImg_clicked()));

    QShortcut *shPrint = new QShortcut(QKeySequence("P"), this);
    connect(shPrint, SIGNAL(activated()), this, SLOT(on_gPrint_clicked()));

    QShortcut *shCopy = new QShortcut(QKeySequence("C"), this);
    connect(shCopy, SIGNAL(activated()), this, SLOT(on_gCopy_clicked()));

    QShortcut *shSave = new QShortcut(QKeySequence("S"), this);
    connect(shSave, SIGNAL(activated()), this, SLOT(on_gSave_clicked()));

    QShortcut *shOcr = new QShortcut(QKeySequence("O"), this);
    connect(shOcr, SIGNAL(activated()), this, SLOT(on_gOcr_clicked()));

    QShortcut *shRec = new QShortcut(QKeySequence("R"), this);
    connect(shRec, SIGNAL(activated()), this, SLOT(on_gRec_clicked()));

    QShortcut *shClose = new QShortcut(QKeySequence("Q"), this);
    connect(shClose, SIGNAL(activated()), this, SLOT(on_gClose_clicked()));

    QShortcut *shUndo = new QShortcut(QKeySequence("Ctrl+Z"), this);
    connect(shUndo, SIGNAL(activated()), this, SLOT(on_gUndo_clicked()));

    QShortcut *shEdit = new QShortcut(QKeySequence("Ctrl+E"), this);
    connect(shEdit, SIGNAL(activated()), this, SLOT(editMode()));

    /* END Key shortcuts */

    /*video*/
    rec = false;
    rec_mov = false;

    rbhotsp = ui->ghotspot;
    //QRect *rect = new QRect(0,0,30,30);
   // QRegion* region = new QRegion(*rect,QRegion::Ellipse);
   // ui->ghotspot->setMask(*region);
    /* end video */

    /* settings */
    loadSettings(0);

    connect(sets->gSave,SIGNAL(clicked()),this,SLOT(settingsSaved()));
    connect(sets->gScan,SIGNAL(clicked()),setw,SLOT(hide()));
    connect(sets->gImgBrowse,SIGNAL(clicked()),this,SLOT(browseSetImageLoc()));
    connect(sets->gVidBrowse,SIGNAL(clicked()),this,SLOT(browseSetVideoLoc()));
    connect(sets->gColor,SIGNAL(clicked()),this,SLOT(changeDefColor()));
    connect(sets->gMarkColor,SIGNAL(clicked()),this,SLOT(changeDefMarkColor()));
    connect(sets->gDefs,SIGNAL(clicked()),this,SLOT(restoreDefSettings()));
    /* end settings */

    /* edit */
    edit = true;
    edo = 0x1;
    /* end edit */

    /*colors*/
    if(!curr_color.isValid())
    {
        curr_color = QColor(Qt::red);
    }
    if(!m_defcolor.isValid())
    {
        m_defcolor = QColor(Qt::yellow);
    }
    border_color = updateColorIcon(curr_color);
    /*end colors*/

    /*about*/
    connect(abt->gOKAbout,SIGNAL(clicked()),abtw,SLOT(close()));
    connect(abt->gChkUpdate,SIGNAL(clicked()),this,SLOT(updateCheck()));

    /* end about */

    /* INITS */
    printer = new QPrinter(QPrinter::HighResolution);
    dlg = new QPrintDialog(printer, this);
    qfd = new QFileDialog(this);

    /* TEST FULLSCREEN VIDEO */
    fullscreend = true;
    sysTrayRec =  "Fullscreen Record";

    /* TEMP PATH */
    tmpDir = QDir::tempPath() +"/PhotonTmpPhotos";
    if(!QDir(tmpDir).exists())
    {
        QDir().mkdir(tmpDir);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

//Main Screen Shot Rectangle : 20 - A
//- Take screen shot A1
//- ..save it on Qwidget A2
//- Set resizable rectangle to crop A3
//- save image temp : tmpImg A4
//- save on disk : save dialog A5

/* Main Screen Shot Rectangle : 20 - A */


void MainWindow::screenShot(HWND actWin, int wOn) // A1
{
    //PWINDOWINFO pwi = (PWINDOWINFO*) malloc(sizeof(PWINDOWINFO));
    //GetWindowInfo(activeWin,&pwi);

    if(rec)
    {
        record_Stop_FS();
    }
    drawing = false;
    fullscreend = false;

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setGeometry(ds->screen()->geometry());
    this->showFullScreen();

#ifdef Q_OS_WIN
    SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif

    this->setFocus();
    dirtyRect = this->rect();
    pixmap = QImage(this->rect().size(),QImage::Format_ARGB32);
    tmp_image = pixmap;
    oimg = pixmap;
    image = pixmap;

    CLOSED = false;
    return;
}

int i=0;
QString MainWindow::saveTmpImage(QString fn) // A4
{

    QRect area = rubberBand->geometry();

    if(textclckd)
    {
        delete tx;
        tx = NULL;
    }

    rubberBand->hide();
    buttsHide();
    ui->gSize->hide();
    repaint();

    pix = QImage(area.size(),QImage::Format_ARGB32_Premultiplied);
    pix = QApplication::screens().at(0)->grabWindow(ds->winId(), area.x(), area.y(), area.width(), area.height()).toImage();

    if(curr_shape == 1)
    {
        QImage target = QImage(area.size(),QImage::Format_ARGB32_Premultiplied);
        //target.fill(Qt::transparent);

        QPainter painter(&target);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.setBackgroundMode(Qt::TransparentMode);
        QPainterPath ellipsePath;
        ellipsePath.addEllipse(pix.rect());
        painter.fillPath(ellipsePath,QBrush(pix));
        painter.end();
        //repaint();

        pix = target;
    }

    ui->gSize->show();
    ui->gHbtns->show();
    ui->gVbtns->show();
    rubberBand->show();

    if(fn.isEmpty())
    {
        fn = QString::number(qrand())+"."+imgFormat.toLower();
        fn = tmpDir + "/" + fn;
    }
    //pix.save(fn,"JPG",100);
    pix.save(fn,imgFormat.toLower().toStdString().c_str(),100);
    return fn;
}

QMenu *menu;
QSystemTrayIcon *icon;

int MainWindow::tray() //K
{
    menu = new QMenu();
    icon = new QSystemTrayIcon();

    connect(icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconClick(QSystemTrayIcon::ActivationReason)));

    QAction* shot = new QAction(tr("Take ScreenShot"), this);
    connect(shot, SIGNAL(triggered()), this, SLOT(shotSlot()));

    QAction* record = new QAction(tr(sysTrayRec.toStdString().c_str()), this);
    connect(record, SIGNAL(triggered()), this, SLOT(record_FS()));

    QAction* settings = new QAction(tr("&Settings..."), this);
    connect(settings, SIGNAL(triggered()), this, SLOT(showSettings()));

    QAction* about = new QAction(tr("&About..."), this);
    connect(about, SIGNAL(triggered()), this, SLOT(showAbout()));

    QAction* quit = new QAction(tr("&Quit"), this);
    connect(quit, SIGNAL(triggered()), this, SLOT(closeApp()));

    menu->addAction(shot);
    menu->addAction(record);
    menu->addAction(settings);
    menu->addAction(about);
    menu->addSeparator();
    menu->addAction(quit);

    icon->setContextMenu(menu);
    icon->setToolTip("Photon");
    icon->setIcon(QIcon(QPixmap(":/photon-02.png")));

    icon->setVisible(true);
    icon->show();
    return 0;
}

//void MainWindow::shotSlot() //K1
//{

//}

void MainWindow::showAbout()
{
    abtw->show();
}

void MainWindow::closeApp()
{
    if(!CLOSED)
    {
        closeCrop();
    }
    if(rec)
    {
        record_Stop_FS();
    }


    //qApp->quit();
    exit(0);
}

void MainWindow::ShowContextMenu(const QPoint& pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    QMenu m;
    QAction* upload = m.addAction("Upload");
    QAction* print = m.addAction("Print");
    QAction* copy = m.addAction("Copy");
    QAction* save = m.addAction("Save");
    QAction* fulls = m.addAction("Fullscreen");
    QAction* cls = m.addAction("Clear selection");
    QAction* cancel = m.addAction("Cancel");

    if(rubberBand == 0)
    {
        upload->setEnabled(false);
        print->setEnabled(false);
        copy->setEnabled(false);
        save->setEnabled(false);
        cls->setEnabled(false);
    }
    //    QAction* shot = new QAction(tr("Take ScreenShot"), this);
    //    connect(shot, SIGNAL(triggered()), this, SLOT(shotSlot()));
    QAction* selectedItem = m.exec(globalPos);
    if (selectedItem == upload)
    {
        on_gUpload_clicked();
    }
    if(selectedItem == print)
    {
        this->print();
    }
    if(selectedItem == copy)
    {
        copyImage();
    }
    if(selectedItem == save)
    {
        Saving();
    }
    if(selectedItem == fulls)
    {
        if(rubberBand == 0)
        {
            rubberBand = new Resizable_rubber_band(this);
            //rubberBand->setWindowOpacity(0.1);
            rubberBand->resize(0,0);
            connect(rubberBand, SIGNAL(buttsHide()),this,SLOT(buttsHide()));
            rubberBand->show();
        }

        setFullScreen();
        ui->gSelect->hide();
    }
    if(selectedItem == cls)
    {

        if(bgroup->checkedId() < -1)
        {
            CLOSED = true;
            uncheck(bgroup->checkedId());
            CLOSED = false;
        }
        buttsHide();
        ui->gSize->hide();
        drawing = false;
        dob = 0;
        if(textclckd)
        {
            textclckd = false;
            delete tx;
            tx = NULL;
        }
        im->clear(); //imagereset and drawingreset functions
        pixmap = QImage(this->rect().size(),QImage::Format_ARGB32);
        tmp_image = pixmap;
        oimg = pixmap;
        image = pixmap;

        delete rubberBand;
        rubberBand = 0;
        this->update();

        QPoint p = QCursor::pos();
        p.setY(p.y()+30);
        ui->gSelect->move(p);
    }
    if(selectedItem == cancel)
    {
        closeCrop();
    }
    else
    {
        // nothing was chosen
    }
}

void MainWindow::iconClick(QSystemTrayIcon::ActivationReason t) //K4
{
    if(t == QSystemTrayIcon::Trigger)
    {
        shotSlot(0);
    }
}

void MainWindow::shotSlot(int wOn) //K2
{

    if(!this->isVisible())
    {
        rubberBand = NULL;
        h = GetForegroundWindow();
        screenShot(h,wOn);
        ui->gSelect->show();
        //rubberBand = 0;
        CLOSED = false;

        if(wOn == 1)
        {
            qApp->setActiveWindow(this);
            //SetActiveWindow()
            ui->gSelect->hide();
            QRect altWin;
            rubberBand = new Resizable_rubber_band(this);
            rubberBand->resize(0,0);
            LPRECT winRect = (LPRECT) malloc(sizeof(LPRECT));;
            GetWindowRect(h,winRect);
            int wx = winRect->left;
            int wy = winRect->top;
            int winWid = winRect->right - winRect->left;
            int winHigt = winRect->bottom - winRect->top;
            altWin = QRect(wx,wy,winWid,winHigt);
            rubberBand->setGeometry(altWin.normalized());
            connect(rubberBand, SIGNAL(buttsHide()),this,SLOT(buttsHide()));
            rubberBand->show();
            repositionSize();
            repositionButts();
        }
    }
}


void MainWindow::quit() //K2
{
    icon->setVisible(false);
    qApp->quit();
}

void MainWindow::moveToolTip(int x, int y)
{
    qWarning() << x << y;
    ui->gSelect->move(x,y);
}

void MainWindow::wheelEvent ( QWheelEvent * event )
{

//    int steps = event->delta() / 15;
//    double scaleFactor = 1.0;
//    const qreal minFactor = 1.0;
//    const qreal maxFactor = 10.0;
//    qreal h11 = 1.0, h22 = 0;
//    if(steps > 0)
//    {
//        h11 = (h11 >= maxFactor) ? h11 : (h11 + scaleFactor);
//        h22 = (h22 >= maxFactor) ? h22 : (h22 + scaleFactor);
//    }
//    else
//    {
//        h11 = (h11 <= minFactor) ? minFactor : (h11 - scaleFactor);
//        h22 = (h22 <= minFactor) ? minFactor : (h22 - scaleFactor);
//    }

//    qWarning() << event->delta() << h11 << h22;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        //buttsHide();

        if(rec && showHotSpot)
        {
            //ui->ghotspot->setStyleSheet("QLabel { background-color : red; color : red; }");
            ui->ghotspot->setStyleSheet("border-image: url(:red-circle.png) 0 0 0 0 stretch stretch;");
            ui->ghotspot->update();
            update();

        }
        if(textclckd)
        {
            rubberBand->setCursor(Qt::ArrowCursor);
            setCursor(Qt::ArrowCursor);
            if(tx != NULL)
            {
                drawText(tx->toPlainText());
                delete tx;
                tx = NULL;
            }

            tx = drawTextBox(event);
            QPalette p = tx->palette();
            p.setBrush(QPalette::Text, curr_color);
            tx->setPalette(p);
            tx->setStyleSheet("QTextEdit{background: transparent;}");
            tx->show();
            txPos = tx->pos();
            return;
        }
        if (drawing)
        {
            rubberBand->setCursor(Qt::ArrowCursor);
            setCursor(Qt::ArrowCursor);
            lastPoint = event->globalPos();
            penDraw(lastPoint,dob);
        }
        else{

            if(rec)
            {
                return;
            }
            if(!rubberBand)
            {
                rubberBand = new Resizable_rubber_band(this);
                rubberBand->resize(0,0);
            }
            connect(rubberBand, SIGNAL(buttsHide()),this,SLOT(buttsHide()));

        }
        origin = event->pos();
    }
}

QPoint crr;

void MainWindow::moveHotSpot()
{

    QPoint p;// = pn;
    p.setY(p.y()+2);
    ui->ghotspot->move(QApplication::activeWindow()->mapFromGlobal(QCursor::pos()));
    ui->ghotspot->show();
    update();
//    ui->ghotspot->raise();
//    ui->ghotspot->show();
//    rbhotsp = ui->ghotspot;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->button() == Qt::NoButton)
    {
        if(rec && showHotSpot)
        {
            moveHotSpot();
        }
    }
    if(event->button() == Qt::RightButton)
    {
        return;
    }

    if(event->buttons() & Qt::LeftButton)
    {
        if(drawing)
        {
            rubberBand->setCursor(Qt::ArrowCursor);
            setCursor(Qt::ArrowCursor);
            penDraw(event->globalPos(),dob);
            //update();
            return;
        }
        if(textclckd)
        {
            rubberBand->setCursor(Qt::ArrowCursor);
            setCursor(Qt::ArrowCursor);
            tx->mouseMoveEvent(event);
            return;
        }
        else
        {

            if(rec)
            {
                return;
            }
            else
            {
                fullscreend = false;
                rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
            }

            rubberBand->show();
            repositionSize();
            repaint();

        }
        if(ui->gSelect->isVisible())
        {
            ui->gSelect->hide();
        }
    }
    else if(event->button() == Qt::NoButton)
    {
        if(ui->gSelect->isVisible() || rubberBand == 0)
        {
            QPoint p = event->pos();
            p.setY(p.y()+30);
            ui->gSelect->move(p);
            ui->gSelect->show();
        }
    }
}

QRect d;
QRect ltst_sz;
bool g;
void Resizable_rubber_band::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        event->ignore();
        down = false;
        return;
    }

    //emit buttsHide();

    if(w->drawing)
    {
        w->mouseMoveEvent(event);
        //update();
        return;

    }
    else
    {
        if(w->rec)
        {
            if(w->showHotSpot)
            {
                w->moveHotSpot();
            }
            update();
            return;
        }
        if(event->buttons() & Qt::LeftButton) {
            d = w->ds->screen()->geometry();
            w = (MainWindow *) qApp->activeWindow();

            w->repositionSize();
            QPoint curPos = event->globalPos();

            if (curPos != lastPos)
            {
                QPoint diff = (lastPos - curPos);
                QRect r = geometry();
                ltst_sz = w->t;
                if(w->rec)
                {
                    return;
                }
                else
                {
                    if(geometry().right() >= d.right())
                    {
                        r.setRight(d.right());
                    }

                    if(geometry().top() <= d.top())
                    {
                        r.setTop(d.top());
                    }

                    if(geometry().left() <=  d.left())
                    {
                        r.setLeft(d.left());
                    }

                    if(geometry().bottom() >=  d.bottom())//w->ds->availableGeometry().bottom())
                    {
                        r.setBottom(d.bottom());
                    }
                    setGeometry(r);
                    move(pos() - diff);
                }
                lastPos = curPos;
            }
        }
    }

}

void Resizable_rubber_band::paintEvent(QPaintEvent *event)
{
    rubberband->hide();
    QPainter painter;
    QPen pen = QPen(Qt::black);

    pen.setStyle(Qt::DashLine);
    pen.setWidthF(0.8);
    painter.begin(this);
    painter.setPen(pen);
    QRect ev = event->rect();
    ev.setHeight(ev.height()-1);
    ev.setWidth(ev.width()-1);
    if(w->curr_shape == 0)
    {
        painter.drawRect(ev);
    }
    else if(w->curr_shape == 1)
    {
        painter.drawEllipse(ev);
    }
    else
    {
        painter.drawRect(ev);
    }

    painter.end();
}

void MainWindow::paintEvent(QPaintEvent *event)
{

    QPainter p;
    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if(CLOSED)
    {
        p.fillRect(this->rect(), QColor(0, 0, 0, 0x80));
        p.end();
        return;
    }
    QPainterPath path;
    path.addRect(rect());

    if(rubberBand != 0)
    {
        if(curr_shape == 0)
        {
            path.addRect(rubberBand->geometry());
        }
        else if(curr_shape == 1)
        {
            path.addEllipse(rubberBand->geometry());
        }
        else
        {
            path.addRect(rubberBand->geometry());
        }
        p.fillRect(rubberBand->geometry(),QColor(0,0,0,edo));
        //p.setCompositionMode(QPainter::RasterOp_SourceOrDestination);

        if(im->isEmpty())
        {
            p.drawImage(dirtyRect, image, dirtyRect);
        }
        else
        {
            p.drawImage(dirtyRect, oimg, dirtyRect);
        }
    }

    p.fillPath(path, QColor(0, 0, 0, 0x80));
    p.drawRect(this->geometry());
    p.end();
    if(!(drawing || textclckd) && rubberBand)
    {
        rubberBand->setCursor(Qt::SizeAllCursor);
    }
}

void Grip::paintEvent(QPaintEvent *event)
{
    QPainter p;
    p.begin(this);
    p.setBrush(Qt::black);
    p.setPen(Qt::white);

    QRect r = this->rect();
    int cx = r.x();
    int cy = r.y();
    p.drawRect(cx,cy, 5,5);
    p.end();
}

void Grip::mousePressEvent(QMouseEvent *e)
{
}

void Lab::mousePressEvent(QMouseEvent *e)
{
    w->gripMousePress(e);
}

QRect r;
void MainWindow::gripMousePress(QMouseEvent *e)
{

    if (e->button() == Qt::LeftButton) {

        lastPos = e->globalPos();
    }

}

void Grip::mouseMoveEvent(QMouseEvent *e)
{
    //w->gripMouseMove(e);
    e->accept();
}

void Lab::mouseMoveEvent(QMouseEvent *e)
{
    if((cursor().shape() == Qt::SizeFDiagCursor) && (abs(w->lastPos.y() - w->t.topLeft().y()) <= 10) && (abs(w->lastPos.x() - w->t.topLeft().x()) <= 10))
    {
        w->gripMouseMove(e,1);
    }
    if((cursor().shape() == Qt::SizeVerCursor) && (abs(w->lastPos.y() - w->t.top()) <= 10) && (abs(w->lastPos.x() - w->t.center().x()) <= 10))
    {
        w->gripMouseMove(e,2);
    }
    if((cursor().shape() == Qt::SizeBDiagCursor) && (abs(w->lastPos.y() - w->t.topRight().y()) <= 10) && (abs(w->lastPos.x() - w->t.topRight().x()) <= 10))
    {
        w->gripMouseMove(e,3);
    }
    if((cursor().shape() == Qt::SizeHorCursor) && (abs(w->lastPos.y() - w->t.center().y()) <= 10) && (abs(w->lastPos.x() - w->t.right()) <= 10))
    {
        w->gripMouseMove(e,4);
    }

    if((cursor().shape() == Qt::SizeFDiagCursor) && (abs(w->lastPos.y() - w->t.bottomRight().y()) <= 10) && (abs(w->lastPos.x() - w->t.bottomRight().x()) <= 10))
    {
        w->gripMouseMove(e,5);
    }
    if((cursor().shape() == Qt::SizeVerCursor) && (abs(w->lastPos.y() - w->t.bottom()) <= 10) && (abs(w->lastPos.x() - w->t.center().x()) <= 10))
    {
        w->gripMouseMove(e,6);
    }

    if((cursor().shape() == Qt::SizeBDiagCursor) && (abs(w->lastPos.y() - w->t.bottomLeft().y()) <= 10) && (abs(w->lastPos.x() - w->t.bottomLeft().x()) <= 10))
    {
        w->gripMouseMove(e,7);
    }

    if((cursor().shape() == Qt::SizeHorCursor) && (abs(w->lastPos.y() - w->t.center().y()) <= 10) && (abs(w->lastPos.x() - w->t.left()) <= 10))
    {
        w->gripMouseMove(e,8);
    }
}

void MainWindow::gripMouseMove(QMouseEvent *e,int o)
{
    if(rec)
    {
        return;
    }
    r = t;

    switch(o)
    {
    case 1:
        r.setTopLeft(e->globalPos());
        break;
    case 2:
        r.setTop(e->globalPos().y());
        break;
    case 3:
        r.setTopRight(e->globalPos());
        break;
    case 4:
        r.setRight(e->globalPos().x());
        break;
    case 5:
        r.setBottomRight(e->globalPos());
        break;
    case 6:
        r.setBottom(e->globalPos().y());
        break;
    case 7:
        r.setBottomLeft(e->globalPos());
        break;
    case 8:
        r.setLeft(e->globalPos().x());
        break;
    default:
        break;
    }

    rubberBand->setGeometry(r.normalized());
}

void Lab::mouseReleaseEvent(QMouseEvent *e)
{
    w->gripMouseRelease(e);
}


void MainWindow::gripMouseRelease(QMouseEvent *e)
{
    if(rubberBand->width() == 0 || rubberBand->height() == 0)
    {
        rubberBand->setGeometry(t.normalized());
    }
    repositionButts();
    t = rubberBand->geometry();
}

void Lab::paintEvent(QPaintEvent *event)
{
    QPainter p;
    p.begin(this);
    p.setBrush(Qt::black);
    p.setPen(Qt::white);

    QRect r = this->rect();
    int cx = r.x();
    int cy = r.y();
    p.drawRect(cx,cy, 5,5);
    p.end();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(rec && showHotSpot)
    {
        //ui->ghotspot->setStyleSheet("QLabel { background-color : yellow; color : yellow;}");
        ui->ghotspot->setStyleSheet("border-image: url(:yellow-circle.png) 0 0 0 0 stretch stretch;");

    }
    if(rubberBand)
    {
        if(drawing) //|| textclckd
        {

            tmp_image = oimg;
            image = tmp_image;
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer,"PNG");//imgFormat.toStdString().c_str()
            im->append(buffer.data());
        }
        if(rubberBand->height() == 0 || rubberBand->width() == 0)
        {
            buttsHide();
            ui->gSize->hide();
            delete rubberBand;
            rubberBand = 0;
            QPoint p = QCursor::pos();
            p.setY(p.y()+30);
            ui->gSelect->move(p);
            ui->gSelect->show();
            this->update();
            return;
        }
        if(rubberBand->geometry().height() > 1 && rubberBand->geometry().width() > 1)
        {
            repositionButts();
            t = rubberBand->geometry();
            this->update();
        }
    }
}


void MainWindow::drawSizeLabel()
{

    ui->gSize->setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Window, QColor("grey"));
    ui->gSize->setPalette(palette);
}

QRect area;

QRect scrn;
QRect hb;
QRect vb;
QPoint hbt;
QPoint vbt;
int h_st;
int v_st;

void MainWindow::repositionButts()
{
    /* Read Notes
      might rewrite the whole function
      fuck
      */

    scrn = ds->screen()->geometry();
    area = rubberBand->geometry();
    hb = ui->gHbtns->geometry();
    vb = ui->gVbtns->geometry();

    /* STATES */
    bool no_rt = false;
    bool no_bott = false;
    bool no_top = false;
    bool no_lf = false;

    /* END OF STATES */

    /* DEFAULT */
    vb.moveLeft(area.right() + 10);
    vb.moveBottom(area.bottom());
    hb.moveRight(area.right());
    hb.moveTop(area.bottom()+10);
    /* EOF DEFAULT */

    /* POSITIONS CONDITIONS */
    //if(abs(scrn.bottom() - area.bottom()) < hb.height() + 10)
    if(scrn.bottom() - hb.bottom() < 5)
    {
        no_bott = true;
    }
    if(vb.top() - scrn.top() < 5)
    {
        no_top = true;
    }
    if(scrn.right() - vb.right() < 5)
    {
        no_rt = true;
    }
    if(hb.left() - scrn.left() < 5)
    {
        no_lf = true;
    }

    if(area.top()-scrn.top()< hb.height()+10 && scrn.bottom()-area.bottom()<hb.height()+10)
    {
        no_top = true;
        no_bott = true;
    }
    if(area.left()-scrn.left()< vb.width()+10 && scrn.right()-area.right() < vb.width()+10)
    {
        no_rt = true;
        no_lf = true;
    }
    /* EOF POS CONDS */

    if(no_top && no_bott)
    {
        band_tb();
    }
    else
    {

        if(no_bott)
        {
            bandBottom();
        }
        if(no_top)
        {
            bandTop();
        }
    }

    if(no_rt && no_lf)
    {
        band_rl();
    }
    else
    {
        if(no_rt)
        {
            bandRight();
        }

        if(no_lf)
        {
            bandLeft();
        }
    }

    if(vb.intersects(hb))
    {
        butts_intersect();
    }
    if(scrn == area)
    {
        bandFS();
    }
    //ui->gSize->move(szp.x(),szp.y());
    ui->gHbtns->setGeometry(hb);
    ui->gVbtns->setGeometry(vb);

    ui->gVbtns->setParent(this);
    ui->gVbtns->show();
    ui->gVbtns->raise();
    ui->gHbtns->setParent(this);
    ui->gHbtns->show();
    ui->gHbtns->raise();
}

/* BUTTS STATES FUNCTIONS */
void MainWindow::bandLeft()
{
    hb.moveLeft(scrn.left()+5);
}

void MainWindow::bandTop()
{
    vb.moveTop(scrn.top()+5);
}

void MainWindow::bandRight()
{
    vb.moveRight(area.left() - 10);
}

void MainWindow::bandBottom()
{
    hb.moveBottom(area.top()-10);
}

void MainWindow::band_rl()
{
    vb.moveRight(area.right()-10);
    vb.moveBottom(area.bottom()-10);

    if(vb.top() - scrn.top() < 10 || scrn.bottom() - vb.bottom() < 10)
    {
        vb.moveRight(hb.left()-10);

        if(hb.bottom() < area.top())
        {
            vb.moveBottom(hb.bottom());
        }
        else
        {
            vb.moveTop(hb.top());
        }
    }
}

void MainWindow::band_tb()
{
    hb.moveRight(area.right()-10);
    hb.moveBottom(area.bottom()-10);

    if(hb.left() - scrn.left() < 10 || scrn.right() - hb.right() < 10)
    {
        //hb.moveRight(hb.left()-10);

        if(vb.left() > area.right())
        {
            hb.moveLeft(area.right()+10);
        }
        else
        {
            hb.moveRight(area.left()+10);
        }
    }
}

void MainWindow::butts_intersect()
{
    if(abs(scrn.left()-area.left()) < abs(scrn.right()-area.right()))
    {
        vb.moveLeft(hb.right()+10);
    }
    else
    {
        vb.moveRight(hb.left()-10);
    }
}

void MainWindow::bandFS()
{
    hb.moveRight(area.right()-10);
    hb.moveBottom(area.bottom()-10);

    vb.moveRight(hb.left()-10);
    vb.moveBottom(area.bottom()-10);
}

void MainWindow::repositionSize()
{
    QRect area = rubberBand->geometry();
    QRect scg = ds->screen()->geometry();

    QPoint szp;

    szp.setX(area.x());
    szp.setY(area.y()-(ui->gSize->height())-5);

    if(abs(area.top() - scg.top()) < ui->gSize->height()+20)
    {
        //szp.setX(area.topLeft().x()+5);
        szp.setY(area.topLeft().y()+5);
    }
    if(abs(area.left() - scg.right()) < ui->gSize->width()+5)
    {
        szp.setX(area.topRight().x()-(ui->gSize->width())-1);
        //szp.setY(area.topLeft().y()+5);
    }
    else
    {

    }

    ui->gSize->setParent(this);
    ui->gSize->move(szp.x(),szp.y());
    ui->gSize->setText("    "+QString::number(area.width())+"x"+QString::number(area.height()));
    ui->gSize->show();
}

Grip::Grip(QWidget *parent) : QSizeGrip (parent)
{
    w = (MainWindow *) qApp->activeWindow();
}

Lab::Lab(QWidget *parent) : QLabel (parent)
{
    w = (MainWindow *) qApp->activeWindow();
    //setCursor(Qt::SizeFDiagCursor);
}

Dragger::Dragger(QWidget *parent) : QLabel (parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::SubWindow);
    w = (MainWindow *) qApp->activeWindow();
    setMinimumSize(10,10);
    //setStyleSheet("QLabel { background-color : red; color : red; }");
    setMouseTracking(true);
    //setCursor(Qt::SizeFDiagCursor);
}

QPoint txPosInit;

void Dragger::mousePressEvent(QMouseEvent *event)
{
    //qWarning() << "DRAG";
    txPosInit = event->pos();
}

void Dragger::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        //qWarning() << "MOVES";

        //qWarning() << w->tx->geometry();
        //w->tx->setGeometry(QRect(txPosInit, event->pos()).normalized());
        w->tx->move(event->globalPos());
        w->repaint();
    }
}

bool ko = true;
Resizable_rubber_band::Resizable_rubber_band(QWidget *parent) : QWidget(parent) {

    setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setWindowFlags(Qt::SubWindow);
    setCursor(Qt::SizeAllCursor);

    layout = new QGridLayout(this);

    rubberband = new QWidget(this);
    //rubberband = new QRubberBand(QRubberBand::Rectangle, this);
    rubberband->setAttribute(Qt::WA_TranslucentBackground, true);
    rubberband->setAttribute(Qt::WA_NoSystemBackground);
    setMinimumSize(0,0);
    layout->setContentsMargins(0,0,0,-10);//L T R B

    grip1 = new Lab(this);
    grip2 = new Lab(this);
    grip3 = new Lab(this);
    grip4 = new Lab(this);
    grip5 = new Lab(this);
    grip6 = new Lab(this);
    grip7 = new Lab(this);
    grip8 = new Lab(this);

    grip1->setCursor(Qt::SizeFDiagCursor);
    grip2->setCursor(Qt::SizeVerCursor);
    grip3->setCursor(Qt::SizeBDiagCursor);
    grip4->setCursor(Qt::SizeHorCursor);
    grip5->setCursor(Qt::SizeFDiagCursor);
    grip6->setCursor(Qt::SizeVerCursor);
    grip7->setCursor(Qt::SizeBDiagCursor);
    grip8->setCursor(Qt::SizeHorCursor);

    layout->addWidget(grip1, 0,0,0,0, Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(grip2, 0,0,0,7, Qt::AlignTop | Qt::AlignCenter);
    layout->addWidget(grip3, 0,0,0,7, Qt::AlignTop | Qt::AlignRight);
    layout->addWidget(grip4, 0,0,7,7, Qt::AlignVCenter | Qt::AlignRight);
    layout->addWidget(grip5, 0,0,7,7, Qt::AlignBottom | Qt::AlignRight);
    layout->addWidget(grip6, 0,0,7,7, Qt::AlignBottom | Qt::AlignHCenter);
    layout->addWidget(grip7, 0,0,7,7, Qt::AlignBottom | Qt::AlignLeft);
    layout->addWidget(grip8, 0,0,7,7, Qt::AlignVCenter | Qt::AlignLeft);
    //v->addWidget(rubberband,0,0,0,0);

    layout->setSizeConstraint(QLayout::SetNoConstraint);
    this->setContentsMargins(0,0,0,-17); // Lab margins
    layout->addWidget(rubberband,0,0,0,0);
    setMouseTracking(true);
    rubberband->show();

}

void Resizable_rubber_band::resizeEvent(QResizeEvent *ev) {
    w = (MainWindow *) qApp->activeWindow();
    if(w->rec)
    {
        //htsp = w->rbhotsp;
        return;
    }
    rubberband->resize(size());
    w->fullscreend = false;
    emit buttsHide();
    w->repositionSize();
}

void MainWindow::setFullScreen()
{
    if(drawing || textclckd)
    {
        return;
    }
    rubberBand->setGeometry(ds->screen()->geometry());
    fullscreend = true;
    repositionButts();
}

void Resizable_rubber_band::mouseDoubleClickEvent(QMouseEvent *event)
{
    w = (MainWindow *) qApp->activeWindow();
    if(w->rec)
    {
        return;
    }
    w->setFullScreen();
}

void Resizable_rubber_band::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        down = false;
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if(w->textclckd)
        {
            w->mousePressEvent(event);
        }
        if (w->drawing) {
            //w->lastPoint = event->globalPos();
            w->mousePressEvent(event);
        }
        else
        {
            emit buttsHide();
            down = true;
            lastPos = event->globalPos();
            w->mousePressEvent(event);
        }
    }

}

void MainWindow::buttsHide()
{
    ui->gHbtns->hide();
    ui->gVbtns->hide();
    ui->gShbtns->hide();
}

void Resizable_rubber_band::mouseReleaseEvent(QMouseEvent * event)
{
    w = (MainWindow *) qApp->activeWindow();
    w->repositionButts();
    down = false;

    if(w->rec && w->rec_mov)
    {
        w->rec_mov = false;
        w->record_start();
    }
    w->mouseReleaseEvent(event);
}

void Grip::mouseReleaseEvent(QMouseEvent * event)
{
    w = (MainWindow *) qApp->activeWindow();
    w->repositionButts();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        qWarning() << event->type();
    }
    return true;
}

void MainWindow::on_gShare_clicked()
{
    if(!ui->gShbtns->isHidden())
    {
        ui->gShbtns->hide();
    }
    else
    {
        sharer();
    }
}

void MainWindow::sharer()
{
    QRect e = ui->gHbtns->geometry();
    ui->gShbtns->move(e.x()+20,e.y()+40);
    ui->gShbtns->show();
}

void MainWindow::on_gSave_clicked()
{
    Saving();
}

void MainWindow::Saving()
{
    saveAs();
    if(textclckd)
    {
        delete tx;
        tx = NULL;
    }

}

QString MainWindow::saveAs()
{

    serim = loadSettings(1);
    saveSettings(1);
    QString imgName = imgLoc+"/"+"Screenshot_"+QString::number(serim);
    QString imgformats = QString("*."+imgFormat.toLower()+";;"+"*.png;;*.jpg;;*.bmp");
    QString fn = qfd->getSaveFileName(this, tr("Save as..."),imgName,tr(imgformats.toStdString().c_str()));

    if (fn.isEmpty())
        return "";
    if (! (fn.endsWith(".jpg", Qt::CaseInsensitive) || fn.endsWith(".jpeg", Qt::CaseInsensitive) || fn.endsWith(".png", Qt::CaseInsensitive) || fn.endsWith(".bmp", Qt::CaseInsensitive)) )
        fn += "."+imgFormat.toLower(); // default

    //emit tmpImg(fn);

    saveTmpImage(fn);
    return fn;
}


bool MainWindow::copyImage()
{
    QString fn = saveTmpImage("");
    QImage image(fn);
    QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
    //QFile::remove(fn);

    return true;
}

void MainWindow::on_gCopy_clicked()
{
    copyImage();
}

void MainWindow::print()
{


    if (dlg->exec() == QDialog::Accepted) {
        QString fn = saveTmpImage("");
        QImage img(fn);
        QPainter painter(printer);
        painter.drawImage(QPoint(0,0),img);
        painter.end();
    }
}

void MainWindow::on_gPrint_clicked()
{
    print();
}

void MainWindow::on_gClose_clicked()
{
    closeCrop();
}

void MainWindow::closeCrop()
{
    CLOSED = true;
    if(bgroup->checkedId() < -1)
    {
        uncheck(bgroup->checkedId());
    }
    if(rec)
    {
        rec = false;
        record_stop();
        //ui->gRec->setText("Rec");
        ui->gRec->setIcon(QIcon(":/record.png"));
        ui->ghotspot->hide();
        rec_mov = false;

    }

    if(rubberBand)
    {
        rubberBand->hide();
        delete rubberBand;
        this->update();
    }

//    im->clear();
//    image = QImage();
//    tmp_image = QImage();
//    oimg = QImage();
//    pixmap = QImage();

    if(textclckd)
    {
        textclckd = false;
        delete tx;
        tx = NULL;
    }
    drawing = false; //////////////////////// recheck
    saveSettings(0);
    dob = 0;
    buttsHide();
    ui->gSize->hide();
    ui->gSelect->hide();


    edo = 0x1;//

    repaint();
    //this->close();
    this->hide();
}

/************ LINKS AND SHARING ********************/
QString link;
bool linked = false;
QString filename;

QString MainWindow::getLink()
{
    prepare_upload();
    uploader();
    //gProgressForm->close();
    return link;
}

void MainWindow::createLink(QString uid)
{
    link = uid;
    uploadFinishUi(link);
}

void MainWindow::openBrowser(QString lnk)
{
    gProgressForm->close();
    QDesktopServices::openUrl(QUrl(lnk, QUrl::TolerantMode));
}

void MainWindow::openBrowserB()
{
    QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
}

void MainWindow::copyText()
{
    QApplication::clipboard()->setText(link, QClipboard::Clipboard);
}

void MainWindow::fb_share(QString link)
{
    QString fb = "https://www.facebook.com/sharer/sharer.php?u="+link;
    openBrowser(fb);
}

void MainWindow::tw_share(QString link)
{
    QString tw = "https://twitter.com/home?status="+link;
    openBrowser(tw);
}

void MainWindow::vk_share(QString link)
{
    QString vk = "http://vk.com/share.php?url="+link;
    openBrowser(vk);
}

void MainWindow::pin_share(QString link)
{
    QStringList nl = link.split("/");
    link = nl.at(0)+"//"+nl.at(2)+"/images/upload/"+nl.at(4)+"."+imgFormat.toLower();
    QString pin = "https://pinterest.com/pin/create/button/?url=&media="+link+"&description=";
    openBrowser(pin);
}

void MainWindow::goog_search(QString link)
{
    QStringList nl = link.split("/");
    link = nl.at(0)+"//"+nl.at(2)+"/images/upload/"+nl.at(4)+"."+imgFormat.toLower();
    QString goog = "https://www.google.com/searchbyimage?image_url="+link;
    openBrowser(goog);
}

QNetworkReply *reply;
QNetworkAccessManager *manager;

void MainWindow::upload(QString fn)
{
    manager = new QNetworkAccessManager();
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QNetworkRequest request(QUrl("https://cdn.qreo.net/images/upload.php"));

    fn = filename;

    QFileInfo finf(fn);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\""+finf.fileName()+"\""));
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("multipart/form-data"));
    QFile f(fn);
    f.open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(&f);
    f.setParent(multiPart);
    multiPart->append(imagePart);
    reply = manager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgressBar(qint64,qint64)));

    QEventLoop eventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkReply(QNetworkReply*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void MainWindow::networkReply(QNetworkReply *rep)
{
    QString rp;
    if(rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString() == "200")
    {
        rp.append(rep->readAll());
        qWarning() << "REPLY"  << rp << endl;
        //        if(!rp.contains("data\":{\"link"))
        //        {
        //            qWarning() << "ERROR";
        //        }
        //        else
        //        {
        QFileInfo fs(filename);
        progressBar->setValue(fs.size());
        qWarning() << fs.size() << endl;
        parseReply(rp);
        //        }
    }
}

void MainWindow::parseReply(QString rep)
{
    //{"fileupload":["HsjAjIaG"]}

    QString lt;


    QJsonDocument jsonResponse = QJsonDocument::fromJson(rep.toUtf8());
    QJsonObject object = jsonResponse.object();

//    QJsonValue value = object.value("data");
//    lt = value.toObject().value("link").toString();
//    qWarning() << "link" << lt;

//    link = lt;

    QJsonValue value = object.value("link");
    lt = value.toString().replace("http://","https://");
    qWarning() << "link" << lt;

    link = lt;
    createLink(lt);
}

void MainWindow::uploadProgressBar(qint64 s,qint64 t)
{
    progressBar->setValue(s);
    qWarning() << s;
    if(s == t)
    {
        linked = true;
    }
}

void MainWindow::ProgressForm(QString fn)
{
    QFileInfo f(fn);

    gProgressForm = new QWidget(0);
    QRect r = ds->availableGeometry();
    gProgressForm->setWindowFlags (Qt::FramelessWindowHint |Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint |  Qt :: WindowCloseButtonHint);
    gProgressForm->setWindowIcon((QIcon(":/logo.ico")));
    gProgressForm->setGeometry(r.width()-360-10,r.height()-50-50,360,70);
    gProgressForm->setObjectName(QString::fromUtf8("gProgressForm"));
    gProgressForm->resize(360, 50);
    gProgressForm->setMinimumSize(QSize(360, 50));
    gProgressForm->setMaximumSize(QSize(400, 50));
    gProgressForm->setWindowTitle("Uploading Image");
    progressBar = new QProgressBar(gProgressForm);
    progressBar->setObjectName(QString::fromUtf8("progressBar"));
    progressBar->setGeometry(QRect(110, 10, 241, 31));
    progressBar->setTextVisible(false);
    progressBar->setMinimum(0);
    progressBar->setMaximum(f.size());
    gCancelUp = new QPushButton(gProgressForm);
    gCancelUp->setObjectName(QString::fromUtf8("gCancelUp"));
    gCancelUp->setGeometry(QRect(20, 10, 60, 25));
    gCancelUp->setText("Cancel");

    connect(gProgressForm,SIGNAL(destroyed()),this,SLOT(upCancel()));
    connect(gCancelUp,SIGNAL(clicked()),this,SLOT(upCancel()));

    gProgressForm->setAttribute(Qt::WA_DeleteOnClose, true);
    gProgressForm->setWindowFlags (Qt::FramelessWindowHint |Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint |  Qt :: WindowCloseButtonHint);
    gProgressForm->show();
    gProgressForm->raise();
#ifdef Q_OS_WIN
    SetWindowPos((HWND)gProgressForm->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif
    gProgressForm->activateWindow();
}

void MainWindow::uploadFinishUi(QString uid)
{
    if(!reply)
    {
        return;
    }
    if(uid.isEmpty())
    {
        uid = "Error couldn't upload image";
    }
    progressBar->hide();
    gCancelUp->hide();

    gOpurl = new QPushButton(gProgressForm);
    gOpurl->setObjectName(QString::fromUtf8("gOpurl"));
    gOpurl->setGeometry(QRect(5, 12, 75, 25));
    gCpurl = new QPushButton(gProgressForm);
    gCpurl->setObjectName(QString::fromUtf8("gCpurl"));
    gCpurl->setGeometry(QRect(90, 12, 75, 25));
    textEdit = new QTextEdit(gProgressForm);
    textEdit->setObjectName(QString::fromUtf8("textEdit"));
    textEdit->setGeometry(QRect(177, 12, 220, 25));
    textEdit->setText(uid);
    textEdit->selectAll();
    gOpurl->setText("Open");
    gCpurl->setText("Copy");
    connect(gOpurl,SIGNAL(clicked()),this,SLOT(openBrowserB()));
    connect(gCpurl,SIGNAL(clicked()),this,SLOT(copyText()));
    connect(gOpurl,SIGNAL(clicked()),gProgressForm,SLOT(close()));
    connect(gCpurl,SIGNAL(clicked()),gProgressForm,SLOT(close()));
    gOpurl->show();
    gCpurl->show();
    textEdit->show();
    gProgressForm->activateWindow();
    textEdit->raise();
    textEdit->activateWindow();
#ifdef Q_OS_WIN
    SetWindowPos((HWND)gProgressForm->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
#endif

}

void MainWindow::stopUpload()
{
    gProgressForm->close();
    reply->abort();
}


void MainWindow::upCancel()
{
    reply->abort();
    gProgressForm->close();
}

/************ LINKS AND SHARING ********************/

void MainWindow::on_gfb_clicked()
{
    fb_share(getLink());
}

void MainWindow::on_gtw_clicked()
{
    tw_share(getLink());
}

void MainWindow::on_gpin_clicked()
{
    pin_share(getLink());
}

void MainWindow::on_gvk_clicked()
{
    vk_share(getLink());
}

void MainWindow::on_gGoogImg_clicked()
{
    goog_search(getLink());
}

void MainWindow::prepare_upload()
{
    QString fn = saveTmpImage("");
    filename = fn;
    closeCrop();
    ProgressForm(fn);
}

void MainWindow::uploader()
{
    upload(filename);
}

void MainWindow::on_gUpload_clicked()
{
    QString lnk = getLink();
    uploadFinishUi(lnk);
}


/* SHELLEX HANDLER */

void MainWindow::filesReceived(QString files)
{
    QStringList fs = files.split("|",QString::KeepEmptyParts);

    for(int fl=0; fl < fs.length(); fl++)
    {
        filename = fs.at(fl);
        if(filename.length() < 1)
        {
            continue;
        }
        qWarning() << filename;
        uploader();
    }
}

/* DRAWING */
void MainWindow::penDraw(QPoint endPoint, int dobj)
{
    //lineWidth = 3;
    textclckd = false;

    QPainter painter;
    QLine l;
    painter.begin(&image);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setPen(QPen(curr_color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    switch(dobj)
    {
    case 1: //Freehand Pen
        painter.drawLine(lastPoint, endPoint);
        lastPoint = endPoint;
        painter.end();
        break;
    case 2: //Straight Line
        l.setP1(lastPoint);
        l.setP2(endPoint);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.drawLine(l);
        painter.end();
        endPoint = lastPoint;
        break;
    case 3: //Arrow
        raStartPoint  = lastPoint;
        raEndPoint = endPoint;

        //RArrow();

        raSetArrowHead();
        raSetArrowLine();

        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.drawPolygon(raArrowPoints,7);
        painter.end();
        break;
    case 4: //Rectangle

        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.drawRect(QRect(lastPoint, endPoint).normalized());
        painter.end();
        break;

    case 5: //Marker
        painter.setOpacity( .5f );
        painter.setPen(QPen(m_defcolor, markWidth, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin));
        painter.setCompositionMode(QPainter::RasterOp_SourceOrDestination);
        painter.drawLine(lastPoint, endPoint);
        lastPoint = endPoint;
        painter.end();
        break;

    case 6: //Ellipse
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.drawEllipse(QRect(lastPoint, endPoint).normalized());
        painter.end();
        break;
    default:
        break;
    }

    oimg = image;
    repaint();
    //update();
    if(dobj == 2 || dobj == 3 || dobj == 4 || dobj == 6)
    {
        image = tmp_image;
    }
}

bool drawingText = false;
void MainWindow::drawText(QString txt)
{
    //qWarning() << "EMP" << txt;
    if(!txt.isEmpty())
    {
        drawingText = true;
        tx->grip1->hide();
        tx->setCursorWidth(0);
        tx->setTextCursor(QTextCursor());
        tx->repaint();
        QPainter p;
        p.begin(&image);

        // TO RENDER OR TO DRAWCONTENTS, that's the question
        if(!foutLine)
        {
            p.setPen(curr_color);
            p.setFont(tx->font());
            QRect txr = tx->rect();
            p.drawText(txPos.x()+3,txPos.y()+5,txr.width(),txr.height(),Qt::TextWordWrap, txt); //txt
        }
        else
        {
            tx->render(&p,tx->pos(),QRegion(tx->rect()));
        }
        p.end();
        drawingText = false;
        tx->close();
        delete tx;
        tx = NULL;
        oimg = image;
        tmp_image = oimg;
        image = tmp_image; //recheck
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG"); //imgFormat.toStdString().c_str()
        im->append(buffer.data());
        repaint();
    }
    else
    {
        tx->close();
        delete tx;
        tx = NULL;
    }
    rubberBand->update();
    update();
}

void MainWindow::drawButton(int dobj)
{
    if(textclckd)
    {
        txtLostFocus();
    }

    if(!drawing)
    {
        unsetCursor();
    }
    else
    {
        rubberBand->setCursor(Qt::ArrowCursor);
        setCursor(Qt::ArrowCursor);
        drawing = true;
        image = tmp_image;
    }

    if(dobj == 5)
    {
        updateColorIcon(m_defcolor);
    }
    else
    {
        updateColorIcon(curr_color);
    }
}

void MainWindow::on_gPen_clicked(bool checked)
{
    if(checked)
    {
        drawing = true;
        drawButton(1);
        dob = 1;
    }
    else
    {
        drawing = false;
        dob = 0;
    }
}

void MainWindow::on_gLine_clicked(bool checked)
{
    if(checked)
    {
        drawing = true;
        drawButton(2);
        dob = 2;
    }
    else
    {
        drawing = false;
        dob = 0;
    }
}

void MainWindow::on_gRect_clicked()
{

}

void MainWindow::on_gRect_clicked(bool checked)
{
    if(checked)
    {
        //uncheck(ui->gRect);
        drawing = true;
        drawButton(4);
        dob = 4;
    }
    else
    {
        drawing = false;
        dob = 0;
    }
}

void MainWindow::on_gMarker_clicked(bool checked)
{
    if(checked)
    {
        drawing = true;
        drawButton(5);
        dob = 5;
    }
    else
    {
        drawing = false;
        dob = 0;
    }
}

void MainWindow::on_gUndo_clicked()
{

    if(im->empty())
    {
        return;
    }
    if(im->length()<=1)
    {
        image = pixmap;
        oimg = pixmap;
        tmp_image = pixmap;

        im->clear();
        update();
        return;
    }
    im->removeLast();
    image.loadFromData(im->last());
    oimg = image;
    update();

}

void MainWindow::on_gColor_clicked()
{
    onColor();
}

void MainWindow::onColor()
{
    QColor color;

    if(dob == 5)
    {
        color = QColorDialog::getColor(m_defcolor, this);
        if( !color.isValid() ){
           color = m_defcolor;
        }
        color.setAlpha(110);
        m_defcolor = color;
    }
    else
    {
        color = QColorDialog::getColor(curr_color, this);
        if( !color.isValid() ){
           color = curr_color;
        }
        curr_color = color;
    }

    pal.setColor(QPalette::Window, color);
    ui->gColor->setPalette(pal);

    updateColorIcon(color);
}

void MainWindow::on_gArrow_clicked(bool checked)
{
    if(checked)
    {
        drawing = true;
        RArrow();
        drawButton(3);
        dob = 3;
    }
    else
    {
        drawing = false;
        dob = 0;
    }
}

Kof::Kof(QTextEdit *parent) : QTextEdit(parent) {
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::SubWindow);

    w = (MainWindow *) qApp->activeWindow();
    ed = new QTextEdit();
    layout = new QGridLayout(this);
    setMinimumSize(0,0);
    layout->setContentsMargins(0,0,0,0);//L T R B

    grip1 = new Dragger(w);
    grip1->setCursor(Qt::OpenHandCursor);
    grip1->setMouseTracking(true);
    QPixmap dragPix(":/drag.png");
    dragPix = dragPix.scaled(20,20);
    grip1->setPixmap(dragPix);
    //grip1->setGeometry(200,200,20,20);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    this->setContentsMargins(0,0,0,0);
    layout->addWidget(grip1, 0,0,0,0, Qt::AlignTop | Qt::AlignLeft);
}

Kof::~Kof(){
    ed->~QTextEdit();
}

void Kof::focusOutEvent(QFocusEvent* e)
{


    if(conMenu)
    {
        return;
    }
    if (e->type() == QEvent::FocusOut)
    {
       // w->txtLostFocus();
    }
}

void Kof::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        conMenu = true;
        event->accept();
        update();
        repaint();
        return;
    }
    if(event->button() == Qt::LeftButton)
    {
        conMenu = false;
        update();
        QTextEdit::mousePressEvent(event);
    }

    update();
    repaint();
    return;
}

void Kof::mouseMoveEvent(QMouseEvent *e)
{
    QTextEdit::mouseMoveEvent(e);
}

QAbstractButton *tmpb = NULL;
void MainWindow::uncheck(int bt)
{
    QAbstractButton *b = NULL;
    b = bgroup->button(bt);
    if(CLOSED && bt < -1)
    {
        b->setChecked(false);
        return;
    }
    if(tmpb != NULL && tmpb != b)
    {
        tmpb->setChecked(false);
    }

    tmpb = b;

    tmpb->setStyleSheet("\
                    QPushButton:checked{ \
                        border-style: outset; \
                        border-width: 2px; \
                        border-color: #"+border_color+"; \
                    }");
}

void MainWindow::on_gText_clicked(bool checked)
{
    textclckd = checked;
    //tx = NULL;
    if(textclckd == false)
    {
        txtLostFocus();
    }
    else
    {
        drawing = false;
    }


}

void MainWindow::txtLostFocus() //When Text button is set OFF or other draw key click while Text button is ON
{
    if(tx != NULL)
    {
        QString txt_tmp = tx->toPlainText();
        if(!txt_tmp.isEmpty())
        {
            drawText(txt_tmp);
        }

        textclckd = false;
        delete tx;
        tx = NULL;
        update();
    }
}

Kof* MainWindow::drawTextBox(QMouseEvent *event)
{
    Kof* txt = new Kof();
    if(foutLine)
    {
    QTextDocument *qtd = txt->document();
    QTextCharFormat qtc;// = new QTextCharFormat();
    QTextCursor cur(qtd);
    qtc.setTextOutline(QPen(QBrush(QColor(Qt::black)),font_size/12)); //recheck with outline size
    cur.setCharFormat(qtc);
    txt->setTextCursor(cur);
    }
    txt->setParent(this);
    txt->setContextMenuPolicy(Qt::DefaultContextMenu);

    txt->setReadOnly(false);
    txt->setTextInteractionFlags(Qt::TextEditorInteraction);
    txt->setFocusPolicy(Qt::WheelFocus);
    txt->setFocus();
    txt->setFrameStyle(QFrame::NoFrame);
    QSizePolicy localSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    txt->setSizePolicy(localSizePolicy);
    txt->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    txt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    txt->setLineWrapMode(QTextEdit::NoWrap);
    //QFont font("Calibri", 12);
    QFont font = def_font;
    font.setPointSize(font_size);

    txt->setFont(font);
    txt->setGeometry(event->globalPos().x(),event->globalPos().y(),font_size,font_size);
    //txt->setFontPointSize(40);

    return txt;
}

void Kof::keyPressEvent(QKeyEvent *e)
{
    QTextEdit::keyPressEvent(e);
   // repaint();
   // w->rubberBand->update();
    //w->update();
}

void Kof::keyReleaseEvent(QKeyEvent *e)
{
//    QTextCharFormat qtc;// = new QTextCharFormat();
//    QTextCursor cur = textCursor();
//    qtc.setTextOutline(QPen(QBrush(QColor(Qt::black)),w->font_size/12)); //recheck with outline size
//    cur.setCharFormat(qtc);
//    setFont(currentFont());
//    setTextCursor(cur);
    resize(document()->size().toSize());
    repaint();
    //w->rubberBand->update();
    //w->update();
}

void Kof::paintEvent(QPaintEvent *e)
{
    if(!drawingText)
    {
        QPainter painter;
        QPen pen = QPen(Qt::black);

        pen.setStyle(Qt::DashLine);
        pen.setWidthF(0.8);
        painter.begin(viewport());
        painter.setPen(pen);
        QRect ev = viewport()->rect();
        ev.setHeight(ev.height()-1);
        ev.setWidth(ev.width()-1);
        painter.drawRect(ev);
        painter.end();
    }
    QTextEdit::paintEvent( e );
}
/* EOF DRAWING */


/* WHAT'S LEFT */
/*
  - Memory issue with undo
  - Text --done
  - Arrow
  - Login
  - Design
  - Mac version
  - icons
  - resetDrawing function
*/

void MainWindow::RArrow()
{
    raStartPoint = QPoint(0,0);
    raEndPoint = QPoint(0,0);
    raLineWidth = .5f;
    raHeadHeight = 12;
    raHeadWidth = 12;
    raBorderThickness = .5f;

    for(int i=0; i<7; i++)
        raArrowPoints[i] = raStartPoint;
}

void MainWindow::raSetArrowHead()
{
    double x1 = raStartPoint.x();
    double y1 = raStartPoint.y();
    double x2 = raEndPoint.x();
    double y2 = raEndPoint.y();

    double distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

    double dx = x2 + (x1 - x2) * raHeadHeight / distance;
    double dy = y2 + (y1 - y2) * raHeadHeight / distance;

    double k = raHeadWidth / raHeadHeight;

    double x2o = x2 - dx;
    double y2o = dy - y2;

    double x3 = y2o * k + dx;
    double y3 = x2o * k + dy;

    double x4 = dx - y2o * k;
    double y4 = dy - x2o * k;

    raArrowPoints[0]=QPointF(x4,y4);
    raArrowPoints[1]=QPointF(x2,y2);
    raArrowPoints[2]=QPointF(x3,y3);
}

void MainWindow::raSetArrowLine()
{
    double x1 = raStartPoint.x();
    double y1 = raStartPoint.y();
    double x2 = (raArrowPoints[0].x()+raArrowPoints[2].x())/2.00000;
    double y2 = (raArrowPoints[0].y()+raArrowPoints[2].y())/2.00000;

    double k = raLineWidth / (sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)));

    double x2o = x2 - x1;
    double y2o = y1 - y2;

    double x3 = y2o * k + x1;
    double y3 = x2o * k + y1;

    double x4 = x1 - y2o * k;
    double y4 = y1 - x2o * k;

    double x1o = x1 - x2;
    double y1o = y2 - y1;

    double x5 = y1o * k + x2;
    double y5 = x1o * k + y2;

    double x6 = x2 - y1o * k;
    double y6 = y2 - x1o * k;

    raArrowPoints[4]=QPointF(x3,y3);
    raArrowPoints[5]=QPointF(x4,y4);
    raArrowPoints[6]=QPointF(x5,y5);
    raArrowPoints[3]=QPointF(x6,y6);
}

void MainWindow::raDrawArrow(QPainter *p)
{
    p->setPen(QPen(QBrush(raBorderColor),raBorderThickness,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p->setBrush(QBrush(raFillColor));

    raSetArrowHead();
    raSetArrowLine();

    p->setRenderHint(QPainter::HighQualityAntialiasing, true);
    p->drawPolygon(raArrowPoints,7);
}



/* BUGS
 * different colors compsite
 * clear function
 * remove textbox on any other action
 * undo delay in text
 * text box stupid shape
 * DESIGN
 */


void MainWindow::ocred(QString link)
{
    QNetworkAccessManager networkManager;

    QUrl url("https://ocr.qreo.net"); //:5000/v1/ocr
    QNetworkRequest request;
    request.setUrl(url);

    QJsonObject json;
    QStringList nl = link.split("/");
    link = nl.at(0)+"//"+nl.at(2)+"/images/upload/"+nl.at(4)+"."+imgFormat.toLower();
    qWarning() << "Ddd" << link;
    json.insert("image_url", link);

    QJsonDocument doc;
    doc.setObject(json);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    QByteArray request_body = doc.toJson();

    QNetworkReply* currentReply = networkManager.post(request, request_body);
    QEventLoop eventLoop;
    connect(&networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(ocrReply(QNetworkReply*)));
    connect(&networkManager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void MainWindow::ocrReply(QNetworkReply *rep)
{
    QString rp;

    if(rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString() == "200")
    {

        rp.append(rep->readAll());
        qWarning() << "REPLY"  << rp << endl;
        QString lt;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(rp.toUtf8());
        QJsonObject object = jsonResponse.object();
        QJsonValue value = object.value("output");
        lt = value.toString();
        ocrtxtDialog->textEdit->setText(lt);
        ocrtxt->show();
    }
}

void MainWindow::on_gOcr_clicked()
{
    ocred(getLink());
    //gProgressForm->close();
}

void MainWindow::editMode()
{
    if(edit)
    {
        edo = 0x0;
        edit = false;
    }
    else
    {
        edo = 0x1;
        edit = true;
    }
    update();
}

void MainWindow::updateCheck()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    qWarning () << QApplication::applicationVersion();
    QNetworkRequest request(QUrl("https://cdn.qreo.net/images/update.php?v="+QApplication::applicationVersion()+".0"));

    reply = manager->get(request);

    QEventLoop eventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(hasUpdate(QNetworkReply*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    reply->abort();
}

void MainWindow::record_start()
{

}

void MainWindow::record_stop()
{

}

void MainWindow::record_pause()
{

}

void MainWindow::record_resume()
{

}

void MainWindow::record_init()
{

}

void MainWindow::hasUpdate(QNetworkReply* rep) {
    QString rp;


    if(rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString() == "200")
    {
        rp.append(rep->readAll());
        qWarning() << "REPLY"  << rp << endl;
        QString lt;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(rp.toUtf8());
        QJsonObject object = jsonResponse.object();
        QJsonValue value = object.value("update");
        lt = value.toString();

        QJsonValue ver = object.value("version");
        QString sVersion = ver.toString();
        sVersion.truncate(sVersion.lastIndexOf('.'));

        if(lt == "yes")
        {
            QJsonValue value = object.value("link");
            QString updLink = value.toString();
            updLink = "<a href=\""+updLink+"\">Click to Update to the latest version</a>";
            abt->labUpdates->setText("New Update Version "+sVersion+" is Available!<br>"+updLink);
            abt->labUpdates->setTextFormat(Qt::RichText);
            abt->labUpdates->setTextInteractionFlags(Qt::TextBrowserInteraction);
            abt->labUpdates->setOpenExternalLinks(true);
            abt->labUpdates->update();
        }
        else
        {
            abt->labUpdates->setText("\nYou're using the Latest Version "+QApplication::applicationVersion());
            abt->labUpdates->update();
        }

    }
    rep->abort();
}

void MainWindow::on_gRec_clicked()
{
    if(ui->gRec->isChecked())
    {
        if(fullscreend)
        {
            closeCrop();
            record_FS();
        }
        rec = true;
        record_start();
    }
    else
    {
        rec = false;
        record_stop();
        //ui->gRec->setText("Rec");
        ui->gRec->setIcon(QIcon(":/record.png"));
        ui->ghotspot->hide();
    }
}

//void MainWindow::record_init()
//{

//    QStringList insargs;
//    insargs = VlcCommon::args();
//    insargs << "--sout-file-append";

//    VlcCommon::setPluginPath(QApplication::applicationDirPath() + "/plugins");
//    _instance = new VlcInstance(insargs, NULL);
//    //_instance->setLogLevel(Vlc::DebugLevel);
//    _player = new VlcMediaPlayer(_instance);

//    _media = new VlcMedia("screen://", _instance);
//}

//int fil=0;

void MainWindow::record_Stop_FS()
{
    record_stop();
    sysTrayRec = "Fullscreen Record";
    menu->actions().at(1)->setText(sysTrayRec);
}

void MainWindow::record_FS()
{
    if(rec)
    {
        record_Stop_FS();
    }
    else
    {
        //closeCrop();
        fullscreend = true;
        rec = true;
        record_start();
        sysTrayRec = "Stop";
        menu->actions().at(1)->setText(sysTrayRec);

    }
}

void MainWindow::showSettings()
{


    if(textclckd)
    {
        if(tx != NULL)
        {
            drawText(tx->toPlainText());
            delete tx;
            tx = NULL;
        }
    }
    update();
    if(rec)
    {
        record_pause();
    }
    setw->show();

    //WINDOWS
#ifdef Q_OS_WIN
    if(this->isVisible())
    {
        SetWindowPos((HWND)setw->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
#endif

    ///

    setw->setFocus();
    /////////////////////

    QColor color = curr_color;
    QString s("background-color: #"
              + QString(color.red() < 16? "0" : "") + QString::number(color.red(),16)
              + QString(color.green() < 16? "0" : "") + QString::number(color.green(),16)
              + QString(color.blue() < 16? "0" : "") + QString::number(color.blue(),16) + ";"
              + QString("border-style: outset;border-width: 1px;border-radius: 5px;border-color: black;"));
    sets->gBcol->setStyleSheet(s);
    sets->gBcol->update();

    color = m_defcolor;

    QString s1("background-color: #"
              + QString(color.red() < 16? "0" : "") + QString::number(color.red(),16)
              + QString(color.green() < 16? "0" : "") + QString::number(color.green(),16)
              + QString(color.blue() < 16? "0" : "") + QString::number(color.blue(),16) + ";"
              + QString("border-style: outset;border-width: 1px;border-radius: 5px;border-color: black;"));
    sets->gBMarkcol->setStyleSheet(s1);
    sets->gBMarkcol->update();

    t_curr_color = curr_color;
    t_m_defcolor = m_defcolor;

    //////////////////////////
    /// \brief connect
    ///
    t_curr_color = curr_color;
    t_m_defcolor = m_defcolor;
    connect(setw,SIGNAL(destroyed()),this,SLOT(record_pause()));
    update();
}

void MainWindow::on_gSets_clicked()
{
    showSettings();
}

void MainWindow::saveSettings(int typ)
{
    QSettings settings(setsFile, QSettings::IniFormat);

    if(typ == 1)
    {
        int img_s = serim+1;
        settings.setValue("imgSerial", QVariant(img_s));
        return;
    }

    if(typ == 2)
    {
        int vid_s = servid+1;
        settings.setValue("vidSerial", QVariant(vid_s));
        return;
    }

    /* ON START UP */
    bool chStartup = sets->gstarton->isChecked();
    if (chStartup)
    {
        //On Startup Function
        setOnStartup(true);
        settings.setValue("startup",QVariant("1"));
    }
    else
    {
        //On Startup Function
        setOnStartup(false);
        settings.setValue("startup",QVariant("0"));
    }
    /* END ON START UP */

    /*image*/
    int simgF = sets->gImgFormat->currentIndex();
    settings.setValue("imageFormat", QVariant(simgF));

    QString sImgL = sets->gImgLoc->text();
    settings.setValue("imageLocation", QVariant(sImgL));
    imgLoc = sImgL;
    /* end image */

    /* video */
    int svidF = sets->gVidFormat->currentIndex();
    settings.setValue("videoFormat", QVariant(svidF));
    vidFormat = svidF;

    bool shMcurs = sets->gchShowCurs->isChecked();
    if (shMcurs)
    {
        //On Startup Function
        settings.setValue("showMouseCursor",QVariant("1"));
    }
    else
    {
        //On Startup Function
        settings.setValue("showMouseCursor",QVariant("0"));
    }

    bool shMhotspot = sets->gChMouseHotspot->isChecked();
    if (shMhotspot)
    {
        //On Startup Function
        settings.setValue("showMouseHotspot",QVariant("1"));
    }
    else
    {
        //On Startup Function
        settings.setValue("showMouseHotspot",QVariant("0"));
    }

    QString sVidL = sets->gVidLoc->text();
    settings.setValue("videoLocation", QVariant(sVidL));
    vidLoc = sVidL;
    /* end video */

    /* drawing */

    QColor defcolor = curr_color;//gdefColor;
    QString s("#"+QString(defcolor.red() < 16? "0" : "") + QString::number(defcolor.red(),16)
            + QString(defcolor.green() < 16? "0" : "") + QString::number(defcolor.green(),16)
            + QString(defcolor.blue() < 16? "0" : "") + QString::number(defcolor.blue(),16));

    settings.setValue("defcolor", QVariant(s));

    /////////// def color global set
    QColor defmarkcolor = m_defcolor;
    QString s1("#"+QString(defmarkcolor.red() < 16? "0" : "") + QString::number(defmarkcolor.red(),16)
            + QString(defmarkcolor.green() < 16? "0" : "") + QString::number(defmarkcolor.green(),16)
            + QString(defmarkcolor.blue() < 16? "0" : "") + QString::number(defmarkcolor.blue(),16));

    settings.setValue("defmarkcolor", QVariant(s1));
    /////////// def marker color global set

    int sFont = sets->fontComboBox->currentIndex();
    settings.setValue("font", QVariant(sFont));

    int sFontSz = sets->fontSz->value();
    settings.setValue("FontSz", QVariant(sFontSz));

    bool shOutLine = sets->gOutline->isChecked();
    if (shOutLine)
    {
        //On Startup Function
        settings.setValue("outLine",QVariant("1"));
    }
    else
    {
        //On Startup Function
        settings.setValue("outLine",QVariant("0"));
    }

    int sLineWid = sets->lineWid->value();
    settings.setValue("LineWidth", QVariant(sLineWid));

    int sMarkWid = sets->MarkWid->value();
    settings.setValue("MarkerWidth", QVariant(sMarkWid));

    /* end drawing */

    settings.sync();

    return;
}

void MainWindow::restoreDefSettings()
{
    defImgloc = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0) + "/Photon";
    if(!QDir(defImgloc).exists())
    {
        QDir().mkdir(defImgloc);
    }

    defVidloc = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0) + "/Photon";
    if(!QDir(defVidloc).exists())
    {
        QDir().mkdir(defVidloc);
    }

    QSettings settings(setsFile, QSettings::IniFormat);


    /* ON START UP */
        settings.setValue("startup",QVariant("1"));
    /* END ON START UP */

    /*image*/
    settings.setValue("imageFormat", QVariant(0));

    settings.setValue("imageLocation", QVariant(defImgloc));

    /* end image */

    /* video */
    settings.setValue("videoFormat", QVariant(2));

    settings.setValue("showMouseCursor",QVariant("0"));

    settings.setValue("showMouseHotspot",QVariant("1"));

    settings.setValue("videoLocation", QVariant(defVidloc));
    /* end video */

    /* drawing */

    settings.setValue("defcolor", QVariant("#ff0000"));

    /////////// def color global set

    settings.setValue("defmarkcolor", QVariant("#ecff17"));
    /////////// def marker color global set

    settings.setValue("font", QVariant(95));

    settings.setValue("FontSz", QVariant(12));

    settings.setValue("outLine", QVariant(1));

    settings.setValue("LineWidth", QVariant(3));

    settings.setValue("MarkerWidth", QVariant(15));

    settings.setValue("imgSerial", QVariant(1));

    settings.setValue("vidSerial", QVariant(1));

    /* end drawing */
    loadSettings(0);
    setw->update();

    return;
}

int MainWindow::loadSettings(int typ)
{
    QSettings settings(setsFile, QSettings::IniFormat);

    if(typ == 1)
    {
        int img_s = settings.value("imgSerial", "").toInt();
        serim = img_s;
        return serim;
    }
    if(typ == 2)
    {
        int vid_s = settings.value("vidSerial", "").toInt();
        servid = vid_s;
        return servid;
    }

    /* ON START UP */
    int chStartup = settings.value("startup", "").toInt();
    if (chStartup == 1)
    {
        //On Startup Function
        setOnStartup(true);
        sets->gstarton->setChecked(true);
    }
    else if(chStartup == 0)
    {
        //On Startup Function
        setOnStartup(false);
        sets->gstarton->setChecked(false);
    }
    /* END ON START UP */

    /*image*/
    int simgF = settings.value("imageFormat", "").toInt();
    sets->gImgFormat->setCurrentIndex(simgF);

    imgFormat = sets->gImgFormat->currentText().toLower(); //

    QString sImgL = settings.value("imageLocation", "").toString();
    sets->gImgLoc->setText(sImgL);
    imgLoc = sImgL;
    /* end image */

    /* video */
    int svidF = settings.value("videoFormat", "").toInt();
    sets->gVidFormat->setCurrentIndex(svidF);
    vidFormat = svidF;

    int shMcurs = settings.value("showMouseCursor", "").toInt();
    if (shMcurs == 1)
    {
        //On Startup Function
        sets->gchShowCurs->setChecked(true);
    }
    else if(shMcurs == 0)
    {
        //On Startup Function
        sets->gchShowCurs->setChecked(false);
    }

    int shMhotspot = settings.value("showMouseHotspot", "").toInt();
    if (shMhotspot == 1)
    {
        //On Startup Function
        sets->gChMouseHotspot->setChecked(true);
        showHotSpot = true;
    }
    else if(shMhotspot == 0)
    {
        //On Startup Function
        sets->gChMouseHotspot->setChecked(false);
        showHotSpot = false;
        if (!ui->ghotspot->isHidden())
        {
            ui->ghotspot->hide();
        }
    }

    QString sVidL = settings.value("videoLocation", "").toString();
    sets->gVidLoc->setText(sVidL);
    vidLoc = sVidL;

    /* end video */

    /* drawing */

    QString sdefcolor = settings.value("defcolor","").toString();
    //qWarning () << sdefcolor;
    QString s("background-color: " + sdefcolor + ";"
    + QString("border-style: outset;border-width: 1px;border-radius: 5px;border-color: black;"));
    //qWarning () << s;
    sets->gBcol->setStyleSheet(s);
    curr_color.setNamedColor(sdefcolor);
    updateColorIcon(curr_color);

    QString sdefmarkcolor = settings.value("defmarkcolor","").toString();
    //qWarning () << sdefmarkcolor;
    QString s1("background-color: " + sdefmarkcolor + ";"
    + QString("border-style: outset;border-width: 1px;border-radius: 5px;border-color: black;"));
    //qWarning () << s1;
    sets->gBMarkcol->setStyleSheet(s1);
    m_defcolor.setNamedColor(sdefmarkcolor);
    if(dob == 5)
    {
        updateColorIcon(m_defcolor);
    }

    int sFont = settings.value("font", "").toInt();
    sets->fontComboBox->setCurrentIndex(sFont);
    def_font.setFamily(sets->fontComboBox->currentText());

    int sFontSz = settings.value("FontSz", "").toInt();
    sets->fontSz->setValue(sFontSz);
    font_size = sFontSz;

    int shOutLine = settings.value("outLine", "").toInt();
    if (shOutLine == 1)
    {
        //On Startup Function
        sets->gOutline->setChecked(true);
        foutLine = true;
    }
    else if(shOutLine == 0)
    {
        //On Startup Function
        sets->gOutline->setChecked(false);
        foutLine = false;
    }
    else
    {
        sets->gOutline->setChecked(true);
        foutLine = true;
    }

    int sLineWid = settings.value("LineWidth", "").toInt();
    sets->lineWid->setValue(sLineWid);
    lineWidth = sLineWid;

    int sMarkWid = settings.value("MarkerWidth", "").toInt();
    sets->MarkWid->setValue(sMarkWid);
    markWidth = sMarkWid;

    update();
    return 0;
    /* end drawing */
}

void MainWindow::settingsSaved()
{
    saveSettings(0);
    loadSettings(0);
    setw->hide();

    return;
}

/* Start up */
void MainWindow::setOnStartup(bool st)
{
    QString st_path = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).at(0);
    QString shortcut = st_path+"/"+qAppName()+".lnk";

    if(st)
    {
        QFile::link(QApplication::applicationFilePath(), shortcut);
    }
    else
    {
        QFile::remove(shortcut);
    }
}

void MainWindow::browseSetImageLoc()
{
    QString pic_path = imgLoc;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Set Default Image Folder"),
                                                 pic_path,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty())
    {
        dir = imgLoc;
    }
    sets->gImgLoc->setText(dir);
}

void MainWindow::browseSetVideoLoc()
{
    QString vid_path = vidLoc;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Set Default Video Folder"),
                                                 vid_path,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty())
    {
        dir = vidLoc;
    }
    sets->gVidLoc->setText(dir);
}

void MainWindow::changeDefColor()
{
    QColor color;

    color = QColorDialog::getColor(curr_color, this);
    if( !color.isValid() ){
       color = curr_color;
    }
    else
    {
        curr_color = color;
    }

    QString s("background-color: #"
              + QString(color.red() < 16? "0" : "") + QString::number(color.red(),16)
              + QString(color.green() < 16? "0" : "") + QString::number(color.green(),16)
              + QString(color.blue() < 16? "0" : "") + QString::number(color.blue(),16) + ";"
              + QString("border-style: outset;border-width: 1px;border-radius: 5px;border-color: black;"));
    sets->gBcol->setStyleSheet(s);
    sets->gBcol->update();
}

void MainWindow::changeDefMarkColor()
{
    QColor color;

    color = QColorDialog::getColor(m_defcolor, this);
    if( !color.isValid() ){
       color = m_defcolor;
    }
    else
    {
        color.setAlpha(110);
        m_defcolor = color;
    }


    QString s("background-color: #"
              + QString(color.red() < 16? "0" : "") + QString::number(color.red(),16)
              + QString(color.green() < 16? "0" : "") + QString::number(color.green(),16)
              + QString(color.blue() < 16? "0" : "") + QString::number(color.blue(),16) + ";"
              + QString("border-style: outset;border-width: 1px;border-radius: 5px;border-color: black;"));
    sets->gBMarkcol->setStyleSheet(s);
    sets->gBMarkcol->update();
}

QString MainWindow::updateColorIcon(QColor color)
{
    QString colstr(QString(color.red() < 16? "0" : "") + QString::number(color.red(),16)
                   + QString(color.green() < 16? "0" : "") + QString::number(color.green(),16)
                   + QString(color.blue() < 16? "0" : "") + QString::number(color.blue(),16));

    QString s("background-color: #"+ colstr +";"+ QString("border-style: outset;border-width: 1px;border-radius: 5px;border-color: black;"));
    ui->gBcol->setStyleSheet(s);
    ui->gBcol->update();
    border_color = colstr;

    return border_color;
}

/* BUGS
 * Checked button on closeCrop() --done
 * scan avast evo-gen lib gather libvlc plugins --stupid avast
 * remove app data folder --done
 * close fullscreen record on take screenshot --done
 * text and record
 */


void MainWindow::on_gShape_clicked()
{
    if(curr_shape == 0)
    {
        ui->gShape->setIcon(QIcon(":/shape-circ.png"));
        curr_shape = 1;
    }
    else if(curr_shape == 1)
    {
        ui->gShape->setIcon(QIcon(":/shape-rect.png"));
        curr_shape = 0;
    }
    else
    {
        ui->gShape->setIcon(QIcon(":/shape-circ.png"));
        curr_shape = 0;
    }
    update();
    //repaint();
}
