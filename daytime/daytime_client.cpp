#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

static const char* DEFAULT_IP = "172.16.40.1";
static const int DEFAULT_PORT = 13;

void printUsage(const char* prog) {
    std::cout << "Использование: " << prog << "\n";
    std::cout << "  -h, --help        Показать эту справку\n";
    std::cout << "  -a, --address <IP> Задать сетевой адрес сервера (по умолчанию " << DEFAULT_IP << ")\n";
    std::cout << "  -p, --port <PORT> Задать UDP-порт сервера (по умолчанию " << DEFAULT_PORT << ")\n";
    std::cout << "  -r, --run         Запустить с параметрами по умолчанию\n";
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printUsage(argv[0]);
        return 0;
    }

    std::string serverIp = DEFAULT_IP;
    int port = DEFAULT_PORT;

    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"address", required_argument, 0, 'a'},
        {"port",    required_argument, 0, 'p'},
        {"run",     no_argument,       0, 'r'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "ha:p:r", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                printUsage(argv[0]);
                return 0;
            case 'a':
                serverIp = optarg;
                break;
            case 'p':
                port = std::atoi(optarg);
                if (port <= 0 || port > 65535) {
                    std::cerr << "Ошибка: неверный порт " << optarg << "\n";
                    return 1;
                }
                break;
            case 'r':
                std::cout << " Используются параметры по умолчанию.\n";
                break;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Ошибка: неверный IP-адрес " << serverIp << "\n";
        close(sock);
        return 1;
    }

    std::cout << "Запрос к " << serverIp << ":" << port << "...\n";
    const char msg[] = "\n";
    ssize_t sent = sendto(sock, msg, 1, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (sent < 0) {
        perror("sendto");
        close(sock);
        return 1;
    }

    char buffer[4096];
    sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);

    ssize_t received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&fromAddr, &fromLen);
    if (received < 0) {
        perror("recvfrom");
        close(sock);
        return 1;
    }

    buffer[received] = '\0';
    std::cout << "Ответ от " << inet_ntoa(fromAddr.sin_addr)
              << ":" << ntohs(fromAddr.sin_port) << "\n";
    std::cout << buffer;
    if (received > 0 && buffer[received - 1] != '\n') {
        std::cout << "\n";
    }

    close(sock);
    return 0;
}
