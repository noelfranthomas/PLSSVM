#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
static inline void __attribute__((overloadable)) AtomicAdd(__global const double *source, const double delta) {
    union {
        double f;
        ulong i;
    } oldVal;
    union {
        double f;
        ulong i;
    } newVal;
    do {
        oldVal.f = *source;
        newVal.f = oldVal.f + delta;
        // ++i;
    } while (atom_cmpxchg((volatile __global ulong *) source, oldVal.i, newVal.i) != oldVal.i);
}

static inline void __attribute__((overloadable)) AtomicAdd(__global const float *source, const float delta) {
    union {
        float f;
        unsigned i;
    } oldVal;
    union {
        float f;
        unsigned i;
    } newVal;
    do {
        oldVal.f = *source;
        newVal.f = oldVal.f + delta;
    } while (atom_cmpxchg((volatile __global unsigned *) source, oldVal.i, newVal.i) != oldVal.i);
}

__kernel void device_kernel_linear(__global const real_type *q, __global real_type *ret, __global const real_type *d, __global const real_type *data_d, const real_type QA_cost, const real_type cost, const int num_rows, const int add, const int first_feature, const int last_feature) {
    size_type i = get_group_id(0) * get_local_size(0) * INTERNAL_BLOCK_SIZE;
    size_type j = get_group_id(1) * get_local_size(1) * INTERNAL_BLOCK_SIZE;

    __local real_type data_intern_i[THREAD_BLOCK_SIZE][INTERNAL_BLOCK_SIZE];
    __local real_type data_intern_j[THREAD_BLOCK_SIZE][INTERNAL_BLOCK_SIZE];
    real_type matr[INTERNAL_BLOCK_SIZE][INTERNAL_BLOCK_SIZE] = { { 0.0 } };
    real_type data_j[INTERNAL_BLOCK_SIZE];

    if (i >= j) {
        i += get_local_id(0) * INTERNAL_BLOCK_SIZE;
        j += get_local_id(1) * INTERNAL_BLOCK_SIZE;
        // cache data
        for (int vec_index = first_feature * num_rows; vec_index < last_feature * num_rows; vec_index += num_rows) {
            barrier(CLK_LOCAL_MEM_FENCE);
            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type block_id = 0; block_id < INTERNAL_BLOCK_SIZE; ++block_id) {
                const size_type idx = 0;  // TODO: load parallel
                if (get_local_id(1) == idx) {
                    data_intern_i[get_local_id(0)][block_id] = data_d[block_id + vec_index + i];
                }
                const size_type idx_2 = 0;  // TODO: TODO: load parallel
                if (get_local_id(0) == idx_2) {
                    data_intern_j[get_local_id(1)][block_id] = data_d[block_id + vec_index + j];
                }
            }
            barrier(CLK_LOCAL_MEM_FENCE);

            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type data_index = 0; data_index < INTERNAL_BLOCK_SIZE; ++data_index) {
                data_j[data_index] = data_intern_j[get_local_id(1)][data_index];
            }

            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type l = 0; l < INTERNAL_BLOCK_SIZE; ++l) {
                const real_type data_i = data_intern_i[get_local_id(0)][l];
                #pragma unroll INTERNAL_BLOCK_SIZE
                for (size_type k = 0; k < INTERNAL_BLOCK_SIZE; ++k) {
                    matr[k][l] += data_i * data_j[k];
                }
            }
        }

        #pragma unroll INTERNAL_BLOCK_SIZE
        for (size_type x = 0; x < INTERNAL_BLOCK_SIZE; ++x) {
            real_type ret_jx = 0.0;
            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type y = 0; y < INTERNAL_BLOCK_SIZE; ++y) {
                real_type temp;
                if (first_feature == 0) {
                    temp = (matr[x][y] + QA_cost - q[i + y] - q[j + x]) * add;
                } else {
                    temp = matr[x][y] * add;
                }
                if (i + x > j + y) {
                    // upper triangular matrix
                    AtomicAdd(&ret[i + y], temp * d[j + x]);
                    ret_jx += temp * d[i + y];
                } else if (i + x == j + y) {
                    // diagonal
                    if (first_feature == 0) {
                        ret_jx += (temp + cost * add) * d[i + y];
                    } else {
                        ret_jx += temp * d[i + y];
                    }
                }
            }
            AtomicAdd(&ret[j + x], ret_jx);
        }
    }
}

