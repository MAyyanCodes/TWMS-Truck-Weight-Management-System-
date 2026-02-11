#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <ctime>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

const int MAX_WEIGHT = 2000;
const string DATA_FILE = "truck_data.txt";
const string REPORT_FILE = "truck_report.txt";
const string CSV_FILE = "truck_export.csv";

struct Box {
    int weight;
    string description;

    Box() : weight(0), description("") {}
    Box(int w, string desc = "") : weight(w), description(desc) {}
};

struct Truck {
    int truckNumber;
    string driverName;
    string licensePlate;
    int emptyWeight;
    vector<Box> boxes;
    int totalWeight;
    bool isOverloaded;
    string timestamp;
    string destination;
    string status;

    Truck() : truckNumber(0), emptyWeight(0), totalWeight(0),
              isOverloaded(false), driverName(""), licensePlate(""),
              destination(""), status("Pending") {}

    Truck(int num, int weight, string driver, string plate, string dest)
        : truckNumber(num), emptyWeight(weight), totalWeight(0),
          isOverloaded(false), driverName(driver), licensePlate(plate),
          destination(dest), status("Pending") {
        timestamp = getCurrentTimestamp();
    }

    static string getCurrentTimestamp() {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buffer);
    }

    void calculateTotalWeight() {
        int boxesWeight = 0;
        for (const auto& box : boxes) {
            boxesWeight += box.weight;
        }
        totalWeight = emptyWeight + boxesWeight;
        isOverloaded = (totalWeight > MAX_WEIGHT);

        if (status != "Delivered" && status != "Cancelled" && status != "In Transit") {
            if (isOverloaded) {
                status = "Overloaded";
            } else if (totalWeight >= MAX_WEIGHT * 0.9) {
                status = "Near Limit";
            } else {
                status = "Ready";
            }
        }
    }

    double getLoadPercentage() const {
        return (totalWeight * 100.0) / MAX_WEIGHT;
    }

    int getRemainingCapacity() const {
        return MAX_WEIGHT - totalWeight;
    }
};

struct Statistics {
    int totalTrucks;
    int overloadedTrucks;
    int readyTrucks;
    int nearLimitTrucks;
    long long totalWeight;
    double averageWeight;
    int maxWeight;
    int minWeight;
    double averageLoadPercentage;

    Statistics() : totalTrucks(0), overloadedTrucks(0), readyTrucks(0),
                   nearLimitTrucks(0), totalWeight(0), averageWeight(0),
                   maxWeight(0), minWeight(0), averageLoadPercentage(0) {}
};

void displayHeader();
void displayMainMenu();
void displayReportsMenu();
void displaySearchMenu();
void addTrucks(vector<Truck>& trucks);
void viewAllTrucks(const vector<Truck>& trucks);
void viewDetailedTruckInfo(const vector<Truck>& trucks);
void searchTrucks(const vector<Truck>& trucks);
void searchByDriver(const vector<Truck>& trucks);
void searchByPlate(const vector<Truck>& trucks);
void searchByDestination(const vector<Truck>& trucks);
void searchByStatus(const vector<Truck>& trucks);
void updateTruckStatus(vector<Truck>& trucks);
void deleteTruck(vector<Truck>& trucks);
void sortTrucks(vector<Truck>& trucks);
void generateStatistics(const vector<Truck>& trucks);
void generateReport(const vector<Truck>& trucks);
void exportToCSV(const vector<Truck>& trucks);
void saveToFile(const vector<Truck>& trucks);
void loadFromFile(vector<Truck>& trucks);
void autoBackup(const vector<Truck>& trucks);
int getValidatedInt(const string& prompt, int min = INT_MIN, int max = INT_MAX);
string getValidatedString(const string& prompt);
void clearScreen();
void pauseScreen();
string getCurrentDateTime();
void displayProgressBar(int current, int total);
string toUpperCase(string str);

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    #endif

    vector<Truck> trucks;
    int choice;
    bool dataModified = false;

    loadFromFile(trucks);

    do {
        clearScreen();
        displayHeader();
        displayMainMenu();

        choice = getValidatedInt("Enter your choice: ", 1, 12);

        switch(choice) {
            case 1:
                addTrucks(trucks);
                dataModified = true;
                break;
            case 2:
                viewAllTrucks(trucks);
                pauseScreen();
                break;
            case 3:
                viewDetailedTruckInfo(trucks);
                pauseScreen();
                break;
            case 4:
                searchTrucks(trucks);
                break;
            case 5:
                updateTruckStatus(trucks);
                dataModified = true;
                break;
            case 6:
                deleteTruck(trucks);
                dataModified = true;
                break;
            case 7:
                sortTrucks(trucks);
                pauseScreen();
                break;
            case 8:
                generateStatistics(trucks);
                pauseScreen();
                break;
            case 9:
                generateReport(trucks);
                pauseScreen();
                break;
            case 10:
                exportToCSV(trucks);
                pauseScreen();
                break;
            case 11:
                saveToFile(trucks);
                dataModified = false;
                pauseScreen();
                break;
            case 12:
                if (dataModified) {
                    cout << "\n\t\tYou have unsaved changes. Save before exiting? (y/n): ";
                    char save;
                    cin >> save;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (save == 'y' || save == 'Y') {
                        saveToFile(trucks);
                    }
                }
                cout << "\n\n\t\t╔════════════════════════════════════════════════╗\n";
                cout << "\t\t║   Thank you for using TWMS Professional!       ║\n";
                cout << "\t\t║   Session ended: " << getCurrentDateTime().substr(11) << "          ║\n";
                cout << "\t\t╚════════════════════════════════════════════════╝\n\n";
                break;
            default:
                cout << "\n\t\tInvalid choice! Please try again.\n";
                pauseScreen();
        }

        if (dataModified && choice != 11 && choice != 12) {
            autoBackup(trucks);
        }

    } while(choice != 12);

    return 0;
}

