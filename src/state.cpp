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

#include "rookmole/state.h"

namespace rookmole {

MoveCoordVec make_coord_moves(std::string_view text, bool reverse) {
    MoveCoordVec out;

    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == ' ') { ++i; continue; }
        assert(i + 5 <= text.size());
        const auto mc = MoveCoord{text.substr(i, 5)};
        out.push_back(!reverse ? mc : other_player(mc));
        i += 6;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const GameState& state)
{
    const auto border_bg_code = "44";
    const auto border_fg_code = "93";
    const auto headline_fg_code = "93";

    auto headline = std::string{"Move "};
    headline.append(std::to_string(state.move_count));
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
            const auto sq = state.get_square({file, rank});
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

GameState make_start_state()
{
    auto state = GameState{};

    state.set_square({1, 1}, Square::WhiteRook);
    state.set_square({2, 1}, Square::WhiteKnight);
    state.set_square({3, 1}, Square::WhiteBishop);
    state.set_square({4, 1}, Square::WhiteQueen);
    state.set_square({5, 1}, Square::WhiteKing);
    state.set_square({6, 1}, Square::WhiteBishop);
    state.set_square({7, 1}, Square::WhiteKnight);
    state.set_square({8, 1}, Square::WhiteRook);

    state.set_square({1, 8}, Square::BlackRook);
    state.set_square({2, 8}, Square::BlackKnight);
    state.set_square({3, 8}, Square::BlackBishop);
    state.set_square({4, 8}, Square::BlackQueen);
    state.set_square({5, 8}, Square::BlackKing);
    state.set_square({6, 8}, Square::BlackBishop);
    state.set_square({7, 8}, Square::BlackKnight);
    state.set_square({8, 8}, Square::BlackRook);

    for (uint8_t file = 1; file <= 8; ++file) {
        state.set_square({file, 2}, Square::WhitePawn);
        state.set_square({file, 7}, Square::BlackPawn);
    }

    state.a1_castling_forbidden = false;
    state.h1_castling_forbidden = false;
    state.a8_castling_forbidden = false;
    state.h8_castling_forbidden = false;

    state.player_to_move = Player::White;

    return state;
}

GameState make_custom_state(std::string_view text, Player player_to_move, bool reverse_players)
{
    auto state = GameState{};

    auto player_to_place = !reverse_players ? Player::White : Player::Black;
    state.player_to_move = !reverse_players ? player_to_move : other_player(player_to_move);

    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == ' ') { ++i; continue; }
        if (text[i] == '|') { player_to_place = other_player(player_to_place); ++i; continue; }
        const auto pc = piece(text[i]);
        auto c = Coord{text.substr(i + 1, 2)};
        if (reverse_players) c = other_player(c);
        state.set_square(c, square(player_to_place, pc));
        i += 3;
    }

    return state;
}

bool is_attacked_by(Player p, Coord c, const GameState& s) noexcept {
    assert(is_valid(c));
    bool attacked = false;

    foreach_vicinity(c, [p, &s, &attacked](Coord c) {
        if (s.get_square(c) == square(p, Piece::King)) { attacked = true; return false; }
        return true;
    });
    if (attacked) return true;

    foreach_knight_attack(c, [p, &s, &attacked](Coord c) {
        if (s.get_square(c) == square(p, Piece::Knight)) { attacked = true; return false; }
        return true;
    });
    if (attacked) return true;

    constexpr Coord hv_dirs[] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

    for (const auto dir : hv_dirs) {
        foreach_in_dir(c, dir, [p, &s, &attacked](Coord c) {
            const auto sq = s.get_square(c);
            attacked = (
                sq == square(p, Piece::Rook) ||
                sq == square(p, Piece::Queen));
            return is_empty(sq);
        });
        if (attacked) return true;
    }

    constexpr Coord diag_dirs[] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

    for (const auto dir : diag_dirs) {
        foreach_in_dir(c, dir, [p, &s, &attacked](Coord c) {
            const auto sq = s.get_square(c);
            attacked = (
                sq == square(p, Piece::Bishop) ||
                sq == square(p, Piece::Queen));
            return is_empty(sq);
        });
        if (attacked) return true;
    }

    const int forward_rank_off = is_white(s.player_to_move) ? 1 : -1;

    for (int file_off : {-1, 1}) {
        auto c_off = c + Coord{file_off, forward_rank_off};
        if (is_invalid(c_off)) continue;
        if (s.get_square(c_off) == square(p, Piece::Pawn))
            return true;
    }

    return false;
}

std::optional<Coord> find_king(Player p, const GameState& s) noexcept {
    const auto my_king_sq = square(p, Piece::King);
    for (uint8_t rank = 1; rank <= 8; ++rank) {
        for (uint8_t file = 1; file <= 8; ++file) {
            auto c = Coord{rank, file};
            if (s.get_square(c) == my_king_sq)
                return c;
        }
    }
    return {};
}

