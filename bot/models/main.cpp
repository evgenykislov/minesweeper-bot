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

#include <iostream>
#include <fstream>

#include "tetragonal_neural.h"
#include "bruteforce.h"

using namespace std;

void PrintHelp() {
  cout << "Usage:" << endl;
  cout << "  models_checker <model> <result>" << endl;
  cout << "    model - name of file with packed model" << endl;
  cout << "    result - name of file with result from test data" << endl;
}

bool LoadField(const std::string& file_name, Classifier::Field& field) {
  field.clear();
  ifstream file(file_name);
  if (!file) return false;
  string line;
  while(getline(file, line)) {
    if (line.empty()) { continue; }
    field.push_back(vector<char>(line.begin(), line.end()));
  }
}

int main(int argc, char** argv)
{
  cout << "Model checker by test data. Evgeny Kislov, 2019" << endl;
  Classifier::Field field;
  LoadField("test.txt", field);
  BruteForce bf;
  unsigned int row;
  unsigned int col;
  Classifier::StepAction step;
  bf.GetStep(field, 100, row, col, step);
  int k = 0;
  return 0;


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
/*
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
*/
  return 0;
}
