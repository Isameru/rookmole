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

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

std::ostream& operator<<(std::ostream& out, const MoveCoordVec& mv) {
    if (!mv.empty()) {
        out << mv[0];
        for (size_t i = 1; i < mv.size(); ++i)
            out << " " << mv[i];
    }
    return out;
}

// TODO: Work out something better.
bool operator==(MoveCoordVec a, MoveCoordVec b) {
    if (a.size() != b.size()) return false;

    auto coord_less = [](MoveCoord a, MoveCoord b) {
        if (a.from.rank < b.from.rank) return true;
        if (a.from.rank > b.from.rank) return false;
        if (a.from.file < b.from.file) return true;
        if (a.from.file > b.from.file) return false;
        if (a.to.rank < b.to.rank) return true;
        if (a.to.rank > b.to.rank) return false;
        if (a.to.file < b.to.file) return true;
        return false;
    };
    std::sort(std::begin(a), std::end(a), coord_less);
    std::sort(std::begin(b), std::end(b), coord_less);

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

MoveCoordVec make_move_coord_vec(std::string_view text, bool reverse) {
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

bool is_move_coord_legal(const MoveCoordVec& legal_moves, MoveCoord move) noexcept
{
    return std::find(std::begin(legal_moves), std::end(legal_moves), move) != std::end(legal_moves);
}

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

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
        const auto pc = piece_of(text[i]);
        auto c = Coord{text.substr(i + 1, 2)};
        if (reverse_players) c = other_player(c);
        state.set_square(c, make_square(player_to_place, pc));
        i += 3;
    }

    return state;
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

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

bool is_attacked_by(Player p, Coord c, const GameState& s) noexcept {
    assert(is_valid(c));
    bool attacked = false;

    foreach_vicinity(c, [p, &s, &attacked](Coord c) {
        if (s.get_square(c) == make_square(p, Piece::King)) { attacked = true; return false; }
        return true;
    });
    if (attacked) return true;

    foreach_knight_attack(c, [p, &s, &attacked](Coord c) {
        if (s.get_square(c) == make_square(p, Piece::Knight)) { attacked = true; return false; }
        return true;
    });
    if (attacked) return true;

    constexpr Coord hv_dirs[] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

    for (const auto dir : hv_dirs) {
        foreach_in_dir(c, dir, [p, &s, &attacked](Coord c) {
            const auto sq = s.get_square(c);
            attacked = (
                sq == make_square(p, Piece::Rook) ||
                sq == make_square(p, Piece::Queen));
            return is_empty(sq);
        });
        if (attacked) return true;
    }

    constexpr Coord diag_dirs[] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

    for (const auto dir : diag_dirs) {
        foreach_in_dir(c, dir, [p, &s, &attacked](Coord c) {
            const auto sq = s.get_square(c);
            attacked = (
                sq == make_square(p, Piece::Bishop) ||
                sq == make_square(p, Piece::Queen));
            return is_empty(sq);
        });
        if (attacked) return true;
    }

    const int forward_rank_off = is_white(s.player_to_move) ? 1 : -1;

    for (int file_off : {-1, 1}) {
        auto c_off = c + Coord{file_off, forward_rank_off};
        if (is_invalid(c_off)) continue;
        if (s.get_square(c_off) == make_square(p, Piece::Pawn))
            return true;
    }

    return false;
}

Coord find_king(Player p, const GameState& s) noexcept {
    const auto my_king_sq = make_square(p, Piece::King);
    for (uint8_t rank = 1; rank <= 8; ++rank) {
        for (uint8_t file = 1; file <= 8; ++file) {
            auto c = Coord{rank, file};
            if (s.get_square(c) == my_king_sq)
                return c;
        }
    }
    return Coord::invalid();
}

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

MoveCoordVec get_legal_moves(const GameState& s)
{
    auto out = MoveCoordVec{};
    out.reserve(20);

    auto my_king_coord_opt = find_my_king(s);
    auto en_passant_coord_opt = s.en_passant_file != 0 ?
        Coord{s.en_passant_file, is_white(s.player_to_move) ? 6 : 3} :
        Coord::invalid();

    auto add_move = [&out, &s, my_king_coord_opt](MoveCoord mc) {
        const auto sq_from = s(mc.from);
        assert(player_of(sq_from) == s.player_to_move);
        bool is_king_in_check_after_move;
        if (is_valid(my_king_coord_opt)) {
            auto king_coord_after_move = (piece_of(sq_from) == Piece::King) ? mc.to : my_king_coord_opt;
            auto temp_move = ScopedPieceMover{const_cast<GameState&>(s), mc};
            is_king_in_check_after_move = is_attacked_by_him(king_coord_after_move, s);
        }
        else {
            // There is no king on the chessboard.
            is_king_in_check_after_move = false;
        }

        if (!is_king_in_check_after_move)
            out.push_back(mc);
    };

    s.foreach_piece([&out, &s, &add_move, my_king_coord_opt, en_passant_coord_opt](Coord c, Player p, Piece pc) {
        if (p != s.player_to_move) return;
        const auto opponent = other_player(p);

        switch (pc) {
            case Piece::Pawn: {
                assert(c.rank > 1 && c.rank < 8);
                const int forward_rank_off = is_white(p) ? 1 : -1;
                auto c_fwd = c + Coord{0, forward_rank_off};
                assert(is_valid(c_fwd));

                if (is_empty(s(c_fwd))) {
                    add_move({c, c_fwd});

                    if (c.rank == (is_white(p) ? 2 : 7)) {
                        auto c_2fwd = c_fwd + Coord{0, forward_rank_off};
                        if (is_valid(c_2fwd) && is_empty(s(c_2fwd))) {
                            add_move({c, c_2fwd});
                        }
                    }
                }

                for (int i : {-1, 1}) {
                    auto c_d = c + Coord{i, forward_rank_off};
                    if (is_invalid(c_d)) continue;

                    const auto sq_d = s(c_d);
                    if ((!is_empty(sq_d) && (player_of(sq_d) == opponent)) ||
                        (c_d == en_passant_coord_opt))
                    {
                        add_move({c, c_d});
                    }
                }

                break;
            }

            case Piece::Knight: {
                foreach_knight_attack(c, [&s, &add_move, c, opponent](Coord c_to) {
                    const auto sq_to = s(c_to);
                    if (is_empty(sq_to) || player_of(sq_to) == opponent)
                        add_move({c, c_to});
                    return true;
                });
                break;
            }

            case Piece::Bishop: {
                constexpr Coord diag_dirs[] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (const auto dir : diag_dirs) {
                    foreach_in_dir(c, dir, [&s, &add_move, c, opponent](Coord c_to) {
                        const auto sq_to = s(c_to);
                        if (is_empty(sq_to) || player_of(sq_to) == opponent)
                            add_move({c, c_to});
                        return is_empty(sq_to);
                    });
                }
                break;
            }

            case Piece::Rook: {
                constexpr Coord hv_dirs[] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};
                for (const auto dir : hv_dirs) {
                    foreach_in_dir(c, dir, [&s, &add_move, c, opponent](Coord c_to) {
                        const auto sq_to = s(c_to);
                        if (is_empty(sq_to) || player_of(sq_to) == opponent)
                            add_move({c, c_to});
                        return is_empty(sq_to);
                    });
                }
                break;
            }

            case Piece::Queen: {
                constexpr Coord all_dirs[] = {
                    {-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1} };
                for (const auto dir : all_dirs) {
                    foreach_in_dir(c, dir, [&s, &add_move, c, opponent](Coord c_to) {
                        const auto sq_to = s(c_to);
                        if (is_empty(sq_to) || player_of(sq_to) == opponent)
                            add_move({c, c_to});
                        return is_empty(sq_to);
                    });
                }
                break;
            }

            case Piece::King: {
                assert(is_valid(my_king_coord_opt));
                assert(c == my_king_coord_opt);

                foreach_vicinity(c, [&s, &add_move, c, opponent](Coord c_to) {
                    const auto sq_to = s(c_to);
                    if (is_empty(sq_to) || player_of(sq_to) == opponent)
                        add_move({c, c_to});
                    return true;
                });

                bool a_castling_possible = is_white(s.player_to_move) ?
                    !s.a1_castling_forbidden : !s.a8_castling_forbidden;
                bool h_castling_possible = is_white(s.player_to_move) ?
                    !s.h1_castling_forbidden : !s.h8_castling_forbidden;

                if ((a_castling_possible || h_castling_possible) &&
                    c == (is_white(s.player_to_move) ? Coord{"e1"} : Coord{"e8"}) &&
                    !is_attacked_by_him(my_king_coord_opt, s))
                {
                    const int8_t castling_rank = is_white(s.player_to_move) ? 1 : 8;
                    const auto c_a = Coord{1, castling_rank};
                    const auto c_h = Coord{8, castling_rank};
                    const auto sq_a = s(c_a);
                    const auto sq_h = s(c_h);

                    if (is_empty(sq_a) || player_of(sq_a) == opponent || piece_of(sq_a) != Piece::Rook) {
                        a_castling_possible = false;
                    }

                    if (a_castling_possible) {
                        for (int file = 2; file < c.file; ++file) {
                            if (!is_empty(s({file, castling_rank}))) {
                                a_castling_possible = false;
                                break;
                            }
                        }
                    }

                    if (a_castling_possible) {
                        for (int file = 1; file < c.file; ++file) {
                            if (is_attacked_by_him({file, castling_rank}, s)) {
                                a_castling_possible = false;
                                break;
                            }
                        }
                    }

                    if (a_castling_possible) {
                        add_move({c, {c.file - 2, c.rank}});
                    }

                    if (is_empty(sq_h) || player_of(sq_h) == opponent || piece_of(sq_h) != Piece::Rook) {
                        h_castling_possible = false;
                    }

                    if (h_castling_possible) {
                        for (int file = c.file + 1; file <= 7; ++file) {
                            if (!is_empty(s({file, castling_rank}))) {
                                h_castling_possible = false;
                                break;
                            }
                        }
                    }

                    if (h_castling_possible) {
                        for (int file = c.file + 1; file <= 8; ++file) {
                            if (is_attacked_by_him({file, castling_rank}, s)) {
                                h_castling_possible = false;
                                break;
                            }
                        }
                    }

                    if (h_castling_possible) {
                        add_move({c, {c.file + 2, c.rank}});
                    }
                }
                break;
            }

        }
    });

    return out;
}

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