string getCurrentDateTime() {
    time_t now = time(0);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buffer);
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pauseScreen() {
    cout << "\n\tPress Enter to continue...";
    if (cin.peek() == '\n') cin.ignore();
    cin.get();
}

string toUpperCase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

int getValidatedInt(const string& prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= min && value <= max) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            } else {
                cout << "\tError: Input must be between " << min << " and " << max << ".\n";
            }
        } else {
            cout << "\tError: Invalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

string getValidatedString(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (!input.empty()) {
            return input;
        }
        cout << "\tError: Input cannot be empty.\n";
    }
}

void displayProgressBar(int current, int total) {
    int barWidth = 50;
    float progress = (float)current / total;
    int pos = barWidth * progress;

    cout << "\t[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) cout << "█";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << int(progress * 100.0) << " %\r";
    cout.flush();

    if (current == total) cout << endl;
}

void displayHeader() {
    cout << "\n\t╔════════════════════════════════════════════════════════════════════╗\n";
    cout << "\t║          TRUCK WEIGHT MANAGEMENT SYSTEM - PROFESSIONAL             ║\n";
    cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
    cout << "\t║  Current Session: " << left << setw(48) << getCurrentDateTime() << "║\n";
    cout << "\t╚════════════════════════════════════════════════════════════════════╝\n";
}

void displayMainMenu() {
    cout << "\n\t╔════════════════════════════════════════════════════════════════════╗\n";
    cout << "\t║                          MAIN MENU                                 ║\n";
    cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
    cout << "\t║  1.  Add New Trucks                                                ║\n";
    cout << "\t║  2.  View All Trucks (Summary)                                     ║\n";
    cout << "\t║  3.  View Detailed Truck Information                               ║\n";
    cout << "\t║  4.  Search Trucks                                                 ║\n";
    cout << "\t║  5.  Update Truck Status                                           ║\n";
    cout << "\t║  6.  Delete Truck                                                  ║\n";
    cout << "\t║  7.  Sort Trucks                                                   ║\n";
    cout << "\t║  8.  Generate Statistics                                           ║\n";
    cout << "\t║  9.  Generate Report (Text File)                                   ║\n";
    cout << "\t║  10. Export to CSV                                                 ║\n";
    cout << "\t║  11. Save Data                                                     ║\n";
    cout << "\t║  12. Exit System                                                   ║\n";
    cout << "\t╚════════════════════════════════════════════════════════════════════╝\n";
}

void displaySearchMenu() {
    cout << "\n\t╔════════════════════════════════════════════════════════════════════╗\n";
    cout << "\t║                       SEARCH OPTIONS                               ║\n";
    cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
    cout << "\t║  1. Search by Driver Name                                          ║\n";
    cout << "\t║  2. Search by License Plate                                        ║\n";
    cout << "\t║  3. Search by Destination                                          ║\n";
    cout << "\t║  4. Filter by Status                                               ║\n";
    cout << "\t║  5. Back to Main Menu                                              ║\n";
    cout << "\t╚════════════════════════════════════════════════════════════════════╝\n";
}

