#include <cassert>
#include <exception>
#include <cmath>

#include <QFile>

#include "model_960_140_21_3.h"


using namespace std;

Model_960_140_21_3::Model_960_140_21_3()
  : model_(nullptr)
{
}

Model_960_140_21_3::~Model_960_140_21_3() {
  delete model_;
}

bool Model_960_140_21_3::LoadModel(const QString& file_name) {
  delete model_;
  model_ = nullptr;
  // Read file data
  QFile model_file(file_name);
  if (!model_file.open(QIODevice::ReadOnly)) {
    return false;
  }
  if (model_file.size() != kFileSize) {
    return false;
  }
  try {
    model_ = new Model;
    static_assert(sizeof(Model) == kFileSize, "Wrong format of model data");
    if (model_file.read((char*)model_, kFileSize) != kFileSize) {
      delete model_;
      model_ = nullptr;
      return false;
    }
  }
  catch (bad_alloc&) {
    return false;
  }
  return true;
}

void Model_960_140_21_3::GetStep(const Field& field, unsigned int& step_row, unsigned int& step_col, bool& sure_step) {
  const float min_clear_probability = 0.6f;
  const float max_mine_probability = 1.0f;
  unsigned int clear_row;
  unsigned int clear_col;
  float clear_probability = min_clear_probability;
  unsigned int non_mine_row;
  unsigned int non_mine_col;
  float mine_probability = max_mine_probability;

  for (size_t row = 0; row < field.size(); ++row) {
    for (size_t col = 0; col < field[row].size(); ++col) {
      if (field[row][col] != '.') {
        continue;
      }
      InputLayer inputs;
      FormInput(field, row, col, inputs);
      LogitsLayer logits;
      unsigned int max_logit;
      CalculateModel(inputs, logits, max_logit);
      if (logits[kClearLogit] > clear_probability) {
        clear_probability = logits[kClearLogit];
        clear_row = row;
        clear_col = col;
      }
      if (logits[kMineLogit] < mine_probability) {
        mine_probability = logits[kMineLogit];
        non_mine_row = row;
        non_mine_col = col;
      }
    }
  }
  if (clear_probability > min_clear_probability) {
    step_row = clear_row;
    step_col = clear_col;
    sure_step = true;
    return;
  }
  if (mine_probability < max_mine_probability) {
    step_row = non_mine_row;
    step_col = non_mine_col;
    sure_step = false;
    return;
  }
  throw runtime_error("There isn't available step");
}

void Model_960_140_21_3::FormInput(const Field& field, unsigned int target_row, unsigned int target_col, InputLayer& inputs) {
  unsigned int input_index = 0;
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
      assert(input_index < kInputs);
      inputs[input_index++] = symbol == ' ' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '.' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '*' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '0' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '1' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '2' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '3' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '4' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '5' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '6' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '7' ? 1 : 0; assert(input_index < kInputs);
      inputs[input_index++] = symbol == '8' ? 1 : 0;
    }
  }
  assert(input_index == kInputs);
}

void Model_960_140_21_3::CalculateModel(const InputLayer& inputs, LogitsLayer& logits, unsigned int& max_logit) {
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
void Model_960_140_21_3::CalculateLayer(const float (&inputs)[input_size]
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
void Model_960_140_21_3::ActivateLayer(float (&layer)[size]) {
  for (unsigned int index = 0; index < size; ++index) {
    if (layer[index] < 0) {
      layer[index] = 0.0f;
    }
  }
}

template<unsigned int size>
void Model_960_140_21_3::LogitsProbability(float (&layer)[size], unsigned int& max_index) {
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
