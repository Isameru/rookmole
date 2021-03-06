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

#include "rookmole/evaluation.h"

namespace rookmole {

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

int evaluate_hardcode(Player eval_player, const GameNode& node) noexcept
{
    const auto& state = node.state;
    int score = 0;

    {
        const auto player_to_move = state.player_to_move;
        const auto score_mul = (player_to_move == eval_player) ? 1 : -1;

        if (node.next_moves.empty()) {
            if (node.king_in_check) {
                // Checkmate
                return -1000000 * score_mul;
            }
            else {
                // Stalemate
                return 0;  // A draw.
            }
        }

        score += 60 * score_mul;

        if (node.king_in_check) {
            score += -80;
        }

        auto opponent_king_coord = find_king(other_player(node.state.player_to_move), node.state);
        if (is_attacked_by(node.state.player_to_move, opponent_king_coord, node.state)) {
            score += 80;
        }

        score += 4 * node.next_moves.size();
    }

    state.foreach_piece([&](Coord c, Player p, Piece pc) {
        const auto score_mul = (p == eval_player) ? 1 : -1;
        auto add_points = [&](int points) { score += score_mul * points; };

        switch (pc) {
            case Pawn: {
                const auto advancement = is_white(p) ? (c.rank - 2) : (7 - c.rank);
                add_points(100 + 20 * advancement);
                break;
            }
            case Knight:
                add_points(300);
                break;
            case Bishop:
                add_points(320);
                break;
            case Rook:
                add_points(500);
                break;
            case Queen:
                add_points(800);
                break;
            default:
                break;
        }
    });

    return score;
}

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

} // namespace rookmole
