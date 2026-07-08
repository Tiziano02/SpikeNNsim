#ifndef UTILITY
#define UTILITY

#include <cstddef>

/**
 * getAvailableRAM – restituisce la RAM attualmente disponibile in byte.
 * Utilizzata da Simulazione per dimensionare i buffer di output.
 * @return dimensione in byte, o un valore di fallback in caso di errore.
 */
size_t getAvailableRAM();

#endif // UTILITY