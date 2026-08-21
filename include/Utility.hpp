/**
 * @file Utility.hpp
 * @brief Funzioni di utilità a livello di sistema operativo.
 */

#ifndef UTILITY
#define UTILITY

#include <cstddef>
#include <random>

/**
 * @ingroup internals
 * @brief Interroga il sistema operativo per determinare la memoria disponibile.
 * * @details Utilizzata internamente dalla classe `Simulazione` per allocare
 * dinamicamente in modo sicuro i chunk di buffer I/O (flush automatico)
 * in base alla RAM fisica rimasta sulla macchina.
 * * @return Dimensione stimata della RAM libera in byte, oppure un valore di fallback di sicurezza.
 */
size_t getAvailableRAM();

enum class DistType {
    Gaussian, ///< Distribuzione normale
    Uniform,  ///< Distribuzione uniforme
    LogNormal ///< Distribuzione log-normale
};

// Da commentare e controlare - Fatto da Deepseek e Gemini interamente

template <typename T> T generaNumero(T min, T max, DistType tipo = DistType::Uniform) {

    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());

    if (min == max)
        return min;

    if constexpr (std::is_integral_v<T>) {
        switch (tipo) {
        case DistType::Uniform: {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(gen);
        }
        case DistType::Gaussian: {
            double mean = (static_cast<double>(min) + max) / 2.0;
            double sigma = (max - min) / 6.0;
            if (sigma == 0.0)
                return min; // evita loop infinito
            std::normal_distribution<double> dist(mean, sigma);
            double value;
            do {
                value = dist(gen);
            } while (value < min || value > max);
            return static_cast<T>(std::round(value));
        }
        case DistType::LogNormal: {
            if (min <= 0)
                min = 1; // log‑normale definita solo per > 0
            double log_min = std::log(static_cast<double>(min));
            double log_max = std::log(static_cast<double>(max));
            double mu = (log_min + log_max) / 2.0;
            double sigma = (log_max - log_min) / 6.0;
            if (sigma == 0.0)
                return min;
            std::lognormal_distribution<double> dist(mu, sigma);
            double value;
            do {
                value = dist(gen);
            } while (value < min || value > max);
            return static_cast<T>(std::round(value));
        }
        }
    } else {
        switch (tipo) {
        case DistType::Uniform: {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(gen);
        }
        case DistType::Gaussian: {
            double mean = (min + max) / 2.0;
            double sigma = (max - min) / 6.0;
            if (sigma == 0.0)
                return min;
            std::normal_distribution<double> dist(mean, sigma);
            double value;
            do {
                value = dist(gen);
            } while (value < min || value > max);
            return static_cast<T>(value);
        }
        case DistType::LogNormal: {
            if (min <= 0.0)
                min = 1e-6;
            double log_min = std::log(min);
            double log_max = std::log(max);
            double mu = (log_min + log_max) / 2.0;
            double sigma = (log_max - log_min) / 6.0;
            if (sigma == 0.0)
                return min;
            std::lognormal_distribution<double> dist(mu, sigma);
            double value;
            do {
                value = dist(gen);
            } while (value < min || value > max);
            return static_cast<T>(value);
        }
        }
    }
    return min; // fallback
}

template <typename T> std::vector<T> generaNumeroVettore(size_t n, T min, T max, DistType tipo = DistType::Uniform) {
    std::vector<T> result;
    result.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        result.push_back(generaNumero<T>(min, max, tipo));
    }
    return result;
}

template <typename T>
T generaAttorno(T mean, double dispersione, DistType tipo = DistType::Gaussian, bool absolute = true) {
    // Motori indipendenti per ogni thread: zero colli di bottiglia nel calcolo parallelo!
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());

    if (dispersione == 0.0)
        return mean;

    double mean_d = static_cast<double>(mean);
    double sigma = absolute ? dispersione : std::abs(mean_d) * dispersione;

    // Gestione rigorosa degli errori tramite eccezioni
    if (tipo == DistType::LogNormal && mean_d <= 0.0) {
        throw std::invalid_argument("Errore critico: la distribuzione LogNormal richiede mean > 0");
    }

    double valore;

    switch (tipo) {
    case DistType::Uniform: {
        double low = mean_d - sigma;
        double high = mean_d + sigma;
        std::uniform_real_distribution<double> dist(low, high);
        valore = dist(gen);
        break;
    }
    case DistType::Gaussian: {
        std::normal_distribution<double> dist(mean_d, sigma);
        valore = dist(gen);
        break;
    }
    case DistType::LogNormal: {
        // La tua formula magistrale per preservare il valore atteso
        double sigma_log = sigma / mean_d;
        double mu = std::log(mean_d) - 0.5 * (sigma_log * sigma_log);
        std::lognormal_distribution<double> dist(mu, sigma_log);
        valore = dist(gen);
        break;
    }
    default:
        valore = mean_d;
    }

    // Risoluzione a compile-time: i numeri negativi ora sono liberi di esistere!
    if constexpr (std::is_integral_v<T>) {
        return static_cast<T>(std::round(valore));
    } else {
        return static_cast<T>(valore);
    }
}

template <typename T>
std::vector<T> generaAttornoVettore(const std::vector<T>& means, double dispersione, DistType tipo = DistType::Gaussian,
                                    bool absolute = true, bool keepSum = false) {
    std::vector<T> values;
    values.reserve(means.size());

    // 1. Genera valori casuali
    for (T mean : means) {
        values.push_back(generaAttorno<T>(mean, dispersione, tipo, absolute));
    }

    // 2. Se richiesto, mantieni la somma invariata
    if (keepSum) {
        double targetSum = std::accumulate(means.begin(), means.end(), 0.0);
        double currentSum = std::accumulate(values.begin(), values.end(), 0.0);

        if (currentSum != 0.0) {
            double scale = targetSum / currentSum;

            // Applica il fattore di scala
            for (auto& v : values) {
                if constexpr (std::is_integral_v<T>) {
                    v = static_cast<T>(std::round(static_cast<double>(v) * scale));
                } else {
                    v *= scale;
                }
            }

            // Correggi eventuali errori di arrotondamento per interi
            if constexpr (std::is_integral_v<T>) {
                int currentTotal = std::accumulate(values.begin(), values.end(), 0);
                int diff = static_cast<int>(targetSum) - currentTotal;
                if (diff != 0 && !values.empty()) {
                    values[0] += diff;
                    if (values[0] < 0)
                        values[0] = 0;
                }
            }
        }
    }

    return values;
}

#endif // UTILITY