cls
if not exist "..\build" mkdir "..\build"
javac -d ..\build StreamFilerClient.java
java -cp ..\build StreamFilerClient localhost 8080 ..\io\input\