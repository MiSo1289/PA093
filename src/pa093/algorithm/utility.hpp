#pragma once

#include <iterator>

namespace pa093::algorithm
{

template<typename Sequence>
requires std::bidirectional_iterator<typename Sequence::iterator>
auto
swap_back_and_pop(Sequence& sequence, typename Sequence::iterator pos) ->
    typename Sequence::value_type
{
    auto const back = std::prev(sequence.end());
    std::ranges::iter_swap(pos, back);

    auto elem = std::ranges::iter_move(back);
    sequence.pop_back();

    return elem;
}

} // namespace pa093::algorithm
