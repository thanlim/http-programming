#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

const int MAX_BUFFER_SIZE = 4096;

void sendResponse(int client_socket, const string& response) {
    send(client_socket, response.c_str(), response.size(), 0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: myserver port_number" << endl;
        return 1;
    }

    int server_port = stoi(argv[1]);
    

    // Create a socket for the server
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind the socket to the specified port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listening failed");
        return 1;
    }

    while (true) {
        // Accept client connection
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("Accepting client connection failed");
            continue;
        }

        // Read the HTTP request
        char request_buffer[MAX_BUFFER_SIZE];
        memset(request_buffer, 0, sizeof(request_buffer));
        int bytes_received = recv(client_socket, request_buffer, sizeof(request_buffer), 0);

        if (bytes_received > 0) {
            istringstream request_stream(request_buffer);
            string method, path, http_version;
            request_stream >> method >> path >> http_version;

            // Handle GET request
            if (method == "GET") {
                string response;
                ifstream file(path.substr(1));  // Remove the leading '/'
                if (file.is_open()) {
                    response = "HTTP/1.0 200 OK\r\n";
                    response += "Connection: close\r\n";
                    response += "Date: Tue, 31 Oct 2023 15:44:04 GMT\r\n";
                    response += "Server: Apache/2.2.3 (CentOS)\r\n";
                    response += "Last-Modified: Tue, 31 Aug 2023 15:11:03 GMT\r\n";
                    response += "Content-Length: 2000\r\n"; 
                    response += "Content-Type: text/html\r\n\r\n";
                    
                    string file_contents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                    response += file_contents;
                } else {
                    response = "HTTP/1.0 404 Not Found\r\n\r\n";
                }

                sendResponse(client_socket, response);
            }
            
           // Handle PUT request
            else if (method == "PUT") {
                ofstream file(path.substr(1));  // Remove the leading '/'
                if (file.is_open()) {
                    char buffer[MAX_BUFFER_SIZE];
                    int total_bytes_written = 0;

                    while (total_bytes_written < bytes_received) {
                        int bytes_to_write = min(MAX_BUFFER_SIZE, bytes_received - total_bytes_written);
                        file.write(request_buffer + total_bytes_written, bytes_to_write);

                        total_bytes_written += bytes_to_write;

                        // If there are more bytes to receive, read them
                        if (total_bytes_written < bytes_received) {
                            bytes_received = recv(client_socket, request_buffer, sizeof(request_buffer), 0);
                        }
                    }

                    file.close();
                    sendResponse(client_socket, "HTTP/1.0 200 OK File Created\r\n\r\n");
                } else {
                    sendResponse(client_socket, "HTTP/1.0 500 Internal Server Error\r\n\r\n");
                }
            }
            
        }

        // Close the client socket
        close(client_socket);
    }

    // Close the server socket (unreachable in this example)
    close(server_socket);

    return 0;
}