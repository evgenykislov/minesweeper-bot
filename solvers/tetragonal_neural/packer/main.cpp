#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>

using namespace std;

const size_t kInputSize = 960;


void PrintHelp() {

  cout << "Usage:" << endl;
  cout << "  packer filename" << endl;
  cout << "    filename - name of output file" << endl;
  cout << endl;
  cout << "Packet reads follow files:" << endl;
  cout << "  dnn_0_kernel.txt," << endl;
  cout << "  dnn_0_bias.txt," << endl;
  cout << "  dnn_1_kernel.txt," << endl;
  cout << "  dnn_1_bias.txt," << endl;
  cout << "  dnn_2_kernel.txt," << endl;
  cout << "  dnn_2_bias.txt," << endl;
  cout << "  dnn_logits_kernel.txt," << endl;
  cout << "  dnn_logits_bias.txt" << endl;
  cout << "and packs them into binary file" << endl;
}

void SaveMark(ofstream& stream) {
  const size_t kMarkSize = 4;
  unsigned char mark[kMarkSize] = {0x03, 0xc0, 0x8c, 0x15};
  stream.write((const char*)mark, kMarkSize);
}

void SaveFileWithValues(ofstream& stream, const char* values_file) {
  assert(values_file);
  ifstream input(values_file);
  if (!input) {
    return;
  }
  float value;
  while (true) {
    input >> value;
    if (!input) {
      break;
    }
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
  }
}


void SaveFirstKernelLayer(ofstream& stream, const char* values_file) {
  assert(values_file);
  using InputNodeWeights = float[kInputSize];
  using InputMatrix = InputNodeWeights[kInputSize];
  unique_ptr<InputNodeWeights[]> readed;
  unique_ptr<InputNodeWeights[]> shifted;
  readed.reset(new InputMatrix);
  shifted.reset(new InputMatrix);
  ifstream input(values_file);
  if (!input) {
    return;
  }
  float value;
  unsigned int row = 0;
  unsigned int col = 0;
  while (true) {
    input >> value;
    if (!input) {
      break;
    }
    if (row >= 960) {
      assert(0);
      return;
    }
    readed[row][col] = value;
    ++col;
    if (col >= 960) {
      ++row;
      col = 0;
    }
  }
  // Shifting
  unsigned int shift_data[80] = {\
    30, 29, 28, 27, 31, 32, 33, 34, 35,\
    21, 20, 19, 18, 22, 23, 24, 25, 26,\
    12, 11, 10,  9, 13, 14, 15, 16, 17,\
     3,  2,  1,  0,  4,  5,  6,  7,  8,\
    39, 38, 37, 36,     40, 41, 42, 43,\
    47, 46, 45, 44, 48, 49, 50, 51, 52,\
    56, 55, 54, 53, 57, 58, 59, 60, 61,\
    65, 64, 63, 62, 66, 67, 68, 69, 70,\
    74, 73, 72, 71, 75, 76, 77, 78, 79
  };
  for (unsigned int index = 0; index < 80; ++index) {
    for (unsigned int line = 0; line < 12; ++line) {
      unsigned int target = index * 12 + line;
      unsigned int source = shift_data[index] * 12 + line;
      for (unsigned int param_index = 0; param_index < 960; ++param_index) {
        shifted[target][param_index] = readed[source][param_index];
      }
    }
  }
  stream.write(reinterpret_cast<const char*>(shifted.get()), sizeof(InputMatrix));
}

void CheckDataSize(ofstream& stream, size_t right_size) {
  if (stream.tellp() != right_size) {
    throw std::runtime_error("wrond data size");
  }
}

int main(int argc, char** argv)
{
  cout << "Tetragonal 960-140-21-3 model packer. Evgeny Kislov, 2019" << endl;
  if (argc != 2) {
    PrintHelp();
    return 0;
  }
  try {
    ofstream output(argv[1], ios_base::binary | ios_base::out | ios_base::trunc);
    SaveMark(output);
    CheckDataSize(output, 4);
    SaveFirstKernelLayer(output, "dnn_0_kernel.txt");
    CheckDataSize(output, 3686404);
    SaveFileWithValues(output, "dnn_0_bias.txt");
    CheckDataSize(output, 3690244);
    SaveFileWithValues(output, "dnn_1_kernel.txt");
    CheckDataSize(output, 4227844);
    SaveFileWithValues(output, "dnn_1_bias.txt");
    CheckDataSize(output, 4228404);
    SaveFileWithValues(output, "dnn_2_kernel.txt");
    CheckDataSize(output, 4240164);
    SaveFileWithValues(output, "dnn_2_bias.txt");
    CheckDataSize(output, 4240248);
    SaveFileWithValues(output, "dnn_logits_kernel.txt");
    CheckDataSize(output, 4240500);
    SaveFileWithValues(output, "dnn_logits_bias.txt");
    CheckDataSize(output, 4240512);
    if (!output) {
      throw std::runtime_error("error in file operation");
    }
  }
  catch (exception& e) {
    cerr << "Error in forming output pack file" << endl;
    cerr << "  description: " << e.what() << endl;
    return 1;
  }
  cout << "Success" << endl;
  return 0;
}
