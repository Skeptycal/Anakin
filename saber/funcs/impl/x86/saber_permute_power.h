/* Copyright (c) 2018 Anakin Authors, Inc. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef ANAKIN_SABER_FUNCS_IMPL_X86_SABER_PERMUTE_POWER_H
#define ANAKIN_SABER_FUNCS_IMPL_X86_SABER_PERMUTE_POWER_H

#include "saber/funcs/impl/impl_permute_power.h"

namespace anakin{

namespace saber{

template <DataType OpDtype>
class SaberPermutePower<X86, OpDtype>:\
    public ImplBase<
        X86,
        OpDtype,
        PermutePowerParam<X86> > {

public:

    SaberPermutePower() {}

    ~SaberPermutePower() {}

    virtual SaberStatus init(const std::vector<Tensor<X86>*>& inputs,
                             std::vector<Tensor<X86>*>& outputs,
                             PermutePowerParam<X86> &param,
                             Context<X86> &ctx) {
        this->_ctx = &ctx;
        return create(inputs, outputs, param, ctx);
    }

    virtual SaberStatus create(const std::vector<Tensor<X86>*>& inputs,
                               std::vector<Tensor<X86>*>& outputs,
                               PermutePowerParam<X86> &param,
                               Context<X86> &ctx) {
        _num_axes = inputs[0]->shape().size();
        PermuteParam<X86> permute_param = param.permute_param;
        for (int i = 0; i < _num_axes; i++) {
            if (std::find(_order_dims.begin(), _order_dims.end(), permute_param.order[i]) == _order_dims.end()) {
                _order_dims.push_back(permute_param.order[i]);
            }
        }
        CHECK_EQ(_num_axes, _order_dims.size());

        // set _need_permute
        _need_permute = false;
        for (int i = 0; i < _num_axes; ++i) {
            if (permute_param.order[i] != i) {
                _need_permute = true;
                break;
            }
        }
        Shape order_shape({_num_axes, 1, 1, 1});
        _permute_order.re_alloc(order_shape, OpDtype);
        _old_steps.re_alloc(order_shape, OpDtype);
        _new_steps.re_alloc(order_shape, OpDtype);
        _out_valid_shape.re_alloc(order_shape, OpDtype);
        Shape in_stride = inputs[0]->get_stride();
        Shape out_stride = outputs[0]->get_stride();
        Shape out_valid_shape  = outputs[0]->valid_shape();
        memcpy(_old_steps.mutable_data(), &in_stride[0], sizeof(int) * _num_axes);
        memcpy(_new_steps.mutable_data(), &out_stride[0], sizeof(int) * _num_axes);
        memcpy(_permute_order.mutable_data(), &(permute_param.order[0]), sizeof(int) * _num_axes);
        memcpy(_out_valid_shape.mutable_data(), &out_valid_shape[0], sizeof(int) * _num_axes);
        return SaberSuccess;
    }

    virtual SaberStatus dispatch(const std::vector<Tensor<X86>*>& inputs,
                                 std::vector<Tensor<X86>*>& outputs,
                                 PermutePowerParam<X86> &param);

private:
    int _num_axes;
    bool _need_permute;
    std::vector<int> _order_dims;
    Tensor<X86> _permute_order;
    Tensor<X86> _out_valid_shape;
    Tensor<X86> _old_steps;
    Tensor<X86> _new_steps;
};

} //namespace saber

} //namespace anakin

#endif //ANAKIN_SABER_FUNCS_IMPL_X86_SABER_PERMUTE_POWER_H
