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
   public:
    enum Type { HAND,
                FOOT };

   private:
    enum Extremity::Type type;
    virtual void isTapped() = 0;  // different ways of how hand and foot resolves a tap

   protected:
    int max_count;
    bool alive = true;
    int count = 1;

   public:
    Extremity(int max_count, enum Extremity::Type type)
        : type(type), max_count(max_count) {}
    virtual ~Extremity() {}
    enum Extremity::Type getType() { return type; }
    string getName() { return type == HAND ? "hand" : "foot"; }
    bool const isAlive() { return alive; }
    string const getStatus() { return alive ? to_string(count) : "X"; }
    int const getCount() { return count; }
    int const getMaxCount() { return max_count; }
    bool tap(Extremity &other);
    bool setCount(int new_count);
};

/**
 * assumes other is not a nullptr
 * @return true if this and other extremity is alive else false 
 */
bool Extremity::tap(Extremity &other) {
    if (!other.isAlive() || !this->isAlive()) return false;
    other.count += this->count;
    other.isTapped();
    return true;
}

/* @return true if new count is in bounds and alive else false */
bool Extremity::setCount(int new_count) {
    if (!alive) return false;
    if (0 <= new_count && new_count < max_count) {
        count = new_count;
        return true;
    }
    return false;
}

class Hand : public Extremity {
   public:
    Hand(int max_count) : Extremity(max_count, HAND) {}
    void isTapped() override;
};

void Hand::isTapped() {
    if (count == max_count) {
        alive = false;
    } else {
        count %= max_count;
    }
}

class Foot : public Extremity {
   public:
    Foot(int max_count) : Extremity(max_count, FOOT) {}
    void isTapped() override;
};

void Foot::isTapped() {
    if (count >= max_count) {
        alive = false;
    }
}

class Player {
   public:
    enum Type { HUMAN,
                ALIEN,
                ZOMBIE,
                DOGGO };

   private:
    Player::Type type;
    string name;
    int player_number;
    int team_number = -1;
    bool skip = false;
    bool alive = true;
    int max_fingers;
    int max_toes;
    ostream *output;
    istream *input;
    int turns;
    Extremity *getExtremity(string raw);
    vector<Extremity *> getAliveExtremities(enum Extremity::Type mode);

   protected:
    vector<Extremity *> hands;
    vector<Extremity *> feet;
    virtual void attackedBy(Player &other, Extremity &other_ex, Extremity &my_ex);

   public:
    Player(Player::Type type, int player_number, int hand_count, int foot_count,
           int finger_count, int toe_count, ostream *output = &cout, istream *input = &cin,
           int turns = 1);
    virtual ~Player();
    Player::Type getType() { return type; }
    string getName() { return name; }
    int const getPlayerNumber() { return player_number; }
    int const getTeamNumber() { return team_number; }
    void setTeamNumber(int new_team_number) { team_number = new_team_number; }
    int const getExtremitiesCount(enum Extremity::Type mode, bool only_alive = false);
    int const getTurns() { return turns; }
    bool const isAlive() { return alive; }
    size_t getHandsSize() { return hands.size(); }
    size_t getFeetSize() { return feet.size(); }
    bool attack(Player &other, string my_stats, string other_stats);
    bool distribute(enum Extremity::Type mode, vector<int> change);
    virtual void skipTurn(bool force = false) { skip = true; }
    void hasBeenSkipped() { skip = false; }
    bool isSkipping() { return skip; }
    bool canMakeAnAction();
    string getStatus();
    string playWith(vector<Player *> &all_players);
};

