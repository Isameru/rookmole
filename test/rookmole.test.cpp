
#include <iostream>
#include <type_traits>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rookmole/rookmole.h>
using namespace rockmole;

TEMPLATE_TEST_CASE("start", "[moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr Player p = TestType::value;
    const auto s = make_start_state(p);

    REQUIRE(s.white_short_castling_possible);
    REQUIRE(s.white_long_castling_possible);
    REQUIRE(s.black_short_castling_possible);
    REQUIRE(s.black_long_castling_possible);
    REQUIRE(s.player_to_move == p);
    REQUIRE(s.en_passant_file == 0);
    REQUIRE(s.move_count == 0);
}

TEMPLATE_TEST_CASE("pawn", "[moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("pa2 | pb3", Player::White, reverse);
    auto ms = get_legal_moves(s);

    if (reverse) other_player_inplace(ms);
    REQUIRE(ms == make_coord_moves("a2:a3 a2:a4 a2:b3"));
}

TEMPLATE_TEST_CASE("en_passant", "[moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr Player p = TestType::value;
    constexpr bool reverse = p == Player::Black;
    auto s = make_custom_state("pf5 | pg5", Player::White, reverse);
    auto sv = SquareView<p>{s};
    s.en_passant_file = reverse ? 2 : 7;
    auto ms = get_legal_moves(s);

    if (reverse) other_player_inplace(ms);
    REQUIRE(ms == make_coord_moves("f5:f6 f5:g6"));
}

TEMPLATE_TEST_CASE("knight", "[moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("Nd4 pf3 | Qc6 pf4", Player::White, reverse);
    auto ms = get_legal_moves(s);

    if (reverse) other_player_inplace(ms);
    REQUIRE(ms == make_coord_moves("d4:e6 d4:f5 d4:e2 d4:c2 d4:b3 d4:b5 d4:c6"));
}

TEMPLATE_TEST_CASE("reveal_check", "[moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("Kd3 pd4 | Nc5 Rd7", Player::White, reverse);
    auto ms = get_legal_moves(s);

    if (reverse) other_player_inplace(ms);
    REQUIRE(ms == make_coord_moves("d3:e3 d3:e2 d3:d2 d3:c2 d3:c3 d3:c4"));
}

TEMPLATE_TEST_CASE("openings", "[moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr Player p = TestType::value;
    constexpr bool reverse = p == Player::Black;
    const auto s = make_start_state(p);
    auto ms = get_legal_moves(s);

    if (reverse) other_player_inplace(ms);
    REQUIRE(ms == make_coord_moves("b1:c3 b1:a3 g1:h3 g1:f3 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 d2:d3 d2:d4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
}

TEMPLATE_TEST_CASE("stalemate", "[moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("Kb1 | pc2 Bh8 Nb4 Ne2", Player::White, reverse);
    auto ms = get_legal_moves(s);

    if (reverse) other_player_inplace(ms);
    REQUIRE(ms == make_coord_moves(""));
}
