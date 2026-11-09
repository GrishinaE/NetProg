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
static const int DEFAULT_PORT = 7;

void printUsage(const char* prog) {
    std::cout << "Использование: " << prog << "\n";
    std::cout << "  -h, --help        Показать эту справку\n";
    std::cout << "  -a, --address <IP> Задать сетевой адрес сервера (по умолчанию " << DEFAULT_IP << ")\n";
    std::cout << "  -p, --port <PORT> Задать TCP-порт сервера (по умолчанию " << DEFAULT_PORT << ")\n";
    std::cout << "  -r, --run         Использовать IP и порт по умолчанию \n";
}

int main(int argc, char* argv[]) {
    std::string serverIp = DEFAULT_IP;
    int port = DEFAULT_PORT;
    std::string message = "";

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
                serverIp = DEFAULT_IP;
                port = DEFAULT_PORT;
                break;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    if (optind < argc) {
        for (int i = optind; i < argc; ++i) {
            message += argv[i];
            if (i < argc - 1) message += " ";
        }
    } else {
        std::cout << "Введите сообщение для отправки: ";
        std::getline(std::cin, message);
        if (message.empty()) {
            std::cerr << "Ошибка: пустое сообщение.\n";
            return 1;
        }
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
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

    std::cout << "Подключение к " << serverIp << ":" << port << "...\n";
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    ssize_t sent = send(sock, message.c_str(), message.length(), 0);
    if (sent < 0) {
        perror("send");
        close(sock);
        return 1;
    }

    char buffer[4096];
    ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (received < 0) {
        perror("recv");
        close(sock);
        return 1;
    }

    buffer[received] = '\0';
    std::cout << "Эхо-ответ: " << buffer << "\n";

    close(sock);
    return 0;
}
