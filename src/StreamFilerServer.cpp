#include <iostream>
#include <fstream>
#include <iomanip>
#include <winsock2.h>
#include <thread>
#include <sys/stat.h>

void printHelp() {
    std::cout << "NAME\n"
                 "streamfiler - TCP/IP stream fogado es lemezre iro alkalmazas\n\n"

                 "SYNOPSIS\n"
                 "streamfiler [-options] portnumber\n\n"

                 "DESCRIPTION\n"
                 "A streamfiler egy porton figyelo alkalmazas, ami a csatlakozott connection "
                 "tartalmat lemezre irja. A kiirt fajl neve megegyezik az erkezesi"
                 "idoponttal (milisecundum pontossaggal). Egyszerre tobb connection-t tud"
                 "fogadni parameterezestol fuggoen.\n\n"

                 "OPTIONS\n"
                 "-c connections\n\t"
                 "A maximalisan fogadhato parhuzamos connection-ok szama\n"

                 "-f folder\n\t"
                 "A mappa, ahova az erkztetett allomanyokat letarolja a program. Ha nem"
                 "letezik indulaskor, akkor a program letrehozza azt.\n"

                 "-h\n\t"
                 "Ez a help\n"

                 "-l limit\n\t"
                 "Ekkora limit adatmennyiseget enged szalankent es masodpercenkent"
                 "kilobyte-ban a portjara folyatni a program\n"

                 "-t timeout\n\t"
                 "Azt az idot hatarozza meg masodpercben, ami utan a program bontja az\n"
                 "idle connection-oket. Timeout -1 eseten nem bontja."
              << std::endl;

}

int readFromClient(SOCKET socket, char buffer[], const int& datalimit) {
    const int receivedData = recv(socket, buffer, datalimit, 0);
    if (receivedData == SOCKET_ERROR) {
        std::cerr << "Error: Reading data from client socket: " << WSAGetLastError() << std::endl;
        closesocket(socket);
        return 0;
    } else if (receivedData == 0) {
//        std::cout << "Client stopped sending data" << std::endl;
        return 0;
    } else {
        return receivedData;
    }
}

bool sendToClient(SOCKET socket) {
    int sentData = send(socket, "OK\n", sizeof("OK\n"), 0);
    if (sentData == SOCKET_ERROR) {
        std::cerr << "Error: Sending data to client socket" << WSAGetLastError() << std::endl;
        closesocket(socket);
        WSACleanup();
        return false;
    }
    return true;

}

int main(int argc, char** argv) {

    // Command line arguments handling
    int maxConnections = SOMAXCONN;
    std::string filePath = "../io/output";
    int datalimit = 4096;
    int timeout = 0;
    int port = std::stoi(argv[argc-1]);

    try {
        for (int i = 1; i <= argc-1; ++i) {
            if (strcmp(argv[i-1], "-h") == 0) {
                printHelp();
                return 0;
            } else if (strcmp(argv[i-1], "-c") == 0) {
                maxConnections = std::stoi(argv[i]);
            } else if (strcmp(argv[i-1], "-f") == 0) {
                filePath = argv[i];
            } else if (strcmp(argv[i-1], "-l") == 0) {
                datalimit = std::stoi(argv[i]);
            } else if (strcmp(argv[i-1], "-t") == 0) {
                timeout = std::stoi(argv[i]);
            }
        }
    } catch (std::exception& ex) {
        std::cerr << "Error: Command line arguments: " << ex.what() << std::endl;
        return 1;
    }

    //Creating output folder if it doesn't exist
    struct stat info{};
    stat(filePath.c_str(), &info);
    if (!(info.st_mode & S_IFDIR)) {
        std::string cmd = "mkdir " + filePath;
        system(cmd.c_str());
    }
    std::cout << "Maximum connections: " << maxConnections << std::endl;
    std::cout << "File path: " << filePath << std::endl;
    std::cout << "Datalimit/client/sec: " << datalimit << std::endl;
    std::cout << "Timeout: " << timeout << std::endl;
    std::cout << "Port: " << port << std::endl << std::endl;

    // Initialize Winsock
    WSADATA wsaData;
    int wsaSuccess = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaSuccess != 0) {
        std::cerr << "Error: WSAStartup failed: " << wsaSuccess << std::endl;
        WSACleanup();
        return 1;
    }

    // Create Server Socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error: Cannot create server socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Bind server socket to port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Binding server socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Start listening
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error: Listening: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }


    // Wait for connections
    int socketCount = 0;
    while (true) {

        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error: Accepting client socket: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        if (socketCount < maxConnections) {

            ++socketCount;
            std::thread t([clientSocket, datalimit, filePath, &socketCount, timeout]{

                // Recieve data
                while (true) {

                    int receivedData;
                    char buffer[datalimit];
                    ZeroMemory(buffer, datalimit);

                    receivedData = readFromClient(clientSocket, buffer, datalimit);
                    if (receivedData <= 0) {
                        if (timeout == -1) {
                            continue;
                        } else {
                            if (timeout != 0) {
                                auto begin = std::chrono::high_resolution_clock::now();
                                std::chrono::time_point<std::chrono::high_resolution_clock> end;
                                do {
                                    end = std::chrono::high_resolution_clock::now();
                                } while (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() < timeout);
                            }
                            closesocket(clientSocket);
                            --socketCount;
                            break;
                        }
                    }
                    auto now = std::chrono::system_clock::now();
                    auto time = std::chrono::system_clock::to_time_t(now);
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) -
                              std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
                    std::ostringstream oss;
                    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H-%M-%S-") << ms.count();
                    const std::string fileName = oss.str();
                    sendToClient(clientSocket);
                    std::string fileFormat = buffer;

                    readFromClient(clientSocket, buffer, datalimit);
                    sendToClient(clientSocket);
                    const long fileSize = std::stoi(buffer);

                    std::ofstream outfile(filePath + "/" + fileName + "." + fileFormat, std::ios_base::app | std::ios::binary);
                    std::cout << "Creating file named: " << fileName + "." + fileFormat << " (" << fileSize << " B)" << std::endl;

                    long bytesRead = 0;
                    do {
                        receivedData = readFromClient(clientSocket, buffer, datalimit);
                        outfile.write(buffer, receivedData);
                        bytesRead += receivedData;
                    } while (fileSize > bytesRead);
                    std::cout << "File's been saved, bytes read: " << bytesRead << std::endl << std::endl;
                    sendToClient(clientSocket);

                }
            });
            t.detach();

        } else {
            std::cout << "Client connection refused: client limit reached" << std::endl;
            closesocket(clientSocket);
        }

    }


    // Cleanup
//    closesocket(serverSocket);
//    WSACleanup();
//
//    return 0;

}
