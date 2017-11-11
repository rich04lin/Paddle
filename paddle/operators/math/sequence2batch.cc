/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/operators/math/sequence2batch.h"

namespace paddle {
namespace operators {
namespace math {

template <typename T>
class CopyMatrixRowsFunctor<platform::CPUPlace, T> {
 public:
  void operator()(const platform::DeviceContext& context,
                  const framework::Tensor& src, const size_t* index,
                  framework::Tensor& dst, bool is_src_index) {
    auto src_dims = src.dims();
    auto dst_dims = dst.dims();
    PADDLE_ENFORCE_EQ(src_dims.size(), 2UL,
                      "The src must be matrix with rank 2.");
    PADDLE_ENFORCE_EQ(dst_dims.size(), 2UL,
                      "The dst must be matrix with rank 2.");
    PADDLE_ENFORCE_EQ(src_dims[1], dst_dims[1],
                      "The width of src and dst must be same.");
    auto height = dst_dims[0];
    auto width = dst_dims[1];
    auto* src_data = src.data<T>();
    auto* dst_data = dst.data<T>();
    for (int i = 0; i < height; ++i) {
      if (is_src_index) {
        memcpy(dst_data + i * width, src_data + index[i] * width,
               width * sizeof(T));
      } else {
        memcpy(dst_data + index[i] * width, src_data + i * width,
               width * sizeof(T));
      }
    }
  }
};

template class CopyMatrixRowsFunctor<platform::CPUPlace, float>;
template class CopyMatrixRowsFunctor<platform::CPUPlace, double>;

template class LoDTensor2BatchFunctor<platform::CPUPlace, float>;
template class LoDTensor2BatchFunctor<platform::CPUPlace, double>;
template class Batch2LoDTensorFunctor<platform::CPUPlace, float>;
template class Batch2LoDTensorFunctor<platform::CPUPlace, double>;

template <typename T>
struct RowwiseAdd<platform::CPUPlace, T> {
  void operator()(const platform::DeviceContext& context,
                  const framework::Tensor& input, const framework::Tensor& bias,
                  framework::Tensor* output) {
    auto in_dims = input.dims();
    auto size = input.numel() / in_dims[0];
    PADDLE_ENFORCE_EQ(bias.numel(), size);
    PADDLE_ENFORCE_EQ(output->dims(), in_dims);

    auto in = EigenMatrix<T>::From(input);
    auto b = EigenMatrix<T>::From(bias);
    auto out = EigenMatrix<T>::From(*output);
    Eigen::array<int, 2> bshape({{1, static_cast<int>(size)}});
    Eigen::array<int, 2> bcast({{static_cast<int>(in_dims[0]), 1}});
    out.device(*context.GetEigenDevice<platform::CPUPlace>()) =
        in + b.reshape(bshape).broadcast(bcast);
  }
};

template struct RowwiseAdd<platform::CPUPlace, float>;
template struct RowwiseAdd<platform::CPUPlace, double>;

}  // namespace math
}  // namespace operators
}  // namespace paddle
