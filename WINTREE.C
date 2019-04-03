/* ----------------------------------------------------------------
   WINLIST -- List the Titles of all the active windows
   ----------------------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

HAB hab;         /* Handle to an anchor block */

MRESULT EXPENTRY ClientWndProc(HWND, USHORT, MPARAM, MPARAM);
void TraverseWindows(HWND hwnd, char **list, USHORT *index, USHORT level);

int main(void)
   {
   static CHAR   szClientClass[] = "Winlist";
   static ULONG  flFrameFlags = FCF_TITLEBAR      |
                                FCF_SYSMENU       |
                                FCF_SIZEBORDER    |
                                FCF_MINMAX        |
                                FCF_SHELLPOSITION |
                                FCF_TASKLIST      |
                                FCF_VERTSCROLL    |
                                FCF_HORZSCROLL    ;

   HMQ           hmq;         /* Handle to a message queue */
   HWND          hwndFrame,   /* Handle to the fram window */
                 hwndClient;  /* Handle to the client window */
   QMSG          qmsg;        /* A queued message */

   hab = WinInitialize(0);
   hmq = WinCreateMsgQueue(hab,0);

   WinRegisterClass(hab,
                    szClientClass,
                    ClientWndProc,
                    CS_SIZEREDRAW,
                    0);

   hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
                                  WS_VISIBLE,
                                  &flFrameFlags,
                                  szClientClass,
                                  NULL,
                                  0L,
                                  NULL,
                                  0,
                                  &hwndClient);

   WinSendMsg(hwndFrame,
              WM_SETICON,
              WinQuerySysPointer(HWND_DESKTOP, SPTR_APPICON, FALSE),
              NULL);

   while(WinGetMsg(hab, &qmsg, NULL, 0, 0))
      WinDispatchMsg(hab,&qmsg);

   WinDestroyWindow(hwndFrame);
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
   } /* Main */



