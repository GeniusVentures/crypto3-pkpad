//---------------------------------------------------------------------------//
// Copyright (c) 2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Ilias Khairullin <ilias@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE emsa1_encoding_test

#include <iostream>
#include <unordered_map>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <nil/crypto3/pkpad/emsa/emsa1.hpp>

#include <nil/crypto3/pkpad/algorithms/encode.hpp>
#include <nil/crypto3/pkpad/algorithms/verify.hpp>

#include <nil/crypto3/algebra/curves/bls12.hpp>
#include <nil/crypto3/algebra/curves/secp_r1.hpp>

#include <nil/crypto3/hash/sha1.hpp>
#include <nil/crypto3/hash/sha2.hpp>

using namespace nil::crypto3;

template<typename FieldParams>
void print_field_element(std::ostream &os, const typename algebra::fields::detail::element_fp<FieldParams> &e) {
    os << e.data;
}

namespace boost {
    namespace test_tools {
        namespace tt_detail {
            template<typename FieldParams>
            struct print_log_value<typename algebra::fields::detail::element_fp<FieldParams>> {
                void operator()(std::ostream &os, typename algebra::fields::detail::element_fp<FieldParams> const &e) {
                    print_field_element(os, e);
                }
            };

            template<template<typename, typename> class P, typename K, typename V>
            struct print_log_value<P<K, V>> {
                void operator()(std::ostream &, P<K, V> const &) {
                }
            };
        }    // namespace tt_detail
    }        // namespace test_tools
}    // namespace boost

const char *test_data = "../../../../libs/pkpad/test/data/emsa.json";

boost::property_tree::ptree string_data(std::string test_name) {
    boost::property_tree::ptree string_data;
    boost::property_tree::read_json(test_data, string_data);
    return string_data.get_child(test_name);
}

template<typename ElementType>
struct field_element_init;

template<typename FieldParams>
struct field_element_init<algebra::fields::detail::element_fp<FieldParams>> {
    using element_type = algebra::fields::detail::element_fp<FieldParams>;

    template<typename ElementData>
    static inline element_type process(const ElementData &element_data) {
        return element_type(typename element_type::integral_type(element_data.second.data()));
    }
};

template<typename FieldValueType, typename Padding, typename DataSet>
void test_emsa(const DataSet &array_element) {
    auto text = array_element.first;
    std::vector<uint8_t> data(text.begin(), text.end());
    FieldValueType out = pubkey::encode<Padding>(array_element.first);
    FieldValueType etalon_out = field_element_init<FieldValueType>::process(array_element);
    BOOST_CHECK_EQUAL(out, etalon_out);
}

// test data generated by botan
BOOST_AUTO_TEST_SUITE(emsa_test_suite)

BOOST_AUTO_TEST_CASE(emsa1_bls12_fr_single_range_encode) {
    using curve_type = algebra::curves::bls12_381;
    using field_type = typename curve_type::scalar_field_type;
    using field_value_type = typename field_type::value_type;
    using hash_type = hashes::sha1;
    using padding_type = pubkey::padding::emsa1<field_value_type, hash_type>;

    std::vector<std::uint8_t> in {0, 0, 0, 0, 1};
    field_value_type out = pubkey::encode<padding_type>(in);
    print_field_element(std::cout, out);
}

BOOST_AUTO_TEST_CASE(emsa1_secp256r1_fr_manual_conformity_test) {
    using curve_type = algebra::curves::secp256r1;
    using field_type = typename curve_type::scalar_field_type;
    using field_value_type = typename field_type::value_type;
    using integral_type = typename field_type::integral_type;
    using hash_type = hashes::sha2<256>;
    using padding_type = pubkey::padding::emsa1<field_value_type, hash_type>;

    std::string text("This is a tasty burger!");
    std::vector<uint8_t> data(text.data(), text.data() + text.length());
    field_value_type out = pubkey::encode<padding_type>(data);
    field_value_type etalon_out =
        integral_type("111474717792720247796999809655932432881783035037226574051829933946736885398526");
    BOOST_CHECK_EQUAL(out, etalon_out);
}

BOOST_DATA_TEST_CASE(emsa1_sha256_secp256r1_fr_conformity_test, string_data("emsa1_sha256_secp256r1_fr"), array_element) {
    using curve_type = algebra::curves::secp256r1;
    using field_type = typename curve_type::scalar_field_type;
    using field_value_type = typename field_type::value_type;
    using hash_type = hashes::sha2<256>;
    using padding_type = pubkey::padding::emsa1<field_value_type, hash_type>;

    test_emsa<field_value_type, padding_type>(array_element);
}

BOOST_DATA_TEST_CASE(emsa1_sha512_secp256r1_fr_conformity_test, string_data("emsa1_sha512_secp256r1_fr"), array_element) {
    using curve_type = algebra::curves::secp256r1;
    using field_type = typename curve_type::scalar_field_type;
    using field_value_type = typename field_type::value_type;
    using hash_type = hashes::sha2<512>;
    using padding_type = pubkey::padding::emsa1<field_value_type, hash_type>;

    test_emsa<field_value_type, padding_type>(array_element);
}

BOOST_AUTO_TEST_SUITE_END()