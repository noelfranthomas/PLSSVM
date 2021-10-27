/**
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the functionality related to the CUDA backend.
 */

#include "backends/CUDA/mock_cuda_csvm.hpp"

#include "backends/generic_tests.hpp"  // generic::write_model_test, generic::generate_q_test, generic::device_kernel_test, generic::predict_test, generic::accuracy_test
#include "utility.hpp"                 // util::google_test::parameter_definition, util::google_test::parameter_definition_to_name, EXPECT_THROW_WHAT

#include "plssvm/backends/CUDA/csvm.hpp"        // plssvm::cuda::csvm
#include "plssvm/backends/CUDA/exceptions.hpp"  // plssvm::cuda::backend_exception
#include "plssvm/kernel_types.hpp"              // plssvm::kernel_type
#include "plssvm/parameter.hpp"                 // plssvm::parameter
#include "plssvm/target_platforms.hpp"          // plssvm::target_platform

#include "gtest/gtest.h"  // ::testing::StaticAssertTypeEq, ::testing::Test, ::testing::Types, TYPED_TEST_SUITE, TYPED_TEST, EXPECT_NO_THROW

// enumerate all floating point type and kernel combinations to test
using parameter_types = ::testing::Types<
    util::google_test::parameter_definition<float, plssvm::kernel_type::linear>,
    util::google_test::parameter_definition<float, plssvm::kernel_type::polynomial>,
    util::google_test::parameter_definition<float, plssvm::kernel_type::rbf>,
    util::google_test::parameter_definition<double, plssvm::kernel_type::linear>,
    util::google_test::parameter_definition<double, plssvm::kernel_type::polynomial>,
    util::google_test::parameter_definition<double, plssvm::kernel_type::rbf>>;

template <typename T>
class CUDA_CSVM : public ::testing::Test {};
TYPED_TEST_SUITE(CUDA_CSVM, parameter_types, util::google_test::parameter_definition_to_name);

// check whether the csvm factory function correctly creates a cuda::csvm
TYPED_TEST(CUDA_CSVM, csvm_factory) {
    generic::csvm_factory_test<plssvm::cuda::csvm, typename TypeParam::real_type, plssvm::backend_type::cuda>();
}

// check whether the constructor correctly fails when using an incompatible target platform
TYPED_TEST(CUDA_CSVM, constructor_invalid_target_platform) {
    // create parameter object
    plssvm::parameter<typename TypeParam::real_type> params;
    params.print_info = false;
    params.kernel = TypeParam::kernel;

    params.parse_train_file(PLSSVM_TEST_PATH "/data/libsvm/5x4.libsvm");

    // only automatic or gpu_nvidia are allowed as target platform for the CUDA backend
    params.target = plssvm::target_platform::automatic;
    EXPECT_NO_THROW(mock_cuda_csvm{ params });
    params.target = plssvm::target_platform::cpu;
    EXPECT_THROW_WHAT(mock_cuda_csvm{ params }, plssvm::cuda::backend_exception, "Invalid target platform 'cpu' for the CUDA backend!");
    params.target = plssvm::target_platform::gpu_nvidia;
    EXPECT_NO_THROW(mock_cuda_csvm{ params });
    params.target = plssvm::target_platform::gpu_amd;
    EXPECT_THROW_WHAT(mock_cuda_csvm{ params }, plssvm::cuda::backend_exception, "Invalid target platform 'gpu_amd' for the CUDA backend!");
    params.target = plssvm::target_platform::gpu_intel;
    EXPECT_THROW_WHAT(mock_cuda_csvm{ params }, plssvm::cuda::backend_exception, "Invalid target platform 'gpu_intel' for the CUDA backend!");
}

// check whether writing the resulting model file is correct
TYPED_TEST(CUDA_CSVM, write_model) {
    generic::write_model_test<mock_cuda_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}

// check whether the q vector is generated correctly
TYPED_TEST(CUDA_CSVM, generate_q) {
    generic::generate_q_test<mock_cuda_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}

// check whether the device kernels are correct
TYPED_TEST(CUDA_CSVM, device_kernel) {
    generic::device_kernel_test<mock_cuda_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}

// check whether the correct labels are predicted
TYPED_TEST(CUDA_CSVM, predict) {
    generic::predict_test<mock_cuda_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}

// check whether the accuracy calculation is correct
TYPED_TEST(CUDA_CSVM, accuracy) {
    generic::accuracy_test<mock_cuda_csvm, typename TypeParam::real_type, TypeParam::kernel>();
}
