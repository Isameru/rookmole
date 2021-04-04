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
#include <vector>

namespace rookmole {

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

enum Player : uint8_t {
    White = 0,
    Black = 1
};

constexpr bool is_white(Player p) noexcept { return p == Player::White; }
constexpr bool is_black(Player p) noexcept { return p == Player::Black; }
constexpr Player other_player(Player p) noexcept { return (Player)(p ^ 1); }
inline std::string to_string(Player p) noexcept { return is_white(p) ? "white" : "black"; }
inline std::ostream& operator<<(std::ostream& out, Player p) { return out << to_string(p); }

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

struct Coord {
    int8_t file;  // x
    int8_t rank;  // y

    constexpr Coord() noexcept : file{0}, rank{0} {}

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

    constexpr static Coord invalid() noexcept { return {0, 0}; }
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
    const char cv[] = { static_cast<char>(c.file - 1 + 'a'), static_cast<char>('0' + c.rank), '\0' };
    return {cv};
}

inline std::ostream& operator<<(std::ostream& out, Coord c) { return out << to_string(c); }

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

struct MoveCoord {
    Coord from;
    Coord to;

    constexpr MoveCoord() noexcept = default;
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
constexpr MoveCoord make_move_coord(std::string_view text) noexcept {
    return !reverse ? MoveCoord{text} : other_player(MoveCoord{text});
}

inline std::ostream& operator<<(std::ostream& out, MoveCoord mc) { return out << mc.from << ':' << mc.to; }

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

using MoveCoordVec = std::vector<MoveCoord>;

std::ostream& operator<<(std::ostream& out, const MoveCoordVec& mv);
bool operator==(MoveCoordVec a, MoveCoordVec b);
inline bool operator!=(MoveCoordVec a, MoveCoordVec b) { return !(std::move(a) == std::move(b)); }
MoveCoordVec make_move_coord_vec(std::string_view text, bool reverse = false);
bool is_move_coord_legal(const MoveCoordVec& legal_moves, MoveCoord move) noexcept;

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

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

constexpr Square make_square(Player player, Piece piece) noexcept { return static_cast<Square>((player << 3) | piece); }
constexpr bool is_empty(Square sq) noexcept { return sq == 0; }
constexpr bool is_white(Square sq) noexcept { return sq > 0 && sq < 0b1000; }
constexpr bool is_black(Square sq) noexcept { return sq >= 0b1000; }
constexpr Player player_of(Square sq) noexcept { assert(!is_empty(sq)); return is_black(sq) ? Player::Black : Player::White; }
constexpr Piece piece_of(Square sq) noexcept { return static_cast<Piece>(sq & 0b111); }
constexpr Piece piece_of(char c) noexcept {
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

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

struct GameState {
    std::array<uint8_t, 8 * 8 / 2> squares;
    bool a1_castling_forbidden : 1;
    bool h1_castling_forbidden : 1;
    bool a8_castling_forbidden : 1;
    bool h8_castling_forbidden : 1;
    uint8_t en_passant_file : 4;  // 0: No en passtant possibility | 1..8: A file where opponent left an openinig for en passant.
    uint8_t move_count : 7;
    Player player_to_move : 1;

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
                cb(Coord{file, rank}, player_of(sq), piece_of(sq));
            }
            if (!is_empty(sq2)) {
                assert(get_square(Coord{file + 1, rank}) == sq2);
                cb(Coord{file + 1, rank}, player_of(sq2), piece_of(sq2));
            }
        }
    }
};

static_assert(sizeof(GameState) == 34);

GameState make_start_state();
GameState make_custom_state(std::string_view text, Player player_to_move, bool reverse_players);
std::ostream& operator<<(std::ostream& out, const GameState& state);

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

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

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

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

Coord find_king(Player p, const GameState& s) noexcept;
inline Coord find_my_king(const GameState& s) noexcept { return find_king(s.player_to_move, s); }
MoveCoordVec get_legal_moves(const GameState& s);

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

struct GameNode {
    GameState state;
    MoveCoordVec next_moves;
    bool king_in_check;
};

GameNode make_start_node();
GameNode make_move(GameState s, MoveCoord m);
bool is_terminal(GameNode n) noexcept;

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

} // namespace rookmole
