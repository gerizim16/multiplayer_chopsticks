#pragma once
#ifndef CHOPSTICKS_HPP
#define CHOPSTICKS_HPP

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "functions.hpp"

using namespace std;

class Extremity {
   protected:
    int max_count;
    string type;
    bool alive;
    int count;
    virtual void tapped() = 0;  // different ways of how hand and foot resolves a tap

   public:
    Extremity(int max_count, string type)
        : max_count(max_count), type(type), alive(true), count(1) {}
    string get_type() { return type; }
    bool const is_alive() { return alive; }
    string const get_status() { return alive ? to_string(count) : "X"; }
    int const get_count() { return count; }
    int const get_max_count() { return max_count; }
    bool tap(Extremity &other);
    bool set_count(int new_count);
};

/* @return true if this and other extremity is alive else false */
bool Extremity::tap(Extremity &other) {
    if (!other.is_alive() || !this->is_alive()) return false;
    other.count += this->count;
    other.tapped();
    return true;
}

/* @return true if new count is in bounds else false */
bool Extremity::set_count(int new_count) {
    if (0 <= new_count && new_count < max_count) {
        count = new_count;
        return true;
    }
    return false;
}

class Hand : public Extremity {
   public:
    Hand(int max_count) : Extremity(max_count, "hand") {}
    void tapped() override;
};

void Hand::tapped() {
    if (count == max_count) {
        alive = false;
    } else {
        count %= max_count;
    }
}

class Foot : public Extremity {
   public:
    Foot(int max_count) : Extremity(max_count, "foot") {}
    void tapped() override;
};

void Foot::tapped() {
    if (count >= max_count) {
        alive = false;
    }
}

class Player {
   protected:
    string type;
    int player_number;
    int team_number;
    bool skip;
    bool alive;
    vector<Hand> hands;
    vector<Foot> feet;
    int max_fingers;
    int max_toes;
    ostream *output;
    istream *input;
    int turns;
    Extremity *get_ex(string mode);

    // to keep track of attacked by events
    virtual void attacked_by(Player &other, Extremity &other_ex, Extremity &my_ex);

   public:
    Player(string type, int player_number, int hand_count, int foot_count,
           int finger_count, int toe_count, ostream *output = &cout, istream *input = &cin,
           int turns = 1);
    virtual ~Player() {}
    string get_type() { return type; }
    int const get_player_number() { return player_number; }
    void set_team_number(int new_team_number) { team_number = new_team_number; }
    int const get_team_number() { return team_number; }
    int const get_hands_count() { return hands.size(); }
    int const get_feet_count() { return feet.size(); }
    bool const is_alive() { return alive; }
    bool attack(Player &other, string my_stats, string other_stats);
    bool distribute(string mode, vector<int> change);
    virtual void skip_turn(bool force = false) { skip = true; }
    bool check_skip(bool modify_skip = false);
    bool const get_skip() { return skip; }
    string get_status();
    vector<string> play_with(vector<Player *> &all_players);
};

Player::Player(string type, int player_number, int hand_count, int foot_count,
               int finger_count, int toe_count, ostream *output, istream *input,
               int turns)
    : type(type), player_number(player_number), skip(false), alive(true), max_fingers(finger_count), max_toes(toe_count) , output(output), input(input), turns(turns) {
    for (int i = 0; i < hand_count; ++i) {
        Hand temp_hand(finger_count);
        hands.push_back(temp_hand);
    }
    for (int i = 0; i < foot_count; ++i) {
        Foot temp_foot(toe_count);
        feet.push_back(temp_foot);
    }
}

/* @return true if valid attack else false */
bool Player::attack(Player &other_player, string my_stats, string other_stats) {
    Extremity *other_ex = other_player.get_ex(other_stats);
    Extremity *my_ex = this->get_ex(my_stats);
    if (other_ex == nullptr || my_ex == nullptr) return false;
    if (!my_ex->tap(*other_ex)) return false;
    if (other_ex->get_type() == "foot" && !other_ex->is_alive()) {
        other_player.skip_turn();
    }
    other_player.attacked_by(*this, *my_ex, *other_ex);
    return true;
}

/* @return true if valid mode and distribution else false */
bool Player::distribute(string mode, vector<int> change) {
    vector<Extremity *> extr;
    if (mode == "hands") {
        for (auto &hand : hands) {
            extr.push_back(&hand);
        }
    } else if (mode == "feet") {
        for (auto &foot : feet) {
            extr.push_back(&foot);
        }
    } else {
        return false;
    }
    if (extr.size() != change.size()) return false;
    int change_sum = 0, extremeties_sum = 0;
    for (size_t i = 0; i < extr.size(); ++i) {
        int temp_change = change[i];
        if (extr[i]->is_alive()) {
            if (!(1 <= temp_change && temp_change < extr[i]->get_max_count())) return false;
        } else {
            continue;
        }
        change_sum += temp_change;
        extremeties_sum += extr[i]->get_count();
    }
    if (change_sum != extremeties_sum) return false;
    for (size_t i = 0; i < extr.size(); ++i) {
        extr[i]->set_count(change[i]);
    }
    return true;
}

