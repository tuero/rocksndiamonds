/**
 * @file: two_level_search.h
 *
 * @brief: Distribution helper functions for the policy cost
 * 
 * @author: Jake Tuero
 * Date: April 2020
 * Contact: tuero@ualberta.ca
 */

#ifndef TLS_DISTRIBUTION_H
#define TLS_DISTRIBUTION_H

// Standard Libary/STL
#include <cmath>


namespace tlsdist {


/**
 * Calculate the PDF of the normal distribution at the given point
 * 
 * @param x The point to evaluate the pdf at
 * @param mu The distribution mean parameter
 * @param sigma The distribution standard deviation parameter
 */
template <typename T>
T normal_pdf(T x, T mu, T sigma) {
    static const T inv_sqrt_2pi = 0.3989422804014327;
    T z = (x - mu) / sigma;
    return inv_sqrt_2pi / sigma * std::exp(-T(0.5) * z * z);
}


/**
 * Calculate the CDF of the normal distribution at the given point
 * 
 * @param x The point to evaluate the pdf at
 * @param mu The distribution mean parameter
 * @param sigma The distribution standard deviation parameter
 */
template <typename T>
T normal_cdf(T x, T mu, T sigma) {
    static const T sqrt_2 = 1.414213562373095;
    T z = (x - mu) / sigma;
    return 0.5 * (1 + std::erf(z / sqrt_2));
}


/**
 * Calculate the survival function (1-CDF) of the normal distribution at the given point
 * 
 * @param x The point to evaluate the pdf at
 * @param mu The distribution mean parameter
 * @param sigma The distribution standard deviation parameter
 */
template <typename T>
T normal_survival(T x, T mu, T sigma) {
    return 1.0 - normal_cdf(x, mu, sigma);
}


/**
 * Calculate the hazard function of the normal distribution at the given point
 * 
 * @param x The point to evaluate the pdf at
 * @param mu The distribution mean parameter
 * @param sigma The distribution standard deviation parameter
 */
template <typename T>
T normal_hazard(T x, T mu, T sigma) {
    static const T sqrt_2_over_pi = 0.7978845608028653558;
    static const T inv_sqrt_2 = 0.707106781186547524;
    T z = (x - mu) / sigma;
    return (sqrt_2_over_pi * std::exp(-T(0.5) * z * z)) / (sigma * std::erfc(z * inv_sqrt_2));

}

} // namespace tlsdist


#endif