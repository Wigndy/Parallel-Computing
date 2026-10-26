#include "parcomp/common/workload.hpp"

namespace parcomp {

int is_prime(int p) {
    int d;
    for (d = 2; d < p; d = d + 1) {
        if (p % d == 0) {
            return 0;
        }
    }
    return 1;
}

} // namespace parcomp
