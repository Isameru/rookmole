
#include "rookmole/state.h"

namespace rockmole
{
    MoveCoordList make_coord_moves(const std::string& s) {
        MoveCoordList out;
        Coord from;
        Coord to;
        Coord* curr = &from;

        auto commit_move = [&] {
            if (!is_valid(from) || !is_valid(to)) return;
            out.push_back(MoveCoord{from, to});
            from = Coord::invalid();
            to = Coord::invalid();
            curr = &from;
        };

        for (char c : s) {
            if (c >= '1' && c <= '8') {
                curr->rank = c - '0';
            }
            else if (c >= 'a' && c <= 'h') {
                curr->file = c - 'a' + 1;
            }
            else if (c == ':') {
                assert(curr == &from);
                curr = &to;
            }
            else if (c == ' ') {
                commit_move();
            }
            else {
                assert(false);
            }
        }
        commit_move();
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const State& state)
    {
        const auto border_bg_code = "44";
        const auto border_fg_code = "93";
        const auto headline_fg_code = "93";

        auto headline = std::string{"Move "};
        headline.append(std::to_string(state.allmove_count / 2));
        headline.append(" for ");
        headline.append(to_string(state.player_to_move));

        out << "\033[1;" << border_bg_code << "m  " << headline;
        for (int i = (int)headline.size(); i < 27; ++i) out << " ";
        out << "\033[0m\n";

        bool light_square = true;

        for (uint8_t rank = 8; rank >= 1; --rank) {
            out << "\033[0;" << border_bg_code << ';' << border_fg_code << "m " << std::to_string(rank) << ' ';
            for (uint8_t file = 1; file <= 8; ++file) {
                const auto bg_code = (light_square ? "47" : "100");
                const auto sq = state.get_square({rank, file});
                const auto fg_code = (is_white(sq) ? "1;37" : "0;30");
                const char piece_char = [sq] {
                    switch (sq) {
                        case Empty:         return ' ';
                        case WhitePawn:     return 'p';
                        case WhiteKnight:   return 'N';
                        case WhiteBishop:   return 'B';
                        case WhiteRook:     return 'R';
                        case WhiteQueen:    return 'Q';
                        case WhiteKing:     return 'K';
                        case BlackPawn:     return 'p';
                        case BlackKnight:   return 'N';
                        case BlackBishop:   return 'B';
                        case BlackRook:     return 'R';
                        case BlackQueen:    return 'Q';
                        case BlackKing:     return 'K';
                    }
                    return '?';
                }();
                out << "\033[" << fg_code << ';' << bg_code << "m " << piece_char << ' ';
                light_square ^= true;
            }
            out << "\033[0;" << border_bg_code << "m  \033[0m\n";
            light_square ^= true;
        }

        return out << "\033[0;" << border_bg_code << ';' << border_fg_code << "m    a  b  c  d  e  f  g  h   \033[0m";
    }

    State make_start_state(Player starting_player)
    {
        auto state = State{};
        auto put_piece = [&state](Coord c, Square sq) {
            state.set_square(c, sq);
            state.set_square(Coord{(uint8_t)(9 - c.rank), c.file}, other_color(sq));
        };

        for (uint8_t file = 1; file <= 8; ++file) {
            put_piece({2, file}, Square::WhitePawn);
        }

        put_piece({1, 1}, Square::WhiteRook);
        put_piece({1, 2}, Square::WhiteKnight);
        put_piece({1, 3}, Square::WhiteBishop);
        put_piece({1, 4}, Square::WhiteQueen);
        put_piece({1, 5}, Square::WhiteKing);
        put_piece({1, 6}, Square::WhiteBishop);
        put_piece({1, 7}, Square::WhiteKnight);
        put_piece({1, 8}, Square::WhiteRook);

        state.white_short_castling_possible = true;
        state.white_long_castling_possible = true;
        state.black_short_castling_possible = true;
        state.black_long_castling_possible = true;
        state.player_to_move = starting_player;

        return state;
    }

    State make_custom_state(std::string placement, Player player_to_move, bool reverse_players)
    {
        auto s = State{};
        auto sv = SquareDynView{s, reverse_players ? Player::Black : Player::White};

        s.player_to_move = reverse_players ? other_player(player_to_move) : player_to_move;

        bool white = true;
        int rank = 0;
        int file = 0;
        Square sq = Square::Empty;

        auto commit_piece = [&] {
            if (rank == 0 && file == 0 && sq == Square::Empty) return;
            if (rank != 0 && file != 0 && sq != Square::Empty) {
                sv.set({rank, file}, sq);
                rank = 0;
                file = 0;
                sq = Square::Empty;
            }
            else {
                assert(false);
            }
        };

        for (char c : placement) {
            if (c >= '1' && c <= '8') {
                assert(rank == 0);
                rank = c - '0';
            }
            else if (c >= 'a' && c <= 'h') {
                assert(file == 0);
                file = c - 'a' + 1;
            }
            else if (c == 'p') {
                assert(is_empty(sq));
                sq = white ? Square::WhitePawn : Square::BlackPawn;
            }
            else if (c == 'N') {
                assert(is_empty(sq));
                sq = white ? Square::WhiteKnight : Square::BlackKnight;
            }
            else if (c == 'B') {
                assert(is_empty(sq));
                sq = white ? Square::WhiteBishop : Square::BlackBishop;
            }
            else if (c == 'R') {
                assert(is_empty(sq));
                sq = white ? Square::WhiteRook : Square::BlackRook;
            }
            else if (c == 'Q') {
                assert(is_empty(sq));
                sq = white ? Square::WhiteQueen : Square::BlackQueen;
            }
            else if (c == 'K') {
                assert(is_empty(sq));
                sq = white ? Square::WhiteKing : Square::BlackKing;
            }
            else if (c == ' ') {
                commit_piece();
            }
            else if (c == '|') {
                assert(white);
                white = false;
            }
            else {
                assert(false);
            }
        }

        commit_piece();

        return s;

    }
}
