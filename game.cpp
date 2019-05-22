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
    int player_count;
    for (;;) {
        string players_argument;
        cout << "How many players are there?" << endl;
        cin >> players_argument;
        cin.ignore();
        if (is_valid_int(players_argument)) {
            player_count = stoi(players_argument);
            if (2 <= player_count && player_count <= 6) {
                break;
            } else {
                cout << "There must be 2 to 6 players in a game." << endl;
            }
        } else {
            cout << "That is not an integer!" << endl;
        }
    }

    swoope::socketstream listeningSocket;
    vector<swoope::socketstream> sockets(player_count);
    // initialize input and output streams
    vector<ostream *> outputs(player_count);
    vector<istream *> inputs(player_count);
    outputs[0] = &cout;
    inputs[0] = &cin;
    for (int i = 1; i < player_count; ++i) {
        outputs[i] = &sockets[i];
        inputs[i] = &sockets[i];
    }
    // initialize connections
    listeningSocket.open(port, player_count);
    for (int i = 1; i < player_count; ++i) {  // connect players
        cout << "Waiting for Player " << (i + 1) << "\n";
        listeningSocket.accept(sockets[i]);
        outputTo(outputs[i], "Waiting for other players...");
    }
    outputToAll(outputs, "Players connected!");
    outputToAll(outputs);

    // ready
    outputToAll(outputs, "Chopsticks will now commence!");
    for (int i = 0; i < player_count; ++i) {
        string line = "You are player " + to_string(i + 1);
        outputTo(outputs[i], line);
        outputTo(outputs[i]);
    }

    // player class phase
    outputToAll(outputs, "Please wait for your turn...", outputs[0]);
    outputToAll(outputs, "", outputs[0]);
    vector<Player *> players;
    for (int i = 0; i < player_count; ++i) {
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
    for (int i = 0; i < player_count; ++i) {
        outputTo(outputs[i], "You are of type " + players[i]->get_type());
        outputTo(outputs[i]);
    }

    // grouping phase
    vector<Team> teams;
    outputToAll(outputs, "Grouping phase.");
    {
        int team_count = 0;
        int group_numbers[player_count];
        int group_player_counts[player_count];
        bool valid_group = false;
        while (!valid_group) {
            // set group:player_count all to 0
            for (int i = 0; i < player_count; ++i) {
                group_player_counts[i] = 0;
            }
            // get input
            outputToAll(outputs, "Please wait for your turn...", outputs[0]);
            for (int i = 0; i < player_count; ++i) {
                string group_arg;
                outputTo(outputs[i], "Enter group number [1 to " + to_string(player_count) + "].");
                group_arg = getlineFrom(inputs[i], outputs[i]);
                if (is_valid_int(group_arg)) {
                    int group = stoi(group_arg);
                    if (1 <= group && group <= player_count) {
                        group_numbers[i] = group;
                        outputTo(outputs[i], "Please wait for other players to choose their group.");
                    } else {
                        outputTo(outputs[i], "Group number out of range.");
                        --i;
                    }
                } else {
                    outputTo(outputs[i], "Group number must be an integer.");
                    --i;
                }
            }
            // check validity
            int check = 0;
            for (int i = 0; i < player_count; ++i) {
                ++group_player_counts[group_numbers[i]-1];
            }
            for (int i = 0; i < player_count && group_player_counts[i] != 0; ++i, team_count = i+1) {
                check += group_player_counts[i];
            }
            valid_group = group_player_counts[0] != 0 && group_player_counts[1] != 0 && check == player_count;
            if (!valid_group) outputToAll(outputs, "Invalid groupings made! Try again.");
        }
        // valid grouping, build teams
        for (int i = 1; i <= team_count; ++i) {
            Team new_team(i);
            teams.push_back(new_team);
        }
        for (int i = 0; i < player_count; ++i) {
            players[i]->set_team_number(group_numbers[i]);
            teams[group_numbers[i]-1].add_player(players[i]);
        }
    }
    outputToAll(outputs, "Grouping successful!");
    for (int i = 0; i < player_count; ++i) {
        outputTo(outputs[i], "You are in group " + to_string(players[i]->get_team_number()) + ".");
    }
    outputToAll(outputs);

    // actual game
    
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
    } else {
        cerr << "Port must be an integer." << endl;
        return 3;
    }
    // run
    if (argc == 2) {
        runServer(argv[1]);
    } else if (argc == 3) {
        runClient(argv[1], argv[2]);
    }
    return 0;
}