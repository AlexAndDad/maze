#include "clear_screen.hpp"
#include "character.hpp"
#include "position.hpp"

#include <iostream>
#include <array>
#include <string>
#include <cctype>
#include <iomanip>
#include <vector>


struct maze {
    static constexpr std::size_t width = 10;
    static constexpr std::size_t height = 10;
    static constexpr std::size_t extent = width * height;

    static constexpr bounding_box bounds() {
        return bounding_box {position {0, 0}, delta {width, height}};
    }

    maze()
            : maze_data_{} {}

    void add_item(position p, character c) {
        at(p.x, p.y) = c;
    }

    character &at(std::size_t x, std::size_t y) {
        return maze_data_[y * width + x];
    }

    character &at(position pos) {
        return at(pos.x, pos.y);
    }

    character const &at(position pos) const {
        return at(pos.x, pos.y);
    }

    character const &at(std::size_t x, std::size_t y) const {
        return maze_data_[y * width + x];
    }


    std::array<character, extent> maze_data_;
};

void redraw(maze const &m) {

    auto draw_horizontal = [&]() {
        std::cout << "+";
        for (std::size_t i = 0; i < m.width; ++i)
            std::cout << "-+";
        std::cout << "\n";
    };

    auto draw_row = [&](std::size_t y) {
        std::cout << '|';
        for (std::size_t x = 0; x < m.width; ++x) {
            switch (m.at(x, y)) {
                case goblin:
                    std::cout << 'G';
                    break;
                case nothing:
                    std::cout << ' ';
                    break;
                default:
                    std::cout << '?';
                    break;
            }
            std::cout << '|';
        }
        std::cout << '\n';
    };

    // top axis is N cells plus N+1 borders
    draw_horizontal();
    for (std::size_t y = 0; y < m.height; ++y) {
        draw_row(y);
        draw_horizontal();
    }
}

void draw_recent_message(std::string const &message) {
    if (message.empty()) {
        std::cout << "\n\n";
    } else {
        std::cout << "\nGoblin: " << std::quoted(message) << '\n';

    }
}

struct world {
    world()
            : maze_{}, recent_message_{}, goblin_pos_{5, 5} {
        maze_.at(goblin_pos_) = goblin;
    }

    void move_goblin(delta d) {
        auto append_message = [&](auto &&s) {
            if (not this->recent_message_.empty())
                this->recent_message_ += ", ";
            this->recent_message_ += s;
        };

        recent_message_.clear();
        bool move_error = false;
        auto new_pos = goblin_pos_ + d;
        switch (test_limits_x(new_pos, maze_.bounds())) {
            case 1:
                append_message("that move would take me into the right wall");
                move_error = true;
                break;
            case -1:
                append_message("that move would take me into the left wall");
                move_error = true;
                break;
            default:
                break;
        }

        switch (test_limits_y(new_pos, maze_.bounds())) {
            case 1:
                append_message("that move would take me into the bottom wall");
                move_error = true;
                break;
            case -1:
                append_message("that move would take me into the top wall");
                move_error = true;
                break;
            default:
                break;
        }

        if (not move_error and new_pos != goblin_pos_) {
            maze_.at(goblin_pos_) = nothing;
            maze_.at(new_pos) = goblin;
            goblin_pos_ = new_pos;
        }

    }

    maze maze_;
    std::string recent_message_;
    position goblin_pos_;
};

void redraw(world const &my_world) {
    clear_screen();
    redraw(my_world.maze_);
    draw_recent_message(my_world.recent_message_);
}

struct command_iterpreter {
    command_iterpreter(world &w)
            : world_(w), entries_{
            {"up",    [&] { this->world_.move_goblin(delta {0, -1}); }},
            {"down",  [&] { this->world_.move_goblin(delta {0, 1}); }},
            {"left",  [&] { this->world_.move_goblin(delta {-1, 0}); }},
            {"right", [&] { this->world_.move_goblin(delta {1, 0}); }},
            {"quit",  [&] { this->quit_signal_ = true; }},
            {"help",  [&] { this->help(); }}
    },
              quit_signal_{false} {}

    struct entry {
        std::string command;
        std::function<void()> action;
    };

    std::vector<std::size_t> find_candidates(std::string const &cmd) {
        std::vector<std::size_t> result;
        for (std::size_t i = 0; i < entries_.size(); ++i) {
            auto comp = [](std::string l, std::string r) -> bool {
                std::transform(std::begin(l), std::end(l), std::begin(l), [](auto ch) { return std::toupper(ch); });
                std::transform(std::begin(r), std::end(r), std::begin(r), [](auto ch) { return std::toupper(ch); });
                auto check = l.substr(0, r.size());
                return check == r;
            };

            if (comp(entries_[i].command, cmd))
                result.push_back(i);
        }
        return result;
    }

    void help() {
        const char *sep = "";
        std::string msg = "commands I understand are: ";
        for (auto &&e : entries_) {
            msg += sep;
            msg += e.command;
            sep = ", ";
        }
        world_.recent_message_ = msg;
    }

    void perform(std::string const &cmd) {
        auto candidates = find_candidates(cmd);
        if (candidates.empty()) {
            world_.recent_message_ = "I do not understand you, Sire...";
        } else if (candidates.size() > 1) {
            world_.recent_message_ = "Your command seems ambiguous, Sire...";
        } else {
            entries_[candidates[0]].action();
        }
    }

    bool has_quit() const {
        return quit_signal_;
    }

    world &world_;
    std::vector<entry> entries_;
    bool quit_signal_;
};

int main() {
    world the_world;
    command_iterpreter interp{the_world};

    while (not interp.has_quit()) {
        redraw(the_world);

        std::cout << "\nwhat is thy command? : ";
        std::string cmd;
        std::cin.clear();
        std::cin >> cmd;
        interp.perform(cmd);
    }
    std::cout << "fine!\n";
    return 0;
}