#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <cmath>

#include "tetragonal_neural.h"

using namespace std;

ModelTetragonalNeural::ModelTetragonalNeural()
  : model_(nullptr)
{
}

ModelTetragonalNeural::~ModelTetragonalNeural() {
}

bool ModelTetragonalNeural::LoadModel(std::vector<uint8_t>&& data) {
  model_ = nullptr;
  model_data_.clear();
  // Read file data
  if (data.size() != kFileSize) {
    return false;
  }
  model_data_ = data;
  model_ = (Model*)model_data_.data();
  return true;
}

void ModelTetragonalNeural::GetStep(const Field& field, unsigned int, unsigned int& step_row, unsigned int& step_col, StepAction& step) {
  Probabilities probes;
  GetProbabilities(field, probes, true);
  if (probes.empty()) {
    throw runtime_error("There isn't available step");
  }
  auto best_clear = probes.begin();
  auto worst_mine = probes.begin();
  for (auto iter = probes.begin(); iter != probes.end(); ++iter) {
    if (iter->value.clear_probe > best_clear->value.clear_probe) {
      best_clear = iter;
    }
    if (iter->value.mine_probe < worst_mine->value.mine_probe) {
      worst_mine = iter;
    }
  }
  if (best_clear->value.clear_probe >= kMinClearProbability) {
    step_row = best_clear->row;
    step_col = best_clear->col;
    step = kOpenWithSure;
    return;
  }
  step_row = worst_mine->row;
  step_col = worst_mine->col;
  step = worst_mine->value.mine_probe < kMaxMineProbability ? kOpenWithSure : kOpenWithProbability;
}

void ModelTetragonalNeural::GetTestResponse(TestResponse& response) {
  Field field;
  Probabilities probes;
  GetTestField(field);
  GetProbabilities(field, probes, false);
  sort(probes.begin(), probes.end(), [](const ProbeInfo& a1, const ProbeInfo& a2) -> bool {
    if (a2.row > a1.row) { return true; }
    if (a2.row < a1.row) { return false; }
    if (a2.col > a1.col) { return true; }
    return false;
  });
  response.clear();
  for (auto iter = probes.begin(); iter != probes.end(); ++iter) {
    response.push_back(iter->value);
  }
}

void ModelTetragonalNeural::FormInput(const Field& field, unsigned int target_row, unsigned int target_col, InputLayer& inputs) {
  unsigned int cell_index = 0;
  for (int row = (int)target_row - kMarginSize; row <= (int)target_row + kMarginSize; ++row) {
    for (int col = (int)target_col - kMarginSize; col <= (int)target_col + kMarginSize; ++col) {
      if (row == (int)target_row && col == (int)target_col) {
        continue;
      }
      char symbol = ' ';
      if (row >= 0 && row < (int)field.size()) {
        if (col >= 0 && col < (int)field[row].size()) {
          symbol = field[row][col];
        }
      }
      // Fill inputs
      unsigned int input_index = cell_index * kInputsPerCell;
      assert(input_index <= (kInputs - kInputsPerCell));
      inputs[input_index++] = symbol == ' ' ? 1 : 0;
      inputs[input_index++] = symbol == '.' ? 1 : 0;
      inputs[input_index++] = symbol == '*' ? 1 : 0;
      inputs[input_index++] = symbol == '0' ? 1 : 0;
      inputs[input_index++] = symbol == '1' ? 1 : 0;
      inputs[input_index++] = symbol == '2' ? 1 : 0;
      inputs[input_index++] = symbol == '3' ? 1 : 0;
      inputs[input_index++] = symbol == '4' ? 1 : 0;
      inputs[input_index++] = symbol == '5' ? 1 : 0;
      inputs[input_index++] = symbol == '6' ? 1 : 0;
      inputs[input_index++] = symbol == '7' ? 1 : 0;
      inputs[input_index++] = symbol == '8' ? 1 : 0;
      ++cell_index;
    }
  }
}

void ModelTetragonalNeural::CalculateModel(const InputLayer& inputs, LogitsLayer& logits, unsigned int& max_logit) {
  if (!model_) {
    throw runtime_error("Model don't loaded");
  }
  float layer0[kLayer0Nodes];
  float layer1[kLayer1Nodes];
  float layer2[kLayer2Nodes];
  CalculateLayer(inputs, layer0, model_->kernel0, model_->bias0);
  ActivateLayer(layer0);
  CalculateLayer(layer0, layer1, model_->kernel1, model_->bias1);
  ActivateLayer(layer1);
  CalculateLayer(layer1, layer2, model_->kernel2, model_->bias2);
  ActivateLayer(layer2);
  CalculateLayer(layer2, logits, model_->kernel_logits, model_->bias_logits);
  LogitsProbability(logits, max_logit);
}

template<unsigned int input_size, unsigned int output_size>
void ModelTetragonalNeural::CalculateLayer(const float (&inputs)[input_size]
    , float (&outputs)[output_size]
    , const float (&kernel)[input_size][output_size]
    , const float (&bias)[output_size]) {
  for (unsigned int index = 0; index < output_size; ++index) {
    outputs[index] = 0;
  }
  for (unsigned int input_index = 0; input_index < input_size; ++input_index) {
    for (unsigned int output_index = 0; output_index < output_size; ++output_index) {
      outputs[output_index] += inputs[input_index] * kernel[input_index][output_index];
    }
  }
  for (unsigned int output_index = 0; output_index < output_size; ++output_index) {
    outputs[output_index] += bias[output_index];
  }
}

template<unsigned int size>
void ModelTetragonalNeural::ActivateLayer(float (&layer)[size]) {
  for (unsigned int index = 0; index < size; ++index) {
    if (layer[index] < 0) {
      layer[index] = 0.0f;
    }
  }
}

template<unsigned int size>
void ModelTetragonalNeural::LogitsProbability(float (&layer)[size], unsigned int& max_index) {
  assert(size != 0);
  max_index = 0;
  float max_value = -1.0f;
  float probability_summ = 0.0f;
  for (unsigned int index = 0; index < size; ++index) {
    layer[index] = exp(layer[index]);
    probability_summ += layer[index];
    if (layer[index] > max_value) {
      max_index = index;
      max_value = layer[index];
    }
  }
  assert(max_value >= 0);
  assert(probability_summ > 0.0f);
  for (unsigned int index = 0; index < size; ++index) {
    layer[index] /= probability_summ;
  }
}

void ModelTetragonalNeural::GetProbabilities(const Field& field, Probabilities& probes, bool hide_cells_only) {
  for (unsigned int row = 0; row < field.size(); ++row) {
    for (unsigned int col = 0; col < field[row].size(); ++col) {
      if (hide_cells_only && field[row][col] != kHideCell) {
        continue;
      }
      InputLayer inputs;
      FormInput(field, row, col, inputs);
      LogitsLayer logits;
      unsigned int max_logit;
      CalculateModel(inputs, logits, max_logit);
      ProbeInfo info;
      info.row = row;
      info.col = col;
      info.value.clear_probe = logits[kClearLogit];
      info.value.mine_probe = logits[kMineLogit];
      info.value.unknown_probe = logits[kUnknownLogit];
      probes.push_back(info);
    }
  }
}
