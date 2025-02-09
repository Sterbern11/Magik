#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h> // Windows-specific socket library
#include <ws2tcpip.h>
#include "Database.h" // Include the Database header

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

// Function to read the content of an HTML file
std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return ""; // Return empty string if file can't be opened
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

// Function to send an HTTP response
void sendResponse(SOCKET clientSocket, const std::string& content, const std::string& contentType, int statusCode = 200) {
    std::string statusText = (statusCode == 200) ? "OK" : "Bad Request";
    std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + statusText + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += content;

    send(clientSocket, response.c_str(), response.size(), 0);
}

// HTML content for the reservation system
const std::string htmlContent = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Movie Reservation</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #1a1a1a; /* Dark background */
            color: #ffffff; /* White text */
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center; /* Center horizontally */
            align-items: center; /* Center vertically */
            height: 100vh; /* Full viewport height */
            flex-direction: column; /* Stack items vertically */
        }
        .container {
            max-width: 400px;
            background: #2a2a2a; /* Darker container */
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.5);
            margin-bottom: 20px; /* Space between form and seating chart */
        }
        h2 {
            text-align: center;
            color: #ff6600; /* Orange color for the title */
        }
        label {
            display: block;
            margin: 10px 0 5px;
        }
        input[type="text"], select {
            width: 100%;
            padding: 10px;
            margin-bottom: 15px;
            border: 1px solid #ff6600; /* Orange border */
            border-radius: 4px;
            background-color: #333; /* Dark input background */
            color: #ffffff; /* White text */
        }
        button {
            width: 100%;
            padding: 10px;
            background-color: #ff6600; /* Orange button */
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
        }
        button:hover {
            background-color: #e65c00; /* Darker orange on hover */
        }
        .seating-chart {
            display: grid;
            grid-template-columns: repeat(5, 1fr);
            gap: 10px;
            margin-top: 20px;
        }
        .seat {
            padding: 15px;
            background-color: #444;
            border: 1px solid #ff6600;
            border-radius: 4px;
            text-align: center;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        .seat:hover {
                    background-color: #ff6600; /* Highlight on hover */
        }
        .reserved {
            background-color: #c00; /* Red for reserved seats */
            cursor: not-allowed;
        }
    </style>
</head>
<body>

<div class="container">
    <h2>Movie Reservation</h2>
    <form id="reservationForm">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" required>

        <label for="movie">Select Movie:</label>
        <select id="movie" name="movie" required>
            <option value="">--Select a Movie--</option>
            <option value="movie1">Movie 1</option>
            <option value="movie2">Movie 2</option>
            <option value="movie3">Movie 3</option>
        </select>

        <label for="seats">Selected Seats:</label>
        <input type="text" id="seats" name="seats" placeholder="e.g., A1, A2" readonly>

        <label for="time">Select Time:</label>
        <select id="time" name="time" required>
            <option value="">--Select a Time--</option>
            <option value="10:00 AM">10:00 AM</option>
            <option value="1:00 PM">1:00 PM</option>
            <option value="4:00 PM">4:00 PM</option>
            <option value="7:00 PM">7:00 PM</option>
        </select>

        <button type="submit">Reserve</button>
    </form>
</div>

<div class="seating-chart">
    <!-- Generate a 5x5 seating chart -->
    <div class="seat" data-seat="1-1">1-1</div>
    <div class="seat" data-seat="1-2">1-2</div>
    <div class="seat" data-seat="1-3">1-3</div>
    <div class="seat" data-seat="1-4">1-4</div>
    <div class="seat" data-seat="1-5">1-5</div>
    <div class="seat" data-seat="2-1">2-1</div>
    <div class="seat" data-seat="2-2">2-2</div>
    <div class="seat" data-seat="2-3">2-3</div>
    <div class="seat" data-seat="2-4">2-4</div>
    <div class="seat" data-seat="2-5">2-5</div>
    <div class="seat" data-seat="3-1">3-1</div>
    <div class="seat" data-seat="3-2">3-2</div>
    <div class="seat" data-seat="3-3">3-3</div>
    <div class="seat" data-seat="3-4">3-4</div>
    <div class="seat" data-seat="3-5">3-5</div>
    <div class="seat" data-seat="4-1">4-1</div>
    <div class="seat" data-seat="4-2">4-2</div>
    <div class="seat" data-seat="4-3">4-3</div>
    <div class="seat" data-seat="4-4">4-4</div>
    <div class="seat" data-seat="4-5">4-5</div>
    <div class="seat" data-seat="5-1">5-1</div>
    <div class="seat" data-seat="5-2">5-2</div>
    <div class="seat" data-seat="5-3">5-3</div>
    <div class="seat" data-seat="5-4">5-4</div>
    <div class="seat" data-seat="5-5">5-5</div>
</div>

<script>
    const seats = document.querySelectorAll('.seat');
    const selectedSeats = [];

    seats.forEach(seat => {
        seat.addEventListener('click', function() {
            const seatNumber = this.getAttribute('data-seat');

            // Check if the seat is already selected
            if (selectedSeats.includes(seatNumber)) {
                // Deselect the seat
                this.classList.remove('reserved');
                const index = selectedSeats.indexOf(seatNumber);
                if (index > -1) {
                    selectedSeats.splice(index, 1);
                }
            } else {
                               // Select the seat
                this.classList.add('reserved');
                selectedSeats.push(seatNumber);
            }

            // Update the input field with selected seats
            document.getElementById('seats').value = selectedSeats.join(', ');
        });
    });

    document.getElementById('reservationForm').addEventListener('submit', function(event) {
        event.preventDefault(); // Prevent the default form submission

        const username = document.getElementById('username').value;
        const movie = document.getElementById('movie').value;
        const seatsSelected = document.getElementById('seats').value;
        const time = document.getElementById('time').value;

        // Here you can handle the form data, e.g., send it to a server
        console.log(`Username: ${username}, Movie: ${movie}, Seats: ${seatsSelected}, Time: ${time}`);

        // Optionally, show a confirmation message
        alert(`Reservation made for ${username}:\nMovie: ${movie}\nSeats: ${seatsSelected}\nTime: ${time}`);
    });
</script>

</body>
</html>
)";