void addTrucks(vector<Truck>& trucks) {
    clearScreen();
    displayHeader();

    cout << "\n\t╔════════════════════════════════════════════════════════════════════╗\n";
    cout << "\t║                      ADD NEW TRUCKS                                ║\n";
    cout << "\t╚════════════════════════════════════════════════════════════════════╝\n";

    int numTrucks = getValidatedInt("\n\tEnter number of trucks to add: ", 1, 100);

    for (int i = 0; i < numTrucks; i++) {
        cout << "\n\t" << string(68, '─') << "\n";
        cout << "\t  TRUCK #" << (trucks.size() + 1) << " - Registration\n";
        cout << "\t" << string(68, '─') << "\n";

        string driver = getValidatedString("\tDriver Name: ");
        string plate = getValidatedString("\tLicense Plate: ");
        string destination = getValidatedString("\tDestination: ");
        int emptyWeight = getValidatedInt("\tEmpty Truck Weight (kg): ", 0, 10000);

        Truck newTruck(trucks.size() + 1, emptyWeight, driver, plate, destination);

        int numBoxes = getValidatedInt("\tNumber of Boxes: ", 0, 1000);

        for (int j = 0; j < numBoxes; j++) {
            cout << "\n\t  Box #" << (j + 1) << ":\n";
            int boxWeight = getValidatedInt("\t    Weight (kg): ", 0, 5000);
            string boxDesc = getValidatedString("\t    Description: ");
            newTruck.boxes.push_back(Box(boxWeight, boxDesc));

            displayProgressBar(j + 1, numBoxes);
        }

        newTruck.calculateTotalWeight();

        cout << "\n\n\t" << string(68, '═') << "\n";
        cout << "\t  TRUCK SUMMARY\n";
        cout << "\t" << string(68, '─') << "\n";
        cout << "\t  Driver: " << newTruck.driverName << "\n";
        cout << "\t  License: " << newTruck.licensePlate << "\n";
        cout << "\t  Destination: " << newTruck.destination << "\n";
        cout << "\t  Total Weight: " << newTruck.totalWeight << " kg\n";
        cout << "\t  Load Percentage: " << fixed << setprecision(1)
              << newTruck.getLoadPercentage() << "%\n";
        cout << "\t  Status: " << newTruck.status << "\n";

        if (newTruck.isOverloaded) {
            cout << "\t  ⚠ WARNING: OVERLOADED BY "
                 << (newTruck.totalWeight - MAX_WEIGHT) << " kg!\n";
        } else {
            cout << "\t  ✓ Remaining Capacity: "
                 << newTruck.getRemainingCapacity() << " kg\n";
        }
        cout << "\t" << string(68, '═') << "\n";

        trucks.push_back(newTruck);
    }

    cout << "\n\t  ✓ Successfully added " << numTrucks << " truck(s)!\n";
    pauseScreen();
}

void viewAllTrucks(const vector<Truck>& trucks) {
    clearScreen();
    displayHeader();

    if (trucks.empty()) {
        cout << "\n\t  ⚠ No trucks in the system!\n";
        return;
    }

    cout << "\n\t" << string(130, '═') << "\n";
    cout << "\t" << left << setw(6) << "ID"
         << setw(20) << "Driver"
         << setw(15) << "License"
         << setw(18) << "Destination"
         << setw(10) << "Weight"
         << setw(10) << "Load %"
         << setw(15) << "Status"
         << setw(20) << "Timestamp" << "\n";
    cout << "\t" << string(130, '─') << "\n";

    for (size_t i = 0; i < trucks.size(); i++) {
        string dName = trucks[i].driverName.length() > 18 ? trucks[i].driverName.substr(0,15) + "..." : trucks[i].driverName;
        string dest = trucks[i].destination.length() > 16 ? trucks[i].destination.substr(0,13) + "..." : trucks[i].destination;

        cout << "\t" << left << setw(6) << trucks[i].truckNumber
             << setw(20) << dName
             << setw(15) << trucks[i].licensePlate
             << setw(18) << dest
             << setw(10) << trucks[i].totalWeight
             << setw(10) << fixed << setprecision(1) << trucks[i].getLoadPercentage()
             << setw(15) << trucks[i].status
             << setw(20) << trucks[i].timestamp.substr(0, 19) << "\n";
    }

    cout << "\t" << string(130, '═') << "\n";
    cout << "\t  Total Trucks: " << trucks.size() << "\n";
}

