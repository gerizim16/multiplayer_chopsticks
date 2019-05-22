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
    string const get_status() { return alive ? to_string(count) + "/" + to_string(max_count) : "X"; }
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
    virtual void skip_turn(bool force = false);
    bool check_skip();
    string get_status();
    vector<string> play_with(vector<Player *> &all_players);
};

Player::Player(string type, int player_number, int hand_count, int foot_count,
               int finger_count, int toe_count, ostream *output, istream *input,
               int turns)
    : type(type), player_number(player_number), skip(false), alive(true), output(output), input(input), turns(turns) {
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

void Player::skip_turn(bool force) { skip = true; }

/* @return true if skipped else false */
bool Player::check_skip() {
    if (skip) {
        skip = false;
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
        result += " ";
    }
    result += ":";
    for (auto &foot : feet) {
        result += " ";
        result += foot.get_status();
    }
    result += ")";
    if (skip) result += " [skipping]";
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
    vector<ostream *> outputs;
    vector<Player *> players;
    int current_player_index;
    Player *current_player;
    bool get_next_available_player();

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
    if (!get_next_available_player()) {
        outputToAll(outputs, "Team " + to_string(team_number) + " has been skipped.");
        return false;
    }
    int player_index = current_player->get_player_number() - 1;
    outputToAll(outputs, "Waiting for player " + to_string(player_index + 1) + ".", outputs[player_index]);
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

/* @return true if there is a next available player else false */
bool Team::get_next_available_player() {
    // if first turn
    if (current_player == nullptr) {
        current_player = players[0];
        return true;
    }

    int fallback_index = current_player_index;

    int players_skipped = 0;
    // start checking with next player
    current_player_index = (current_player_index + 1) % players.size();
    current_player = players[current_player_index];
    // keep track last player who was checked
    Player *first_player_checked = current_player;

    // while not returning back to first check player
    do {
        if (current_player->is_alive()) {
            if (current_player->check_skip()) {
                outputToAll(outputs, "Player " + to_string(current_player->get_player_number()) + " has been skipped.");
                ++players_skipped;
            } else {
                break;
            }
        }
        // current player can't play so move to next
        current_player_index = (current_player_index + 1) % players.size();
        current_player = players[current_player_index];
    } while (current_player != first_player_checked);

    if (players_skipped == get_players_alive_count()) {  // all alive players skip turns
        current_player_index = fallback_index;
        current_player = players[current_player_index];
        return false;  // no available player in team to play
    }
    return true;
}

string Team::get_status() {
    string result = "Team " + to_string(team_number) + ": ";
    for (size_t i = 0; i < players.size(); ++i) {
        result += players[i]->get_status();
        result += " | ";
    }
    result.erase(result.end() - 3, result.end());
    return result;
}