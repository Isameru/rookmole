//                       __                  .__
// _______  ____   ____ |  | __ _____   ____ |  |   ____
// \_  __ \/  _ \ /  _ \|  |/ //     \ /  _ \|  | _/ __ \
//  |  | \(  <_> |  <_> )    <|  Y Y  (  <_> )  |_\  ___/
//  |__|   \____/ \____/|__|_ \__|_|  /\____/|____/\___  >
//                           \/     \/                 \/
//

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace rockmole
{
    constexpr uint8_t RANKS = 8;
    constexpr uint8_t FILES = 8;
    constexpr uint8_t SQUARES = RANKS * FILES;

    // --------------------------------------------------------------------------------------------
    // Player

    enum Player : uint8_t {
        White = 0,
        Black = 1
    };

    inline std::string to_string(Player p) { return p == Player::White ? "white" : "black"; }
    inline Player other_player(Player p) { return (Player)(p ^ 1); }
    constexpr bool is_white(Player p) { return p == Player::White; }
    constexpr bool is_black(Player p) { return p == Player::Black; }

    // --------------------------------------------------------------------------------------------
    // Coord

    struct Coord {
        uint8_t rank : 4;  // -y: 8...1
        uint8_t file : 4;  //  x: 1...8

        Coord() : rank{0}, file{0} {}
        template<typename RankT, typename FileT> Coord(RankT rank, FileT file) :
            rank{static_cast<uint8_t>(rank)}, file{static_cast<uint8_t>(file)} {
            assert(rank >= 1 && rank <= 8 && file >= 1 && file <= 8);
        }

        static Coord invalid() { const uint8_t zero = 0; return reinterpret_cast<const Coord&>(zero); }

        Coord get_off(int off_rank, int off_file) const {
            int new_rank = rank + off_rank;
            int new_file = file + off_file;
            return (new_rank < 1 || new_rank > 8 || new_file < 1 || new_file > 8) ? invalid() : Coord{new_rank, new_file};
        }
    };

    static_assert(sizeof(Coord) == 1);

    inline bool operator==(Coord a, Coord b) { return a.rank == b.rank && a.file == b.file; }
    inline bool operator!=(Coord a, Coord b) { return !(a == b); }

    inline bool is_valid(Coord c) {
        assert((c.rank == 0 && c.file == 0) || (c.rank >= 1 && c.rank <= 8 && c.file >= 1 && c.file <= 8));
        return c.rank != 0;
    }

    inline bool is_invalid(Coord c) { return !is_valid(c); }

    inline std::string to_string(Coord c) {
        const char cv[] = {
            (char)((char)c.file - 1 + 'a'),
            (char)('0' + c.rank),
            '\0'
        };
        return std::string{cv};
    }

    inline std::ostream& operator<<(std::ostream& out, Coord c) { return out << to_string(c); }
    inline Coord other_player(Coord c) { assert(is_valid(c)); return Coord{9 - c.rank, 9 - c.file}; }

    // --------------------------------------------------------------------------------------------
    // MoveCoord

    struct MoveCoord {
        Coord from;
        Coord to;

        friend bool operator==(MoveCoord a, MoveCoord b) { return a.from == b.from && a.to == b.to; }
        friend bool operator!=(MoveCoord a, MoveCoord b) { return !(a == b); }
    };

    inline std::ostream& operator<<(std::ostream& out, MoveCoord m) { return out << m.from << ':' << m.to; }
    inline MoveCoord other_player(MoveCoord mv) { return {other_player(mv.from), other_player(mv.to)}; }

    // --------------------------------------------------------------------------------------------
    // MoveCoord

    using MoveCoordList = std::vector<MoveCoord>;

    inline std::ostream& operator<<(std::ostream& out, const MoveCoordList& mv) {
        if (!mv.empty()) {
            out << mv[0];
            for (size_t i = 1; i < mv.size(); ++i) out << " " << mv[i];
        }
        return out;
    }
    inline void other_player_inplace(MoveCoordList& mv) { for (auto& m : mv) m = other_player(m); }

    inline bool operator==(MoveCoordList a, MoveCoordList b) {
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

    inline bool operator!=(MoveCoordList a, MoveCoordList b) {
        return !(std::move(a) == std::move(b));
    }

    MoveCoordList make_coord_moves(const std::string& s);

    // --------------------------------------------------------------------------------------------
    // Square

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

    constexpr bool is_empty(Square sq) { return sq == 0; }
    constexpr bool is_white(Square sq) { return sq > 0 && sq < 0b1000; }
    constexpr bool is_black(Square sq) { return sq >= 0b1000; }
    constexpr Square other_color(Square sq) { return is_empty(sq) ? sq : (Square)(sq ^ 0b1000); }
    constexpr Player get_player(Square sq) { assert(!is_empty(sq)); return (sq < 0b1000) ? Player::White : Player::Black; }

    // --------------------------------------------------------------------------------------------
    // State

    struct State {
        uint8_t squares[8 * 8 / 2];
        bool white_short_castling_possible : 1;
        bool white_long_castling_possible : 1;
        bool black_short_castling_possible : 1;
        bool black_long_castling_possible : 1;
        Player player_to_move : 1;
        uint8_t en_passant_file : 4;
        uint8_t allmove_count;

        Square get_square(Coord coord) const {
            assert(is_valid(coord));
            const uint8_t sq_index = (uint8_t)(8 * (8 - coord.rank) + (coord.file - 1));
            const bool high_nibble = sq_index % 2;
            const uint8_t tsq = squares[sq_index / 2];
            return (Square)(high_nibble ? tsq >> 4 : tsq & 0x0F);
        }

        void set_square(Coord coord, Square sq) {
            assert(is_valid(coord));
            //return squares[FILES * (8 - coord.rank) + (coord.file - 1)];
            const uint8_t sq_index = (uint8_t)(8 * (8 - coord.rank) + (coord.file - 1));
            const bool high_nibble = sq_index % 2;
            uint8_t& tsq = squares[sq_index / 2];
            if (high_nibble) {
                tsq = (Square)((tsq & 0x0F) | (sq << 4));
            }
            else {
                tsq = (Square)((tsq & 0xF0) | sq);
            }
        }
    };

    // --------------------------------------------------------------------------------------------
    // ScopedTemporaryPieceMover

    class ScopedTemporaryPieceMover {
        State& _state;
        Coord _from;
        Coord _to;
        Square _sq_from;
        Square _sq_to;

    public:
        ScopedTemporaryPieceMover(State& state, Coord from, Coord to) :
            _state{state},
            _from{from},
            _to{to},
            _sq_from{_state.get_square(from)},
            _sq_to{_state.get_square(to)}
        {
            assert(!is_empty(_sq_from));
            _state.set_square(_to, _sq_from);
            _state.set_square(_from, Square::Empty);
        }

        ~ScopedTemporaryPieceMover() {
            _state.set_square(_to, _sq_to);
            _state.set_square(_from, _sq_from);
        }
    };

    // --------------------------------------------------------------------------------------------
    // SquareDynView, SquareReadView, SquareView

    class SquareDynView {
        State& _state;
        Player _player;

    public:
        SquareDynView(State& state, Player player) :
            _state{state},
            _player{player}
        {}

        Player player() const { return _player; }
        Player switch_player() { return _player = other_player(_player); }
        Square operator()(Coord c) const { return view_square(_state.get_square(view_coord(c))); }
        void set(Coord c, Square sq) { _state.set_square(view_coord(c), view_square(sq)); }

        Coord view_coord(Coord c) const { return is_white(player()) ? c : other_player(c); }
        Square view_square(Square sq) const { return is_white(player()) ? sq : other_color(sq); }
    };

    template<Player player_>
    class SquareReadView {
        const State& _state;

    public:
        SquareReadView(const State& state) : _state{state} {}

        Player player() const { return player_; }
        Square operator()(Coord c) const { return view_square(_state.get_square(view_coord(c))); }

        Coord view_coord(Coord c) const { return is_white(player()) ? c : other_player(c); }
        Square view_square(Square sq) const { return is_white(player()) ? sq : other_color(sq); }
    };

    template<Player player_>
    class SquareView {
        State& _state;

    public:
        SquareView(State& state) : _state{state} {}

        Player player() const { return player_; }
        Square operator()(Coord c) const { return view_square(_state.get_square(view_coord(c))); }
        void set(Coord c, Square sq) { _state.get_square(view_coord(c), view_square(sq)); }

        Coord view_coord(Coord coord) const { return player() == Player::White ? coord : Coord{9 - coord.rank, 9 - coord.file}; }
        Square view_square(Square sq) const { return (is_empty(sq) || player() == Player::White) ? sq : (Square)(sq ^ 0b10000000); }
    };

    std::ostream& operator<<(std::ostream& out, const State& state);
    State make_start_state(Player starting_player);
    State make_custom_state(std::string placement, Player player_to_move, bool reverse_players);

    // --------------------------------------------------------------------------------------------
    // get_legal_moves

    template<typename CbT>
    void foreach_vicinity(Coord c, const CbT& cb) {
        assert(is_valid(c));
        const std::pair<int8_t, int8_t> offs[] = {
            {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1} };
        for (const auto& off : offs) {
            const auto ca = c.get_off(off.first, off.second);
            if (is_invalid(ca)) continue;
            if (!cb(ca)) return;
        }
    }

    template<typename CbT>
    void foreach_knight_attack(Coord c, const CbT& cb) {
        assert(is_valid(c));
        const std::pair<int8_t, int8_t> offs[] = {
            {2,1}, {1,2}, {-1,2}, {-2,1}, {-2,-1}, {-1,-2}, {1,-2}, {2,-1} };
        for (const auto& off : offs) {
            const auto ca = c.get_off(off.first, off.second);
            if (is_invalid(ca)) continue;
            if (!cb(ca)) return;
        }
    }

    template<typename CbT>
    void foreach_in_dir(Coord c, int rank_off, int file_off, const CbT& cb) {
        assert(is_valid(c));
        while (true) {
            c = c.get_off(rank_off, file_off);
            if (is_invalid(c)) return;
            if (!cb(c)) return;
        }
    }

    template<Player player_>
    bool is_attacked(Coord c0, const SquareReadView<player_>& sv) {
        assert(is_valid(c0));
        bool attacked = false;

        foreach_vicinity(c0, [&sv, &attacked](Coord c) {
            if (sv(c) == Square::BlackKing) { attacked = true; return false; }
            return true;
        });
        if (attacked) return true;

        foreach_knight_attack(c0, [&sv, &attacked](Coord c) {
            if (sv(c) == Square::BlackKnight) { attacked = true; return false; }
            return true;
        });
        if (attacked) return true;

        const std::pair<int8_t, int8_t> hv_dir_offs[] = {{0,-1}, {0,1}, {-1,0}, {1,0}};

        for (const auto off : hv_dir_offs) {
            foreach_in_dir(c0, off.first, off.second, [&sv, &attacked](Coord c) {
                const auto sq = sv(c);
                if (sq == Square::BlackRook || sq == Square::BlackQueen) {
                    attacked = true;
                }
                return is_empty(sq);
            });
            if (attacked) return true;
        }

        const std::pair<int8_t, int8_t> diag_dir_offs[] = {{-1,-1}, {1,1}, {-1,1}, {1,-1}};

        for (const auto off : diag_dir_offs) {
            foreach_in_dir(c0, off.first, off.second, [&sv, &attacked](Coord c) {
                const auto sq = sv(c);
                if (sq == Square::BlackBishop || sq == Square::BlackQueen) {
                    attacked = true;
                    return false;
                }
                return true;
            });
            if (attacked) return true;
        }

        for (int i : {-1, 1}) {
            auto c = c0.get_off(1, i);
            if (is_invalid(c)) continue;
            if (sv(c) == Square::BlackPawn) return true;
        }

        return false;
    }

    template<Player player_>
    Coord find_white_king(const SquareReadView<player_>& sv) {
        for (uint8_t rank = 1; rank <= 8; ++rank) {
            for (uint8_t file = 1; file <= 8; ++file) {
                auto c = Coord{rank, file};
                if (sv(c) == Square::WhiteKing)
                    return c;
            }
        }
        return Coord::invalid();
    }

    template<Player player_>
    MoveCoordList _get_legal_moves(const State& s, const SquareReadView<player_>& sv, MoveCoordList&& out)
    {
        assert(s.player_to_move == sv.player());
        out.clear();

        auto white_king_coord = find_white_king(sv);
        auto en_passant_coord = s.en_passant_file != 0 ? sv.view_coord(Coord{6, s.en_passant_file}) : Coord::invalid();

        for (uint8_t rank = 1; rank <= 8; ++rank) {
            for (uint8_t file = 1; file <= 8; ++file) {
                const auto c = Coord{rank, file};
                auto add_move = [&out, &s, &sv, c, white_king_coord](Coord c_to) {
                    assert(is_white(sv(c)));
                    const auto c_from_view = sv.view_coord(c);
                    const auto c_to_view = sv.view_coord(c_to);

                    auto king_coord_after_move = (sv(c) == Square::WhiteKing) ? c_to : white_king_coord;
                    bool is_king_in_check_after_move;
                    if (is_valid(king_coord_after_move)) {
                        auto temp_move = ScopedTemporaryPieceMover{const_cast<State&>(s), c_from_view, c_to_view};
                        is_king_in_check_after_move = is_attacked(king_coord_after_move, sv);
                    }
                    else {
                        // There is no king on the chessboard.
                        is_king_in_check_after_move = false;
                    }

                    if (!is_king_in_check_after_move)
                        out.push_back(MoveCoord{c_from_view, c_to_view});
                };

                const auto sq = sv(c);
                switch (sq) {
                    case Square::WhitePawn: {
                        assert(c.rank < 8);
                        auto c_up = c.get_off(1, 0);
                        assert(is_valid(c_up));

                        if (is_empty(sv(c_up))) {
                            add_move(c_up);

                            if (rank == 2) {
                                auto c_2up = c.get_off(2, 0);
                                if (is_valid(c_2up) && is_empty(sv(c_2up))) {
                                    add_move(c_2up);
                                }
                            }

                        }

                        for (int i : {-1, 1}) {
                            auto c_d = c.get_off(1, i);
                            if (is_invalid(c_d)) continue;
                            if (is_black(sv(c_d)) || c_d == en_passant_coord) {
                                add_move(c_d);
                            }
                        }

                        break;
                    }

                    case Square::WhiteKnight: {
                        foreach_knight_attack(c, [&](Coord c_to) {
                            if (!is_white(sv(c_to))) add_move(c_to);
                            return true;
                        });
                        break;
                    }

                    case Square::WhiteBishop: {
                        const std::pair<int8_t, int8_t> diag_dir_offs[] = {{-1,-1}, {1,1}, {-1,1}, {1,-1}};
                        for (const auto off : diag_dir_offs) {
                            foreach_in_dir(c, off.first, off.second, [&](Coord c_to) {
                                const auto sq = sv(c);
                                if (!is_white(sq)) add_move(c_to);
                                return is_empty(sq);
                            });
                        }
                        break;
                    }

                    case Square::WhiteRook: {
                        const std::pair<int8_t, int8_t> hv_dir_offs[] = {{0,-1}, {0,1}, {-1,0}, {1,0}};
                        for (const auto off : hv_dir_offs) {
                            foreach_in_dir(c, off.first, off.second, [&](Coord c_to) {
                                const auto sq = sv(c);
                                if (!is_white(sq)) add_move(c_to);
                                return is_empty(sq);
                            });
                        }
                        break;
                    }

                    case Square::WhiteQueen: {
                        const std::pair<int8_t, int8_t> all_dir_offs[] = {
                            {-1,-1}, {1,1}, {-1,1}, {1,-1}, {0,-1}, {0,1}, {-1,0}, {1,0} };
                        for (const auto off : all_dir_offs) {
                            foreach_in_dir(c, off.first, off.second, [&](Coord c_to) {
                                const auto sq = sv(c);
                                if (!is_white(sq)) add_move(c_to);
                                return is_empty(sq);
                            });
                        }
                        break;
                    }

                    case Square::WhiteKing: {
                        foreach_vicinity(c, [&](Coord c_to) {
                            if (!is_white(sv(c_to))) add_move(c_to);
                            return true;
                        });
                        break;
                    }

                }
            }
        }

        return out;
    }

    inline MoveCoordList get_legal_moves(const State& s, MoveCoordList&& out = {})
    {
        return is_white(s.player_to_move) ?
            _get_legal_moves(s, SquareReadView<Player::White>{s}, std::move(out)) :
            _get_legal_moves(s, SquareReadView<Player::Black>{s}, std::move(out));
    }
}