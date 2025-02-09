#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

// Struct to represent a movie reservation
struct Reservation {
    std::string movieName;
    std::string showTime;
    std::string seatNumber;
    std::string userName;
    bool isApproved;
};

// Class to manage the database
class Database {
private:
    std::string fileName;

public:
    // Constructor
    Database(const std::string& dbFileName);

    // Methods for managing reservations
    bool addReservation(const Reservation& reservation);
    std::vector<Reservation> getReservations();
    bool updateReservation(const std::string& seatNumber, bool approvalStatus);
    bool deleteReservation(const std::string& seatNumber);

    // Helper methods
    void clearDatabase(); // Clears all data in the database
    bool saveToFile(const std::vector<Reservation>& reservations);
    std::vector<Reservation> loadFromFile();
};

#endif // DATABASE_H
