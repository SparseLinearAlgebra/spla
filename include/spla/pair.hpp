#ifndef SPLA_PAIR_HPP
#define SPLA_PAIR_HPP
#include <limits>


namespace spla {
        struct Pair {
                float weight;
                int vertex;

                Pair(): weight(std::numeric_limits<float>::infinity()), vertex(-1){}
                Pair(float w, int v): weight(w), vertex(v){}

                bool operator<(const Pair& other) const {
                        return weight < other.weight;
                }
                bool operator==(const Pair& other) const {
                        return weight == other.weight && vertex == other.vertex;
                }
                
                bool operator!=(const Pair& other) const {
                        return !(*this == other);
                }
                
                Pair& operator=(const Pair& other) = default;
        };
}
#endif