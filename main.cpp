#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::ios;

// Minimal file-based storage system
template<typename T>
class FileStorage {
private:
    fstream file;
    string filename;
    int count;

public:
    FileStorage(const string& fname) : filename(fname), count(0) {
        file.open(filename, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            file.open(filename, ios::out | ios::binary);
            file.close();
            file.open(filename, ios::in | ios::out | ios::binary);
        }
        file.seekg(0, ios::beg);
        file.read((char*)&count, sizeof(int));
        if (file.fail()) {
            count = 0;
            file.clear();
        }
    }

    ~FileStorage() {
        if (file.is_open()) {
            file.seekp(0, ios::beg);
            file.write((char*)&count, sizeof(int));
            file.close();
        }
    }

    int add(const T& data) {
        file.seekp(0, ios::end);
        file.write((char*)&data, sizeof(T));
        return count++;
    }

    bool get(int index, T& data) {
        if (index >= count) return false;
        file.seekg(sizeof(int) + index * sizeof(T), ios::beg);
        file.read((char*)&data, sizeof(T));
        return !file.fail();
    }

    bool update(int index, const T& data) {
        if (index >= count) return false;
        file.seekp(sizeof(int) + index * sizeof(T), ios::beg);
        file.write((char*)&data, sizeof(T));
        return true;
    }

    void clear() {
        file.close();
        file.open(filename, ios::out | ios::binary | ios::trunc);
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);
        count = 0;
    }

    int size() { return count; }
};

struct User {
    char username[21];
    char password[31];
    char name[31];
    char mailAddr[31];
    int privilege;
    bool active;
};

struct Train {
    char trainID[21];
    int stationNum;
    char stations[100][31];
    int seatNum;
    int prices[100];
    int startTime;
    int travelTimes[100];
    int stopoverTimes[100];
    int saleDate[2];
    char type;
    bool released;
    bool active;
};

struct LoginSession {
    char username[21];
    bool loggedIn;
};

// Simple in-memory index
struct Index {
    char key[21];
    int value;
};

Index userIndex[100000];
int userIndexCount = 0;

Index trainIndex[10000];
int trainIndexCount = 0;

LoginSession sessions[1000];
int sessionCount = 0;

FileStorage<User>* userStorage = nullptr;
FileStorage<Train>* trainStorage = nullptr;

int findUserIndex(const string& username) {
    for (int i = 0; i < userIndexCount; i++) {
        if (strcmp(userIndex[i].key, username.c_str()) == 0) {
            return userIndex[i].value;
        }
    }
    return -1;
}

int findTrainIndex(const string& trainID) {
    for (int i = 0; i < trainIndexCount; i++) {
        if (strcmp(trainIndex[i].key, trainID.c_str()) == 0) {
            return trainIndex[i].value;
        }
    }
    return -1;
}

bool isLoggedIn(const string& username) {
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].username, username.c_str()) == 0) {
            return sessions[i].loggedIn;
        }
    }
    return false;
}

void setLoggedIn(const string& username, bool status) {
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].username, username.c_str()) == 0) {
            sessions[i].loggedIn = status;
            return;
        }
    }
    if (sessionCount < 1000) {
        strcpy(sessions[sessionCount].username, username.c_str());
        sessions[sessionCount].loggedIn = status;
        sessionCount++;
    }
}

int parseTime(const string& time) {
    int h = (time[0] - '0') * 10 + (time[1] - '0');
    int m = (time[3] - '0') * 10 + (time[4] - '0');
    return h * 60 + m;
}

int parseDate(const string& date) {
    int m = (date[0] - '0') * 10 + (date[1] - '0');
    int d = (date[3] - '0') * 10 + (date[4] - '0');
    int days = 0;
    if (m == 6) days = d - 1;
    else if (m == 7) days = 30 + d - 1;
    else if (m == 8) days = 61 + d - 1;
    return days;
}

