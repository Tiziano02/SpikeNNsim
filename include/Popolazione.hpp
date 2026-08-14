/**
 * @file Popolazione.hpp
 * @brief Definizione del concetto di popolazione di neuroni.
 *
 * Una popolazione è un gruppo contiguo di neuroni all'interno della rete,
 * identificato dall'indice di partenza e dal numero di neuroni.
 * Fornisce metodi per operare collettivamente su tutti i neuroni della popolazione.
 */

#ifndef POPOLAZIONE_HPP
#define POPOLAZIONE_HPP

#include <cstddef> // per std::size_t

/**
 * @ingroup publicapi
 * @brief Rappresenta un gruppo omogeneo di neuroni contigui nella rete.
 *
 * @details
 * Una popolazione è un wrapper leggero che definisce un intervallo contiguo
 * di neuroni all'interno del vettore `neuroni_` di `Rete`.
 * Tutti i metodi operano su tutti i neuroni dell'intervallo [start, start+size).
 *
 * L'oggetto Popolazione non possiede i neuroni, ma fornisce un'interfaccia
 * per operare su di essi collettivamente.
 */
class Popolazione {

    friend class Rete; // Permette a Rete di accedere agli attributi privati

  public:
    /**
     * @brief Costruttore con parametri.
     * @param start Indice del primo neurone della popolazione.
     * @param size Numero di neuroni nella popolazione.
     * @param
     */
    Popolazione(size_t start, size_t size) : indexStart_(start), size_(size) {}

    /**
     * @brief Distruttore di default.
     */
    ~Popolazione() = default;

  private:
    // -- ATTRIBUTI PRIVATI --------------------------------------------------------------

    size_t indexStart_; ///< Indice del primo neurone nel vettore neuroni_ di Rete
    size_t size_;       ///< Numero di neuroni nella popolazione

    // -- METODI PRIVATI -----------------------------------------------------------------

    // 1. metodi getter
    inline size_t getStart() const { return indexStart_; }
    inline size_t getSize() const { return size_; }
};

#endif // POPOLAZIONE_HPP