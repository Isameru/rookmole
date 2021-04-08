/*
  MIT License
  Copyright (c) Mariusz Łapiński <gmail:isameru>

   ██████╗  ██████╗  ██████╗ ██╗  ██╗███╗   ███╗ ██████╗ ██╗     ███████╗
   ██╔══██╗██╔═══██╗██╔═══██╗██║ ██╔╝████╗ ████║██╔═══██╗██║     ██╔════╝
   ██████╔╝██║   ██║██║   ██║█████╔╝ ██╔████╔██║██║   ██║██║     █████╗
   ██╔══██╗██║   ██║██║   ██║██╔═██╗ ██║╚██╔╝██║██║   ██║██║     ██╔══╝
   ██║  ██║╚██████╔╝╚██████╔╝██║  ██╗██║ ╚═╝ ██║╚██████╔╝███████╗███████╗
   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚══════╝
*/

#include <iostream>
#include <optional>

#include <rookmole/rookmole.h>
using namespace rookmole;

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

int main(int argc, char* argv[])
{
    srand(12345);

    std::cout << "\n"
        R"( ██████╗  ██████╗  ██████╗ ██╗  ██╗███╗   ███╗ ██████╗ ██╗     ███████╗)" "\n"
        R"( ██╔══██╗██╔═══██╗██╔═══██╗██║ ██╔╝████╗ ████║██╔═══██╗██║     ██╔════╝)" "\n"
        R"( ██████╔╝██║   ██║██║   ██║█████╔╝ ██╔████╔██║██║   ██║██║     █████╗)" "\n"
        R"( ██╔══██╗██║   ██║██║   ██║██╔═██╗ ██║╚██╔╝██║██║   ██║██║     ██╔══╝)" "\n"
        R"( ██║  ██║╚██████╔╝╚██████╔╝██║  ██╗██║ ╚═╝ ██║╚██████╔╝███████╗███████╗)" "\n"
        R"( ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚══════╝)" "\n" << std::endl;

    auto human_player = Player::White;
    std::cout << "You play as: " << human_player << std::endl;

    auto s0 = make_start_state();
    auto lm0 = get_legal_moves(s0);
    auto n = GameNode{std::move(s0), std::move(lm0), false};

    while (true)
    {
        std::cout << n.state << std::endl;
        std::cout << "Possible moves: " << n.next_moves << std::endl;

        std::optional<MoveCoord> move_to_make_opt{};

        if (!is_terminal(n)) {
            if (n.state.player_to_move == human_player) {
                while (true)
                {
                    std::cout << "Your move: ";
                    std::string user_move_text;
                    std::cin >> user_move_text;
                    const auto user_move = make_move_coord(user_move_text);

                    bool is_move_legal = false;
                    for (const auto legal_move : n.next_moves) {
                        if (user_move == legal_move) {
                            is_move_legal = true;
                            break;
                        }
                    }

                    if (!is_move_legal) {
                        std::cout << "This move is illegal. Valid moves are:\n" << n.next_moves << std::endl;
                        continue;
                    }

                    move_to_make_opt = user_move;
                    break;
                }
            }
            else {
                std::cout << "Thinking..." << std::endl;
                const auto search_result = alphabeta(n, 6);
                move_to_make_opt = search_result.move;
            }
        }

        if (!move_to_make_opt) {
            std::cout << "No moves to make." << std::endl;
            if (n.king_in_check) {
                std::cout << "The winner is " << other_player(n.state.player_to_move) << "!" << std::endl;
            }
            else {
                std::cout << "The match ends with a draw." << std::endl;
            }

            return 0;
        }

        const auto move_to_make = *move_to_make_opt;

        std::cout << "Making move: " << move_to_make << std::endl;
        n = make_move(n.state, move_to_make);
    }

    return 0;
}

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-
