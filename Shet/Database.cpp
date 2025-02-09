#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

struct Reservation {
    std::string movieName;
    std::string showTime;
    std::string seatNumber;
    std::string userName;
    bool isApproved;
};

class Database {
private:
    std::string filename;

public:
    Database(const std::string& file) : filename(file) {}

    bool addReservation(const Reservation& reservation) {
        std::ofstream ofs(filename, std::ios::app);
        if (!ofs) {
            std::cerr << "Error: Could not open file for writing.\n";
            return false;
        }
        ofs << reservation.movieName << "," 
            << reservation.showTime << "," 
            << reservation.seatNumber << "," 
            << reservation.userName << "," 
            << reservation.isApproved << "\n";
        return true;
    }

    std::vector<Reservation> getReservations() {
        std::vector<Reservation> reservations;
        std::ifstream ifs(filename);
        if (!ifs) {
            std::cerr << "Error: Could not open file for reading.\n";
            return reservations;
        }

        std::string line;
        while (std::getline(ifs, line)) {
            std::istringstream iss(line);
            Reservation res;
            std::string approved;
            if (std::getline(iss, res.movieName, ',') &&
                std::getline(iss, res.showTime, ',') &&
                std::getline(iss, res.seatNumber, ',') &&
                std::getline(iss, res.userName, ',') &&
                std::getline(iss, approved)) {
                res.isApproved = (approved == "1");
                reservations.push_back(res);
            }
        }
        return reservations;
    }
};