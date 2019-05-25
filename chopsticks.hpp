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
    virtual ~Extremity() {}
    string get_type() { return type; }
    bool const is_alive() { return alive; }
    string const get_status() { return alive ? to_string(count) : "X"; }
    int const get_count() { return count; }
    int const get_max_count() { return max_count; }
    bool tap(Extremity &other);
    bool set_count(int new_count);
};

/**
 * assumes other is not a nullptr
 * @return true if this and other extremity is alive else false 
 */
bool Extremity::tap(Extremity &other) {
    if (!other.is_alive() || !this->is_alive()) return false;
    other.count += this->count;
    other.tapped();
    return true;
}

/* @return true if new count is in bounds and alive else false */
bool Extremity::set_count(int new_count) {
    if (!alive) return false;
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
    vector<Extremity *> hands;
    vector<Extremity *> feet;
    int max_fingers;
    int max_toes;
    ostream *output;
    istream *input;
    int turns;
    Extremity *get_ex(string mode);
    virtual void attacked_by(Player &other, Extremity &other_ex, Extremity &my_ex);
    vector<Extremity *> get_alive_extremities(string mode);

   public:
    Player(string type, int player_number, int hand_count, int foot_count,
           int finger_count, int toe_count, ostream *output = &cout, istream *input = &cin,
           int turns = 1);
    virtual ~Player();
    string get_type() { return type; }
    int const get_player_number() { return player_number; }
    int const get_team_number() { return team_number; }
    void set_team_number(int new_team_number) { team_number = new_team_number; }
    int const get_extremities_count(string mode, bool only_alive = false);
    int const get_turns() { return turns; }
    bool const is_alive() { return alive; }
    bool attack(Player &other, string my_stats, string other_stats);
    bool distribute(string mode, vector<int> change);
    virtual void skip_turn(bool force = false) { skip = true; }
    bool check_skip(bool modify_skip = false);
    bool can_make_an_action();
    string get_status();
    string play_with(vector<Player *> &all_players);
};

Player::Player(string type, int player_number, int hand_count, int foot_count,
               int finger_count, int toe_count, ostream *output, istream *input,
               int turns)
    : type(type), player_number(player_number), skip(false), alive(true), max_fingers(finger_count), max_toes(toe_count), output(output), input(input), turns(turns) {
    for (int i = 0; i < hand_count; ++i) {
        Hand *temp_hand = new Hand(finger_count);
        hands.push_back(temp_hand);
    }
    for (int i = 0; i < foot_count; ++i) {
        Foot *temp_foot = new Foot(toe_count);
        feet.push_back(temp_foot);
    }
}

Player::~Player() {
    for (auto &hand : hands) {
        delete hand;
    }
    for (auto &foot : feet) {
        delete foot;
    }
}

/* @return true if valid attack else false */
bool Player::attack(Player &other_player, string my_stats, string other_stats) {
    Extremity *other_ex = other_player.get_ex(other_stats);
    Extremity *my_ex = this->get_ex(my_stats);
    if (my_ex == nullptr) {
        outputTo(output, "Your chosen " + (string)(my_stats[0] == 'H' ? "hand" : "foot") + " is out of bounds.");
        return false;
    } else if (other_ex == nullptr) {
        outputTo(output, "Chosen target " + (string)(other_stats[0] == 'H' ? "hand" : "foot") + " is out of bounds.");
        return false;
    }
    if (!my_ex->is_alive()) {
        outputTo(output, "Your chosen " + my_ex->get_type() + " is dead.");
        return false;
    } else if (!other_ex->is_alive()) {
        outputTo(output, "Chosen target " + other_ex->get_type() + " is already dead.");
        return false;
    }
    if (my_ex->get_count() == 0) {
        outputTo(output, "Your free " + my_ex->get_type() + " cannot attack!");
        return false;
    }
    if (!my_ex->tap(*other_ex)) {
        outputTo(output, "Tap error.");
        return false;
    }
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
        extr = hands;
    } else if (mode == "feet") {
        extr = feet;
    } else {
        return false;
    }
    if (extr.size() != change.size()) return false;
    // check if original sum and new sum are equal
    int change_sum = 0, extremeties_sum = 0;
    for (size_t i = 0; i < extr.size(); ++i) {
        int temp_change = change[i];
        if (extr[i]->is_alive()) {
            if (!(0 <= temp_change && temp_change < extr[i]->get_max_count())) {
                outputTo(output, "Distribution out of valid bounds.");
                return false;
            }
        } else {
            continue;
        }
        change_sum += temp_change;
        extremeties_sum += extr[i]->get_count();
    }
    if (change_sum != extremeties_sum) {
        outputTo(output, "Distribution sum are not equal.");
        return false;
    }
    // modify
    bool changed = false;
    for (size_t i = 0; i < extr.size(); ++i) {
        if (extr[i]->is_alive()) {
            if (change[i] != extr[i]->get_count()) changed = true;
            extr[i]->set_count(change[i]);
        }
    }
    if (!changed) outputTo(output, "No change in distribution.");
    return changed;
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
    for (Extremity *&ex : hands) {
        if (ex->is_alive()) {
            alive = true;
            return;
        }
    }
    for (Extremity *&ex : feet) {
        if (ex->is_alive()) {
            alive = true;
            return;
        }
    }
}