void viewDetailedTruckInfo(const vector<Truck>& trucks) {
    clearScreen();
    displayHeader();

    if (trucks.empty()) {
        cout << "\n\t  ⚠ No trucks in the system!\n";
        return;
    }

    viewAllTrucks(trucks);
    int truckId = getValidatedInt("\n\tEnter Truck ID to view details: ", 1, 9999);

    for (size_t i = 0; i < trucks.size(); i++) {
        if (trucks[i].truckNumber == truckId) {
            clearScreen();
            displayHeader();

            cout << "\n\t╔════════════════════════════════════════════════════════════════════╗\n";
            cout << "\t║  TRUCK #" << left << setw(60) << trucks[i].truckNumber << "║\n";
            cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
            cout << "\t║  Driver Name    : " << left << setw(50) << trucks[i].driverName << "║\n";
            cout << "\t║  License Plate  : " << left << setw(50) << trucks[i].licensePlate << "║\n";
            cout << "\t║  Destination    : " << left << setw(50) << trucks[i].destination << "║\n";
            cout << "\t║  Added On       : " << left << setw(50) << trucks[i].timestamp << "║\n";
            cout << "\t║  Status         : " << left << setw(50) << trucks[i].status << "║\n";
            cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
            cout << "\t║  Empty Weight   : " << left << setw(40) << (to_string(trucks[i].emptyWeight) + " kg") << "         ║\n";
            cout << "\t║  Number of Boxes: " << left << setw(40) << trucks[i].boxes.size() << "         ║\n";
            cout << "\t╚════════════════════════════════════════════════════════════════════╝\n";

            if (!trucks[i].boxes.empty()) {
                cout << "\n\t  Box Details:\n";
                cout << "\t  " << string(66, '─') << "\n";
                cout << "\t  " << left << setw(8) << "Box #"
                      << setw(15) << "Weight (kg)"
                      << setw(43) << "Description" << "\n";
                cout << "\t  " << string(66, '─') << "\n";

                int boxTotal = 0;
                for (size_t j = 0; j < trucks[i].boxes.size(); j++) {
                    cout << "\t  " << left << setw(8) << (j + 1)
                          << setw(15) << trucks[i].boxes[j].weight
                          << setw(43) << trucks[i].boxes[j].description.substr(0, 41) << "\n";
                    boxTotal += trucks[i].boxes[j].weight;
                }
                cout << "\t  " << string(66, '─') << "\n";
                cout << "\t  Total Cargo Weight: " << boxTotal << " kg\n";
            }

            cout << "\n\t╔════════════════════════════════════════════════════════════════════╗\n";
            cout << "\t║  WEIGHT ANALYSIS                                                   ║\n";
            cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
            cout << "\t║  Total Weight      : " << left << setw(30) << (to_string(trucks[i].totalWeight) + " kg") << "                  ║\n";
            cout << "\t║  Maximum Allowed   : " << left << setw(30) << (to_string(MAX_WEIGHT) + " kg") << "                  ║\n";
            cout << "\t║  Load Percentage   : " << left << setw(30) << (to_string((int)trucks[i].getLoadPercentage()) + "%") << "                  ║\n";

            if (trucks[i].isOverloaded) {
                cout << "\t║  ⚠ OVERWEIGHT BY   : " << left << setw(30) << (to_string(trucks[i].totalWeight - MAX_WEIGHT) + " kg") << "                  ║\n";
            } else {
                cout << "\t║  ✓ Available Space : " << left << setw(30) << (to_string(trucks[i].getRemainingCapacity()) + " kg") << "                  ║\n";
            }
            cout << "\t╚════════════════════════════════════════════════════════════════════╝\n";

            return;
        }
    }

    cout << "\n\t  ⚠ Truck not found!\n";
}

void searchTrucks(const vector<Truck>& trucks) {
    int choice;
    do {
        clearScreen();
        displayHeader();
        displaySearchMenu();
        choice = getValidatedInt("Enter your choice: ", 1, 5);
        switch(choice) {
            case 1: searchByDriver(trucks); pauseScreen(); break;
            case 2: searchByPlate(trucks); pauseScreen(); break;
            case 3: searchByDestination(trucks); pauseScreen(); break;
            case 4: searchByStatus(trucks); pauseScreen(); break;
            case 5: break;
        }
    } while(choice != 5);
}

