
#include <chrono>
#include <iostream>
#include <type_traits>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rookmole/rookmole.h>
using namespace rockmole;

TEST_CASE("start", "[get_legal_moves]") {
    const auto s = make_start_state();
    REQUIRE(s.white_short_castling_possible);
    REQUIRE(s.white_long_castling_possible);
    REQUIRE(s.black_short_castling_possible);
    REQUIRE(s.black_long_castling_possible);
    REQUIRE(is_white(s.player_to_move));
    REQUIRE(s.en_passant_file == 0);
    REQUIRE(s.move_count == 0);
}

TEMPLATE_TEST_CASE("pawn", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("pa2 | pb3", Player::White, reverse);
    auto lm = get_legal_moves(s);

    if (reverse) other_player_inplace(lm);
    REQUIRE(lm == make_coord_moves("a2:a3 a2:a4 a2:b3"));
}

TEMPLATE_TEST_CASE("en_passant", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr Player p = TestType::value;
    constexpr bool reverse = p == Player::Black;
    auto s = make_custom_state("pf5 | pg5", Player::White, reverse);
    s.en_passant_file = reverse ? 2 : 7;
    auto lm = get_legal_moves(s);

    if (reverse) other_player_inplace(lm);
    REQUIRE(lm == make_coord_moves("f5:f6 f5:g6"));
}

TEMPLATE_TEST_CASE("knight", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("Nd4 pf3 | Qc6 pf4", Player::White, reverse);
    auto lm = get_legal_moves(s);

    if (reverse) other_player_inplace(lm);
    REQUIRE(lm == make_coord_moves("d4:e6 d4:f5 d4:e2 d4:c2 d4:b3 d4:b5 d4:c6"));
}

TEMPLATE_TEST_CASE("reveal_check", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("Kd3 pd4 | Nc5 Rd7", Player::White, reverse);
    auto lm = get_legal_moves(s);

    if (reverse) other_player_inplace(lm);
    REQUIRE(lm == make_coord_moves("d3:e3 d3:e2 d3:d2 d3:c2 d3:c3 d3:c4"));
}

