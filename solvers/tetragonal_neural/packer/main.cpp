#include <cassert>
#include <iostream>
#include <fstream>

using namespace std;


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
  float value;
  while (true) {
    input >> value;
    if (!input) {
      break;
    }
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
  }
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
    SaveFileWithValues(output, "dnn_0_kernel.txt");
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
  catch (exception&) {
    cerr << "Error in forming output pack file" << endl;
    return 1;
  }
  cout << "Success" << endl;
  return 0;
}