void searchByDriver(const vector<Truck>& trucks) {
    string searchTerm = getValidatedString("\n\tEnter driver name to search: ");
    searchTerm = toUpperCase(searchTerm);
    cout << "\n\t  Search Results:\n";
    cout << "\t  " << string(68, '─') << "\n";
    bool found = false;
    for (const auto& truck : trucks) {
        if (toUpperCase(truck.driverName).find(searchTerm) != string::npos) {
            cout << "\t  ID: " << truck.truckNumber << " | Driver: " << truck.driverName
                 << " | Plate: " << truck.licensePlate << " | Status: " << truck.status << "\n";
            found = true;
        }
    }
    if (!found) cout << "\t  No matches found.\n";
    cout << "\t  " << string(68, '─') << "\n";
}

void searchByPlate(const vector<Truck>& trucks) {
    string searchTerm = getValidatedString("\n\tEnter license plate to search: ");
    searchTerm = toUpperCase(searchTerm);
    cout << "\n\t  Search Results:\n";
    cout << "\t  " << string(68, '─') << "\n";
    bool found = false;
    for (const auto& truck : trucks) {
        if (toUpperCase(truck.licensePlate).find(searchTerm) != string::npos) {
             cout << "\t  ID: " << truck.truckNumber << " | Driver: " << truck.driverName
                 << " | Plate: " << truck.licensePlate << " | Dest: " << truck.destination << "\n";
            found = true;
        }
    }
    if (!found) cout << "\t  No matches found.\n";
    cout << "\t  " << string(68, '─') << "\n";
}

void searchByDestination(const vector<Truck>& trucks) {
    string searchTerm = getValidatedString("\n\tEnter destination to search: ");
    searchTerm = toUpperCase(searchTerm);
    cout << "\n\t  Search Results:\n";
    cout << "\t  " << string(68, '─') << "\n";
    bool found = false;
    for (const auto& truck : trucks) {
        if (toUpperCase(truck.destination).find(searchTerm) != string::npos) {
             cout << "\t  ID: " << truck.truckNumber << " | Dest: " << truck.destination
                 << " | Weight: " << truck.totalWeight << "kg\n";
            found = true;
        }
    }
    if (!found) cout << "\t  No matches found.\n";
    cout << "\t  " << string(68, '─') << "\n";
}

void searchByStatus(const vector<Truck>& trucks) {
    cout << "\n\t  Status Options: 1. Ready, 2. Near Limit, 3. Overloaded, 4. Pending\n";
    int choice = getValidatedInt("\n\tSelect status: ", 1, 4);
    string status = (choice==1) ? "Ready" : (choice==2) ? "Near Limit" : (choice==3) ? "Overloaded" : "Pending";

    cout << "\n\t  Trucks with status '" << status << "':\n";
    cout << "\t  " << string(68, '─') << "\n";
    bool found = false;
    for (const auto& truck : trucks) {
        if (truck.status == status) {
             cout << "\t  ID: " << truck.truckNumber << " | Driver: " << truck.driverName
                 << " | Weight: " << truck.totalWeight << " kg\n";
            found = true;
        }
    }
    if (!found) cout << "\t  No trucks with this status.\n";
    cout << "\t  " << string(68, '─') << "\n";
}

void updateTruckStatus(vector<Truck>& trucks) {
    clearScreen();
    displayHeader();
    if (trucks.empty()) { cout << "\n\t  ⚠ No trucks!\n"; pauseScreen(); return; }

    viewAllTrucks(trucks);
    int truckId = getValidatedInt("\n\tEnter Truck ID to update: ", 1, 9999);

    for (auto& truck : trucks) {
        if (truck.truckNumber == truckId) {
            cout << "\n\t  Current Status: " << truck.status << "\n";
            cout << "\n\t  New Status Options:\n\t  1. Pending\n\t  2. In Transit\n\t  3. Delivered\n\t  4. Cancelled\n";
            int choice = getValidatedInt("\n\tSelect new status: ", 1, 4);
            switch(choice) {
                case 1: truck.status = "Pending"; break;
                case 2: truck.status = "In Transit"; break;
                case 3: truck.status = "Delivered"; break;
                case 4: truck.status = "Cancelled"; break;
            }
            cout << "\n\t  ✓ Status updated successfully!\n";
            pauseScreen();
            return;
        }
    }
    cout << "\n\t  ⚠ Truck not found!\n";
    pauseScreen();
}

