/* Minesweeper Bot: Cross-platform bot for playing in minesweeper game.
 * Copyright (C) 2019 Evgeny Kislov.
 * https://www.evgenykislov.com/minesweeper-bot
 * https://github.com/evgenykislov/minesweeper-bot
 *
 * This file is part of Minesweeper Bot.
 *
 * Minesweeper Bot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Minesweeper Bot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Minesweeper Bot.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MODEL_960_140_21_3_H
#define MODEL_960_140_21_3_H

#include <cstdint>

#include "classifier.h"

class ModelTetragonalNeural: public Classifier
{
 public:
  struct ProbeValues {
    float clear_probe;
    float mine_probe;
    float unknown_probe;
  };
  typedef std::vector<ProbeValues> TestResponse;

  ModelTetragonalNeural();
  virtual ~ModelTetragonalNeural();
  virtual bool LoadModel(std::vector<uint8_t>&& data) override;
  virtual void GetStep(const Field& field, unsigned int mines_amount, unsigned int& step_row, unsigned int& step_col, StepAction& action) override;
  virtual void GetTestResponse(TestResponse& response);

 private:
  ModelTetragonalNeural(const ModelTetragonalNeural&) = delete;
  ModelTetragonalNeural(ModelTetragonalNeural&&) = delete;
  ModelTetragonalNeural& operator=(const ModelTetragonalNeural&) = delete;
  ModelTetragonalNeural& operator=(ModelTetragonalNeural&&) = delete;

  enum {
    kFileSize = 4240512,
    kInputs = 960,
    kInputsPerCell = 12,
    kLayer0Nodes = 960,
    kLayer1Nodes = 140,
    kLayer2Nodes = 21,
    kLayerLogitsNodes = 3,
    kMarginSize = 4,
    kRowsAmount = 9,
  };

  enum {
    kClearLogit = 0,
    kMineLogit = 1,
    kUnknownLogit = 2,
  };

  const float kMinClearProbability = 0.6f;
  const float kMaxMineProbability = 0.2f;

  typedef float InputLayer[kInputs];
  typedef float LogitsLayer[kLayerLogitsNodes];

  #pragma pack(push, 1)
  struct Model {
    uint32_t Mark;
    float kernel0[kInputs][kLayer0Nodes];
    float bias0[kLayer0Nodes];
    float kernel1[kLayer0Nodes][kLayer1Nodes];
    float bias1[kLayer1Nodes];
    float kernel2[kLayer1Nodes][kLayer2Nodes];
    float bias2[kLayer2Nodes];
    float kernel_logits[kLayer2Nodes][kLayerLogitsNodes];
    float bias_logits[kLayerLogitsNodes];
  };
  #pragma pack(pop)
  static_assert(sizeof(Model) == kFileSize, "Wrong format of model data");

  struct ProbeInfo {
    unsigned int row;
    unsigned int col;
    ProbeValues value;
  };
  using Probabilities = std::vector<ProbeInfo>;

  std::vector<uint8_t> model_data_;
  Model* model_;

  void FormInput(const Field& field, unsigned int target_row, unsigned int target_col, InputLayer& inputs);
  void CalculateModel(const InputLayer& inputs, LogitsLayer& logits, unsigned int& max_logit);
  template<unsigned int input_size, unsigned int output_size>
  void CalculateLayer(const float (&inputs)[input_size]
    , float (&outputs)[output_size]
    , const float (&kernel)[input_size][output_size]
    , const float (&bias)[output_size]);
  template<unsigned int size>
  void ActivateLayer(float (&layer)[size]);
  template<unsigned int size>
  void LogitsProbability(float (&layer)[size], unsigned int& max_index);
  void GetProbabilities(const Field& field, Probabilities& probes, bool hide_cells_only);
};

#endif // MODEL_960_140_21_3_H
