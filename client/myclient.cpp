
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <streambuf>

using namespace std;

int main(int argc, char* argv[]) {
    
    // get all the arguments needed 
    
    if (argc < 5) {
        cerr << "Usage: myclient host port_number GET/PUT filename [data_to_upload for PUT]" << endl;
        return 1;
    }

    const char* server_host = argv[1];
    const int server_port = stoi(argv[2]); //stoi(argv[2]);
    const char* http_method = argv[3];
    const char* filename = argv[4];
    const char* data_to_upload = (argc == 6) ? argv[5] : ""; // Data to be uploaded for PUT (if provided)

    // Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Define the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

  
    // Convert server host to IP address
    if (inet_pton(AF_INET, server_host, &server_addr.sin_addr) <= 0) {
        if (inet_pton(AF_INET6, server_host, &server_addr.sin_addr) <= 0) {
            perror("Invalid address or address resolution failure");
            close(client_socket); // Close the socket before returning
            return 1;
        }
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) <0) {
        perror("Connection failed");
        close(client_socket); // Close the socket before returning
        return 1;
    }
    
  
    // Prepare the HTTP GET request
      string request; 
      if (strcmp(http_method, "GET") == 0) {
            request = http_method + string(" /") + filename + " HTTP/1.0\r\n";
            //request += "Host: " + string(server_host) + "\r\n";
            request += "Host: www.google.com\r\n";
            request += "Connection: close\r\n";
            request += "User-agent: Mozilla/5.0\r\n";
            request += "Accept-language: en\r\n\r\n";
    
      }
   
      // Prepare the HTTP PUT request
       else if (strcmp(http_method, "PUT") == 0) {
         request = "PUT /" + string(filename) + " HTTP/1.0\r\n";
        
        // Read the contents of the file to be uploaded
        ifstream file_to_upload(filename);
        string file_contents((istreambuf_iterator<char>(file_to_upload)), istreambuf_iterator<char>());
        file_to_upload.close();

        request += file_contents; // Include the data to be uploaded for PUT
    }
    else {
        cerr << "Invalid HTTP method. Please use GET or PUT." << endl;
        close(client_socket);
        return 1;
    }
    
    // Send the request to the server
    send(client_socket, request.c_str(), request.size(), 0);

    // Receive and display the server's response
    char response_buffer[4096];
    memset(response_buffer, 0, sizeof(response_buffer));
    int bytes_received = recv(client_socket, response_buffer, sizeof(response_buffer), 0);
    if (bytes_received > 0) {
        cout << response_buffer;
    }

    // Close the socket
    close(client_socket);

    return 0;
}