int main() {
    // Initialize the database
    Database db("reservations.txt");

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: Winsock initialization failed.\n";
        return 1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error: Could not create socket.\n";
        WSACleanup();
        return 1;
    }

    // Set up the sockaddr_in structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    // Bind the socket
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Could not bind socket.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error: Could not listen on socket.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server running at http://localhost:8080\n";

    while (true) {
        // Accept a client connection
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error: Could not accept connection.\n";
            continue;
        }

        // Receive the request
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the buffer
            std::string request(buffer);

            std::string content, contentType;
            if (request.find("GET /") != std::string::npos) {
                // Serve the embedded HTML content for the root path
                content = htmlContent;
                contentType = "text/html";
            } else if (request.find("GET /reservations") != std::string::npos) {
                // Fetch all reservations
                auto reservations = db.getReservations();
                content = "[";
                for (const auto& res : reservations) {
                    content += "{\"movieName\":\"" + res.movieName + "\",\"showTime\":\"" + res.showTime +
                               "\",\"seatNumber\":\"" + res.seatNumber + "\",\"userName\":\"" + res.userName +
                               "\",\"isApproved\":" + (res.isApproved ? "true" : "false") + "},";
                }
                if (content.size() > 1) content.pop_back(); // Remove trailing comma
                content += "]";
                contentType = "application/json";
            } else if (request.find("POST /add_reservation") != std::string::npos) {
                // Example to handle adding reservations (parse data from request)
                Reservation newRes = {"Movie A", "7:00 PM", "A1", "John Doe", false}; // Example data
                bool success = db.addReservation(newRes);
                content = success ? "{\"status\":\"success\"}" : "{\"status\":\"failure\"}";
                contentType = "application/json";
            } else {
                            // Serve static HTML files for other routes
                std::string filePath = "./www" + request.substr(4, request.find(' ', 4) - 4); // Extract file path
                if (filePath == "./www/") filePath += "index.html"; // Default to index.html for root path

                content = readFile(filePath);
                if (content.empty()) {
                    content = "404 Not Found";
                    contentType = "text/plain";
                } else {
                    contentType = "text/html";
                }
            }

            // Send the response back to the client
            sendResponse(clientSocket, content, contentType);
        }

        // Close the client connection
        closesocket(clientSocket);
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}