string formatTime(int minutes, int day) {
    int actualDay = day + (minutes / 1440);
    int h = (minutes / 60) % 24;
    int m = minutes % 60;

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

    char buffer[30];
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

void addUser(const string params[20], const string values[20], int paramCount) {
    string u = getParam(params, values, paramCount, "u");

    if (findUserIndex(u) != -1) {
        cout << "-1" << endl;
        return;
    }

    User newUser;
    memset(&newUser, 0, sizeof(User));
    strcpy(newUser.username, u.c_str());
    strcpy(newUser.password, getParam(params, values, paramCount, "p").c_str());
    strcpy(newUser.name, getParam(params, values, paramCount, "n").c_str());
    strcpy(newUser.mailAddr, getParam(params, values, paramCount, "m").c_str());
    newUser.active = true;

    if (userIndexCount == 0) {
        newUser.privilege = 10;
    } else {
        string c = getParam(params, values, paramCount, "c");
        int cIdx = findUserIndex(c);
        if (cIdx == -1 || !isLoggedIn(c)) {
            cout << "-1" << endl;
            return;
        }

        User curUser;
        userStorage->get(cIdx, curUser);

        int g = std::stoi(getParam(params, values, paramCount, "g"));
        if (g >= curUser.privilege) {
            cout << "-1" << endl;
            return;
        }
        newUser.privilege = g;
    }

    int idx = userStorage->add(newUser);
    strcpy(userIndex[userIndexCount].key, u.c_str());
    userIndex[userIndexCount].value = idx;
    userIndexCount++;

    cout << "0" << endl;
}

void login(const string params[20], const string values[20], int paramCount) {
    string u = getParam(params, values, paramCount, "u");
    string p = getParam(params, values, paramCount, "p");

    int idx = findUserIndex(u);
    if (idx == -1) {
        cout << "-1" << endl;
        return;
    }

    if (isLoggedIn(u)) {
        cout << "-1" << endl;
        return;
    }

    User user;
    userStorage->get(idx, user);

    if (strcmp(user.password, p.c_str()) != 0) {
        cout << "-1" << endl;
        return;
    }

    setLoggedIn(u, true);
    cout << "0" << endl;
}

void logout(const string params[20], const string values[20], int paramCount) {
    string u = getParam(params, values, paramCount, "u");

    if (!isLoggedIn(u)) {
        cout << "-1" << endl;
        return;
    }

    setLoggedIn(u, false);
    cout << "0" << endl;
}

void queryProfile(const string params[20], const string values[20], int paramCount) {
    string c = getParam(params, values, paramCount, "c");
    string u = getParam(params, values, paramCount, "u");

    int cIdx = findUserIndex(c);
    int uIdx = findUserIndex(u);

    if (cIdx == -1 || uIdx == -1 || !isLoggedIn(c)) {
        cout << "-1" << endl;
        return;
    }

    User curUser, targetUser;
    userStorage->get(cIdx, curUser);
    userStorage->get(uIdx, targetUser);

    if (curUser.privilege <= targetUser.privilege && c != u) {
        cout << "-1" << endl;
        return;
    }

    cout << targetUser.username << " " << targetUser.name << " "
         << targetUser.mailAddr << " " << targetUser.privilege << endl;
}

void modifyProfile(const string params[20], const string values[20], int paramCount) {
    string c = getParam(params, values, paramCount, "c");
    string u = getParam(params, values, paramCount, "u");

    int cIdx = findUserIndex(c);
    int uIdx = findUserIndex(u);

    if (cIdx == -1 || uIdx == -1 || !isLoggedIn(c)) {
        cout << "-1" << endl;
        return;
    }

    User curUser, targetUser;
    userStorage->get(cIdx, curUser);
    userStorage->get(uIdx, targetUser);

    if (curUser.privilege <= targetUser.privilege && c != u) {
        cout << "-1" << endl;
        return;
    }

    string p = getParam(params, values, paramCount, "p");
    string n = getParam(params, values, paramCount, "n");
    string m = getParam(params, values, paramCount, "m");
    string g = getParam(params, values, paramCount, "g");

    if (!p.empty()) strcpy(targetUser.password, p.c_str());
    if (!n.empty()) strcpy(targetUser.name, n.c_str());
    if (!m.empty()) strcpy(targetUser.mailAddr, m.c_str());
    if (!g.empty()) {
        int privilege = std::stoi(g);
        if (privilege >= curUser.privilege) {
            cout << "-1" << endl;
            return;
        }
        targetUser.privilege = privilege;
    }

    userStorage->update(uIdx, targetUser);

    cout << targetUser.username << " " << targetUser.name << " "
         << targetUser.mailAddr << " " << targetUser.privilege << endl;
}

void addTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");

    if (findTrainIndex(i) != -1) {
        cout << "-1" << endl;
        return;
    }

    Train train;
    memset(&train, 0, sizeof(Train));
    strcpy(train.trainID, i.c_str());
    train.stationNum = std::stoi(getParam(params, values, paramCount, "n"));
    train.seatNum = std::stoi(getParam(params, values, paramCount, "m"));
    train.active = true;
    train.released = false;

    string s = getParam(params, values, paramCount, "s");
    int pos = 0;
    for (int j = 0; j < train.stationNum; j++) {
        int start = pos;
        while (pos < s.length() && s[pos] != '|') pos++;
        strcpy(train.stations[j], s.substr(start, pos - start).c_str());
        pos++;
    }

    string p = getParam(params, values, paramCount, "p");
    pos = 0;
    for (int j = 0; j < train.stationNum - 1; j++) {
        int start = pos;
        while (pos < p.length() && p[pos] != '|') pos++;
        train.prices[j] = std::stoi(p.substr(start, pos - start));
        pos++;
    }

    train.startTime = parseTime(getParam(params, values, paramCount, "x"));

    string t = getParam(params, values, paramCount, "t");
    pos = 0;
    for (int j = 0; j < train.stationNum - 1; j++) {
        int start = pos;
        while (pos < t.length() && t[pos] != '|') pos++;
        train.travelTimes[j] = std::stoi(t.substr(start, pos - start));
        pos++;
    }

    string o = getParam(params, values, paramCount, "o");
    if (o != "_") {
        pos = 0;
        for (int j = 0; j < train.stationNum - 2; j++) {
            int start = pos;
            while (pos < o.length() && o[pos] != '|') pos++;
            train.stopoverTimes[j] = std::stoi(o.substr(start, pos - start));
            pos++;
        }
    }

    string d = getParam(params, values, paramCount, "d");
    pos = d.find('|');
    train.saleDate[0] = parseDate(d.substr(0, pos));
    train.saleDate[1] = parseDate(d.substr(pos + 1));

    train.type = getParam(params, values, paramCount, "y")[0];

    int idx = trainStorage->add(train);
    strcpy(trainIndex[trainIndexCount].key, i.c_str());
    trainIndex[trainIndexCount].value = idx;
    trainIndexCount++;

    cout << "0" << endl;
}

void releaseTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");

    int idx = findTrainIndex(i);
    if (idx == -1) {
        cout << "-1" << endl;
        return;
    }

    Train train;
    trainStorage->get(idx, train);

    if (train.released) {
        cout << "-1" << endl;
        return;
    }

    train.released = true;
    trainStorage->update(idx, train);
    cout << "0" << endl;
}

void queryTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");
    string d = getParam(params, values, paramCount, "d");

    int idx = findTrainIndex(i);
    if (idx == -1) {
        cout << "-1" << endl;
        return;
    }

    Train train;
    trainStorage->get(idx, train);

    int day = parseDate(d);
    cout << train.trainID << " " << train.type << endl;

    int currentTime = train.startTime;
    int cumulPrice = 0;

    for (int j = 0; j < train.stationNum; j++) {
        if (j == 0) {
            cout << train.stations[j] << " xx-xx xx:xx -> "
                 << formatTime(currentTime, day) << " " << cumulPrice << " ";
            cout << train.seatNum << endl;
        } else if (j == train.stationNum - 1) {
            cout << train.stations[j] << " "
                 << formatTime(currentTime, day) << " -> xx-xx xx:xx "
                 << cumulPrice << " x" << endl;
        } else {
            string arrTime = formatTime(currentTime, day);
            int depTime = currentTime + train.stopoverTimes[j - 1];
            string depTimeStr = formatTime(depTime, day);
            cout << train.stations[j] << " " << arrTime << " -> "
                 << depTimeStr << " " << cumulPrice << " " << train.seatNum << endl;
            currentTime = depTime;
        }

        if (j < train.stationNum - 1) {
            cumulPrice += train.prices[j];
            currentTime += train.travelTimes[j];
        }
    }
}

void deleteTrain(const string params[20], const string values[20], int paramCount) {
    string i = getParam(params, values, paramCount, "i");

    int idx = findTrainIndex(i);
    if (idx == -1) {
        cout << "-1" << endl;
        return;
    }

    Train train;
    trainStorage->get(idx, train);

    if (train.released) {
        cout << "-1" << endl;
        return;
    }

    train.active = false;
    trainStorage->update(idx, train);
    cout << "0" << endl;
}

void queryTicket(const string params[20], const string values[20], int paramCount) {
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
    if (!isLoggedIn(u)) {
        cout << "-1" << endl;
        return;
    }
    cout << "0" << endl;
}

void refundTicket(const string params[20], const string values[20], int paramCount) {
    cout << "-1" << endl;
}

void clean() {
    userStorage->clear();
    trainStorage->clear();
    userIndexCount = 0;
    trainIndexCount = 0;
    sessionCount = 0;
    cout << "0" << endl;
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(0);

    userStorage = new FileStorage<User>("users.dat");
    trainStorage = new FileStorage<Train>("trains.dat");

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

    delete userStorage;
    delete trainStorage;

    return 0;
}