Player::Player(Player::Type type, int player_number, int hand_count, int foot_count,
               int finger_count, int toe_count, ostream *output, istream *input,
               int turns)
    : type(type), player_number(player_number), max_fingers(finger_count), max_toes(toe_count), output(output), input(input), turns(turns) {
    for (int i = 0; i < hand_count; ++i) {
        Hand *temp_hand = new Hand(finger_count);
        hands.push_back(temp_hand);
    }
    for (int i = 0; i < foot_count; ++i) {
        Foot *temp_foot = new Foot(toe_count);
        feet.push_back(temp_foot);
    }
    if (type == HUMAN) {
        name = "human";
    } else if (type == ALIEN) {
        name = "alien";
    } else if (type == ZOMBIE) {
        name = "zombie";
    } else if (type == DOGGO) {
        name = "doggo";
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
    Extremity *other_ex = other_player.getExtremity(other_stats);
    Extremity *my_ex = this->getExtremity(my_stats);
    if (my_ex == nullptr) {
        outputTo(output, "Your chosen " + (string)(my_stats[0] == 'H' ? "hand" : "foot") + " is out of bounds.");
        return false;
    } else if (other_ex == nullptr) {
        outputTo(output, "Chosen target " + (string)(other_stats[0] == 'H' ? "hand" : "foot") + " is out of bounds.");
        return false;
    }
    if (!my_ex->isAlive()) {
        outputTo(output, "Your chosen " + my_ex->getName() + " is dead.");
        return false;
    } else if (!other_ex->isAlive()) {
        outputTo(output, "Chosen target " + other_ex->getName() + " is already dead.");
        return false;
    }
    if (my_ex->getCount() == 0) {
        outputTo(output, "Your free " + my_ex->getName() + " cannot attack!");
        return false;
    }
    if (!my_ex->tap(*other_ex)) {
        outputTo(output, "Tap error.");
        return false;
    }
    if (other_ex->getType() == Extremity::FOOT && !other_ex->isAlive()) {
        other_player.skipTurn();
    }
    other_player.attackedBy(*this, *my_ex, *other_ex);
    return true;
}

/* @return true if valid mode and distribution else false */
bool Player::distribute(enum Extremity::Type mode, vector<int> change) {
    vector<Extremity *> extr;
    if (mode == Extremity::HAND) {
        extr = hands;
    } else if (mode == Extremity::FOOT) {
        extr = feet;
    } else {
        return false;
    }
    if (extr.size() != change.size()) return false;
    // check if original sum and new sum are equal
    int change_sum = 0, extremeties_sum = 0;
    for (size_t i = 0; i < extr.size(); ++i) {
        int temp_change = change[i];
        if (extr[i]->isAlive()) {
            if (!(0 <= temp_change && temp_change < extr[i]->getMaxCount())) {
                outputTo(output, "Distribution out of valid bounds.");
                return false;
            }
        } else {
            continue;
        }
        change_sum += temp_change;
        extremeties_sum += extr[i]->getCount();
    }
    if (change_sum != extremeties_sum) {
        outputTo(output, "Distribution sum are not equal.");
        return false;
    }
    // modify
    bool changed = false;
    for (size_t i = 0; i < extr.size(); ++i) {
        if (extr[i]->isAlive()) {
            if (change[i] != extr[i]->getCount()) changed = true;
            extr[i]->setCount(change[i]);
        }
    }
    if (!changed) outputTo(output, "No change in distribution.");
    return changed;
}

void Player::attackedBy(Player &other, Extremity &other_ex,
                        Extremity &my_ex) {
    alive = false;
    for (Extremity *&ex : hands) {
        if (ex->isAlive()) {
            alive = true;
            return;
        }
    }
    for (Extremity *&ex : feet) {
        if (ex->isAlive()) {
            alive = true;
            return;
        }
    }
}

vector<Extremity *> Player::getAliveExtremities(enum Extremity::Type mode) {
    vector<Extremity *> *extremities;
    if (mode == Extremity::HAND) {
        extremities = &hands;
    } else if (mode == Extremity::FOOT) {
        extremities = &feet;
    }
    vector<Extremity *> alive_extremities;
    for (auto &extremity : *extremities) {
        if (extremity->isAlive()) {
            alive_extremities.push_back(extremity);
        }
    }
    return alive_extremities;
}

int const Player::getExtremitiesCount(enum Extremity::Type mode, bool only_alive) {
    vector<Extremity *> *extremities;
    if (mode == Extremity::HAND) {
        extremities = &hands;
    } else if (mode == Extremity::FOOT) {
        extremities = &feet;
    }
    if (!only_alive) return extremities->size();
    int count = 0;
    for (auto &extremity : *extremities) {
        if (extremity->isAlive()) ++count;
    }
    return count;
}

/* finds out if player can do any action */
bool Player::canMakeAnAction() {
    vector<Extremity *> hands_alive = this->getAliveExtremities(Extremity::HAND);
    vector<Extremity *> feet_alive = this->getAliveExtremities(Extremity::FOOT);
    size_t free_hands = 0;
    size_t free_feet = 0;
    for (auto &hand : hands_alive) {
        if (hand->getCount() == 0) ++free_hands;
    }

    for (auto &foot : feet_alive) {
        if (foot->getCount() == 0) ++free_feet;
    }

    if (free_hands == hands_alive.size() && free_feet == feet_alive.size()) {
        return false;
    }
    return true;
}

/* @return valid pointer if valid parameter and in bounds else std::nullptr */
Extremity *Player::getExtremity(string mode) {
    if (mode.size() != 2) return nullptr;
    unsigned int index = (unsigned int)mode[1] - (unsigned int)'A';
    if (mode[0] == 'H' && 0 <= index && index < hands.size()) {
        return hands[index];
    } else if (mode[0] == 'F' && 0 <= index && index < feet.size()) {
        return feet[index];
    }
    return nullptr;
}

string Player::getStatus() {
    string result = "P" + to_string(player_number) + name[0] + " (";

    if (!alive) {
        result.pop_back();
        result += "[dead]";
        return result;
    }
    for (auto &hand : hands) {
        result += hand->getStatus();
    }
    result += ":";
    for (auto &foot : feet) {
        result += foot->getStatus();
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
string Player::playWith(vector<Player *> &all_players) {
    string action_made;
    bool valid_action = false;
    while (!valid_action) {
        string action, line_string;
        outputTo(output, "Player " + to_string(getPlayerNumber()) + ", enter your move. [tap | disthands | distfeet]");
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
            if (!target->isAlive()) {
                outputTo(output, "Target player is dead. Enter action again.");
                continue;
            }
            if (target->getTeamNumber() == this->getTeamNumber()) {
                outputTo(output, "Friendly fire is not allowed! Enter action again.");
                continue;
            }
            if (!attack(*target, from, to)) continue;

        } else if (action == "disthands" || action == "distfeet") {
            int alive_hands_count = getExtremitiesCount(Extremity::HAND, true);
            int alive_feet_count = getExtremitiesCount(Extremity::FOOT, true);
            if (!is_valid_string(line_string, (action == "disthands" ? alive_hands_count : alive_feet_count) + 1)) {
                outputTo(output, "Please enter a valid number of arguments.");
                continue;
            }
            if ((action == "disthands" ? alive_hands_count : alive_feet_count) <= 1) {
                outputTo(output, "Unable to redistribute with only 1 alive " + (string)(action == "disthands" ? "hand" : "foot") + ".");
                continue;
            }

            vector<int> changes(action == "disthands" ? getExtremitiesCount(Extremity::HAND) : getExtremitiesCount(Extremity::FOOT));
            vector<Extremity *> *extremities = action == "disthands" ? &hands : &feet;

            bool is_valid = true;
            for (size_t i = 0; i < changes.size(); ++i) {
                string to_check;
                if (extremities->at(i)->isAlive()) {
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

            if (!distribute(action == "disthands" ? Extremity::HAND : Extremity::FOOT, changes)) continue;

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
        : Player(HUMAN, player_number, 2, 2, 5, 5, output, input) {}
};

class Alien : public Player {
   public:
    Alien(int player_number, ostream *output = &cout, istream *input = &cin)
        : Player(ALIEN, player_number, 4, 2, 3, 2, output, input) {}
    void skipTurn(bool force = false) override {
        if (force) Player::skipTurn();
    }
};

class Zombie : public Player {
   public:
    Zombie(int player_number, ostream *output = &cout, istream *input = &cin)
        : Player(ZOMBIE, player_number, 1, 0, 4, 0, output, input, 2) {}
    void attackedBy(Player &other, Extremity &other_ex, Extremity &my_ex) override {
        if (hands.size() == 1 && !my_ex.isAlive()) {  // starting hand dies
            Hand *new_hand = new Hand(4);
            hands.push_back(new_hand);
        }
        Player::attackedBy(other, other_ex, my_ex);
    }
};

class Doggo : public Player {
   public:
    Doggo(int player_number, ostream *output = &cout, istream *input = &cin)
        : Player(DOGGO, player_number, 0, 4, 0, 4, output, input) {}
    void attackedBy(Player &other, Extremity &other_ex, Extremity &my_ex) override {
        Player::attackedBy(other, other_ex, my_ex);
        if (other.getType() != this->getType()) {  // not doggo type
            other.skipTurn(true);
        }
    }
};

class Team {
   private:
    int team_number;
    vector<Player *> players;
    int current_player_index;
    Player *current_player;

   public:
    Team(int team_number);
    int getTeamNumber() { return team_number; }
    bool isAlive();
    bool isSkipping();
    void skip();
    int getPlayersAliveCount();
    void addPlayer(Player *new_player);
    string getStatus();
    string getCurrentStatus();
    Player *getNextAlivePlayer();
    Player *getAndSetNextAlivePlayer();
    Player *getCurrentPlayer() { return current_player; }
};

Team::Team(int team_number) : team_number(team_number), current_player_index(0), current_player(nullptr) {
}

bool Team::isAlive() {
    for (auto &player : players) {
        if (player->isAlive()) {
            return true;
        }
    }
    return false;
}

bool Team::isSkipping() {
    for (auto &player : players) {
        if (!player->isSkipping() && player->isAlive() && player->canMakeAnAction()) return false;
    }
    return true;
}

void Team::skip() {
    for (auto &player : players) {
        player->hasBeenSkipped();
    }
}

int Team::getPlayersAliveCount() {
    int result = 0;
    for (auto &player : players) {
        if (player->isAlive()) ++result;
    }
    return result;
}

void Team::addPlayer(Player *new_player) {
    players.push_back(new_player);
}

Player *Team::getAndSetNextAlivePlayer() {
    if (!isAlive()) return nullptr;
    // if first turn
    if (current_player == nullptr) {
        current_player = players[0];
        return current_player;
    }
    do {
        current_player_index = (current_player_index + 1) % players.size();
        current_player = players[current_player_index];
    } while (!current_player->isAlive());
    return current_player;
}

Player *Team::getNextAlivePlayer() {
    if (!isAlive()) return nullptr;
    // if first turn
    if (current_player == nullptr) {
        return players[0];
    }
    Player *next_player = current_player;
    int next_player_index = current_player_index;
    do {
        next_player_index = (next_player_index + 1) % players.size();
        next_player = players[next_player_index];
    } while (!next_player->isAlive());
    return next_player;
}

string Team::getStatus() {
    Player *next_alive_player = getNextAlivePlayer();
    int current_player_number = next_alive_player != nullptr ? next_alive_player->getPlayerNumber() : -1;
    string result = "Team " + to_string(team_number) + " | ";
    for (size_t i = 0; i < players.size(); ++i) {
        if (current_player_number == players[i]->getPlayerNumber()) result += ">>";
        result += players[i]->getStatus();
        if (current_player_number == players[i]->getPlayerNumber()) result += "<<";
        result += " | ";
    }
    if (next_alive_player == nullptr) {
        result += "Team is dead.";
    } else if (isSkipping()) {
        result += "Team will be skipped.";
    }
    return result;
}

string Team::getCurrentStatus() {
    int current_player_number = current_player->getPlayerNumber();
    string result = "Team " + to_string(team_number) + " | ";
    for (size_t i = 0; i < players.size(); ++i) {
        if (current_player_number == players[i]->getPlayerNumber()) result += ">>";
        result += players[i]->getStatus();
        if (current_player_number == players[i]->getPlayerNumber()) result += "<<";
        result += " | ";
    }
    return result;
}

#endif /* CHOPSTICKS_HPP */