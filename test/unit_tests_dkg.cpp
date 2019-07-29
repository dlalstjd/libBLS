/*
  Copyright (C) 2018-2019 SKALE Labs

  This file is part of libBLS.

  libBLS is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  libBLS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with libBLS.  If not, see <https://www.gnu.org/licenses/>.

  @file unit_tests_dkg.cpp
  @author Oleh Nikolaiev
  @date 2019
*/


#include <dkg/dkg.h>

#include <cstdlib>
#include <ctime>
#include <map>
#include <set>

#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/algebra/exponentiation/exponentiation.hpp>

#include "bls/BLSPrivateKeyShare.h"
#include "bls/BLSSigShareSet.h"
#include "bls/BLSSignature.h"
#include "bls/BLSPublicKey.h"
#include "bls/BLSPrivateKey.h"
#include "bls/BLSutils.cpp"

#define BOOST_TEST_MODULE

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DkgAlgorithm)

    BOOST_AUTO_TEST_CASE(PolynomialValue) {
        signatures::Dkg obj = signatures::Dkg(3, 4);
        std::vector<libff::alt_bn128_Fr> polynomial = {libff::alt_bn128_Fr("1"),
                                                       libff::alt_bn128_Fr("0"), libff::alt_bn128_Fr("1")};

        libff::alt_bn128_Fr value = obj.PolynomialValue(polynomial, 5);

        BOOST_REQUIRE(value == libff::alt_bn128_Fr("26"));

        polynomial.clear();

        polynomial = {libff::alt_bn128_Fr("0"),
                      libff::alt_bn128_Fr("1"), libff::alt_bn128_Fr("0")};
        bool is_exception_caught = false;

        try {
            value = obj.PolynomialValue(polynomial, 5);
        } catch (std::runtime_error) {
            is_exception_caught = true;
        }

        BOOST_REQUIRE(is_exception_caught);
    }

    BOOST_AUTO_TEST_CASE(verification) {
        signatures::Dkg obj = signatures::Dkg(2, 2);

        auto polynomial_fst = obj.GeneratePolynomial();
        auto polynomial_snd = obj.GeneratePolynomial();

        std::vector<libff::alt_bn128_G2> verification_vector_fst = obj.VerificationVector(polynomial_fst);
        std::vector<libff::alt_bn128_G2> verification_vector_snd = obj.VerificationVector(polynomial_snd);

        libff::alt_bn128_Fr shared_by_fst_to_snd = obj.SecretKeyContribution(polynomial_snd)[1];
        libff::alt_bn128_Fr shared_by_snd_to_fst = obj.SecretKeyContribution(polynomial_fst)[0];

        BOOST_REQUIRE(obj.Verification(0, shared_by_snd_to_fst, verification_vector_fst));
        BOOST_REQUIRE(obj.Verification(1, shared_by_fst_to_snd, verification_vector_snd));


        // these lines show that only correctly generated by the algorithm values can be verified
        BOOST_REQUIRE(obj.Verification(0, shared_by_snd_to_fst + libff::alt_bn128_Fr::random_element(),
                                       verification_vector_fst) == false);
        BOOST_REQUIRE(obj.Verification(1, shared_by_fst_to_snd + libff::alt_bn128_Fr::random_element(),
                                       verification_vector_snd) == false);
    }

    std::default_random_engine rand_gen((unsigned int) time(0));



BOOST_AUTO_TEST_SUITE_END()