GameNode make_start_node() {
    auto state = make_start_state();
    auto legal_moves = get_legal_moves(state);
    return GameNode{std::move(state), std::move(legal_moves), false};
}

GameNode make_move(GameState s, MoveCoord m) {
    assert(is_move_coord_legal(get_legal_moves(s), m) && "Illegal move");

    const auto sq_from = s.get_square(m.from);
    assert(!is_empty(sq_from));
    assert(s.player_to_move == player_of(sq_from));
    const auto moved_piece = piece_of(sq_from);

    s.set_square(m.from, Square::Empty);
    s.set_square(m.to, sq_from);

    if (s.en_passant_file != 0) {
        auto en_passant_coord = Coord{s.en_passant_file, is_white(s.player_to_move) ? 6 : 3};
        if (m.to == en_passant_coord) {
            if (moved_piece == Piece::Pawn) {
                const auto en_passtant_culprit_coord = en_passant_coord + Coord{0, is_white(s.player_to_move) ? -1 : 1 };
                assert(player_of(s(en_passtant_culprit_coord)) == other_player(s.player_to_move));
                assert(piece_of(s(en_passtant_culprit_coord)) == Piece::Pawn);
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
                assert(player_of(s({1, castling_rank})) == s.player_to_move);
                assert(piece_of(s({1, castling_rank})) == Piece::Rook);
                s.set_square({1, castling_rank}, Square::Empty);
                s.set_square({4, castling_rank}, make_square(s.player_to_move, Piece::Rook));
            }
            else if (static_cast<int>(m.from.file) + 2 == m.to.file) {
                assert(m.from == (is_white(s.player_to_move) ? Coord{"e1"} : Coord{"e8"}));
                assert(m.to == (is_white(s.player_to_move) ? Coord{"g1"} : Coord{"g8"}));
                assert(is_white(s.player_to_move) ? !s.h1_castling_forbidden : !s.h8_castling_forbidden);
                const int castling_rank = m.from.rank;
                assert(!is_empty(s({8, castling_rank})));
                assert(player_of(s({8, castling_rank})) == s.player_to_move);
                assert(piece_of(s({8, castling_rank})) == Piece::Rook);
                s.set_square({8, castling_rank}, Square::Empty);
                s.set_square({6, castling_rank}, make_square(s.player_to_move, Piece::Rook));
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
            s.set_square(m.to, make_square(s.player_to_move, Piece::Queen));
        }
    }

    // Switch to the opponent.
    s.player_to_move = other_player(s.player_to_move);
    if (is_white(s.player_to_move)) ++s.move_count;

    // Determine whether the king is in check.
    const bool king_in_check = [&s, moved_piece] {
        auto my_king_coord_opt = find_my_king(s);
        if (is_invalid(my_king_coord_opt)) return false;
        return is_attacked_by_him(my_king_coord_opt, s);
    }();

    // Find next legal moves.
    auto next_moves = get_legal_moves(s);

    return {s, std::move(next_moves), king_in_check};
}

bool is_terminal(GameNode n) noexcept {
    return n.next_moves.empty() || n.state.move_count == 80;
}

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

} // namespace rookmole
