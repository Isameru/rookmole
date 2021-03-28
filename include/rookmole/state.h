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

#pragma once

#include <array>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace rookmole {

enum Player : uint8_t {
    White = 0,
    Black = 1
};

inline std::string to_string(Player p) noexcept { return p == Player::White ? "white" : "black"; }
inline std::ostream& operator<<(std::ostream& out, Player p) { return out << to_string(p); }
inline Player other_player(Player p) noexcept { return (Player)(p ^ 1); }
constexpr bool is_white(Player p) noexcept { return p == Player::White; }
constexpr bool is_black(Player p) noexcept { return p == Player::Black; }


struct Coord {
    int8_t file;  // x
    int8_t rank;  // y

    template<typename FileT, typename RankT,
        std::enable_if_t<std::is_integral<FileT>::value, bool> = true,
        std::enable_if_t<std::is_integral<RankT>::value, bool> = true>
    constexpr Coord(FileT file, RankT rank) noexcept :
        file{static_cast<int8_t>(file)}, rank{static_cast<int8_t>(rank)} {}

    constexpr Coord(std::string_view text) noexcept :
        Coord{text[0] - 'a' + 1, text[1] - '0'}
    {
        assert(text.size() == 2);
        assert(text[0] >= 'a' && text[0] <= 'h');
        assert(text[1] >= '1' && text[1] <= '8');
    }
};

static_assert(sizeof(Coord) == 2);

constexpr Coord operator+(Coord c1, Coord c2) noexcept { return {c1.file + c2.file, c1.rank + c2.rank}; }
constexpr bool operator==(Coord c1, Coord c2) noexcept { return c1.file == c2.file && c1.rank == c2.rank; }
constexpr bool operator!=(Coord c1, Coord c2) noexcept { return !(c1 == c2); }
constexpr bool is_valid(Coord c) noexcept { return c.file >= 1 && c.file <= 8 && c.rank >= 1 && c.rank <= 8; }
constexpr bool is_invalid(Coord c) noexcept { return !is_valid(c); }
constexpr Coord other_player(Coord c) noexcept { assert(is_valid(c)); return Coord{9 - c.file, 9 - c.rank}; }
template<bool reverse = false>
constexpr Coord make_coord(std::string_view text) noexcept {
    return !reverse ? Coord{text} : other_player(Coord{text});
}

inline std::string to_string(Coord c) noexcept {
    if (is_invalid(c)) return {"??"};

    const char cv[] = {
        static_cast<char>(c.file - 1 + 'a'),
        static_cast<char>('0' + c.rank),
        '\0'
    };
    return {cv};
}

inline std::ostream& operator<<(std::ostream& out, Coord c) { return out << to_string(c); }


struct MoveCoord {
    Coord from;
    Coord to;

    constexpr MoveCoord(Coord from_, Coord to_) noexcept : from{from_}, to{to_} {}
    constexpr MoveCoord(std::string_view text) noexcept :
        from{text.substr(0, 2)},
        to{text.substr(3, 2)}
    {
        assert(text.size() == 5);
        assert(text[2] == ':');
    }
};

static_assert(sizeof(MoveCoord) == 4);

constexpr bool operator==(MoveCoord mc0, MoveCoord mc1) noexcept { return mc0.from == mc1.from && mc0.to == mc1.to; }
constexpr bool operator!=(MoveCoord mc0, MoveCoord mc1) noexcept { return !(mc0 == mc1); }
constexpr MoveCoord other_player(MoveCoord c) noexcept { return {other_player(c.from), other_player(c.to)}; }
template<bool reverse = false>
constexpr MoveCoord make_coord_move(std::string_view text) {
    return !reverse ? MoveCoord{text} : other_player(MoveCoord{text});
}

inline std::ostream& operator<<(std::ostream& out, MoveCoord mc) { return out << mc.from << ':' << mc.to; }


using MoveCoordVec = std::vector<MoveCoord>;

inline std::ostream& operator<<(std::ostream& out, const MoveCoordVec& mv) {
    if (!mv.empty()) {
        out << mv[0];
        for (size_t i = 1; i < mv.size(); ++i)
            out << " " << mv[i];
    }
    return out;
}

