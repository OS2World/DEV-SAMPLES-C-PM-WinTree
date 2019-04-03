wintree.obj: wintree.c
  cl -c -Alfu -G2s -W3 -Zi -Od wintree.c >> wintree.tmp

wintree.exe: wintree.obj wintree.def
  link /BATCH /CODEVIEW @wintree.l       >> wintree.tmp
