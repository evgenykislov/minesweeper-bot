#ifndef MODEL_960_140_21_3_H
#define MODEL_960_140_21_3_H

#include <cstdint>

#include <dnnclassifier.h>

class Model_960_140_21_3: public DnnClassifier
{
 public:
  Model_960_140_21_3();
  virtual ~Model_960_140_21_3();
  virtual bool LoadModel(const QString& file_name) override;
  virtual void GetStep(const Field& field, unsigned int& step_row, unsigned int& step_col, bool& sure_step) override;

 private:
  Model_960_140_21_3(const Model_960_140_21_3&) = delete;
  Model_960_140_21_3(Model_960_140_21_3&&) = delete;
  Model_960_140_21_3& operator=(const Model_960_140_21_3&) = delete;
  Model_960_140_21_3& operator=(Model_960_140_21_3&&) = delete;

  enum {
    kFileSize = 4240512,
    kInputs = 960,
    kLayer0Nodes = 960,
    kLayer1Nodes = 140,
    kLayer2Nodes = 21,
    kLayerLogitsNodes = 3,
    kMarginSize = 4,
  };

  enum {
    kClearLogit = 0,
    kMineLogit = 1,
    kUnknownLogit = 2,
  };

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
};

#endif // MODEL_960_140_21_3_H
