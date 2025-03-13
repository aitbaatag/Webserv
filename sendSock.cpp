#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

void send_message(const std::string& message = "hello world") {
    const char* host = "127.0.0.1"; // Use "localhost" or "127.0.0.1"
    const int port = 8082;

    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error: Could not create socket\n";
        return;
    }

    // Define server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Connection failed\n";
        close(sock);
        return;
    }

    // Define the HTTP request message
    std::string request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // Send the message
    if (send(sock, request.c_str(), request.length(), 0) < 0) {
        std::cerr << "Error: Failed to send message\n";
        close(sock);
        return;
    }

    // Receive response
    char buffer[1024] = {0};
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "Received: \n" << buffer << std::endl;
    } else {
        std::cerr << "Error: No response received\n";
    }

    // Close the socket
    close(sock);
}

int main(int argc, char* argv[]) {
    std::string message = (argc > 1) ? argv[1] : "hello world";
    send_message(message);
    return 0;
}
