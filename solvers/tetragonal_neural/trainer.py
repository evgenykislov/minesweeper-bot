# Author: Evgeny Kislov

import copy
import os
import random
import miner_dnn
import time


class Step:
    def __init__(self, folder: str, filename: str) -> None:
        self.__ReadStep(folder, filename)


    def FullClosedField(self):
        for row in self.__field_:
            for col in row:
                if col != '.' and col != ' ':
                    return False
        return True


    def GetField(self):
        return self.__field_


    def GetTraining(self):
        list = []
        list.append([self.__step_row_, self.__step_col_, self.__step_result_])
        if random.randint(0, 9) == 0:
            list.append([self.__step_row_, self.__step_col_, '?'])
        return list


    def RemoveStep(self):
        os.remove(self.__step_filename_)
        os.remove(self.__field_filename_)


    def __ReadStep(self, folder, filename):
        self.__step_filename_ = os.path.join(folder, filename)
        with open(self.__step_filename_, 'r') as f:
            # Read first line
            line = f.read()
            words = line.split()
            if len(words) != 4:
                raise ValueError("Step file wrong format")
            self.__step_row_ = int(words[0])
            self.__step_col_ = int(words[1])
            result = words[2]
            self.__step_result_ = '?'
            if result == "1":
                self.__step_result_ = 'c'
            else:
                self.__step_result_ = '*'
            self.__field_filename_ = os.path.join(folder, words[3])
            self.__field_ = self.__ReadField(self.__field_filename_)


    def __ReadField(self, file_name):
        field = []
        with open(file_name, 'r') as f:
            for line in f:
                char_line = list(line)
                if len(char_line) > 0:
                    if char_line[-1] == '\n':
                        del char_line[-1]
                if len(char_line) == 0:
                    continue
                field.append(char_line)
        # check read field
        if len(field) == 0:
            raise ValueError("Empty file")
        first_len = len(field[0])
        if first_len == 0:
            raise ValueError("Narrow columns")
        for row in range(len(field)):
            if len(field[row]) != first_len:
                raise ValueError("Not aligned data")
        return field



# main() - Base logic of gaming
random.seed()
sweeper = miner_dnn.TensorFlowSweeper()
data_folder = "train"
print("Tetragonal model trainer. Evgeny Kislov, 2019")
# Model training
while (True):
    files = [f for f in os.listdir(data_folder) if (len(f) >= 9 and f[0:5] == "step_" and f[-4:] == ".txt")]
    for f in files:
        step = Step(data_folder, f)
        if step.FullClosedField():
            print("Field ", f, " is full closed. Pass it", sep='')
        else:
            sweeper.Train(step.GetField(), step.GetTraining())
            print("Field ", f, " processed", sep='')
        step.RemoveStep()
    # There aren't files in list
    time.sleep(5)
