
#include <iostream>

#include <rookmole/rookmole.h>
using namespace rockmole;

int main(int argc, char* argv[])
{
    srand(124);
    std::cout <<
        R"(                       __                  .__)" "\n"
        R"( _______  ____   ____ |  | __ _____   ____ |  |   ____)" "\n"
        R"( \_  __ \/  _ \ /  _ \|  |/ //     \ /  _ \|  | _/ __ \)" "\n"
        R"(  |  | \(  <_> |  <_> )    <|  Y Y  (  <_> )  |_\  ___/)" "\n"
        R"(  |__|   \____/ \____/|__|_ \__|_|  /\____/|____/\___  >)" "\n"
        R"(                           \/     \/                 \/)" << std::endl;

    auto human_player = Player::White;
    std::cout << "You play as: " << human_player << std::endl;

    auto s0 = make_start_state();
    auto lm0 = get_legal_moves(s0);
    auto ms = MoveState{std::move(s0), std::move(lm0), false};

    while (true)
    {
        std::cout << ms.state << std::endl;
        MoveCoord move_to_make{};
        if (ms.state.player_to_move == human_player) {
            while (true)
            {
                std::cout << "Your move: ";
                std::string user_move_str;
                std::cin >> user_move_str;
                auto user_moves = make_coord_moves(user_move_str);
                if (user_moves.size() != 1) {
                    std::cout << "You have to specify exactly one move, e.g.: d3:e3" << std::endl;
                    continue;
                }

                auto user_move = user_moves[0];
                bool is_move_legal = false;
                for (const auto legal_move : ms.next_moves) {
                    if (user_move == legal_move) {
                        is_move_legal = true;
                        break;
                    }
                }

                if (!is_move_legal) {
                    std::cout << "This move is illegal. Valid moves are:\n" << ms.next_moves << std::endl;
                    continue;
                }

                move_to_make = user_move;
                break;
            }
        }
        else {
            std::cout << "Possible moves: " << ms.next_moves << std::endl;
            if (!ms.next_moves.empty()) {
                move_to_make = ms.next_moves[rand() % ms.next_moves.size()];
            }
        }

        if (is_invalid(move_to_make.from)) {
            std::cout << "No move to make. Bye!" << std::endl;
            return 0;
        }

        std::cout << "Making move: " << move_to_make << std::endl;
        ms = make_move(ms.state, move_to_make);
    }

    return 0;
}