/**
 * @param if set to true, modifies skip status. default: false
 * @return true if skipped else false
 */
bool Player::check_skip(bool modify_skip) {
    if (skip) {
        if (modify_skip) skip = false;
        return true;
    } else {
        return false;
    }
}

void Player::attacked_by(Player &other, Extremity &other_ex,
                         Extremity &my_ex) {
    alive = false;
    for (Hand &ex : hands) {
        if (ex.is_alive()) {
            alive = true;
            return;
        }
    }
    for (Foot &ex : feet) {
        if (ex.is_alive()) {
            alive = true;
            return;
        }
    }
}

/* @return valid pointer if valid parameter and in bounds else std::nullptr */
Extremity *Player::get_ex(string mode) {
    if (mode.size() != 2) return nullptr;
    unsigned int index = (unsigned int)mode[1] - (unsigned int)'A';
    if (mode[0] == 'H' && 0 <= index && index < hands.size()) {
        return &hands[index];
    } else if (mode[0] == 'F' && 0 <= index && index < feet.size()) {
        return &feet[index];
    }
    return nullptr;
}

string Player::get_status() {
    string result = "P" + to_string(player_number) + type[0] + " (";
    for (auto &hand : hands) {
        result += hand.get_status();
    }
    result += ":";
    for (auto &foot : feet) {
        result += foot.get_status();
    }
    result += ") [" + to_string(max_fingers) + ":" + to_string(max_toes) + "]";
    if (!alive) result += " [dead]";
    else if (skip) result += " [skipping]";
    return result;
}

/**
 * assumes player is available to play
 * @return vector<string> of actions madde
 */
vector<string> Player::play_with(vector<Player *> &all_players) {
    vector<string> actions;
    for (int i = 0; i < turns; ++i) {
        string action, line_string;
        outputTo(output, "Enter your move.");
        line_string = getlineFrom(input, output);
        istringstream line(line_string);
        line >> action;
        if (action == "tap") {
            string from, to, player_num_arg;
            unsigned int player_number;
            line >> from >> player_num_arg >> to;
            if (is_valid_int(player_num_arg)) {
                player_number = stoi(player_num_arg);
                if (!(1 <= player_number && player_number <= all_players.size())) {
                    outputTo(output, "Player number out of bounds! Enter action again.");
                    --i;
                    continue;
                }
            } else {
                outputTo(output, "Player number must be an integer! Enter action again.");
            }
            Player *target = all_players[player_number - 1];
            if (!target->is_alive()) {
                outputTo(output, "Target player is dead. Enter action again.");
                --i;
                continue;
            }
            if (target->get_team_number() == this->get_team_number()) {
                outputTo(output, "Friendly fire is not allowed! Enter action again.");
                --i;
                continue;
            }
            if (!attack(*target, from, to)) {
                outputTo(output, "Attack invalid! Enter action again.");
                --i;
                continue;
            }
        } else if (action == "disthands") {
            vector<int> changes(get_hands_count());
            for (auto &item : changes) {
                line >> item;
            }
            if (!distribute("hands", changes)) {
                outputTo(output, "Invalid distribution! Enter action again.");
                --i;
                continue;
            }
        } else if (action == "distfeet") {
            vector<int> changes(get_feet_count());
            for (auto &item : changes) {
                line >> item;
            }
            if (!distribute("feet", changes)) {
                outputTo(output, "Invalid distribution! Enter action again.");
                --i;
                continue;
            }
        } else {
            outputTo(output, "Invalid keyword! Try again.");
            --i;
            continue;
        }
        actions.push_back(line_string);
    }
    return actions;
}

class Human : public Player {
   public:
    Human(int player_number, ostream *output = &cout, istream *input = &cin)
        : Player("human", player_number, 2, 2, 5, 5, output, input) {}
};

class Alien : public Player {
   public:
    Alien(int player_number, ostream *output = &cout, istream *input = &cin)
        : Player("alien", player_number, 4, 2, 3, 2, output, input) {}
    void skip_turn(bool force = false) override {
        if (force) Player::skip_turn();
    }
};

