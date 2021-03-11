#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QMainWindow>
#include <QRubberBand>
#include <QLabel>
#include <QNetworkReply>
#include "localsocketipc.h"
#include "ui_ocredtext.h"
#include "ui_settings.h"
#include "ui_about.h"

#include <QPrinter>
#include <QPrintDialog>

namespace Ui {
class MainWindow;
}

class Dragger;
class Kof;
class Resizable_rubber_band;

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
   void tmpImg(QString);
   void checkUpdate();
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

   QDialog *ocrtxt;
   QWidget *setw;
   QWidget *abtw;

   bool edit;
   bool fullscreend;
   int edo;
   int serim;
   int servid;
   QString sysTrayRec;
   QString tmpDir;

   QTextEdit *qTextE;

   QPrinter *printer;
   QPrintDialog *dlg;

   QFileDialog *qfd;

   QString imgFormat;
   QString imgLoc;
   QString vidLoc;
   int vidFormat;
   bool showHotSpot;
   bool foutLine;
   QFont def_font;
   int font_size;

   int curr_shape;
   QLabel *rbhotsp;
   bool rec;
   bool rec_mov;
   bool down;
   bool drawing;
   bool eventFilter(QObject *o, QEvent *e);
   bool textclckd;
   #ifdef Q_OS_WIN
   HWND h;
   void screenShot(HWND actWin, int wOn);
   #endif
   int lineWidth;
   int markWidth;
   int tray();
   Kof *tx;
   LocalSocketIpcClient* m_client;
   LocalSocketIpcServer* m_server;
   QButtonGroup* bgroup;
   QColor curr_color;
   QString border_color;
   QColor t_curr_color;
   QColor defcolor;
   QColor m_defcolor;
   QColor t_m_defcolor;
   QColor myPenColor;
   QColor oldPenColor;
   QDesktopWidget* ds;
   QGroupBox *gHbtns;
   QGroupBox *gShbtns;
   QGroupBox *gVbtns;
   QHBoxLayout *hl;
   QImage ima;
   QLabel *gSelect;
   QLabel *gSize;
   QList<QByteArray> *im;
   QPalette pal;
   QImage curr_image;
   QImage combined;
   QImage image;
   QImage oimg;
   QImage msk;
   QImage pix;
   QImage pixmap;
   QImage tmp_image;
   QImage marks;
   QPoint lastPoint;
   QPoint lastPos;
   QPoint oldPoint;
   QPoint txPos;
   QProgressBar *progressBar;
   QPushButton *gCancelUp;
   QPushButton *gClose;
   QPushButton *gCopy;
   QPushButton *gCpurl;
   QPushButton *gfb;
   QPushButton *gGoogImg;
   QPushButton *gOpurl;
   QPushButton *gpin;
   QPushButton *gPrint;
   QPushButton *gSave;
   QPushButton *gShare;
   QPushButton *gtw;
   QPushButton *gUpload;
   QPushButton *gvk;
   QPushButton *pb;
   QPushButton *pushButton_10;
   QPushButton *pushButton_11;
   QPushButton *pushButton_12;
   QPushButton *pushButton_13;
   QPushButton *pushButton_14;
   QPushButton *pushButton_8;
   QPushButton *pushButton_9;
   QRect dirtyRect;
   QRect t;

   QString getLink();
   QString saveAs();
   QString setsFile;
   QString defImgloc;
   QString defVidloc;

   QTextEdit *textEdit;
   QVBoxLayout *vl;
   QWidget *gProgressForm;
   Resizable_rubber_band *rubberBand;
   void band_rl();
   void band_tb();
   void bandBottom();
   void bandFS();
   void bandLeft();
   void bandRight();
   void bandTop();
   void butts_intersect();
   void closeCrop();
   void createLink(QString uid);
   void drawButton(int dobj);
   void drawSizeLabel();
   void fb_share(QString link);
   void goog_search(QString link);
   void mouseMoveEvent(QMouseEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);
   void moveToolTip(int x, int y);
   void onColor();
   void paintEvent(QPaintEvent *event);
   void penDraw(QPoint endPoint, int dobj);
   void pin_share(QString link);
   void prepare_upload();
   void print();
   void ProgressForm(QString fn);
   void Saving();
   void setFullScreen();
   void sharer();
   void tw_share(QString link);
   void vk_share(QString link);
   void wheelEvent ( QWheelEvent * event );

public slots:
    QString saveTmpImage(QString fn);
    bool copyImage();
    void networkReply(QNetworkReply *rep);
    void buttsHide();
    void repositionButts();
    void repositionSize();
    void quit();
    void iconClick(QSystemTrayIcon::ActivationReason t);
    void shotSlot(int wOn);
    void upload(QString fn);
    void parseReply(QString rep);
    void uploader();
    void uploadProgressBar(qint64 s, qint64 t);
    void uploadFinishUi(QString uid);
    void openBrowser(QString link);
    void openBrowserB();
    void copyText();
    void upCancel();
    void gripMouseMove(QMouseEvent *e, int o);
    void gripMousePress(QMouseEvent *e);
    void gripMouseRelease(QMouseEvent *e);
    void ShowContextMenu(const QPoint & pos);
    void stopUpload();
    void filesReceived(QString files);

    Kof *drawTextBox(QMouseEvent *event);
    void drawText(QString txt);

    /* ARROW */
    void txtLostFocus();
    void uncheck(int bt);
    void ocred(QString link);
    void ocrReply(QNetworkReply *rep);
    void hasUpdate(QNetworkReply *rep);
    void updateCheck();
    void record_start();
    void record_stop();
    void record_pause();
    void record_resume();
    void record_init();
    void moveHotSpot();
    void showSettings();
    int loadSettings(int typ);
    void saveSettings(int typ);
    void settingsSaved();
    void editMode();
    void setOnStartup(bool st);
    void browseSetImageLoc();
    void browseSetVideoLoc();
    void changeDefColor();
    void changeDefMarkColor();
    QString updateColorIcon(QColor color);
    void restoreDefSettings();
    void showAbout();
    void closeApp();
    void record_FS();
    void record_Stop_FS();