MRESULT EXPENTRY ClientWndProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
   {
   static HWND   hwndHscroll,  /* Handle to a horizontal scroll window */
                 hwndVscroll;  /* Handle to a vertical scroll window */
   static SHORT  sHscrollMax,  /* Horizontal scroll max */
                 sVscrollMax,  /* Vertical Scroll Max */
                 sHscrollPos,  /* Horizontal Scroll position */
                 sVscrollPos,  /* Vertical Scroll position */
                 cxChar,
                 cxCaps,
                 cyChar,
                 cyDesc,
                 cxClient,
                 cyClient,
                 MaxLines = 0,
                 cxTextTotal;
   static CHAR   *szBuffer[250];

   FONTMETRICS   fm;
   HPS           hps;          /* Handle to a presentation space */
   POINTL        ptl;
   SHORT         sLine,
                 sPaintBeg,
                 sPaintEnd,
                 sHscrollInc,
                 sVscrollInc;
   RECTL         rclInvalid;

   switch(msg)
      {
      case WM_CREATE:
         hps = WinGetPS(hwnd);

         GpiQueryFontMetrics(hps,(LONG) sizeof fm, &fm);

         cxChar = (SHORT) fm.lAveCharWidth;
         cxCaps = (SHORT) fm.lEmInc;
         cyChar = (SHORT) fm.lMaxBaselineExt;
         cyDesc = (SHORT) fm.lMaxDescender;

         WinReleasePS(hps);

         cxTextTotal = 80 * cxCaps;

         hwndHscroll = WinWindowFromID(
                          WinQueryWindow(hwnd, QW_PARENT, FALSE),
                          FID_HORZSCROLL);

         hwndVscroll = WinWindowFromID(
                          WinQueryWindow(hwnd, QW_PARENT, FALSE),
                          FID_VERTSCROLL);

         TraverseWindows(HWND_DESKTOP, szBuffer, &MaxLines, 0);
         MaxLines--;

         return 0;

      case WM_SIZE:
         cxClient = SHORT1FROMMP(mp2);
         cyClient = SHORT2FROMMP(mp2);

         sHscrollMax = max(0, cxTextTotal - cxClient);
         sHscrollPos = min(sHscrollPos, sHscrollMax);

         WinSendMsg(hwndHscroll,
                    SBM_SETSCROLLBAR,
                    MPFROM2SHORT(sHscrollPos,0),
                    MPFROM2SHORT(0,sHscrollMax));

         WinEnableWindow(hwndHscroll, sHscrollMax ? TRUE : FALSE);

         sVscrollMax = max(0, MaxLines - cyClient / cyChar);
         sVscrollPos = min(sVscrollPos, sVscrollMax);

         WinSendMsg(hwndVscroll,
                    SBM_SETSCROLLBAR,
                    MPFROM2SHORT(sVscrollPos,0),
                    MPFROM2SHORT(0,sVscrollMax));

         WinEnableWindow(hwndVscroll, sVscrollMax ? TRUE : FALSE);
         return 0;

      case WM_HSCROLL:
         switch(SHORT2FROMMP(mp2))
            {
            case SB_LINELEFT:
               sHscrollInc = -cxCaps;
               break;
            case SB_LINERIGHT:
               sHscrollInc = cxCaps;
               break;
            case SB_PAGELEFT:
               sHscrollInc = -8 * cxCaps;
               break;
            case SB_PAGERIGHT:
               sHscrollInc = 8 * cxCaps;
               break;
            case SB_SLIDERPOSITION:
               sHscrollInc = SHORT1FROMMP(mp2) - sHscrollPos;
               break;
            default:
               sHscrollInc = 0;
               break;
            } /* switch */

         sHscrollInc = max(-sHscrollPos,
                           min(sHscrollInc, sHscrollMax - sHscrollPos));

         if (sHscrollInc != 0)
            {
            sHscrollPos += sHscrollInc;
            WinScrollWindow(hwnd, -sHscrollInc, 0, NULL, NULL, NULL, NULL,
                            SW_INVALIDATERGN);

            WinSendMsg(hwndHscroll, SBM_SETPOS,
                       MPFROMSHORT(sHscrollPos), NULL);
            }

         return 0;

      case WM_VSCROLL:
         switch(SHORT2FROMMP(mp2))
            {
            case SB_LINEUP:
               sVscrollInc = -1;
               break;
            case SB_LINEDOWN:
               sVscrollInc = 1;
               break;
            case SB_PAGEUP:
               sVscrollInc = min(-1,-cyClient / cyChar);
               break;
            case SB_PAGEDOWN:
               sVscrollInc = max(1, cyClient / cyChar);
               break;
            case SB_SLIDERTRACK:
               sVscrollInc = SHORT1FROMMP(mp2) - sVscrollPos;
               break;
            default:
               sVscrollInc = 0;
               break;
            } /* switch */

         sVscrollInc = max(-sVscrollPos,
                           min(sVscrollInc, sVscrollMax - sVscrollPos));

         if (sVscrollInc != 0)
            {
            sVscrollPos += sVscrollInc;
            WinScrollWindow(hwnd, 0, cyChar * sVscrollInc, NULL, NULL, NULL,
                            NULL, SW_INVALIDATERGN);
            WinSendMsg(hwndVscroll, SBM_SETPOS,
                       MPFROMSHORT(sVscrollPos), NULL);

            WinUpdateWindow(hwnd);
            }

         return 0;

      case WM_CHAR:
         switch(CHARMSG(&msg)->vkey)
            {
            case VK_LEFT:
            case VK_RIGHT:
               return WinSendMsg(hwndHscroll, msg, mp1, mp2);
            case VK_UP:
            case VK_DOWN:
            case VK_PAGEUP:
            case VK_PAGEDOWN:
               return WinSendMsg(hwndVscroll, msg, mp1, mp2);
            }
         break;

      case WM_PAINT:
         hps = WinBeginPaint(hwnd, NULL, &rclInvalid);
         GpiErase(hps);
         sPaintBeg = max(0, sVscrollPos +
                            (cyClient - (SHORT) rclInvalid.yTop) / cyChar);
         sPaintEnd = min(MaxLines, sVscrollPos +
                            (cyClient - (SHORT) rclInvalid.yBottom) /
                             cyChar + 1);

         for(sLine = sPaintBeg; sLine < sPaintEnd; sLine++)
            {
            ptl.x = cxCaps - sHscrollPos;
            ptl.y = cyClient - cyChar * (sLine + 1 - sVscrollPos) + cyDesc;

            GpiCharStringAt(hps, &ptl,
                            (LONG) strlen(szBuffer[sLine]),
                            szBuffer[sLine]);
            }

         WinEndPaint(hps);
         return 0;
      }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
   }



void TraverseWindows(HWND hwnd, char **list, USHORT *index, USHORT level)
   {
   HWND   hwndNext;     /* Handle to a window frame */
   HENUM  hEnum;        /* Handle to an enumeration list */
   CHAR   szTemp[80];   /* Temp char buffer */
   USHORT i;

   /* Enumerate the Windows on the desktop */

   hEnum = WinBeginEnumWindows(hwnd);
   while ((hwndNext = WinGetNextWindow(hEnum)) != NULL)
      {

      /* Unlock the Window */

      WinLockWindow(hwndNext,FALSE);

      /* Get the name of the window */

      if ((list[*index] = calloc(1,80)) == NULL)
         WinMessageBox(HWND_DESKTOP, hwnd,
                       "Unable to allocate buffer",
                       "Window List", 0, MB_OK | MB_ICONEXCLAMATION);

      /* Get the title of the window */

      for (i = 0; i < level; i++)
         strcat(list[*index],"Ä");

      WinQueryWindowText(hwndNext,80,szTemp);
      if (!strlen(szTemp))
         strcpy(szTemp,"[No Title]");

      strcat(list[*index],szTemp);

      /* Append the handle to the window */

      sprintf(szTemp,"....%x:%x",SELECTOROF(hwndNext),OFFSETOF(hwndNext));
      strcat(list[*index],szTemp);

      /* Append the class name of the window */

      WinQueryClassName(hwndNext,80,szTemp);
      strcat(list[*index],"....");
      strcat(list[*index],szTemp);

      /* Go find the children */

      (*index)++;
      level += 3;
      TraverseWindows(hwndNext, list, index, level);
      level -= 3;

      } /* end While */
   WinEndEnumWindows(hEnum);
   }
