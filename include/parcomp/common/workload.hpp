#pragma once

namespace parcomp {

/**
 * @brief Primality test as specified in Assignment 1 PDF (pages 1-2).
 * Represents simulated work performed inside or outside the critical section.
 * 
 * @param p Number to test for primality
 * @return 1 if prime, 0 otherwise
 */
int is_prime(int p);

} // namespace parcomp
