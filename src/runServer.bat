cls
if not exist "..\build" mkdir "..\build"
g++ StreamFilerServer.cpp -std=c++17 -lwsock32 -o ..\build\StreamFilerServer
..\build\StreamFilerServer -c 3 -f "..\io\output" -l 8192 -t 0 8080