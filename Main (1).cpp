#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

using namespace std;

struct Member {
    int id;
    string name;
    string room;
    string phone;
    string status;
};

vector<Member> loadMembers(const string& filename) {
    vector<Member> members;
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "\nCould not open " << filename << ".\n";
        cout << "Make sure the file is in the same folder as the program.\n";
        return members;
    }

    string line;
    getline(file, line); // Skip the header row.

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string idText;
        Member member;

        getline(ss, idText, ',');
        getline(ss, member.name, ',');
        getline(ss, member.room, ',');
        getline(ss, member.phone, ',');
        getline(ss, member.status, ',');

        try {
            member.id = stoi(idText);
            members.push_back(member);
        } catch (const invalid_argument&) {
            cout << "Skipping a row with an invalid member ID: " << line << '\n';
        } catch (const out_of_range&) {
            cout << "Skipping a row with an out-of-range member ID: " << line << '\n';
        }
    }

    file.close();
    return members;
}

void showMembers(const vector<Member>& members) {
    cout << "\n================ MEMBER LIST ================\n";

    if (members.empty()) {
        cout << "No member records were found.\n";
        return;
    }

    cout << "ID\tName\t\tRoom\tPhone\t\tStatus\n";
    cout << "---------------------------------------------------------------\n";

    for (const Member& member : members) {
        cout << member.id << '\t'
             << member.name << "\t\t"
             << member.room << '\t'
             << member.phone << '\t'
             << member.status << '\n';
    }

    cout << "---------------------------------------------------------------\n";
    cout << "Total members: " << members.size() << '\n';
}

void showDashboard(const vector<Member>& members) {
    int activeMembers = 0;

    for (const Member& member : members) {
        if (member.status == "Active" || member.status == "active") {
            ++activeMembers;
        }
    }

    cout << "\n========================================\n";
    cout << "\t      MESS MANAGER\n";
    cout << "========================================\n";
    cout << "  Simplify your mess management.\n\n";
    cout << "  Total members : " << members.size() << '\n';
    cout << "  Active members: " << activeMembers << '\n';
    cout << "========================================\n";
}

int main() {
    const string filename = "members.csv";
    vector<Member> members = loadMembers(filename);
    int choice = 0;

    do {
        showDashboard(members);

        cout << "\n1. View Members\n";
        cout << "2. Reload Members From File\n";
        cout << "3. About the Project\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "\nInvalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = 0;
            continue;
        }

        if (choice == 1) {
            showMembers(members);
        } else if (choice == 2) {
            members = loadMembers(filename);
            cout << "\nMember data reloaded from " << filename << ".\n";
        } else if (choice == 3) {
            cout << "\n=============== ABOUT ===============\n";
            cout << "Mess Manager is a C++17 university project.\n";
            cout << "This prototype demonstrates file handling by\n";
            cout << "reading member records from members.csv.\n";
            cout << "The planned application will include member\n";
            cout << "management, meal tracking, expenses, billing,\n";
            cout << "and monthly reports.\n";
            cout << "=====================================\n";
        } else if (choice == 4) {
            cout << "\nThank you for using Mess Manager!\n";
        } else {
            cout << "\nInvalid choice. Please choose 1, 2, 3, or 4.\n";
        }

        if (choice != 4) {
            cout << "\nPress Enter to return to the menu...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }

    } while (choice != 4);

    return 0;
}
