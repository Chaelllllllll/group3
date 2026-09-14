/*
    Simple Login UI (Console-based) in C++
    ---------------------------------------
    Features:
      - Register a new account (username + password)
      - Login with existing credentials
      - Password input is hidden (shows * as you type)
      - Accounts are saved to a text file (users.txt) so they persist
      - Basic input validation and a simple menu-driven interface

    Compile:
      g++ -o login_ui login_ui.cpp
    Run:
      ./login_ui
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <limits>

#if defined(_WIN32)
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;

const string USER_FILE = "users.txt";

// ---------- Utility: hidden password input ----------
#if defined(_WIN32)
string getHiddenInput() {
    string password;
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') { // backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else {
            password.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
    return password;
}
#else
string getHiddenInput() {
    string password;
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char ch;
    while (read(STDIN_FILENO, &ch, 1) == 1 && ch != '\n') {
        if (ch == 127 || ch == 8) { // backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b" << flush;
            }
        } else {
            password.push_back(ch);
            cout << '*' << flush;
        }
    }
    cout << endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return password;
}
#endif

// ---------- Simple screen helpers ----------
void printLine(char c = '-', int len = 40) {
    cout << string(len, c) << "\n";
}

void printHeader(const string& title) {
    cout << "\n";
    printLine('=');
    cout << "   " << title << "\n";
    printLine('=');
}

// ---------- User storage ----------
map<string, string> loadUsers() {
    map<string, string> users;
    ifstream file(USER_FILE);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string user, pass;
        if (getline(ss, user, '|') && getline(ss, pass)) {
            users[user] = pass;
        }
    }
    return users;
}

bool saveUser(const string& username, const string& password) {
    ofstream file(USER_FILE, ios::app);
    if (!file) return false;
    file << username << "|" << password << "\n";
    return true;
}

// ---------- Core actions ----------
void registerUser() {
    printHeader("REGISTER NEW ACCOUNT");

    map<string, string> users = loadUsers();
    string username, password, confirm;

    cout << "Choose a username: ";
    cin >> username;

    if (users.count(username)) {
        cout << "\n[!] That username is already taken. Try logging in instead.\n";
        return;
    }

    cout << "Choose a password: ";
    password = getHiddenInput();

    cout << "Confirm password: ";
    confirm = getHiddenInput();

    if (password != confirm) {
        cout << "\n[!] Passwords do not match. Registration cancelled.\n";
        return;
    }

    if (password.empty() || username.empty()) {
        cout << "\n[!] Username and password cannot be empty.\n";
        return;
    }

    if (saveUser(username, password)) {
        cout << "\n[OK] Account created successfully! You can now log in.\n";
    } else {
        cout << "\n[!] Something went wrong saving your account.\n";
    }
}

bool loginUser() {
    printHeader("LOGIN");

    map<string, string> users = loadUsers();
    string username, password;

    cout << "Username: ";
    cin >> username;

    cout << "Password: ";
    password = getHiddenInput();

    auto it = users.find(username);
    if (it != users.end() && it->second == password) {
        cout << "\n[OK] Login successful! Welcome, " << username << ".\n";
        return true;
    }

    cout << "\n[!] Invalid username or password.\n";
    return false;
}

void loggedInMenu(const string& username) {
    int choice;
    do {
        printHeader("DASHBOARD - " + username);
        cout << "1. View profile\n";
        cout << "2. Logout\n";
        cout << "Choose an option: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "\nUsername: " << username << "\n";
                cout << "Status  : Logged in\n";
                break;
            case 2:
                cout << "\nLogging out...\n";
                break;
            default:
                cout << "\n[!] Invalid option.\n";
        }
    } while (choice != 2);
}

// ---------- Main menu ----------
int main() {
    int choice;
    bool running = true;

    while (running) {
        printHeader("SIMPLE LOGIN SYSTEM (C++)");
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        cout << "Choose an option: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\n[!] Please enter a valid number.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                // We need the username again for the dashboard, so redo a light version
                map<string, string> users = loadUsers();
                string username, password;
                printHeader("LOGIN");
                cout << "Username: ";
                cin >> username;
                cout << "Password: ";
                password = getHiddenInput();

                auto it = users.find(username);
                if (it != users.end() && it->second == password) {
                    cout << "\n[OK] Login successful! Welcome, " << username << ".\n";
                    loggedInMenu(username);
                } else {
                    cout << "\n[!] Invalid username or password.\n";
                }
                break;
            }
            case 2:
                registerUser();
                break;
            case 3:
                cout << "\nGoodbye!\n";
                running = false;
                break;
            default:
                cout << "\n[!] Invalid option, please choose 1-3.\n";
        }
    }

    return 0;
}