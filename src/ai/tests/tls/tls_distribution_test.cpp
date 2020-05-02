/**
 * @file: tls_distribution_test.cpp
 *
 * @brief: Test functions for distribution helper functions for controller TLS
 * @author: Jake Tuero
 * Date: April 2020
 * Contact: tuero@ualberta.ca
 */


// Testing framework
#include <catch2/catch.hpp>

// Standard Libary/STL
#include <vector>

// Includes
#include "../../controller/two_level_search/util/tls_distribution.h"


static const double EPSILON = 1e-4;


/**
 * Test the normal distribution pdf
 */
TEST_CASE("TLS normal distribution pdf", "[tls_distribution]") {
    std::vector<double> xs{0, 0, -1, -1, 1, 1};
    std::vector<double> mus{0, 0, 1, 1, 0.1, 0.1};
    std::vector<double> stds{1, 0.5, 1, 0.25, 1, 1.6};
    std::vector<double> pdfs{0.3989422804014, 0.797885, 0.053990966513, 2.02091e-14, 0.266085, 0.212855};

    for (std::size_t i = 0; i < xs.size(); ++i) {
        REQUIRE(std::abs(tlsdist::normal_pdf(xs[i], mus[i], stds[i]) - pdfs[i]) < EPSILON);
    }
}


/**
 * Test the normal distribution cdf
 */
TEST_CASE("TLS normal distribution CDF", "[tls_distribution]") {
    std::vector<double> xs{0, 0, -1, -1, 1, 1, 1};
    std::vector<double> mus{0, 0, 1, 1, 0.1, 0.1, 0.1};
    std::vector<double> stds{1, 0.5, 1, 0.25, 1, 1.6, 0.1};
    std::vector<double> cdfs{0.5, 0.5, 0.022750131948179, 6.22096e-16, 0.81594, 0.713112, 1};

    for (std::size_t i = 0; i < xs.size(); ++i) {
        REQUIRE(std::abs(tlsdist::normal_cdf(xs[i], mus[i], stds[i]) - cdfs[i]) < EPSILON);
    }
}


/**
 * Test the normal distribution survival function
 */
TEST_CASE("TLS normal distribution survival function", "[tls_distribution]") {
    std::vector<double> xs{0, 0, -1, -1, 1, 1, 1};
    std::vector<double> mus{0, 0, 1, 1, 0.1, 0.1, 0.1};
    std::vector<double> stds{1, 0.5, 1, 0.25, 1, 1.6, 0.1};
    std::vector<double> sfs{0.5, 0.5, 0.977249868051821, 0.999999999999999377904, 0.18406, 0.286888, 0};

    for (std::size_t i = 0; i < xs.size(); ++i) {
        REQUIRE(std::abs(tlsdist::normal_survival(xs[i], mus[i], stds[i]) - sfs[i]) < EPSILON);
    }
}


/**
 * Test the normal distribution hazard function
 */
TEST_CASE("TLS normal distribution hazard function", "[tls_distribution]") {
    std::vector<double> xs{0, 0, -1, -1, 1, 1, 1};
    std::vector<double> mus{0, 0, 1, 1, 0.1, 0.1, 0.1};
    std::vector<double> stds{1, 0.5, 1, 0.25, 1, 1.6, 0.1};
    std::vector<double> hfs{0.79788456080286, 1.59577, 0.05524786267899, 2.02091e-14, 1.44564, 0.741944, 91.0852};

    for (std::size_t i = 0; i < xs.size(); ++i) {
        REQUIRE(std::abs(tlsdist::normal_hazard(xs[i], mus[i], stds[i]) - hfs[i]) < EPSILON);
    }
}