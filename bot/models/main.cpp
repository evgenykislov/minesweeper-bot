#include <iostream>
#include <fstream>

#include "tetragonal_neural.h"

using namespace std;

void PrintHelp() {
  cout << "Usage:" << endl;
  cout << "  models_checker <model> <result>" << endl;
  cout << "    model - name of file with packed model" << endl;
  cout << "    result - name of file with result from test data" << endl;
}

int main(int argc, char** argv)
{
  cout << "Model checker by test data. Evgeny Kislov, 2019" << endl;
  if (argc != 3) {
    PrintHelp();
    return 0;
  }
  ifstream model_file(argv[1], ios_base::binary);
  if (!model_file) {
    // TODO
    return 1;
  }
  model_file.seekg(0, ios_base::end);
  if (!model_file) {
    // TODO
    return 1;
  }
  auto model_size = model_file.tellg();
  if (model_size <= 0) {
    // TODO
    return 1;
  }
  model_file.seekg(0, ios_base::beg);
  vector<uint8_t> model_data;
  model_data.resize(model_size);
  model_file.read((char*)model_data.data(), model_size);
  ModelTetragonalNeural model;
  model.LoadModel(move(model_data));
  Classifier::TestResponse response;
  model.GetTestResponse(response);
  ifstream result(argv[2], ios_base::trunc);



  ofstream output("result.txt", ios_base::trunc);
  const size_t kOutputBufferSize = 64;
  char output_buffer[kOutputBufferSize];
  for (auto iter = response.begin(); iter != response.end(); ++iter) {
    snprintf(output_buffer, kOutputBufferSize, "%.5f %.5f %.5f"
      , iter->clear_probe
      , iter->mine_probe
      , iter->unknown_probe);
    output << output_buffer << endl;
  }
  return 0;
}
