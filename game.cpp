#include <iostream>
#include <string>
#include <typeinfo>
#include "chopsticks.hpp"
#include "socketstream/socketstream.hh"

using namespace std;

const int CLIENT_END = -1;
const int CLIENT_OUTPUT = 0;
const int CLIENT_INPUT = 1;

/* don't include \n in output */
void outputTo(ostream *output, string line = "") {
    if (output != &cout) *output << CLIENT_OUTPUT << endl;
    *output << line << endl;
}

/* output to all except 3rd param, don't include \n in output */
void outputToAll(vector<ostream *> outputs, string line = "", ostream *except = nullptr) {
    for (ostream *output : outputs) {
        if (output != except) {
            outputTo(output, line);
        }
    }
}

string getlineFrom(istream *input, ostream *output) {
    string result;
    if (input != &cin) *output << CLIENT_INPUT << endl;
    getline(*input, result);
    return result;
}

bool is_valid_int(string s) {
    string::const_iterator it = s.begin();
    if (*it != '-' && !isdigit(*it)) return false;
    while (it != s.end() && (isdigit(*it))) it++;
    return !s.empty() && it == s.end();
}

void runServer(string port) {
    // check player number validity
    string players_argument;
    int players_count;
    for (;;) {
        cout << "How many players are there?" << endl;
        cin >> players_argument;
        cin.ignore();
        if (is_valid_int(players_argument)) {
            players_count = stoi(players_argument);
            if (!(2 <= players_count && players_count <= 6)) {
                cout << "There must be 2 to 6 players in a game." << endl;
            } else {
                break;
            }
        } else {
            cout << "That is not an integer!" << endl;
        }
    }


    swoope::socketstream listeningSocket;
    vector<swoope::socketstream> sockets(players_count);
    // initialize input and output streams
    vector<ostream *> outputs(players_count);
    vector<istream *> inputs(players_count);
    outputs[0] = &cout;
    inputs[0] = &cin;
    for (int i = 1; i < players_count; ++i) {
        outputs[i] = &sockets[i];
        inputs[i] = &sockets[i];
    }
    // initialize connections
    listeningSocket.open(port, players_count);
    for (int i = 1; i < players_count; ++i) {  // connect players
        cout << "Waiting for Player " << (i + 1) << "\n";
        listeningSocket.accept(sockets[i]);
        outputTo(outputs[i], "Waiting for other players...");
    }
    outputToAll(outputs, "Players connected!");
    outputToAll(outputs);

    // ready
    outputToAll(outputs, "Chopsticks will now commence!");
    for (int i = 0; i < players_count; ++i) {
        string line = "You are player " + to_string(i + 1);
        outputTo(outputs[i], line);
        outputTo(outputs[i]);
    }

    // player class phase
    outputToAll(outputs, "Please wait for your turn...", outputs[0]);
    outputToAll(outputs, "", outputs[0]);
    vector<Player *> players;
    for (int i = 0; i < players_count; ++i) {
        outputTo(outputs[i], "Which player class would you like to play?");
        outputTo(outputs[i], "Choose 1: Human || Alien || Zombie || Doggo");
        
        string type = getlineFrom(inputs[i], outputs[i]);

        Player *new_player;
        if (type == "Human" || type == "human") {
            new_player = new Human(i + 1);
        } else if (type == "Alien" || type == "alien") {
            new_player = new Alien(i + 1);
        } else if (type == "Zombie" || type == "zombie") {
            new_player = new Zombie(i + 1);
        } else if (type == "Doggo" || type == "doggo") {
            new_player = new Doggo(i + 1);
        } else {
            outputTo(outputs[i], "Invalid keyword! Try again.");
            outputTo(outputs[i]);
            --i;
            continue;
        }
        outputTo(outputs[i], "Waiting for other players to choose...");
        players.push_back(new_player);
    }

    // output class types
    for (int i = 0; i < players_count; ++i) {
        outputTo(outputs[i], "You are of type " + players[i]->get_type());
        outputTo(outputs[i]);
    }

    // grouping phase

}

/* generic client, no logic */
void runClient(string ip, string port) {
    swoope::socketstream server;
    server.open(ip, port);  // if ip address is invalid, it takes too long to disconnect

    bool running = true;
    while (running) {
        int mode;
        string line = "";
        server >> mode;
        server.ignore();
        switch (mode) {
            case CLIENT_END:
                running = false;
                break;
            case CLIENT_OUTPUT:
                getline(server, line);
                cout << line << endl;
                break;
            case CLIENT_INPUT:
                getline(cin, line);
                server << line << endl;
                break;
            default:
                break;
        }
    }
    server.close();
    cout << "Connection closed." << endl;
}

int main(int argc, char *argv[]) {
    // check port validity
    if (!(argc == 2 || argc == 3)) {
        cerr << "Invalid argument count." << endl;
        return 1;
    }
    int port_index = argc - 1;
    if (is_valid_int(argv[port_index])) {
        int port = stoi(argv[port_index]);
        if (!(1024 <= port && port <= 65535)) {
            cerr << "Port must be from 1024 to 65535 only." << endl;
            return 2;
        }
    }
    // run
    if (argc == 2) {
        runServer(argv[1]);
    } else if (argc == 3) {
        runClient(argv[1], argv[2]);
    }
    return 0;
}