__kernel void device_kernel_poly(__global const real_type *q, __global real_type *ret, __global const real_type *d, __global const real_type *data_d, const real_type QA_cost, const real_type cost, const int num_rows, const int num_cols, const int add, const real_type degree, const real_type gamma, const real_type coef0) {
    size_type i = get_group_id(0) * get_local_size(0) * INTERNAL_BLOCK_SIZE;
    size_type j = get_group_id(1) * get_local_size(1) * INTERNAL_BLOCK_SIZE;

    __local real_type data_intern_i[THREAD_BLOCK_SIZE][INTERNAL_BLOCK_SIZE];
    __local real_type data_intern_j[THREAD_BLOCK_SIZE][INTERNAL_BLOCK_SIZE];
    real_type matr[INTERNAL_BLOCK_SIZE][INTERNAL_BLOCK_SIZE] = { { 0.0 } };
    real_type data_j[INTERNAL_BLOCK_SIZE];

    if (i >= j) {
        i += get_local_id(0) * INTERNAL_BLOCK_SIZE;
        j += get_local_id(1) * INTERNAL_BLOCK_SIZE;
        // cache data
        for (int vec_index = 0; vec_index < num_cols * num_rows; vec_index += num_rows) {
            barrier(CLK_LOCAL_MEM_FENCE);
            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type block_id = 0; block_id < INTERNAL_BLOCK_SIZE; ++block_id) {
                const size_type idx = 0;  // TODO: load parallel
                if (get_local_id(1) == idx) {
                    data_intern_i[get_local_id(0)][block_id] = data_d[block_id + vec_index + i];
                }
                const size_type idx_2 = 0;  // TODO: TODO: load parallel
                if (get_local_id(0) == idx_2) {
                    data_intern_j[get_local_id(1)][block_id] = data_d[block_id + vec_index + j];
                }
            }
            barrier(CLK_LOCAL_MEM_FENCE);

            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type data_index = 0; data_index < INTERNAL_BLOCK_SIZE; ++data_index) {
                data_j[data_index] = data_intern_j[get_local_id(1)][data_index];
            }

            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type l = 0; l < INTERNAL_BLOCK_SIZE; ++l) {
                const real_type data_i = data_intern_i[get_local_id(0)][l];
                #pragma unroll INTERNAL_BLOCK_SIZE
                for (size_type k = 0; k < INTERNAL_BLOCK_SIZE; ++k) {
                    matr[k][l] += data_i * data_j[k];
                }
            }
        }

        #pragma unroll INTERNAL_BLOCK_SIZE
        for (size_type x = 0; x < INTERNAL_BLOCK_SIZE; ++x) {
            real_type ret_jx = 0.0;
            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type y = 0; y < INTERNAL_BLOCK_SIZE; ++y) {
                const real_type temp = (pow(gamma * matr[x][y] + coef0, degree) + QA_cost - q[i + y] - q[j + x]) * add;
                if (i + x > j + y) {
                    // upper triangular matrix
                    AtomicAdd(&ret[i + y], temp * d[j + x]);
                    ret_jx += temp * d[i + y];
                } else if (i + x == j + y) {
                    // diagonal
                    ret_jx += (temp + cost * add) * d[i + y];
                }
            }
            AtomicAdd(&ret[j + x], ret_jx);
        }
    }
}

__kernel void device_kernel_radial(__global const real_type *q, __global real_type *ret, __global const real_type *d, __global const real_type *data_d, const real_type QA_cost, const real_type cost, const int num_rows, const int num_cols, const int add, const real_type gamma) {
    size_type i = get_group_id(0) * get_local_size(0) * INTERNAL_BLOCK_SIZE;
    size_type j = get_group_id(1) * get_local_size(1) * INTERNAL_BLOCK_SIZE;

    __local real_type data_intern_i[THREAD_BLOCK_SIZE][INTERNAL_BLOCK_SIZE];
    __local real_type data_intern_j[THREAD_BLOCK_SIZE][INTERNAL_BLOCK_SIZE];
    real_type matr[INTERNAL_BLOCK_SIZE][INTERNAL_BLOCK_SIZE] = { { 0.0 } };
    real_type data_j[INTERNAL_BLOCK_SIZE];

    if (i >= j) {
        i += get_local_id(0) * INTERNAL_BLOCK_SIZE;
        j += get_local_id(1) * INTERNAL_BLOCK_SIZE;
        // cache data
        for (int vec_index = 0; vec_index < num_cols * num_rows; vec_index += num_rows) {
            barrier(CLK_LOCAL_MEM_FENCE);
            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type block_id = 0; block_id < INTERNAL_BLOCK_SIZE; ++block_id) {
                const size_type idx = 0;  // TODO: load parallel
                if (get_local_id(1) == idx) {
                    data_intern_i[get_local_id(0)][block_id] = data_d[block_id + vec_index + i];
                }
                const size_type idx_2 = 0;  // TODO: TODO: load parallel
                if (get_local_id(0) == idx_2) {
                    data_intern_j[get_local_id(1)][block_id] = data_d[block_id + vec_index + j];
                }
            }
            barrier(CLK_LOCAL_MEM_FENCE);

            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type data_index = 0; data_index < INTERNAL_BLOCK_SIZE; ++data_index) {
                data_j[data_index] = data_intern_j[get_local_id(1)][data_index];
            }

            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type l = 0; l < INTERNAL_BLOCK_SIZE; ++l) {
                const real_type data_i = data_intern_i[get_local_id(0)][l];
                #pragma unroll INTERNAL_BLOCK_SIZE
                for (size_type k = 0; k < INTERNAL_BLOCK_SIZE; ++k) {
                    matr[k][l] += (data_i - data_j[k]) * (data_i - data_j[k]);
                }
            }
        }

        #pragma unroll INTERNAL_BLOCK_SIZE
        for (size_type x = 0; x < INTERNAL_BLOCK_SIZE; ++x) {
            real_type ret_jx = 0.0;
            #pragma unroll INTERNAL_BLOCK_SIZE
            for (size_type y = 0; y < INTERNAL_BLOCK_SIZE; ++y) {
                const real_type temp = (exp(-gamma * matr[x][y]) + QA_cost - q[i + y] - q[j + x]) * add;
                if (i + x > j + y) {
                    // upper triangular matrix
                    AtomicAdd(&ret[i + y], temp * d[j + x]);
                    ret_jx += temp * d[i + y];
                } else if (i + x == j + y) {
                    // diagonal
                    ret_jx += (temp + cost * add) * d[i + y];
                }
            }
            AtomicAdd(&ret[j + x], ret_jx);
        }
    }
}