public:
    void RArrow();

    void raDrawArrow(QPainter *);

    void raSetArrowHead();
    void raSetArrowLine();

public:
    QPointF raArrowPoints[7];

    QPointF raStartPoint;
    QPointF raEndPoint;

    double raLineWidth;

    double raHeadWidth;
    double raHeadHeight;

    QColor raBorderColor;
    QColor raFillColor;
    Dragger *grips;
    double raBorderThickness;
    /* EOF ARROW */
private slots:
    void on_gShare_clicked();
    void on_gSave_clicked();
    void on_gCopy_clicked();
    void on_gPrint_clicked();
    void on_gClose_clicked();
    void on_gfb_clicked();
    void on_gtw_clicked();
    void on_gpin_clicked();
    void on_gvk_clicked();
    void on_gGoogImg_clicked();
    void on_gUpload_clicked();
    void on_gPen_clicked(bool checked);
    void on_gLine_clicked(bool checked);
    void on_gRect_clicked();
    void on_gMarker_clicked(bool checked);
    void on_gUndo_clicked();
    void on_gColor_clicked();
    void on_gArrow_clicked(bool checked);
    void on_gText_clicked(bool checked);
    void on_gRect_clicked(bool checked);
    void on_gOcr_clicked();
    void on_gRec_clicked();
    void on_gSets_clicked();

    void on_gShape_clicked();

private:
    Ui::MainWindow *ui;
    QPoint origin;
    Ui::Dialog *ocrtxtDialog;

    Ui::Form *sets;
    Ui::About *abt;


};

#endif // MAINWINDOW_H
class MainWindow;
class Grip;
class Lab;

class Resizable_rubber_band : public QWidget {
    Q_OBJECT

public:
  Resizable_rubber_band(QWidget *parent = 0);
MainWindow *w;
//QGridLayout *v;
Grip *qsg;

bool down;
QPoint lastPos;
QGridLayout* layout;

Lab* grip1;
Lab* grip2;
Lab* grip3;
Lab* grip4;
Lab* grip5;
Lab* grip6;
Lab* grip7;
Lab* grip8;

QLabel *grips;
QWidget* rubberband;

QLabel *htsp;

int x,y,wd,h;
QPoint initial,current;
void mouseDoubleClickEvent(QMouseEvent *event);
void resizeEvent(QResizeEvent * ev);

public slots:

private:
  void mousePressEvent(QMouseEvent *);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *);
  void paintEvent(QPaintEvent *event);

signals:
  void bandResize();
  void buttsHide();
};

class Resizable_rubber_band;

class Grip : public QSizeGrip {
    Q_OBJECT
public:
  Grip(QWidget *parent = 0);
  QSizeGrip* qsg;
    MainWindow *w;
    Resizable_rubber_band *rb;
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);

private:
  void mouseReleaseEvent(QMouseEvent *);
signals:
  void resize(int x, int y);
  void bandResize();
  void buttsHide();
};

class Lab : public QLabel {
    Q_OBJECT

public:
  Lab(QWidget *parent = 0);
   MainWindow *w;

private:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *);
   void paintEvent(QPaintEvent *event);
signals:
  void bandResize();
  void buttsHide();
};

class myLabel: public QLabel
{

  public:
    myLabel()
    {
      this->setAlignment(Qt::AlignCenter);

      //Default Label Value
      this->setText("No Value");

      //set MouseTracking true to capture mouse event even its key is not pressed
      this->setMouseTracking(true);
    }
    ~ myLabel(){}

    void mouseMoveEvent ( QMouseEvent * event )
    {
      //Show x and y coordinate values of mouse cursor here
      this->setText("X:"+QString::number(event->x())+"-- Y:"+QString::number(event->y()));
    }

};

class Dragger : public QLabel {
    Q_OBJECT

public:
  Dragger(QWidget *parent = 0);
   MainWindow *w;

private:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *event);
   //void mouseReleaseEvent(QMouseEvent *);
   //void paintEvent(QPaintEvent *event);

};

class Kof: public QTextEdit
{
Q_OBJECT

  public:
    explicit Kof(QTextEdit *parent = 0);
    ~ Kof();


    QTextEdit *ed;
    MainWindow *w;
    QGridLayout* layout;

    bool conMenu;
    Dragger* grip1;
//    Lab* grip2;
//    Lab* grip3;
//    Lab* grip4;
//    Lab* grip5;
//    Lab* grip6;
//    Lab* grip7;
//    Lab* grip8;

    void focusOutEvent(QFocusEvent* e);
    //void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *e);

signals:
    void drawTxt(QString);
protected:
    void keyReleaseEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);
    void keyPressEvent(QKeyEvent *e);
};
