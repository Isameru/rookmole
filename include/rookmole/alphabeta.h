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

#include "rookmole/evaluation.h"
#include "rookmole/state.h"
#include <limits>

namespace rookmole {

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

// Alpha-Beta prunning algorithm: https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning

struct SearchResult {
    MoveCoord move;
    int value;
};

template<bool maximize>
inline SearchResult alphabeta(Player eval_player, const GameNode& node, int depth, int alpha, int beta) noexcept {
    if (depth == 0 || is_terminal(node)) {
        return SearchResult{MoveCoord{}, evaluate_hardcode(eval_player, node)};
    }

    auto best_result = SearchResult{};

    const size_t child_count = node.next_moves.size();

    auto child_nodes = std::vector<GameNode>{};
    child_nodes.reserve(child_count);
    for (const auto move : node.next_moves) {
        child_nodes.push_back(make_move(node.state, move));
    }

    auto child_scores = std::vector<int>{};
    child_scores.reserve(child_count);
    for (const auto& child_node : child_nodes) {
        child_scores.push_back(evaluate_hardcode(child_node.state.player_to_move, child_node));
    }

    auto search_order_indices = std::vector<size_t>{};
    search_order_indices.reserve(child_count);
    for (size_t i = 0; i < child_count; ++i) {
        search_order_indices.push_back(i);
    }

    std::sort(std::begin(search_order_indices), std::end(search_order_indices), [&](int i1, int i2) {
        return child_scores[i1] < child_scores[i2];
    });

    if (maximize) {
        best_result.value = std::numeric_limits<int>::min();
        for (size_t search_index = 0; search_index < child_count; ++search_index) {
            const int child_index = search_order_indices[search_index];
            const auto& child_node = child_nodes[child_index];
            const auto child_result = alphabeta<false>(eval_player, child_node, depth - 1, alpha, beta);
            if (child_result.value > best_result.value) {
                best_result.value = child_result.value;
                best_result.move = node.next_moves[child_index];
            }

            alpha = std::max(alpha, child_result.value);
            if (alpha >= beta) {
                break;  // Beta-cutoff
            }
        }
    }
    else {
        best_result.value = std::numeric_limits<int>::max();
        for (size_t search_index = 0; search_index < child_count; ++search_index) {
            const int child_index = search_order_indices[search_index];
            const auto& child_node = child_nodes[child_index];
            const auto child_result = alphabeta<true>(eval_player, child_node, depth - 1, alpha, beta);
            if (child_result.value < best_result.value) {
                best_result.value = child_result.value;
                best_result.move = node.next_moves[child_index];
            }

            beta = std::min(beta, child_result.value);
            if (beta <= alpha) {
                break;  // Alpha-cutoff
            }
        }
    }

    return best_result;
}

inline SearchResult alphabeta(const GameNode& node, int depth) noexcept {
    return alphabeta<true>(node.state.player_to_move, node, depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
}

//=≡=-=♔=-=≡=-=♕=-=≡=-=♖=-=≡=-=♗=-=≡=-=♘=-=≡=-=♙=-=≡=-=♚=-=≡=-=♛=-=≡=-=♜=-=≡=-=♝=-=≡=-=♞=-=≡=-=♟︎=-

} // namespace rookmole