// TODO: Work out something better.
inline bool operator==(MoveCoordVec a, MoveCoordVec b) {
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

inline bool operator!=(MoveCoordVec a, MoveCoordVec b) {
    return !(std::move(a) == std::move(b));
}

MoveCoordVec make_coord_moves(std::string_view text, bool reverse = false);


enum Piece : uint8_t {
    None        = 0,

    Pawn        = 0b001,
    Knight      = 0b010,
    Bishop      = 0b011,
    Rook        = 0b100,
    Queen       = 0b101,
    King        = 0b110,
};

enum Square : uint8_t {
    Empty       = 0,

    WhitePawn   = 0b0001,
    WhiteKnight = 0b0010,
    WhiteBishop = 0b0011,
    WhiteRook   = 0b0100,
    WhiteQueen  = 0b0101,
    WhiteKing   = 0b0110,

    BlackPawn   = 0b1001,
    BlackKnight = 0b1010,
    BlackBishop = 0b1011,
    BlackRook   = 0b1100,
    BlackQueen  = 0b1101,
    BlackKing   = 0b1110,
};

constexpr Square square(Player player, Piece piece) noexcept { return static_cast<Square>((player << 3) | piece); }
constexpr bool is_empty(Square sq) noexcept { return sq == 0; }
constexpr bool is_white(Square sq) noexcept { return sq > 0 && sq < 0b1000; }
constexpr bool is_black(Square sq) noexcept { return sq >= 0b1000; }
constexpr Player player(Square sq) noexcept { assert(!is_empty(sq)); return is_black(sq) ? Player::Black : Player::White; }
constexpr Piece piece(Square sq) noexcept { return static_cast<Piece>(sq & 0b111); }
constexpr Piece piece(char c) noexcept {
    switch (c) {
        case 'p': return Piece::Pawn;
        case 'N': return Piece::Knight;
        case 'B': return Piece::Bishop;
        case 'R': return Piece::Rook;
        case 'Q': return Piece::Queen;
        case 'K': return Piece::King;
        default:
            assert(false);
            return Piece::None;
    }
}

//constexpr Square other_color(Square sq) { return is_empty(sq) ? sq : (Square)(sq ^ 0b1000); }
//constexpr Player get_player(Square sq) { assert(!is_empty(sq)); return (sq < 0b1000) ? Player::White : Player::Black; }

struct GameState {
    std::array<uint8_t, 8 * 8 / 2> squares;
    bool a1_castling_forbidden : 1;
    bool h1_castling_forbidden : 1;
    bool a8_castling_forbidden : 1;
    bool h8_castling_forbidden : 1;
    Player player_to_move : 1;
    uint8_t en_passant_file : 4;  // 0: No en passtant possibility
    uint8_t move_count : 7;

    Square get_square(Coord coord) const noexcept {
        assert(is_valid(coord));
        const auto sq_index = 8 * coord.rank + coord.file - 9;
        const bool high_nibble = sq_index % 2;
        const auto tsq = squares[sq_index / 2];
        return static_cast<Square>(high_nibble ? tsq >> 4 : tsq & 0x0F);
    }

    Square operator()(Coord coord) const noexcept { return get_square(coord); }

    void set_square(Coord coord, Square sq) noexcept {
        assert(is_valid(coord));
        const auto sq_index = 8 * coord.rank + coord.file - 9;
        const bool high_nibble = sq_index % 2;
        auto& tsq = squares[sq_index / 2];
        if (high_nibble) {
            tsq = (tsq & 0x0F) | (sq << 4);
        }
        else {
            tsq = (tsq & 0xF0) | sq;
        }
    }

    template<typename CbT>
    void foreach_piece(const CbT& cb) const noexcept {
        for (int i = 0; i < static_cast<int>(squares.size()); ++i) {
            const auto tsq = squares[i];
            if (tsq == 0) continue;

            const int file = 2 * (i % 4) + 1;
            const int rank = (i / 4) + 1;
            const auto sq = static_cast<Square>(tsq & 0x0F);
            const auto sq2 = static_cast<Square>(tsq >> 4);

            if (!is_empty(sq)) {
                assert(get_square(Coord{file, rank}) == sq);
                cb(Coord{file, rank}, player(sq), piece(sq));
            }
            if (!is_empty(sq2)) {
                assert(get_square(Coord{file + 1, rank}) == sq2);
                cb(Coord{file + 1, rank}, player(sq2), piece(sq2));
            }
        }
    }
};

std::ostream& operator<<(std::ostream& out, const GameState& state);


class ScopedPieceMover {
    GameState& _state;
    MoveCoord _move_coord;
    Square _sq_from;
    Square _sq_to;

public:
    ScopedPieceMover(GameState& state, MoveCoord move_coord) noexcept :
        _state{state},
        _move_coord{move_coord},
        _sq_from{_state(move_coord.from)},
        _sq_to{_state(move_coord.to)}
    {
        assert(!is_empty(_sq_from));
        _state.set_square(_move_coord.from, Square::Empty);
        _state.set_square(_move_coord.to, _sq_from);
    }

    ~ScopedPieceMover() noexcept {
        _state.set_square(_move_coord.from, _sq_from);
        _state.set_square(_move_coord.to, _sq_to);
    }
};

GameState make_start_state();
GameState make_custom_state(std::string_view text, Player player_to_move, bool reverse_players);


template<typename CbT>
void foreach_vicinity(Coord c, const CbT& cb) {
    assert(is_valid(c));
    constexpr Coord offs[] = { {-1,-1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1} };
    for (const auto off : offs) {
        const auto c_off = c + off;
        if (is_invalid(c_off)) continue;
        if (!cb(c_off)) return;
    }
}

template<typename CbT>
void foreach_knight_attack(Coord c, const CbT& cb) {
    assert(is_valid(c));
    constexpr Coord offs[] = { {-1, -2}, {1, -2}, {-2, -1}, {2, -1}, {-2, 1}, {2, 1}, {-1, 2}, {1, 2} };
    for (const auto off : offs) {
        const auto c_off = c + off;
        if (is_invalid(c_off)) continue;
        if (!cb(c_off)) return;
    }
}

template<typename CbT>
void foreach_in_dir(Coord c, Coord dir, const CbT& cb) {
    assert(is_valid(c));
    while (true) {
        c = c + dir;
        if (is_invalid(c)) return;
        if (!cb(c)) return;
    }
}

bool is_attacked_by(Player p, Coord c, const GameState& s) noexcept;
inline bool is_attacked_by_him(Coord c, const GameState& s) noexcept {
    return is_attacked_by(other_player(s.player_to_move), c, s);
}

std::optional<Coord> find_king(Player p, const GameState& s) noexcept;
inline std::optional<Coord> find_my_king(const GameState& s) noexcept {
    return find_king(s.player_to_move, s);
}

inline MoveCoordVec get_legal_moves(const GameState& s)
{
    auto out = MoveCoordVec{};
    out.reserve(20);

    auto my_king_coord_opt = find_my_king(s);
    auto en_passant_coord_opt = s.en_passant_file != 0 ?
        std::optional<Coord>{{s.en_passant_file, is_white(s.player_to_move) ? 6 : 3}} :
        std::optional<Coord>{};

    auto add_move = [&out, &s, my_king_coord_opt](MoveCoord mc) {
        const auto sq_from = s(mc.from);
        assert(player(sq_from) == s.player_to_move);
        bool is_king_in_check_after_move;
        if (my_king_coord_opt) {
            auto king_coord_after_move = (piece(sq_from) == Piece::King) ? mc.to : *my_king_coord_opt;
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
                    if ((!is_empty(sq_d) && (player(sq_d) == opponent)) ||
                        (en_passant_coord_opt && c_d == *en_passant_coord_opt))
                    {
                        add_move({c, c_d});
                    }
                }

                break;
            }

            case Piece::Knight: {
                foreach_knight_attack(c, [&s, &add_move, c, opponent](Coord c_to) {
                    const auto sq_to = s(c_to);
                    if (is_empty(sq_to) || player(sq_to) == opponent)
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
                        if (is_empty(sq_to) || player(sq_to) == opponent)
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
                        if (is_empty(sq_to) || player(sq_to) == opponent)
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
                        if (is_empty(sq_to) || player(sq_to) == opponent)
                            add_move({c, c_to});
                        return is_empty(sq_to);
                    });
                }
                break;
            }

            case Piece::King: {
                assert(my_king_coord_opt);
                const auto my_king_coord = *my_king_coord_opt;
                assert(c == my_king_coord);

                foreach_vicinity(c, [&s, &add_move, c, opponent](Coord c_to) {
                    const auto sq_to = s(c_to);
                    if (is_empty(sq_to) || player(sq_to) == opponent)
                        add_move({c, c_to});
                    return true;
                });

                bool a_castling_possible = is_white(s.player_to_move) ?
                    !s.a1_castling_forbidden : !s.a8_castling_forbidden;
                bool h_castling_possible = is_white(s.player_to_move) ?
                    !s.h1_castling_forbidden : !s.h8_castling_forbidden;

                if ((a_castling_possible || h_castling_possible) &&
                    c == (is_white(s.player_to_move) ? Coord{"e1"} : Coord{"e8"}) &&
                    !is_attacked_by_him(my_king_coord, s))
                {
                    const int8_t castling_rank = is_white(s.player_to_move) ? 1 : 8;
                    const auto c_a = Coord{1, castling_rank};
                    const auto c_h = Coord{8, castling_rank};
                    const auto sq_a = s(c_a);
                    const auto sq_h = s(c_h);

                    if (is_empty(sq_a) || player(sq_a) == opponent || piece(sq_a) != Piece::Rook) {
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

                    if (is_empty(sq_h) || player(sq_h) == opponent || piece(sq_h) != Piece::Rook) {
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


struct GameNode {
    GameState state;
    MoveCoordVec next_moves;
    bool king_in_check;
};

GameNode make_move(GameState s, MoveCoord m);

} // namespace rookmole
