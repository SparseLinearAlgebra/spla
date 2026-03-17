#ifndef SPLA_PAIR_HPP
#define SPLA_PAIR_HPP

namespace spla {
        struct Pair {
                float weight;
                int vertex;

                Pair(): weight(0), vertex(-1){}
                Pair(float w, int v): weight(w), vertex(v){}

                bool operator<(const Pair& other) const {
                        return weight < other.weight;
                }
        };
}
#endif