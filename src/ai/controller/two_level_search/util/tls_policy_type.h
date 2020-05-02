/**
 * @file: tls_policy_type.h
 *
 * @brief: Policy type enum, controls whether to use trivial, DistNet, or Bayesian policy
 * 
 * @author: Jake Tuero
 * Date: April 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef TLS_POLICY_TYPE_H
#define TLS_POLICY_TYPE_H


enum class PolicyType {
    Trivial,                // Policy cost is number of constraints
    DistNet,                // Policy cost is determined from DistNet model
    Bayesian                // Policy cost is determined from Bayesian DistNet model
};

#endif