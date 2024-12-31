#include <string>
#include <istream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>
#include <iterator>
#include "WebServer.h"

// Checks if a file exists
bool fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}
// Checks if name ends with a suffix
bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Handler for when a message is received from the client
void WebServer::onMessageReceived(int clientSocket, const char* msg, int length) {
	
    // Parse the client's request by reading one string at a time using istream_iterator
	// first arg tells to read iss istringstream and second arg tells to stop reading
    std::istringstream iss(msg);
    std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    std::string content = "<h1>404 Not Found</h1>";
    std::string requestedFile = "/index.html";
    std::string contentType = "text/html";
    int errorCode = 404;

    // If the GET request is valid, extract the requested file
    if (parsed.size() >= 3 && parsed[0] == "GET") {
        requestedFile = parsed[1];

        if (requestedFile == "/") {
            requestedFile = "/index.html";
        }
    }

    // Build the file path (relative to the server's working directory)
    std::string filePath = "./wwwroot" + requestedFile;

    // Check if the file exists and read its contents
    if (fileExists(filePath)) {
        std::ifstream file(filePath, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        errorCode = 200;

        // Determine the file type
        if (endsWith(filePath, ".html")) contentType = "text/html";
        else if (endsWith(filePath, ".css")) contentType = "text/css";
        else if (endsWith(filePath, ".js")) contentType = "application/javascript";
        else if (endsWith(filePath, ".png")) contentType = "image/png";
        else if (endsWith(filePath, ".jpg") || endsWith(filePath, ".jpeg")) contentType = "image/jpeg";
        else if (endsWith(filePath, ".gif")) contentType = "image/gif";
        else if (endsWith(filePath, ".ico")) contentType = "image/x-icon";
        else contentType = "application/octet-stream";
    }

    // Send the response to the client
    std::ostringstream oss;
    oss << "HTTP/1.1 " << errorCode << " OK\r\n";
    oss << "Cache-Control: no-cache, private\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Content-Length: " << content.size() << "\r\n";
    oss << "\r\n";
    oss << content;

    std::string output = oss.str();
    sendToClient(clientSocket, output.c_str(), output.size());
}