TEST_CASE("openings", "[get_legal_moves]") {
    const auto s = make_start_state();
    auto lm = get_legal_moves(s);
    REQUIRE(lm == make_coord_moves("b1:c3 b1:a3 g1:h3 g1:f3 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 d2:d3 d2:d4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
}

TEMPLATE_TEST_CASE("stalemate", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("Kb1 | pc2 Bh8 Nb4 Ne2", Player::White, reverse);
    auto lm = get_legal_moves(s);

    if (reverse) other_player_inplace(lm);
    REQUIRE(lm == make_coord_moves(""));
}

TEMPLATE_TEST_CASE("castling", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    auto s = make_start_state();
    s.player_to_move = TestType::value;
    for (int f = 2; f <= 7; ++f) {
        if (f == 5) continue;  // Save the king.
        s.set_square(Coord{1,f}, Square::Empty);
        s.set_square(Coord{8,f}, Square::Empty);
    }

    auto lm = get_legal_moves(s);

    if (TestType::value == Player::White) {
        REQUIRE(lm == make_coord_moves("a1:b1 a1:c1 a1:d1 e1:f1 e1:d1 e1:c1 e1:g1 h1:g1 h1:f1 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 d2:d3 d2:d4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
    }
    else {
        REQUIRE(lm == make_coord_moves("h8:g8 h8:f8 e8:d8 e8:f8 e8:g8 e8:c8 a8:b8 a8:c8 a8:d8 h7:h6 h7:h5 g7:g6 g7:g5 f7:f6 f7:f5 e7:e6 e7:e5 d7:d6 d7:d5 c7:c6 c7:c5 b7:b6 b7:b5 a7:a6 a7:a5"));
    }
}

TEMPLATE_TEST_CASE("castling_blocked", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    auto s = make_start_state();
    s.player_to_move = TestType::value;
    for (int f = 2; f <= 7; ++f) {
        if (f == 4 || f == 5) continue;  // Save the king and queen (for now).
        s.set_square(Coord{1,f}, Square::Empty);
        s.set_square(Coord{8,f}, Square::Empty);
    }

    if (TestType::value == Player::White) {
        s.set_square(Coord{1,4}, Square::Empty);
    }
    else {
        s.set_square(Coord{8,4}, Square::Empty);
    }

    s.set_square(Coord{2,4}, Square::Empty);
    s.set_square(Coord{7,4}, Square::Empty);

    auto lm = get_legal_moves(s);

    if (TestType::value == Player::White) {
        REQUIRE(lm == make_coord_moves("a1:b1 a1:c1 a1:d1 e1:f1 e1:g1 h1:g1 h1:f1 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
    }
    else {
        REQUIRE(lm == make_coord_moves("h8:g8 h8:f8 e8:f8 e8:g8 a8:b8 a8:c8 a8:d8 h7:h6 h7:h5 g7:g6 g7:g5 f7:f6 f7:f5 e7:e6 e7:e5 c7:c6 c7:c5 b7:b6 b7:b5 a7:a6 a7:a5"));
    }
}

TEMPLATE_TEST_CASE("en_passant_capture", "[make_move]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr Player p = TestType::value;
    constexpr bool reverse = p == Player::Black;
    auto s0 = make_custom_state("pf5 | pg5 ph7", Player::White, reverse);
    s0.en_passant_file = reverse ? 2 : 7;
    auto sv0 = SquareView<p>{s0};
    auto m0 = view_move(MoveCoord{{5,6}, {6,7}}, sv0);

    auto ms1 = make_move(s0, m0);
    auto sv1 = SquareView<p>{ms1.state};
    REQUIRE(ms1.state.move_count == (is_white(p) ? 0 : 1));
    REQUIRE(sv1(Coord{6,7}) == Square::WhitePawn);
    REQUIRE(is_empty(sv1(Coord{5,7})));
    REQUIRE(ms1.next_moves == view_moves(make_coord_moves("h7:g6 h7:h6 h7:h5"), sv1));
    REQUIRE(!ms1.king_in_check);
    auto m1 = view_move(MoveCoord{{7,8}, {5,8}}, sv0);

    auto ms2 = make_move(ms1.state, m1);
    auto sv2 = SquareView<p>{ms2.state};
    REQUIRE(ms2.state.move_count == (is_white(p) ? 1 : 1));
    REQUIRE(ms2.next_moves == view_moves(make_coord_moves("g6:g7"), sv2));
    REQUIRE(!ms2.king_in_check);
}

TEST_CASE("italian_game", "[make_move]") {
    const auto s0 = make_start_state();
    const auto lm0 = get_legal_moves(s0);
    auto ms = MoveState{std::move(s0), std::move(lm0), false};

    ms = make_move(ms.state, {{2,5},{4,5}});
    ms = make_move(ms.state, {{7,5},{5,5}});
    ms = make_move(ms.state, {{1,7},{3,6}});
    ms = make_move(ms.state, {{8,2},{6,3}});
    ms = make_move(ms.state, {{1,6},{4,3}});
    ms = make_move(ms.state, {{8,7},{6,6}});
    ms = make_move(ms.state, {{1,5},{1,7}});
    ms = make_move(ms.state, {{8,6},{5,3}});
    ms = make_move(ms.state, {{2,4},{3,4}});
    ms = make_move(ms.state, {{8,5},{8,7}});
}

TEST_CASE("openings_depth3", "[perf]") {
    using Clock = std::chrono::high_resolution_clock;
    constexpr size_t depth = 3;
    const auto s0 = make_start_state();
    const auto lm0 = get_legal_moves(s0);
    auto msv = std::vector<MoveState>{MoveState{std::move(s0), std::move(lm0), false}};

    auto start_time = Clock::now();
    for (size_t i = 0; i < depth; ++i) {
        auto msv_next = std::vector<MoveState>{};
        msv_next.reserve(msv.size() * 32);
        for (const auto& ms : msv) {
            for (const auto m : ms.next_moves) {
                msv_next.push_back(make_move(ms.state, m));
            }
        }
        msv = std::move(msv_next);
    }

    auto end_time = Clock::now();
    auto dur_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Generated all " << msv.size() << " states for depth " << depth << " in " << (double)dur_msec / 1000.0 << " sec" << std::endl;
}

TEST_CASE("random_moves", "[perf]") {
    using Clock = std::chrono::high_resolution_clock;
    constexpr size_t game_count = 1000;
    auto s0 = make_start_state();
    auto lm0 = get_legal_moves(s0);

    auto start_time = Clock::now();
    for (size_t i = 0; i < game_count; ++i) {
        auto ms = MoveState{s0, lm0, false};

        do {
            if (ms.next_moves.empty()) break;
            int move_index = (int)(rand() % ms.next_moves.size());
            auto m = ms.next_moves[move_index];
            ms = make_move(ms.state, ms.next_moves[move_index]);
        } while (!ms.next_moves.empty() && ms.state.move_count <= 80);
    }

    auto end_time = Clock::now();
    auto dur_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Random games played (80 moves): " << (1000.0 * (double)game_count / (double)dur_msec) << " games/sec" << std::endl;
}