class Zombie : public Player {
   public:
    Zombie(int player_number, ostream *output = &cout, istream *input = &cin)
        : Player("zombie", player_number, 1, 0, 4, 0, output, input, 2) {}
    void attacked_by(Player &other, Extremity &other_ex, Extremity &my_ex) override {
        if (hands.size() == 1 && !my_ex.is_alive()) {  // starting hand dies
            Hand new_hand(4);
            hands.push_back(new_hand);
        }
        Player::attacked_by(other, other_ex, my_ex);
    }
};

class Doggo : public Player {
   public:
    Doggo(int player_number, ostream *output = &cout, istream *input = &cin)
        : Player("doggo", player_number, 0, 4, 0, 4, output, input) {}
    void attacked_by(Player &other, Extremity &other_ex, Extremity &my_ex) override {
        Player::attacked_by(other, other_ex, my_ex);
        if (other.get_type() != this->get_type()) {  // not doggo type
            other.skip_turn(true);
        }
    }
};

class Team {
   private:
    int team_number;
    bool first_turn = true;
    vector<ostream *> outputs;
    vector<Player *> players;
    int current_player_index;
    Player *current_player;
    Player *get_next_available_player(bool inplace = false, bool output_status = false);
    Player *check_for_next_player();

   public:
    Team(int team_number, vector<ostream *> outputs = {});
    int get_team_number() { return team_number; }
    bool is_alive();
    int get_players_alive_count();
    bool play_with(vector<Player *> &all_players);
    void add_player(Player *new_player);
    string get_status();
};

Team::Team(int team_number, vector<ostream *> outputs) : team_number(team_number), outputs(outputs), current_player_index(0), current_player(nullptr) {
}

bool Team::is_alive() {
    for (auto &player : players) {
        if (player->is_alive()) {
            return true;
        }
    }
    return false;
}

int Team::get_players_alive_count() {
    int result = 0;
    for (auto &player : players) {
        if (player->is_alive()) {
            ++result;
        }
    }
    return result;
}

/* @return true if team can play else false */
bool Team::play_with(vector<Player *> &all_players) {
    if (!is_alive()) {
        return false;
    }

    if (get_next_available_player(true, true) == nullptr) {
        outputToAll(outputs, "Team " + to_string(team_number) + " has been skipped.");
        return false;
    }
    int player_index = current_player->get_player_number() - 1;
    outputToAll(outputs, "Waiting for player " + to_string(player_index + 1) + " from team " + to_string(team_number) + ".", outputs[player_index]);
    vector<string> actions_made = current_player->play_with(all_players);
    outputToAll(outputs, "Player " + to_string(player_index + 1) + " actions:", outputs[player_index]);
    for (auto &&action : actions_made) {
        outputToAll(outputs, "=> " + action, outputs[player_index]);
    }
    return true;
}

void Team::add_player(Player *new_player) {
    players.push_back(new_player);
}

/**
 * @param modify, if set to true -> modifies internal variables
 * @return pointer to new player if there is a next available player else nullptr 
 */
Player *Team::get_next_available_player(bool modify, bool output_status) {
    Player *next_player = current_player;
    int next_player_index = current_player_index;
    // if first turn
    if (next_player == nullptr) {
        next_player = players[0];
        if (modify) {
            current_player_index = 0;
            current_player = next_player;
        }
        return next_player;
    }

    // start checking with next player
    int players_skipped = 0;
    next_player_index = (next_player_index + 1) % players.size();
    next_player = players[next_player_index];
    Player *first_player_checked = next_player;  // keep track last player who was checked

    do {  // while not returning back to first check player
        if (next_player->is_alive()) {
            if (next_player->check_skip(modify)) {
                if (output_status) outputToAll(outputs, "Player " + to_string(next_player->get_player_number()) + " has been skipped.");
                ++players_skipped;
            } else {
                break;
            }
        }
        // current player can't play so move to next
        next_player_index = (next_player_index + 1) % players.size();
        next_player = players[next_player_index];
    } while (next_player != first_player_checked);

    if (players_skipped == get_players_alive_count()) return nullptr;  // no available player in team to play
    // next player available
    if (modify) {
        current_player_index = next_player_index;
        current_player = next_player;
    }
    return next_player;
}

string Team::get_status() {
    int next_player_index = 0;
    Player *next = get_next_available_player();
    if (next != nullptr) next_player_index = next->get_player_number();
    string result = "Team " + to_string(team_number) + ": ";
    for (size_t i = 0; i < players.size(); ++i) {
        if (next_player_index == players[i]->get_player_number()) result += ">>";
        result += players[i]->get_status();
        if (next_player_index == players[i]->get_player_number()) result += "<<";
        result += " | ";
    }

    if (!is_alive()) result += "Team is dead.";
    else if (next == nullptr) result += "Team will be skipped.";
    return result;
}

#endif /* CHOPSTICKS_HPP */