void deleteTruck(vector<Truck>& trucks) {
    clearScreen();
    displayHeader();
    if (trucks.empty()) { cout << "\n\t  ⚠ No trucks!\n"; pauseScreen(); return; }

    viewAllTrucks(trucks);
    int truckId = getValidatedInt("\n\tEnter Truck ID to delete: ", 1, 9999);

    for (size_t i = 0; i < trucks.size(); i++) {
        if (trucks[i].truckNumber == truckId) {
            cout << "\n\t  Delete Truck #" << trucks[i].truckNumber << " (" << trucks[i].driverName << ")?\n";
            cout << "\t  Confirm? (y/n): ";
            char confirm; cin >> confirm;
            if (confirm == 'y' || confirm == 'Y') {
                trucks.erase(trucks.begin() + i);
                for (size_t j = 0; j < trucks.size(); j++) trucks[j].truckNumber = j + 1;
                cout << "\n\t  ✓ Truck deleted successfully!\n";
            } else {
                cout << "\n\t  Deletion cancelled.\n";
            }
            pauseScreen();
            return;
        }
    }
    cout << "\n\t  ⚠ Truck not found!\n";
    pauseScreen();
}

void sortTrucks(vector<Truck>& trucks) {
    if (trucks.empty()) { cout << "\n\t  ⚠ No trucks to sort!\n"; return; }
    cout << "\n\t  Sort By: 1. Weight (Asc), 2. Weight (Desc), 3. Driver, 4. Timestamp\n";
    int choice = getValidatedInt("\n\tSelect sort option: ", 1, 4);

    switch(choice) {
        case 1: sort(trucks.begin(), trucks.end(), [](const Truck& a, const Truck& b) { return a.totalWeight < b.totalWeight; }); break;
        case 2: sort(trucks.begin(), trucks.end(), [](const Truck& a, const Truck& b) { return a.totalWeight > b.totalWeight; }); break;
        case 3: sort(trucks.begin(), trucks.end(), [](const Truck& a, const Truck& b) { return a.driverName < b.driverName; }); break;
        case 4: sort(trucks.begin(), trucks.end(), [](const Truck& a, const Truck& b) { return a.timestamp < b.timestamp; }); break;
    }
    for (size_t i = 0; i < trucks.size(); i++) trucks[i].truckNumber = i + 1;
    cout << "\n\t  ✓ Trucks sorted!\n";
    viewAllTrucks(trucks);
}

void generateStatistics(const vector<Truck>& trucks) {
    clearScreen();
    displayHeader();
    if (trucks.empty()) { cout << "\n\t  ⚠ No data available!\n"; return; }

    Statistics stats;
    stats.totalTrucks = trucks.size();

    for (const auto& truck : trucks) {
        stats.totalWeight += truck.totalWeight;
        if (truck.status == "Overloaded") stats.overloadedTrucks++;
        else if (truck.status == "Ready") stats.readyTrucks++;
        else if (truck.status == "Near Limit") stats.nearLimitTrucks++;

        if (stats.maxWeight == 0 || truck.totalWeight > stats.maxWeight) stats.maxWeight = truck.totalWeight;

        stats.averageLoadPercentage += truck.getLoadPercentage();
    }

    stats.minWeight = trucks[0].totalWeight;
    for(const auto& t : trucks) if(t.totalWeight < stats.minWeight) stats.minWeight = t.totalWeight;

    stats.averageWeight = (double)stats.totalWeight / stats.totalTrucks;
    stats.averageLoadPercentage /= stats.totalTrucks;

    cout << "\n\t╔════════════════════════════════════════════════════════════════════╗\n";
    cout << "\t║                    STATISTICAL ANALYSIS                            ║\n";
    cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
    cout << "\t║  Total Trucks           : " << left << setw(44) << stats.totalTrucks << "║\n";
    cout << "\t║  Ready for Dispatch     : " << left << setw(44) << stats.readyTrucks << "║\n";
    cout << "\t║  Overloaded             : " << left << setw(44) << stats.overloadedTrucks << "║\n";
    cout << "\t╠════════════════════════════════════════════════════════════════════╣\n";
    cout << "\t║  Total Weight           : " << left << setw(34) << (to_string(stats.totalWeight) + " kg") << "        ║\n";
    cout << "\t║  Average Weight         : " << left << setw(34) << (to_string((int)stats.averageWeight) + " kg") << "        ║\n";
    cout << "\t║  Maximum Weight         : " << left << setw(34) << (to_string(stats.maxWeight) + " kg") << "        ║\n";
    cout << "\t║  Minimum Weight         : " << left << setw(34) << (to_string(stats.minWeight) + " kg") << "        ║\n";
    cout << "\t║  Avg Load Percentage    : " << left << setw(34) << (to_string((int)stats.averageLoadPercentage) + "%") << "        ║\n";
    cout << "\t╚════════════════════════════════════════════════════════════════════╝\n";
}

