#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using std::string;
using std::cin;
using std::cout;
using std::endl;

const int MAX_USERS = 100000;
const int MAX_TRAINS = 10000;

// Simple hash map implementation
template<typename K, typename V, int SIZE>
class HashMap {
private:
    struct Node {
        K key;
        V value;
        bool used;
        Node() : used(false) {}
    };
    Node data[SIZE];

    int hash(const string& s) {
        unsigned int h = 0;
        for (char c : s) {
            h = h * 131 + c;
        }
        return h % SIZE;
    }

public:
    bool insert(const K& key, const V& value) {
        int pos = hash(key);
        for (int i = 0; i < SIZE; i++) {
            int idx = (pos + i) % SIZE;
            if (!data[idx].used) {
                data[idx].key = key;
                data[idx].value = value;
                data[idx].used = true;
                return true;
            }
            if (data[idx].key == key) {
                return false; // Already exists
            }
        }
        return false;
    }

    bool find(const K& key, V& value) {
        int pos = hash(key);
        for (int i = 0; i < SIZE; i++) {
            int idx = (pos + i) % SIZE;
            if (!data[idx].used) {
                return false;
            }
            if (data[idx].key == key) {
                value = data[idx].value;
                return true;
            }
        }
        return false;
    }

    bool update(const K& key, const V& value) {
        int pos = hash(key);
        for (int i = 0; i < SIZE; i++) {
            int idx = (pos + i) % SIZE;
            if (!data[idx].used) {
                return false;
            }
            if (data[idx].key == key) {
                data[idx].value = value;
                return true;
            }
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < SIZE; i++) {
            data[i].used = false;
        }
    }
};

struct User {
    char username[21];
    char password[31];
    char name[31];
    char mailAddr[31];
    int privilege;
    bool exists;

    User() : privilege(0), exists(false) {
        username[0] = password[0] = name[0] = mailAddr[0] = '\0';
    }
};

struct Train {
    char trainID[21];
    int stationNum;
    char stations[100][31];
    int seatNum;
    int prices[100];
    int startTime; // minutes from 00:00
    int travelTimes[100];
    int stopoverTimes[100];
    int saleDate[2]; // days from 06-01
    char type;
    bool released;
    bool exists;

    Train() : stationNum(0), seatNum(0), startTime(0), type('G'), released(false), exists(false) {
        trainID[0] = '\0';
        saleDate[0] = saleDate[1] = 0;
    }
};

// Global data structures
HashMap<string, int, MAX_USERS> userMap;
User users[MAX_USERS];
int userCount = 0;

HashMap<string, int, MAX_TRAINS> trainMap;
Train trains[MAX_TRAINS];
int trainCount = 0;

HashMap<string, bool, MAX_USERS> loggedIn;

// Helper functions
int parseTime(const string& time) {
    int h = (time[0] - '0') * 10 + (time[1] - '0');
    int m = (time[3] - '0') * 10 + (time[4] - '0');
    return h * 60 + m;
}

int parseDate(const string& date) {
    int m = (date[0] - '0') * 10 + (date[1] - '0');
    int d = (date[3] - '0') * 10 + (date[4] - '0');
    // Calculate days from 06-01
    int days = 0;
    if (m == 6) days = d - 1;
    else if (m == 7) days = 30 + d - 1;
    else if (m == 8) days = 61 + d - 1;
    return days;
}

string formatTime(int minutes, int day) {
    int h = (minutes / 60) % 24;
    int m = minutes % 60;
    int actualDay = day + (minutes / 1440);

    int month, date;
    if (actualDay < 30) {
        month = 6;
        date = actualDay + 1;
    } else if (actualDay < 61) {
        month = 7;
        date = actualDay - 30 + 1;
    } else {
        month = 8;
        date = actualDay - 61 + 1;
    }

    char buffer[20];
    sprintf(buffer, "%02d-%02d %02d:%02d", month, date, h, m);
    return string(buffer);
}