vector<Extremity *> Player::get_alive_extremities(string mode) {
    vector<Extremity *> *extremities;
    if (mode == "hands") {
        extremities = &hands;
    } else if (mode == "feet") {
        extremities = &feet;
    }
    vector<Extremity *> alive_extremities;
    for (auto &extremity : *extremities) {
        if (extremity->is_alive()) {
            alive_extremities.push_back(extremity);
        }
    }
    return alive_extremities;
}

int const Player::get_extremities_count(string mode, bool only_alive) {
    vector<Extremity *> *extremities;
    if (mode == "hands") {
        extremities = &hands;
    } else if (mode == "feet") {
        extremities = &feet;
    }
    if (!only_alive) return extremities->size();
    int count = 0;
    for (auto &extremity : *extremities) {
        if (extremity->is_alive()) ++count;
    }
    return count;
}

/* finds out if player can do any action */
bool Player::can_make_an_action() {
    vector<Extremity *> hands_alive = this->get_alive_extremities("hands");
    vector<Extremity *> feet_alive = this->get_alive_extremities("feet");
    size_t free_hands = 0;
    size_t free_feet = 0;
    for (auto &hand : hands_alive) {
        if (hand->get_count() == 0) ++free_hands;
    }

    for (auto &foot : feet_alive) {
        if (foot->get_count() == 0) ++free_feet;
    }

    if (free_hands == hands_alive.size() && free_feet == feet_alive.size()) {
        return false;
    }
    return true;
}

/* @return valid pointer if valid parameter and in bounds else std::nullptr */
Extremity *Player::get_ex(string mode) {
    if (mode.size() != 2) return nullptr;
    unsigned int index = (unsigned int)mode[1] - (unsigned int)'A';
    if (mode[0] == 'H' && 0 <= index && index < hands.size()) {
        return hands[index];
    } else if (mode[0] == 'F' && 0 <= index && index < feet.size()) {
        return feet[index];
    }
    return nullptr;
}

string Player::get_status() {
    string result = "P" + to_string(player_number) + type[0] + " (";

    if (!alive) {
        result.pop_back();
        result += "[dead]";
        return result;
    }
    for (auto &hand : hands) {
        result += hand->get_status();
    }
    result += ":";
    for (auto &foot : feet) {
        result += foot->get_status();
    }
    result += ") [" + to_string(max_fingers) + ":" + to_string(max_toes) + "]";
    if (skip)
        result += " [skipping]";
    return result;
}

/**
 * assumes player is available to play
 * @return a string of action madde
 */