void generateReport(const vector<Truck>& trucks) {
    if (trucks.empty()) { cout << "\n\t  ⚠ No data available!\n"; return; }

    ofstream report(REPORT_FILE);
    if (!report) { cout << "\n\t  ⚠ Error creating report file!\n"; return; }

    report << "TRUCK WEIGHT MANAGEMENT SYSTEM - REPORT\n";
    report << "Generated: " << getCurrentDateTime() << "\n";
    report << "---------------------------------------\n\n";

    for (const auto& truck : trucks) {
        report << "Truck #" << truck.truckNumber << "\n";
        report << "Driver: " << truck.driverName << "\n";
        report << "Plate: " << truck.licensePlate << "\n";
        report << "Destination: " << truck.destination << "\n";
        report << "Total Weight: " << truck.totalWeight << " kg\n";
        report << "Status: " << truck.status << "\n";
        report << "Timestamp: " << truck.timestamp << "\n";
        report << "Boxes: " << truck.boxes.size() << "\n\n";
    }

    cout << "\n\t  ✓ Report generated: " << REPORT_FILE << "\n";
}

void exportToCSV(const vector<Truck>& trucks) {
    if (trucks.empty()) { cout << "\n\t  ⚠ No data available!\n"; return; }

    ofstream file(CSV_FILE);
    if (!file) { cout << "\n\t  ⚠ Error creating CSV file!\n"; return; }

    file << "ID,Driver,Plate,Destination,EmptyWeight,TotalWeight,Status,Timestamp,BoxCount\n";

    for (const auto& truck : trucks) {
        file << truck.truckNumber << ","
             << truck.driverName << ","
             << truck.licensePlate << ","
             << truck.destination << ","
             << truck.emptyWeight << ","
             << truck.totalWeight << ","
             << truck.status << ","
             << truck.timestamp << ","
             << truck.boxes.size() << "\n";
    }

    cout << "\n\t  ✓ Exported to: " << CSV_FILE << "\n";
}

void saveToFile(const vector<Truck>& trucks) {
    ofstream file(DATA_FILE);
    if (!file) {
        cout << "\n\t  ⚠ Error saving data!\n";
        return;
    }

    for (const auto& t : trucks) {
        file << t.truckNumber << "\n";
        file << t.driverName << "\n";
        file << t.licensePlate << "\n";
        file << t.destination << "\n";
        file << t.emptyWeight << "\n";
        file << t.status << "\n";
        file << t.timestamp << "\n";

        file << t.boxes.size() << "\n";
        for (const auto& b : t.boxes) {
            file << b.weight << "\n";
            file << b.description << "\n";
        }
    }
    cout << "\n\t  ✓ Data saved successfully.\n";
}

void loadFromFile(vector<Truck>& trucks) {
    ifstream file(DATA_FILE);
    if (!file) return;

    trucks.clear();
    Truck t;
    while (file >> t.truckNumber) {
        file.ignore();
        getline(file, t.driverName);
        getline(file, t.licensePlate);
        getline(file, t.destination);
        file >> t.emptyWeight;
        file.ignore();
        getline(file, t.status);
        getline(file, t.timestamp);

        int numBoxes;
        file >> numBoxes;
        file.ignore();

        t.boxes.clear();
        for (int i = 0; i < numBoxes; i++) {
            Box b;
            file >> b.weight;
            file.ignore();
            getline(file, b.description);
            t.boxes.push_back(b);
        }
        t.calculateTotalWeight();
        trucks.push_back(t);
    }
}

void autoBackup(const vector<Truck>& trucks) {
    ofstream file("backup.txt");
    if (!file) return;

    for (const auto& t : trucks) {
        file << t.truckNumber << "|" << t.driverName << "|" << t.totalWeight << "\n";
    }
}