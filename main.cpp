#include <QApplication>
#include "mainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <tchar.h>
#endif

#include <iostream>
#include <vector>

MainWindow *mw;

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif

using namespace std;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("QREO");
    QApplication::setOrganizationDomain("qreo.net");
    QApplication::setApplicationName("Photon");
    QApplication::setApplicationVersion("0.9");

    QApplication::setWindowIcon(QIcon(QPixmap(":/photon-02.png")));
    QApplication::setQuitOnLastWindowClosed(false);

    MainWindow w;
    mw = &w;
    w.tray();


#ifdef Q_OS_WIN
    HHOOK hhkLowLevelKybd  = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,NULL, 0);
#endif
    a.exec();

#ifdef Q_OS_WIN
    UnhookWindowsHookEx(hhkLowLevelKybd);
#endif
    return 0;
}

#ifdef Q_OS_WIN
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM
lParam)
{

  BOOL fEatKeystroke = FALSE;

  if (nCode == HC_ACTION)
  {
     switch (wParam)
     {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        //case WM_KEYUP:
        //case WM_SYSKEYUP:
            {
                PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;
                if (p->vkCode == VK_SNAPSHOT && (GetKeyState(VK_MENU) & 0x8000))
                {
                    mw->shotSlot(1);
                    fEatKeystroke = TRUE;
                }
                else if(p->vkCode == VK_SNAPSHOT)
                {
                    mw->shotSlot(0);
                    fEatKeystroke = TRUE;
                }
                if (p->vkCode == VK_ESCAPE)
                {
                    mw->closeCrop();
                    fEatKeystroke = TRUE;
                }
                break;
            }
     }
  }

  return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam,
lParam));
}

#endif