string Player::play_with(vector<Player *> &all_players) {
    string action_made;
    bool valid_action = false;
    while (!valid_action) {
        string action, line_string;
        outputTo(output, "Player " + to_string(get_player_number()) + ", enter your move. [tap | disthands | distfeet]");
        line_string = getlineFrom(input, output);
        istringstream line(line_string);
        line >> action;

        if (action == "tap") {
            if (!is_valid_string(line_string, 4)) {
                outputTo(output, "Please enter a valid number of arguments.");
                continue;
            }
            string from, to, player_num_arg;
            unsigned int player_number;
            line >> from >> player_num_arg >> to;

            if (from.size() != 2 || to.size() != 2 || (from[0] != 'H' && from[0] != 'F') || (to[0] != 'H' && to[0] != 'F')) {
                outputTo(output, "Please enter valid attack arguments");
                continue;
            }
            if (is_valid_int(player_num_arg)) {
                player_number = stoi(player_num_arg);
                if (!(1 <= player_number && player_number <= all_players.size())) {
                    outputTo(output, "Player number out of bounds! Enter action again.");
                    continue;
                }
            } else {
                outputTo(output, "Player number must be an integer! Enter action again.");
                continue;
            }
            Player *target = all_players[player_number - 1];
            if (!target->is_alive()) {
                outputTo(output, "Target player is dead. Enter action again.");
                continue;
            }
            if (target->get_team_number() == this->get_team_number()) {
                outputTo(output, "Friendly fire is not allowed! Enter action again.");
                continue;
            }
            if (!attack(*target, from, to)) continue;

        } else if (action == "disthands" || action == "distfeet") {
            int alive_hands_count = get_extremities_count("hands", true);
            int alive_feet_count = get_extremities_count("feet", true);
            if (!is_valid_string(line_string, (action == "disthands" ? alive_hands_count : alive_feet_count) + 1)) {
                outputTo(output, "Please enter a valid number of arguments.");
                continue;
            }
            if ((action == "disthands" ? alive_hands_count : alive_feet_count) <= 1) {
                outputTo(output, "Unable to redistribute with only 1 alive " + (string)(action == "disthands" ? "hand" : "foot") + ".");
                continue;
            }

            vector<int> changes(action == "disthands" ? get_extremities_count("hands") : get_extremities_count("feet"));
            vector<Extremity *> *extremities = action == "disthands" ? &hands : &feet;

            bool is_valid = true;
            for (size_t i = 0; i < changes.size(); ++i) {
                string to_check;
                if (extremities->at(i)->is_alive()) {
                    line >> to_check;
                    if (!is_valid_int(to_check)) {
                        is_valid = false;
                        break;
                    }
                    changes[i] = stoi(to_check);
                }
            }

            if (!is_valid) {
                outputTo(output, "Please enter integer arguments only after " + action + ".");
                continue;
            }

            if (!distribute(action == "disthands" ? "hands" : "feet", changes)) continue;

        } else {
            outputTo(output, "Invalid keyword! Try again.");
            continue;
        }
        action_made = line_string;
        valid_action = true;
    }
    return action_made;
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
            Hand *new_hand = new Hand(4);
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

   public:
    Team(int team_number, vector<ostream *> outputs = {});
    int get_team_number() { return team_number; }
    bool is_alive();
    int get_players_alive_count();
    void add_player(Player *new_player);
    string get_status(bool current_turn = false);
    Player *get_next_available_player(bool inplace = false, bool output_status = false);
    Player *get_next_player();
    Player *get_current_player() { return current_player; }
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
        if (player->is_alive()) ++result;
    }
    return result;
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
            } else if (!next_player->can_make_an_action()) {
                if (output_status) outputToAll(outputs, "Player " + to_string(next_player->get_player_number()) + " cannot do any action.");
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

Player *Team::get_next_player() {
    // if first turn
    if (current_player == nullptr) return players[0];
    return players[(current_player_index + 1) % players.size()];
}

string Team::get_status(bool current_turn) {
    int player_number = current_turn ? current_player->get_player_number() : get_next_player()->get_player_number();
    string result = "Team " + to_string(team_number) + " | ";
    for (size_t i = 0; i < players.size(); ++i) {
        if (player_number == players[i]->get_player_number()) result += ">>";
        result += players[i]->get_status();
        if (player_number == players[i]->get_player_number()) result += "<<";
        result += " | ";
    }
    if (current_turn) return result;
    if (!is_alive()) {
        result += "Team is dead.";
    } else if (get_next_available_player() == nullptr) {
        result += "Team will be skipped.";
    }
    return result;
}

#endif /* CHOPSTICKS_HPP */