GameNode make_move(GameState s, MoveCoord m) {
#ifndef NDEBUG
    const auto all_legal_moves = get_legal_moves(s);
    assert(std::find(std::begin(all_legal_moves), std::end(all_legal_moves), m) != std::end(all_legal_moves) && "Illegal move");
#endif

    const auto sq_from = s.get_square(m.from);
    assert(!is_empty(sq_from));
    assert(s.player_to_move == player(sq_from));
    const auto moved_piece = piece(sq_from);

    s.set_square(m.from, Square::Empty);
    s.set_square(m.to, sq_from);

    if (s.en_passant_file != 0) {
        auto en_passant_coord = Coord{s.en_passant_file, is_white(s.player_to_move) ? 6 : 3};
        if (m.to == en_passant_coord) {
            if (moved_piece == Piece::Pawn) {
                const auto en_passtant_culprit_coord = en_passant_coord + Coord{0, is_white(s.player_to_move) ? -1 : 1 };
                assert(player(s(en_passtant_culprit_coord)) == other_player(s.player_to_move));
                assert(piece(s(en_passtant_culprit_coord)) == Piece::Pawn);
                s.set_square(en_passtant_culprit_coord, Square::Empty);
            }
        }
    }

    // Castling.
    {
        if (moved_piece == Piece::King) {
            if (static_cast<int>(m.from.file) - 2 == m.to.file) {
                assert(m.from == (is_white(s.player_to_move) ? Coord{"e1"} : Coord{"e8"}));
                assert(m.to == (is_white(s.player_to_move) ? Coord{"c1"} : Coord{"c8"}));
                assert(is_white(s.player_to_move) ? !s.a1_castling_forbidden : !s.a8_castling_forbidden);
                const int castling_rank = m.from.rank;
                assert(!is_empty(s({1, castling_rank})));
                assert(player(s({1, castling_rank})) == s.player_to_move);
                assert(piece(s({1, castling_rank})) == Piece::Rook);
                s.set_square({1, castling_rank}, Square::Empty);
                s.set_square({4, castling_rank}, square(s.player_to_move, Piece::Rook));
            }
            else if (static_cast<int>(m.from.file) + 2 == m.to.file) {
                assert(m.from == (is_white(s.player_to_move) ? Coord{"e1"} : Coord{"e8"}));
                assert(m.to == (is_white(s.player_to_move) ? Coord{"g1"} : Coord{"g8"}));
                assert(is_white(s.player_to_move) ? !s.h1_castling_forbidden : !s.h8_castling_forbidden);
                const int castling_rank = m.from.rank;
                assert(!is_empty(s({8, castling_rank})));
                assert(player(s({8, castling_rank})) == s.player_to_move);
                assert(piece(s({8, castling_rank})) == Piece::Rook);
                s.set_square({8, castling_rank}, Square::Empty);
                s.set_square({6, castling_rank}, square(s.player_to_move, Piece::Rook));
            }

            if (is_white(s.player_to_move)) {
                s.a1_castling_forbidden = s.h1_castling_forbidden = true;
            } else {
                s.h1_castling_forbidden = s.h8_castling_forbidden = true;
            }
        }
        else if (moved_piece == Piece::Rook) {
            if (m.from == Coord{"a1"}) { s.a1_castling_forbidden = true; }
            else if (m.from == Coord{"a8"}) { s.a8_castling_forbidden = true; }
            else if (m.from == Coord{"h1"}) { s.h1_castling_forbidden = true; }
            else if (m.from == Coord{"h8"}) { s.h8_castling_forbidden = true; }
        }
    }

    // For a pawn's long leap, mark the possible en passant file for the opponent.
    {
        const bool en_passant = moved_piece == Piece::Pawn && abs(m.from.rank - m.to.rank) == 2;
        s.en_passant_file = en_passant ? m.from.file : 0;
    }

    // Pawn to Queen promotion.
    {
        const int8_t promotion_rank = is_white(s.player_to_move) ? 8 : 1;
        if (moved_piece == Piece::Pawn && m.to.rank == promotion_rank) {
            s.set_square(m.to, square(s.player_to_move, Piece::Queen));
        }
    }

    // Switch to the opponent.
    s.player_to_move = other_player(s.player_to_move);
    if (is_white(s.player_to_move)) ++s.move_count;

    // Determine whether the king is in check.
    const bool king_in_check = [&s, moved_piece] {
        auto my_king_coord_opt = find_my_king(s);
        if (!my_king_coord_opt) return false;
        return is_attacked_by_him(*my_king_coord_opt, s);
    }();

    // Find next legal moves.
    auto next_moves = get_legal_moves(s);

    return {s, std::move(next_moves), king_in_check};
}

} // namespace rookmole
