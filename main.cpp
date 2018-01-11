#include <iostream>
#include <array>
#include <string>
#include <cctype>
#include <iomanip>
#include <vector>


#if defined(__unix__) || defined(__APPLE__)

#include <stdio.h>

void clear_screen(void) {
    printf("\x1B[2J");
}

#elif defined(_WIN32)

#include <windows.h>

void clear_screen(void) {
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = {0, 0};
    DWORD dwCount, dwSize;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    dwSize = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hOutput, 0x20, dwSize, topLeft, &dwCount);
    FillConsoleOutputAttribute(hOutput, 0x07, dwSize, topLeft, &dwCount);
    SetConsoleCursorPosition(hStdOut, topLeft);
}

#endif /* __unix__ */


enum character {
    nothing,
    goblin
};

struct position {
    std::size_t x;
    std::size_t y;

    bool operator==(position const &other) const {
        return x == other.x and y == other.y;
    }

    bool operator!=(position const &other) const {
        return not(*this == other);
    }
};

struct delta {
    int x;
    int y;
};


struct maze {
    static constexpr std::size_t width = 10;
    static constexpr std::size_t height = 10;
    static constexpr std::size_t extent = width * height;

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
        auto is_positive = [](int n) { return n > 0; };
        auto is_negative = [](int n) { return n < 0; };
        auto append_message = [&](auto &&s) {
            if (not this->recent_message_.empty())
                this->recent_message_ += ", ";
            this->recent_message_ += s;
        };

        auto old_pos = goblin_pos_;
        recent_message_.clear();
        auto x_mag = std::size_t(std::abs(d.x));
        if (is_negative(d.x)) {
            if (x_mag > goblin_pos_.x) {
                append_message("that move would take me into the left wall");
            } else {
                goblin_pos_.x -= x_mag;
            }
        } else if (is_positive(d.x)) {
            if (x_mag + goblin_pos_.x >= maze_.width) {
                append_message("that move would take me into the right wall");
            } else {
                goblin_pos_.x += x_mag;
            }
        }

        auto y_mag = std::size_t(std::abs(d.y));
        if (is_negative(d.y)) {
            if (y_mag > goblin_pos_.y) {
                append_message("that move would take me into the top wall");
            } else {
                goblin_pos_.y -= y_mag;
            }
        } else if (is_positive(d.y)) {
            if (y_mag + goblin_pos_.y >= maze_.height) {
                append_message("that move would take me into the bottom wall");
            } else {
                goblin_pos_.y += y_mag;
            }
        }
        if (goblin_pos_ != old_pos) {
            maze_.at(old_pos) = nothing;
            maze_.at(goblin_pos_) = goblin;
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
            {"quit", [&] { this->quit_signal_ = true;}},
            {"help", [&] { this->help(); }}
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

    void help()
    {
        const char* sep = "";
        std::string msg = "commands I understand are: ";
        for (auto&& e : entries_)
        {
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