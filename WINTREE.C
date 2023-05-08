/* ----------------------------------------------------------------
   WINLIST -- List the Titles of all the active windows
   ----------------------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define IDL_LIST    100

HAB hab;         /* Handle to an anchor block */

MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);
extern VOID TraverseWindows(HWND hwnd, HWND hwndList, SHORT sLevel);

int main(void)
   {
   static CHAR   szClientClass[] = "Winlist";
   static ULONG  flFrameFlags = FCF_TITLEBAR      |
                                FCF_SYSMENU       |
                                FCF_SIZEBORDER    |
                                FCF_MINMAX        |
                                FCF_SHELLPOSITION |
                                FCF_TASKLIST      ;

   HMQ           hmq;         /* Handle to a message queue */
   HWND          hwndFrame,   /* Handle to the fram window */
                 hwndClient;  /* Handle to the client window */
   QMSG          qmsg;        /* A queued message */

   hab = WinInitialize(0);
   hmq = WinCreateMsgQueue(hab,0);

   WinRegisterClass(hab,
                    (PCSZ) szClientClass,
                    (PFNWP) ClientWndProc,
                    CS_SIZEREDRAW,
                    0);

   hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
                                  WS_VISIBLE,
                                  &flFrameFlags,
                                  (PCSZ) szClientClass,
                                  NULLHANDLE,
                                  0L,
                                  NULLHANDLE,
                                  0,
                                  &hwndClient);

   WinSendMsg(hwndFrame, WM_SETICON, (MPARAM) WinQuerySysPointer(HWND_DESKTOP, SPTR_APPICON, FALSE), NULL);

   while(WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
      WinDispatchMsg(hab,&qmsg);

   WinDestroyWindow(hwndFrame);
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
   } /* Main */



MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
   {
   switch(msg)
      {
      case WM_CREATE:
         WinCreateWindow(hwnd, WC_LISTBOX, (PCSZ) "Window List",
                         WS_VISIBLE | LS_NOADJUSTPOS | LS_HORZSCROLL,
                         0, 0, 0, 0,
                         hwnd, HWND_TOP, IDL_LIST, NULL, NULL);

         WinSetPresParam(hwnd, PP_FONTNAMESIZE, 1+strlen("8.Courier"), "8.Courier");

         /* Traverse the desktop windows */

         TraverseWindows(HWND_DESKTOP, WinWindowFromID(hwnd, IDL_LIST), 0);
         return 0;

      case WM_SIZE:
         {
         RECTL rclWin;   /* Size of the window */

         WinQueryWindowRect(hwnd, &rclWin);

         /* Make the MLE fill the entire window */

         WinSetWindowPos(WinWindowFromID(hwnd, IDL_LIST),
            HWND_TOP, 0, 0, rclWin.xRight, rclWin.yTop, SWP_SIZE | SWP_MOVE);
         return 0;
         }

      case WM_PAINT:
         {
         HPS   hps;
         RECTL rclInvalid;

         hps = WinBeginPaint(hwnd, NULLHANDLE, &rclInvalid);
         GpiErase(hps);
         WinEndPaint(hps);
         return 0;
         } /* case block */
      }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
   } /* ClientWndProc */


VOID TraverseWindows(HWND hwnd, HWND hwndList, SHORT sLevel)
   {
   HWND   hwndNext;     /* Handle to a window frame      */
   HENUM  hEnum;        /* Handle to an enumeration list */
   CHAR   szLine[512];  /* Composed line for the listbox */
   CHAR   szTitle[80];  /* Window title                  */
   CHAR   szClass[80];  /* Window class                  */
   CHAR   szLeader[90]; /* Leading blanks                */
   USHORT i;            /* Loop index                    */

   /* Enumerate the Windows on the desktop */

   hEnum = WinBeginEnumWindows(hwnd);
   while(NULLHANDLE != (hwndNext = WinGetNextWindow(hEnum)))
      {

      /* Unlock the Window */

	#if 0
      // WinLockWindow(hwndNext,FALSE);
	  WinEnableWindow(hwndNext, FALSE);
	#endif
      /* Build the proper indentation string */

      for(i = 0; (i < 89) & (i < sLevel); i++)
         szLeader[i] = ' ';
      szLeader[i] = '\0';

      /* Get the window's title */

      WinQueryWindowText(hwndNext, 80, (PCH) szTitle);
      if (!strlen(szTitle))
         strcpy(szTitle,"[No Title]");

      /* Get the class name */

      WinQueryClassName(hwndNext, 80, (PCH) szClass);

      /* Append the handle to the window */

      sprintf(szLine,"%s%lX...%s...%s",szLeader, hwndNext, szTitle, szClass);
      WinSendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(szLine));


      /* Go find the children */

      sLevel += 3;
      TraverseWindows(hwndNext, hwndList, sLevel);
      sLevel -= 3;
      } /* while */

   WinEndEnumWindows(hEnum);
   return;
   } /* TraverseWindows */
