/**
 * @file Utility.hpp
 * @brief Funzioni di utilità a livello di sistema operativo.
 */

#ifndef UTILITY
#define UTILITY

#include <cstddef>

/**
 * @ingroup internals
 * @brief Interroga il sistema operativo per determinare la memoria disponibile.
 * * @details Utilizzata internamente dalla classe `Simulazione` per allocare
 * dinamicamente in modo sicuro i chunk di buffer I/O (flush automatico)
 * in base alla RAM fisica rimasta sulla macchina.
 * * @return Dimensione stimata della RAM libera in byte, oppure un valore di fallback di sicurezza.
 */
size_t getAvailableRAM();

#endif // UTILITY