rem gdb %1 -d=../src -d=../src/unix -d=../src/win9x
rem gdb %1 -d=../src -d=../src/inet -d=../src/wsock -d=../src/win3x -d=../src/win9x
gdb %1 --dir=../src --dir=../src/inet --dir=../src/wsock --dir=../src/csock
