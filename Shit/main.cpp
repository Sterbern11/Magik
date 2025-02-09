#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

// Structure to hold reservation data
struct Reservation {
    std::string movieName;
    std::string showTime;
    std::string seatNumber;
    std::string userName;
    bool isApproved;
};

// Class to handle database operations
class Database {
private:
    std::string filename;

public:
    Database(const std::string& file) : filename(file) {}

    bool addReservation(const Reservation& reservation) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open " << filename << " for writing.\n";
            return false;
        }
        file << reservation.movieName << "," 
             << reservation.showTime << "," 
             << reservation.seatNumber << "," 
             << reservation.userName << "," 
             << (reservation.isApproved ? "1" : "0") << "\n"; // Store 1 for true, 0 for false
        return true;
    }

    std::vector<Reservation> getReservations() {
        std::vector<Reservation> reservations;
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            Reservation reservation;
            std::string isApprovedStr;
            std::getline(ss, reservation.movieName, ',');
            std::getline(ss, reservation.showTime, ',');
            std::getline(ss, reservation.seatNumber, ',');
            std::getline(ss, reservation.userName, ',');
            std::getline(ss, isApprovedStr, ',');
            reservation.isApproved = (isApprovedStr == "1");
            reservations.push_back(reservation);
        }
        return reservations;
    }
};

// Function to send HTTP response
void sendResponse(SOCKET clientSocket, const std::string& content, const std::string& contentType) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += content;

    send(clientSocket, response.c_str(), response.size(), 0);
}

// Function to read an HTML file
std::string readHTMLFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "<html><body><h1>404 Not Found</h1></body></html>";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Main function
int main() {
    // Initialize Winsock
    WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: Winsock initialization failed.\n";
        return 1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error: Could not create socket.\n";
        WSACleanup();
        return 1;
    }

    // Bind the socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    serverAddr.sin_port = htons(8080); // Port number

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Could not bind socket.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error: Could not listen on socket.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 8080...\n";

    Database db("reservations.txt"); // Create a database instance

    while (true) {
        // Accept a client connection
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error: Could not accept client connection.\n";
            continue; // Continue to the next iteration
        }

        // Buffer to hold the incoming request
        char buffer[1024] = {0};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received data
            std::string request(buffer);

            // Process the request
            if (request.find("GET / ") == 0 || request.find("GET /index.html") == 0) {
                // Serve the main HTML page
                std::string htmlContent = readHTMLFile("test.html");
                sendResponse(clientSocket, htmlContent, "text/html");
            } else if (request.find("POST /add_reservation") == 0) {
                // Handle POST request to add a reservation
                std::string body = request.substr(request.find("\r\n\r\n") + 4); // Get the body of the request
                std::istringstream iss(body);
                std::string movieName, showTime, seatNumber, userName, isApprovedStr;
                bool isApproved = false; // Default to false

                // Parse the body (assuming URL-encoded format)
                std::string token;
                while (std::getline(iss, token, '&')) {
                    size_t pos = token.find('=');
                    if (pos != std::string::npos) {
                        std::string key = token.substr(0, pos);
                        std::string value = token.substr(pos + 1);
                        // Decode URL-encoded values (replace '+' with space and decode %XX)
                        std::replace(value.begin(), value.end(), '+', ' ');
                        for (size_t i = 0; i < value.length(); i++) {
                            if (value[i] == '%' && i + 2 < value.length()) {
                                std::string hex = value.substr(i + 1, 2);
                                char decodedChar = static_cast<char>(strtol(hex.c_str(), nullptr, 16));
                                value.replace(i, 3, 1, decodedChar);
                            }
                        }

                        // Debugging output to check parsed values
                        std::cout << "Parsed key: " << key << ", value: " << value << std::endl;

                        if (key == "movieName") movieName = value;
                        else if (key == "showTime") showTime = value;
                        else if (key == "seatNumber") seatNumber = value;
                        else if (key == "userName") userName = value;
                        else if (key == "isApproved") isApproved = (value == "1");
                    }
                }

                // Debugging output to check final values before creating Reservation
                std::cout << "Final values: " 
                          << "Movie Name: " << movieName << ", "
                          << "Show Time: " << showTime << ", "
                          << "Seat Number: " << seatNumber << ", "
                          << "User  Name: " << userName << ", "
                          << "Is Approved: " << isApproved << std::endl;

                // Create a Reservation object
                Reservation newReservation = {movieName, showTime, seatNumber, userName, isApproved};

                // Add the reservation to the database
                if (db.addReservation(newReservation)) {
                    std::string successMessage = "Reservation added successfully!";
                    sendResponse(clientSocket, successMessage, "text/plain");
                } else {
                    std::string errorMessage = "Failed to add reservation.";
                    sendResponse(clientSocket, errorMessage, "text/plain");
                }
            } else {
                // Handle 404 Not Found
                std::string notFoundMessage = "404 Not Found";
                sendResponse(clientSocket, notFoundMessage, "text/plain");
            }
        } else {
            std::cerr << "Error: No data received from client.\n";
        }

        // Close the client socket
        closesocket(clientSocket);
    }

    // Clean up
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}