
#include <iostream>
#include <type_traits>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rookmole/rookmole.h>
using namespace rockmole;

TEMPLATE_TEST_CASE("start", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
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

TEMPLATE_TEST_CASE("openings", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr Player p = TestType::value;
    constexpr bool reverse = p == Player::Black;
    const auto s = make_start_state(p);
    auto lm = get_legal_moves(s);

    if (reverse) other_player_inplace(lm);
    REQUIRE(lm == make_coord_moves("b1:c3 b1:a3 g1:h3 g1:f3 a2:a3 a2:a4 b2:b3 b2:b4 c2:c3 c2:c4 d2:d3 d2:d4 e2:e3 e2:e4 f2:f3 f2:f4 g2:g3 g2:g4 h2:h3 h2:h4"));
}

TEMPLATE_TEST_CASE("stalemate", "[get_legal_moves]", (std::integral_constant<Player, Player::White>), (std::integral_constant<Player, Player::Black>)) {
    constexpr bool reverse = TestType::value == Player::Black;
    auto s = make_custom_state("Kb1 | pc2 Bh8 Nb4 Ne2", Player::White, reverse);
    auto lm = get_legal_moves(s);

    if (reverse) other_player_inplace(lm);
    REQUIRE(lm == make_coord_moves(""));
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
    REQUIRE(ms1.state.move_count == 0);
    REQUIRE(sv1(Coord{6,7}) == Square::WhitePawn);
    REQUIRE(is_empty(sv1(Coord{5,7})));
    REQUIRE(ms1.next_moves == view_moves(make_coord_moves("h7:g6 h7:h6 h7:h5"), sv1));
    REQUIRE(!ms1.king_in_check);
    auto m1 = view_move(MoveCoord{{7,8}, {5,8}}, sv0);

    auto ms2 = make_move(ms1.state, m1);
    auto sv2 = SquareView<p>{ms2.state};
    REQUIRE(ms2.state.move_count == 1);
    REQUIRE(ms2.next_moves == view_moves(make_coord_moves("g6:g7"), sv2));
    REQUIRE(!ms2.king_in_check);
}
