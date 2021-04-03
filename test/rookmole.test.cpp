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

#include <chrono>
#include <iostream>
#include <type_traits>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rookmole/rookmole.h>
using namespace rookmole;

TEST_CASE("openings", "[get_legal_moves]") {
    const auto s = make_start_state();
    REQUIRE(!s.a1_castling_forbidden);
    REQUIRE(!s.h1_castling_forbidden);
    REQUIRE(!s.a8_castling_forbidden);
    REQUIRE(!s.h8_castling_forbidden);
    REQUIRE(is_white(s.player_to_move));
    REQUIRE(s.en_passant_file == 0);
    REQUIRE(s.move_count == 0);
    REQUIRE(get_legal_moves(s) == make_move_coord_vec("b1:c3 b1:a3 g1:h3 g1:f3 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 d2:d3 d2:d4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
}

TEMPLATE_TEST_CASE("pawn", "[get_legal_moves]", (std::integral_constant<bool, false>), (std::integral_constant<bool, true>)) {
    constexpr bool reverse = TestType::value;
    auto s = make_custom_state("pa2 | pb3", Player::White, reverse);
    REQUIRE(get_legal_moves(s) == make_move_coord_vec("a2:a3 a2:a4 a2:b3", reverse));
}

TEMPLATE_TEST_CASE("en_passant", "[get_legal_moves]", (std::integral_constant<bool, false>), (std::integral_constant<bool, true>)) {
    constexpr bool reverse = TestType::value;
    auto s = make_custom_state("pf5 | pg5", Player::White, reverse);
    s.en_passant_file = reverse ? 2 : 7;
    REQUIRE(get_legal_moves(s) == make_move_coord_vec("f5:f6 f5:g6", reverse));
}

TEMPLATE_TEST_CASE("knight", "[get_legal_moves]", (std::integral_constant<bool, false>), (std::integral_constant<bool, true>)) {
    constexpr bool reverse = TestType::value;
    auto s = make_custom_state("Nd4 pf3 | Qc6 pf4", Player::White, reverse);
    REQUIRE(get_legal_moves(s) == make_move_coord_vec("d4:e6 d4:f5 d4:e2 d4:c2 d4:b3 d4:b5 d4:c6", reverse));
}

TEMPLATE_TEST_CASE("reveal_check", "[get_legal_moves]", (std::integral_constant<bool, false>), (std::integral_constant<bool, true>)) {
    constexpr bool reverse = TestType::value;
    auto s = make_custom_state("Kd3 pd4 | Nc5 Rd7", Player::White, reverse);
    REQUIRE(get_legal_moves(s) == make_move_coord_vec("d3:e3 d3:e2 d3:d2 d3:c2 d3:c3 d3:c4", reverse));
}

TEMPLATE_TEST_CASE("stalemate", "[get_legal_moves]", (std::integral_constant<bool, false>), (std::integral_constant<bool, true>)) {
    constexpr bool reverse = TestType::value;
    auto s = make_custom_state("Kb1 | pc2 Bh8 Nb4 Ne2", Player::White, reverse);
    REQUIRE(get_legal_moves(s) == make_move_coord_vec(""));
}

TEMPLATE_TEST_CASE("castling", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    auto s = make_start_state();
    s.player_to_move = TestType::value;
    for (int f = 2; f <= 7; ++f) {
        if (f == 5) continue;  // Save the king.
        s.set_square(Coord{f, 1}, Square::Empty);
        s.set_square(Coord{f, 8}, Square::Empty);
    }

    if (is_white(TestType::value)) {
        REQUIRE(get_legal_moves(s) == make_move_coord_vec("a1:b1 a1:c1 a1:d1 e1:f1 e1:d1 e1:c1 e1:g1 h1:g1 h1:f1 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 d2:d3 d2:d4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
    }
    else {
        REQUIRE(get_legal_moves(s) == make_move_coord_vec("h8:g8 h8:f8 e8:d8 e8:f8 e8:g8 e8:c8 a8:b8 a8:c8 a8:d8 h7:h6 h7:h5 g7:g6 g7:g5 f7:f6 f7:f5 e7:e6 e7:e5 d7:d6 d7:d5 c7:c6 c7:c5 b7:b6 b7:b5 a7:a6 a7:a5"));
    }
}

TEMPLATE_TEST_CASE("castling_blocked", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    auto s = make_start_state();
    s.player_to_move = TestType::value;
    for (int f = 2; f <= 7; ++f) {
        if (f == 4 || f == 5) continue;  // Save the king and queen (for now).
        s.set_square(Coord{f, 1}, Square::Empty);
        s.set_square(Coord{f, 8}, Square::Empty);
    }

    s.set_square(Coord{is_white(TestType::value) ? "d1" : "d8"}, Square::Empty);
    s.set_square(Coord{"d2"}, Square::Empty);
    s.set_square(Coord{"d7"}, Square::Empty);

    if (is_white(TestType::value)) {
        REQUIRE(get_legal_moves(s) == make_move_coord_vec("a1:b1 a1:c1 a1:d1 e1:f1 e1:g1 h1:g1 h1:f1 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
    }
    else {
        REQUIRE(get_legal_moves(s) == make_move_coord_vec("h8:g8 h8:f8 e8:f8 e8:g8 a8:b8 a8:c8 a8:d8 h7:h6 h7:h5 g7:g6 g7:g5 f7:f6 f7:f5 e7:e6 e7:e5 c7:c6 c7:c5 b7:b6 b7:b5 a7:a6 a7:a5"));
    }
}

TEMPLATE_TEST_CASE("en_passant_capture", "[make_move]", (std::integral_constant<bool, false>), (std::integral_constant<bool, true>)) {
    constexpr bool reverse = TestType::value;
    auto s0 = make_custom_state("pf5 | pg5 ph7", Player::White, reverse);
    s0.en_passant_file = reverse ? 2 : 7;

    auto n1 = make_move(s0, make_move_coord<reverse>("f5:g6"));
    REQUIRE(n1.state.move_count == (is_white(s0.player_to_move) ? 0 : 1));
    REQUIRE(piece_of(n1.state(make_coord<reverse>("g6"))) == Piece::Pawn);
    REQUIRE(is_empty(n1.state(make_coord<reverse>("g5"))));
    REQUIRE(n1.next_moves == make_move_coord_vec("h7:g6 h7:h6 h7:h5", reverse));
    REQUIRE(!n1.king_in_check);

    auto n2 = make_move(n1.state, make_move_coord<reverse>("h7:h5"));
    REQUIRE(n2.state.move_count == (is_white(s0.player_to_move) ? 1 : 1));
    REQUIRE(n2.next_moves == make_move_coord_vec("g6:g7", reverse));
    REQUIRE(!n2.king_in_check);
}

TEST_CASE("italian_game", "[make_move]") {
    const auto s0 = make_start_state();
    auto n = GameNode{s0, get_legal_moves(s0), false};

    auto moves = make_move_coord_vec("e2:e4 e7:e5 g1:f3 b8:c6 f1:c4 g8:f6 e1:g1 f8:c5 d2:d3 e8:g8");
    for (const auto move : moves) {
        n = make_move(n.state, move);
    }
}

TEST_CASE("openings_depth3", "[perf]") {
    using Clock = std::chrono::high_resolution_clock;
    constexpr size_t depth = 3;
    const auto s0 = make_start_state();
    auto msv = std::vector<GameNode>{GameNode{std::move(s0), get_legal_moves(s0), false}};

    auto start_time = Clock::now();
    for (size_t i = 0; i < depth; ++i) {
        auto msv_next = std::vector<GameNode>{};
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
    const auto s0 = make_start_state();
    const auto lm0 = get_legal_moves(s0);

    auto start_time = Clock::now();
    for (size_t i = 0; i < game_count; ++i) {
        auto n = GameNode{s0, lm0, false};

        do {
            if (n.next_moves.empty()) break;
            int move_index = (int)(rand() % n.next_moves.size());
            n = make_move(n.state, n.next_moves[move_index]);
        } while (!n.next_moves.empty() && n.state.move_count <= 80);
    }

    auto end_time = Clock::now();
    auto dur_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Random games played (80 moves): " << (1000.0 * (double)game_count / (double)dur_msec) << " games/sec" << std::endl;
}