void parseCommand(const string& cmd, string& op, string params[20], string values[20], int& paramCount) {
    paramCount = 0;
    int pos = 0;
    while (pos < cmd.length() && cmd[pos] != ' ') pos++;
    op = cmd.substr(0, pos);

    while (pos < cmd.length()) {
        while (pos < cmd.length() && cmd[pos] == ' ') pos++;
        if (pos >= cmd.length()) break;

        if (cmd[pos] == '-') {
            pos++;
            string param;
            param += cmd[pos];
            pos++;
            while (pos < cmd.length() && cmd[pos] == ' ') pos++;

            string value;
            while (pos < cmd.length() && cmd[pos] != ' ') {
                value += cmd[pos];
                pos++;
            }

            params[paramCount] = param;
            values[paramCount] = value;
            paramCount++;
        }
    }
}

string getParam(const string params[20], const string values[20], int paramCount, const string& key) {
    for (int i = 0; i < paramCount; i++) {
        if (params[i] == key) {
            return values[i];
        }
    }
    return "";
}

// Command handlers
void addUser(const string params[20], const string values[20], int paramCount) {
    string c = getParam(params, values, paramCount, "c");
    string u = getParam(params, values, paramCount, "u");
    string p = getParam(params, values, paramCount, "p");
    string n = getParam(params, values, paramCount, "n");
    string m = getParam(params, values, paramCount, "m");
    string g = getParam(params, values, paramCount, "g");

    // Check if user already exists
    int idx;
    if (userMap.find(u, idx)) {
        cout << "-1" << endl;
        return;
    }

    // First user special case
    if (userCount == 0) {
        users[userCount].exists = true;
        strcpy(users[userCount].username, u.c_str());
        strcpy(users[userCount].password, p.c_str());
        strcpy(users[userCount].name, n.c_str());
        strcpy(users[userCount].mailAddr, m.c_str());
        users[userCount].privilege = 10;
        userMap.insert(u, userCount);
        userCount++;
        cout << "0" << endl;
        return;
    }

    // Check permissions
    if (!userMap.find(c, idx)) {
        cout << "-1" << endl;
        return;
    }

    bool isLoggedIn;
    if (!loggedIn.find(c, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    int privilege = std::stoi(g);
    if (privilege >= users[idx].privilege) {
        cout << "-1" << endl;
        return;
    }

    users[userCount].exists = true;
    strcpy(users[userCount].username, u.c_str());
    strcpy(users[userCount].password, p.c_str());
    strcpy(users[userCount].name, n.c_str());
    strcpy(users[userCount].mailAddr, m.c_str());
    users[userCount].privilege = privilege;
    userMap.insert(u, userCount);
    userCount++;
    cout << "0" << endl;
}

void login(const string params[20], const string values[20], int paramCount) {
    string u = getParam(params, values, paramCount, "u");
    string p = getParam(params, values, paramCount, "p");

    int idx;
    if (!userMap.find(u, idx)) {
        cout << "-1" << endl;
        return;
    }

    bool isLoggedIn;
    if (loggedIn.find(u, isLoggedIn) && isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    if (strcmp(users[idx].password, p.c_str()) != 0) {
        cout << "-1" << endl;
        return;
    }

    loggedIn.insert(u, true);
    loggedIn.update(u, true);
    cout << "0" << endl;
}

void logout(const string params[20], const string values[20], int paramCount) {
    string u = getParam(params, values, paramCount, "u");

    bool isLoggedIn;
    if (!loggedIn.find(u, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    loggedIn.update(u, false);
    cout << "0" << endl;
}

void queryProfile(const string params[20], const string values[20], int paramCount) {
    string c = getParam(params, values, paramCount, "c");
    string u = getParam(params, values, paramCount, "u");

    int cIdx, uIdx;
    if (!userMap.find(c, cIdx) || !userMap.find(u, uIdx)) {
        cout << "-1" << endl;
        return;
    }

    bool isLoggedIn;
    if (!loggedIn.find(c, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    if (users[cIdx].privilege <= users[uIdx].privilege && c != u) {
        cout << "-1" << endl;
        return;
    }

    cout << users[uIdx].username << " " << users[uIdx].name << " "
         << users[uIdx].mailAddr << " " << users[uIdx].privilege << endl;
}

void modifyProfile(const string params[20], const string values[20], int paramCount) {
    string c = getParam(params, values, paramCount, "c");
    string u = getParam(params, values, paramCount, "u");

    int cIdx, uIdx;
    if (!userMap.find(c, cIdx) || !userMap.find(u, uIdx)) {
        cout << "-1" << endl;
        return;
    }

    bool isLoggedIn;
    if (!loggedIn.find(c, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    if (users[cIdx].privilege <= users[uIdx].privilege && c != u) {
        cout << "-1" << endl;
        return;
    }

    string p = getParam(params, values, paramCount, "p");
    string n = getParam(params, values, paramCount, "n");
    string m = getParam(params, values, paramCount, "m");
    string g = getParam(params, values, paramCount, "g");

    if (!p.empty()) strcpy(users[uIdx].password, p.c_str());
    if (!n.empty()) strcpy(users[uIdx].name, n.c_str());
    if (!m.empty()) strcpy(users[uIdx].mailAddr, m.c_str());
    if (!g.empty()) {
        int privilege = std::stoi(g);
        if (privilege >= users[cIdx].privilege) {
            cout << "-1" << endl;
            return;
        }
        users[uIdx].privilege = privilege;
    }

    cout << users[uIdx].username << " " << users[uIdx].name << " "
         << users[uIdx].mailAddr << " " << users[uIdx].privilege << endl;
}

void addTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");

    int idx;
    if (trainMap.find(i, idx)) {
        cout << "-1" << endl;
        return;
    }

    trains[trainCount].exists = true;
    strcpy(trains[trainCount].trainID, i.c_str());
    trains[trainCount].stationNum = std::stoi(getParam(params, values, paramCount, "n"));
    trains[trainCount].seatNum = std::stoi(getParam(params, values, paramCount, "m"));

    // Parse stations
    string s = getParam(params, values, paramCount, "s");
    int pos = 0;
    for (int j = 0; j < trains[trainCount].stationNum; j++) {
        int start = pos;
        while (pos < s.length() && s[pos] != '|') pos++;
        strcpy(trains[trainCount].stations[j], s.substr(start, pos - start).c_str());
        pos++;
    }

    // Parse prices
    string p = getParam(params, values, paramCount, "p");
    pos = 0;
    for (int j = 0; j < trains[trainCount].stationNum - 1; j++) {
        int start = pos;
        while (pos < p.length() && p[pos] != '|') pos++;
        trains[trainCount].prices[j] = std::stoi(p.substr(start, pos - start));
        pos++;
    }

    trains[trainCount].startTime = parseTime(getParam(params, values, paramCount, "x"));

    // Parse travel times
    string t = getParam(params, values, paramCount, "t");
    pos = 0;
    for (int j = 0; j < trains[trainCount].stationNum - 1; j++) {
        int start = pos;
        while (pos < t.length() && t[pos] != '|') pos++;
        trains[trainCount].travelTimes[j] = std::stoi(t.substr(start, pos - start));
        pos++;
    }

    // Parse stopover times
    string o = getParam(params, values, paramCount, "o");
    if (o != "_") {
        pos = 0;
        for (int j = 0; j < trains[trainCount].stationNum - 2; j++) {
            int start = pos;
            while (pos < o.length() && o[pos] != '|') pos++;
            trains[trainCount].stopoverTimes[j] = std::stoi(o.substr(start, pos - start));
            pos++;
        }
    }

    // Parse sale dates
    string d = getParam(params, values, paramCount, "d");
    pos = d.find('|');
    trains[trainCount].saleDate[0] = parseDate(d.substr(0, pos));
    trains[trainCount].saleDate[1] = parseDate(d.substr(pos + 1));

    trains[trainCount].type = getParam(params, values, paramCount, "y")[0];
    trains[trainCount].released = false;

    trainMap.insert(i, trainCount);
    trainCount++;
    cout << "0" << endl;
}

void releaseTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");

    int idx;
    if (!trainMap.find(i, idx) || trains[idx].released) {
        cout << "-1" << endl;
        return;
    }

    trains[idx].released = true;
    cout << "0" << endl;
}

void queryTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");
    string d = getParam(params, values, paramCount, "d");

    int idx;
    if (!trainMap.find(i, idx)) {
        cout << "-1" << endl;
        return;
    }

    int day = parseDate(d);
    cout << trains[idx].trainID << " " << trains[idx].type << endl;

    int currentTime = trains[idx].startTime;
    int currentDay = day;
    int cumulPrice = 0;

    for (int j = 0; j < trains[idx].stationNum; j++) {
        if (j == 0) {
            cout << trains[idx].stations[j] << " xx-xx xx:xx -> "
                 << formatTime(currentTime, currentDay) << " " << cumulPrice << " ";
            if (j < trains[idx].stationNum - 1) {
                cout << trains[idx].seatNum << endl;
            } else {
                cout << "x" << endl;
            }
        } else if (j == trains[idx].stationNum - 1) {
            cout << trains[idx].stations[j] << " "
                 << formatTime(currentTime, currentDay) << " -> xx-xx xx:xx "
                 << cumulPrice << " x" << endl;
        } else {
            string arrTime = formatTime(currentTime, currentDay);
            currentTime += trains[idx].stopoverTimes[j - 1];
            string depTime = formatTime(currentTime, currentDay);
            cout << trains[idx].stations[j] << " " << arrTime << " -> "
                 << depTime << " " << cumulPrice << " " << trains[idx].seatNum << endl;
        }

        if (j < trains[idx].stationNum - 1) {
            cumulPrice += trains[idx].prices[j];
            currentTime += trains[idx].travelTimes[j];
            currentDay += currentTime / 1440;
            currentTime %= 1440;
        }
    }
}

void deleteTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");

    int idx;
    if (!trainMap.find(i, idx) || trains[idx].released) {
        cout << "-1" << endl;
        return;
    }

    trains[idx].exists = false;
    cout << "0" << endl;
}

void queryTicket(const string params[20], const string values[20], int paramCount) {
    // Simplified implementation
    cout << "0" << endl;
}

void queryTransfer(const string params[20], const string values[20], int paramCount) {
    cout << "0" << endl;
}

void buyTicket(const string params[20], const string values[20], int paramCount) {
    cout << "-1" << endl;
}

void queryOrder(const string params[20], const string values[20], int paramCount) {
    string u = getParam(params, values, paramCount, "u");

    bool isLoggedIn;
    if (!loggedIn.find(u, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    cout << "0" << endl;
}

void refundTicket(const string params[20], const string values[20], int paramCount) {
    cout << "-1" << endl;
}

void clean() {
    userMap.clear();
    trainMap.clear();
    loggedIn.clear();
    userCount = 0;
    trainCount = 0;
    cout << "0" << endl;
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(0);

    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;

        string op;
        string params[20];
        string values[20];
        int paramCount;

        parseCommand(line, op, params, values, paramCount);

        if (op == "add_user") {
            addUser(params, values, paramCount);
        } else if (op == "login") {
            login(params, values, paramCount);
        } else if (op == "logout") {
            logout(params, values, paramCount);
        } else if (op == "query_profile") {
            queryProfile(params, values, paramCount);
        } else if (op == "modify_profile") {
            modifyProfile(params, values, paramCount);
        } else if (op == "add_train") {
            addTrain(params, values, paramCount);
        } else if (op == "release_train") {
            releaseTrain(params, values, paramCount);
        } else if (op == "query_train") {
            queryTrain(params, values, paramCount);
        } else if (op == "delete_train") {
            deleteTrain(params, values, paramCount);
        } else if (op == "query_ticket") {
            queryTicket(params, values, paramCount);
        } else if (op == "query_transfer") {
            queryTransfer(params, values, paramCount);
        } else if (op == "buy_ticket") {
            buyTicket(params, values, paramCount);
        } else if (op == "query_order") {
            queryOrder(params, values, paramCount);
        } else if (op == "refund_ticket") {
            refundTicket(params, values, paramCount);
        } else if (op == "clean") {
            clean();
        } else if (op == "exit") {
            cout << "bye" << endl;
            break;
        }
    }

    return 0;
}
