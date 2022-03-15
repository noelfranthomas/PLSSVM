/**
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the functionality related to the SYCL backend.
 */

#include "backends/SYCL/@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@/mock_sycl_csvm.hpp"

#include "backends/generic_tests.hpp"  // generic::write_model_test, generic::generate_q_test, generic::device_kernel_test, generic::predict_test, generic::accuracy_test
#include "utility.hpp"                 // util::google_test::parameter_definition, util::google_test::parameter_definition_to_name

#include "plssvm/backends/@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@/csvm.hpp" // plssvm::@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@::csvm
#include "plssvm/backends/SYCL/implementation_type.hpp"                // plssvm::sycl_generic::implementation_type
#include "plssvm/backends/SYCL/kernel_invocation_type.hpp"             // plssvm::sycl_generic::kernel_invocation_type
#include "plssvm/kernel_types.hpp"                                     // plssvm::kernel_type
#include "plssvm/parameter.hpp"                                        // plssvm::parameter

#include "gtest/gtest.h"  // ::testing::StaticAssertTypeEq, ::testing::Test, ::testing::Types, TYPED_TEST_SUITE, TYPED_TEST

// enumerate all floating point type and kernel combinations to test
using parameter_types = ::testing::Types<
    util::google_test::parameter_definition<float, plssvm::kernel_type::linear>,
    util::google_test::parameter_definition<float, plssvm::kernel_type::polynomial>,
    util::google_test::parameter_definition<float, plssvm::kernel_type::rbf>,
    util::google_test::parameter_definition<double, plssvm::kernel_type::linear>,
    util::google_test::parameter_definition<double, plssvm::kernel_type::polynomial>,
    util::google_test::parameter_definition<double, plssvm::kernel_type::rbf>>;

template <typename T>
class @PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM : public ::testing::Test {};
TYPED_TEST_SUITE(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, parameter_types, util::google_test::parameter_definition_to_name);

// check whether the std::string <-> plssvm::sycl::kernel_invocation_type conversions are correct
TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, kernel_invocation_type) {
    // check conversions to std::string
    util::gtest_expect_enum_to_string_string_conversion(plssvm::sycl_generic::kernel_invocation_type::automatic, "automatic");
    util::gtest_expect_enum_to_string_string_conversion(plssvm::sycl_generic::kernel_invocation_type::nd_range, "nd_range");
    util::gtest_expect_enum_to_string_string_conversion(plssvm::sycl_generic::kernel_invocation_type::hierarchical, "hierarchical");
    util::gtest_expect_enum_to_string_string_conversion(static_cast<plssvm::sycl_generic::kernel_invocation_type>(3), "unknown");

    // check conversion from std::string
    util::gtest_expect_string_to_enum_conversion("automatic", plssvm::sycl_generic::kernel_invocation_type::automatic);
    util::gtest_expect_string_to_enum_conversion("AUTOMATIC", plssvm::sycl_generic::kernel_invocation_type::automatic);
    util::gtest_expect_string_to_enum_conversion("nd_range", plssvm::sycl_generic::kernel_invocation_type::nd_range);
    util::gtest_expect_string_to_enum_conversion("ND_RANGE", plssvm::sycl_generic::kernel_invocation_type::nd_range);
    util::gtest_expect_string_to_enum_conversion("hierarchical", plssvm::sycl_generic::kernel_invocation_type::hierarchical);
    util::gtest_expect_string_to_enum_conversion("HIERARCHICAL", plssvm::sycl_generic::kernel_invocation_type::hierarchical);
    util::gtest_expect_string_to_enum_conversion<plssvm::sycl_generic::kernel_invocation_type>("foo");
}

// check whether the std::string <-> plssvm::sycl_generic::implementation_type conversions are correct
TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, implementation_type) {
    // check conversions to std::string
    util::gtest_expect_enum_to_string_string_conversion(plssvm::sycl_generic::implementation_type::automatic, "automatic");
    util::gtest_expect_enum_to_string_string_conversion(plssvm::sycl_generic::implementation_type::dpcpp, "dpcpp");
    util::gtest_expect_enum_to_string_string_conversion(plssvm::sycl_generic::implementation_type::hipsycl, "hipsycl");
    util::gtest_expect_enum_to_string_string_conversion(static_cast<plssvm::sycl_generic::implementation_type>(3), "unknown");

    // check conversion from std::string
    util::gtest_expect_string_to_enum_conversion("automatic", plssvm::sycl_generic::implementation_type::automatic);
    util::gtest_expect_string_to_enum_conversion("AUTOMATIC", plssvm::sycl_generic::implementation_type::automatic);
    util::gtest_expect_string_to_enum_conversion("dpcpp", plssvm::sycl_generic::implementation_type::dpcpp);
    util::gtest_expect_string_to_enum_conversion("DPCPP", plssvm::sycl_generic::implementation_type::dpcpp);
    util::gtest_expect_string_to_enum_conversion("dpc++", plssvm::sycl_generic::implementation_type::dpcpp);
    util::gtest_expect_string_to_enum_conversion("DPC++", plssvm::sycl_generic::implementation_type::dpcpp);
    util::gtest_expect_string_to_enum_conversion("hipsycl", plssvm::sycl_generic::implementation_type::hipsycl);
    util::gtest_expect_string_to_enum_conversion("hipSYCL", plssvm::sycl_generic::implementation_type::hipsycl);
    util::gtest_expect_string_to_enum_conversion<plssvm::sycl::implementation_type>("foo");
}

// check whether the csvm factory function correctly creates a sycl::csvm
TYPED_TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, csvm_factory) {
    generic::csvm_factory_test<plssvm::@PLSSVM_SYCL_BACKEND_NAMESPACE_NAME@::csvm, typename TypeParam::real_type, plssvm::backend_type::sycl, plssvm::sycl_generic::implementation_type::@PLSSVM_SYCL_BACKEND_NAMESPACE_NAME@>();
}

// check whether writing the resulting model file is correct
TYPED_TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, write_model) {
    generic::write_model_test<mock_sycl_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}

// check whether the q vector is generated correctly
TYPED_TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, generate_q) {
    generic::generate_q_test<mock_sycl_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}

// check whether the nd_range device kernels are correct
TYPED_TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, device_kernel_nd_range) {
    generic::device_kernel_test<mock_sycl_csvm, typename TypeParam::real_type, TypeParam::kernel, plssvm::sycl_generic::kernel_invocation_type::nd_range>();
}
// check whether the hierarchical device kernels are correct
TYPED_TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, device_kernel_hierarchical) {
    generic::device_kernel_test<mock_sycl_csvm, typename TypeParam::real_type, TypeParam::kernel, plssvm::sycl_generic::kernel_invocation_type::hierarchical>();
}

// check whether the correct labels are predicted
TYPED_TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, predict) {
    generic::predict_test<mock_sycl_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}

// check whether the accuracy calculation is correct
TYPED_TEST(@PLSSVM_SYCL_BACKEND_INCLUDE_NAME@_CSVM, accuracy) {
    generic::accuracy_test<mock_sycl_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}