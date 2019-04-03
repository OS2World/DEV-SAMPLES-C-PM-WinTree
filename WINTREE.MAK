wintree.obj: wintree.c
  cl -c -G2sw -W3 -Zi -Op wintree.c >> tcout.tmp


wintree.exe: wintree.obj wintree.def
  link wintree,/CODEVIEW /align:16, NUL, os2, wintree >> tcout.tmp
