#include <fstream>
#include <iostream>
#include <string>
#include <typeinfo>
#include "chopsticks.hpp"
#include "functions.hpp"
#include "socketstream/socketstream.hh"

using namespace std;

void runServer(string port) {
    // check player number validity
    int player_count;
    for (;;) {
        string players_argument;
        cout << "How many players are there?" << endl;
        getline(cin, players_argument);
        if (is_valid_int(players_argument)) {
            player_count = stoi(players_argument);
            if (2 <= player_count && player_count <= 6) {
                break;
            } else {
                cout << "There must be 2 to 6 players in a game." << endl;
            }
        } else {
            cout << "That is not a valid integer!" << endl;
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
    ifstream banner("banner.txt");
    if (banner.is_open()) {
        string line;
        while (getline(banner, line)) {
            outputToAll(outputs, line);
        }
        outputToAll(outputs);
        banner.close();
    } else {
        outputToAll(outputs, "Chopsticks will now commence!");
    }

    // show mechanics
    outputToAll(outputs, "Please wait...");
    for (int i = 0; i < player_count; ++i) {
        outputTo(outputs[i], "Show mechanics? (y/n) default: n");
        istringstream strm(getlineFrom(inputs[i], outputs[i]));
        string answer;
        strm >> answer;
        if (answer == "y" || answer == "Y") {
            ifstream rules("rules.txt");
            if (rules.is_open()) {
                string line;
                while (getline(rules, line)) {
                    outputTo(outputs[i], line);
                }
                outputTo(outputs[i]);
                rules.close();
            }
        }
        outputTo(outputs[i], "Please wait...");
    }

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
        if (!is_valid_string(type, 1)) {
            outputTo(outputs[i], "Enter only one keyword.");
            --i;
            continue;
        }
        istringstream line(type);
        line >> type;

        Player *new_player;
        if (type == "Human" || type == "human" || type == "1") {
            new_player = new Human(i + 1, outputs[i], inputs[i]);
        } else if (type == "Alien" || type == "alien" || type == "2") {
            new_player = new Alien(i + 1, outputs[i], inputs[i]);
        } else if (type == "Zombie" || type == "zombie" || type == "3") {
            new_player = new Zombie(i + 1, outputs[i], inputs[i]);
        } else if (type == "Doggo" || type == "doggo" || type == "4") {
            new_player = new Doggo(i + 1, outputs[i], inputs[i]);
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
        outputTo(outputs[i], "You are of type " + players[i]->getName());
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
                    outputTo(outputs[i], "Group number must be a valid integer.");
                    --i;
                }
            }
            // check validity
            int check = 0;
            for (int i = 0; i < player_count; ++i) {
                ++group_player_counts[group_numbers[i] - 1];
            }
            for (int i = 0; i < player_count && group_player_counts[i] != 0; ++i, team_count = i) {
                check += group_player_counts[i];
            }
            valid_group = group_player_counts[0] != 0 && group_player_counts[1] != 0 && check == player_count;
            if (!valid_group) outputToAll(outputs, "Invalid groupings made! Try again.");
        }
        // valid grouping, build teams
        for (int i = 1; i <= team_count; ++i) {
            Team new_team(i, outputs);
            teams.push_back(new_team);
        }
        for (int i = 0; i < player_count; ++i) {
            players[i]->setTeamNumber(group_numbers[i]);
            teams[group_numbers[i] - 1].addPlayer(players[i]);
        }
    }
    outputToAll(outputs, "Grouping successful!");
    for (int i = 0; i < player_count; ++i) {
        outputTo(outputs[i], "You are in group " + to_string(players[i]->getTeamNumber()) + ".");
    }
    outputToAll(outputs);

    // actual game
    Team *winning_team = nullptr;
    for (unsigned int teams_alive = 0, current_team_index = 0; teams_alive != 1; current_team_index = (current_team_index + 1) % teams.size()) {
        if (!teams[current_team_index].isAlive()) continue;
        // output game status
        for (size_t i = 0; i < teams.size(); ++i) {
            outputToAll(outputs, (i == current_team_index ? '>' : ' ') + teams[i].getStatus());
        }
        outputToAll(outputs);

        // check team and player skips
        Player *supposed_next_player = teams[current_team_index].getNextPlayer();
        Player *next_available_player = teams[current_team_index].getNextAvailablePlayer(true, true);
        if (supposed_next_player != next_available_player) {  // a skip happened
            if (next_available_player == nullptr) {           // whole team skipped
                outputToAll(outputs, "Team " + to_string(teams[current_team_index].getTeamNumber()) + " has been skipped.");
                outputToAll(outputs);
                continue;
            } else {  // only a player was skipped, reoutput game status
                outputToAll(outputs);
                for (size_t i = 0; i < teams.size(); ++i) {
                    if (i == current_team_index) {
                        outputToAll(outputs, '>' + teams[i].getStatus(true));
                    } else {
                        outputToAll(outputs, ' ' + teams[i].getStatus());
                    }
                }
                outputToAll(outputs);
            }
        }

        // do turn
        int player_index = teams[current_team_index].getCurrentPlayer()->getPlayerNumber() - 1;
        outputToAll(outputs, "Waiting for player " + to_string(player_index + 1) + " from team " + to_string(teams[current_team_index].getTeamNumber()) + ".", outputs[player_index]);
        vector<string> actions_made;
        for (int i = 0; i < teams[current_team_index].getCurrentPlayer()->getTurns(); ++i) {
            // player move
            actions_made.push_back(teams[current_team_index].getCurrentPlayer()->playWith(players));
            // check win
            teams_alive = 0;
            for (auto &team : teams) {
                if (team.isAlive()) {
                    ++teams_alive;
                    winning_team = &team;
                }
                if (teams_alive >= 2) break;
            }
            if (teams_alive == 1) break;
        }

        // broadcast moves made
        outputToAll(outputs, "Player " + to_string(player_index + 1) + " actions:", outputs[player_index]);
        for (auto &&action : actions_made) {
            outputToAll(outputs, "=> " + action, outputs[player_index]);
        }
        outputToAll(outputs);
    }
    // output final game status
    for (auto &team : teams) {
        outputToAll(outputs, team.getStatus());
    }
    outputToAll(outputs);
    // game conclusion
    int winning_team_number = winning_team->getTeamNumber();
    for (int i = 0; i < player_count; ++i) {
        if (players[i]->getTeamNumber() == winning_team_number) {
            outputTo(outputs[i], "Congratulations! Team " + to_string(winning_team_number) + " wins!");
        } else {
            outputTo(outputs[i], "You lose. Team " + to_string(winning_team_number) + " wins!");
        }
    }
    // close clients
    for (int i = 1; i < player_count; ++i) {
        *outputs[i] << CLIENT_END;
        sockets[i].close();
    }
    // clean up
    for (auto &player_ptr : players) {
        delete player_ptr;
    }
    listeningSocket.close();
    cout << "Connections closed." << endl;
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
        return 0;
    }
    int port_index = argc - 1;
    if (is_valid_int(argv[port_index])) {
        int port = stoi(argv[port_index]);
        if (!(1024 <= port && port <= 65535)) {
            cerr << "Port must be from 1024 to 65535 only." << endl;
            return 0;
        }
    } else {
        cerr << "Port must be an integer." << endl;
        return 0;
    }
    // run
    if (argc == 2) {
        runServer(argv[1]);
    } else if (argc == 3) {
        runClient(argv[1], argv[2]);
    }
    return 0;
}