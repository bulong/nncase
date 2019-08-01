#include "../tflite_importer.h"
#include <ir/ops/matmul.h>
#include <ir/ops/reshape.h>

using namespace nncase;
using namespace nncase::importer;
using namespace nncase::ir;

DEFINE_TFLITE_LOWER(FULLY_CONNECTED)
{
    auto &input_a = get_tensor(op.inputs(), 0);
    auto &input_b = get_tensor(op.inputs(), 1);
    auto &bias = get_tensor(op.inputs(), 2);
    auto &options = *op.builtin_options_as_FullyConnectedOptions();
    auto in_shape_a = get_shape(*input_a.shape());

    assert(options.weights_format() == tflite::FullyConnectedOptionsWeightsFormat_DEFAULT);

    auto bias_tensor = load_tensor<float, 1>(bias);

    auto input_b_trans = graph_.emplace<transpose>(dt_float32, get_shape(*input_b.shape()), axis_t { 1, 0 });
    auto rshape = graph_.emplace<reshape>(dt_float32, in_shape_a, axis_t { -1, (int32_t)in_shape_a.back() });
    auto fc = graph_.emplace<matmul>(rshape->output().shape(), input_b_trans->output().shape(), std::move(bias_tensor),
        to_float_clamp_range(options.fused_activation_function()));
    fc->input_a().connect(rshape->output());
    fc->input_b().connect(input_b_trans->output());

    input_tensors_.emplace(&rshape->input(), op.inputs()->Get(0));
    input_tensors_.emplace(&input_b_trans->input(), op.inputs()->Get(1));
    output_tensors_.emplace(op.outputs()->Get(0), &fc->output());
}
