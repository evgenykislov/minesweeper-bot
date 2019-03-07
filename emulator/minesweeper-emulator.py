# Author: Evgeny Kislov

import copy
import os
import random
import miner_dnn


# Mines field
class MineField:
    # __map_ - loaded full-opened field
    # __milestone_field_ - partially opened field for last milestone (f.e. each 5-th step).
    #       used as start point in case of fault
    # __filed_ - current opened field, used for calculation, etc
    # __max_filed_ - maximum opened field in that running (for showing only)
    # __training_field_ - copy of __field_, used for detection unknown state (50/50)


    def __init__(self, field_file_name: str) -> None:
        random.seed()
        self.__unknown_probability_ = 0.1
        self.__death_amount_ = 0
        self.__map_ = self.__ReadField(field_file_name)
        self.__row_amount_ = len(self.__map_)
        self.__col_amount_ = len(self.__map_[0])
        self.__mines_amount_ = 0
        for row_index in range(self.__row_amount_):
            for col_index in range(self.__col_amount_):
                if self.__map_[row_index][col_index] == '*':
                    self.__mines_amount_ += 1
        self.__milestone_interval_ = 1
        self.__milestone_step_ = 0
        # Generate empty field (initial milestone)
        self.__milestone_field_ = []
        self.__milestone_hide_cells_amount_ = 0
        for row_index in range(self.__row_amount_):
            self.__milestone_field_.append([])
            for col_index in range(self.__col_amount_):
                cell = ' '
                if self.__map_[row_index][col_index] != ' ':
                    cell = '.'
                    self.__milestone_hide_cells_amount_ += 1
                self.__milestone_field_[row_index].append(cell)
        self.__max_steps_ = 0
        self.__max_field_ = copy.deepcopy(self.__milestone_field_)
        self.Reset()


    def Reset(self) -> None:
        self.__step_ = self.__milestone_step_
        self.__field_ = copy.deepcopy(self.__milestone_field_)
        self.__hide_cells_amount_ = self.__milestone_hide_cells_amount_
        self.__training_ = copy.deepcopy(self.__field_)


    def Completed(self) -> bool:
        return self.__hide_cells_amount_ == self.__mines_amount_


    def GetField(self) -> [[str]]:
        return self.__field_


    def GetFieldParams(self) -> [int]:
        # Return: [row_amount, col_amount, mines_amount]
        return [self.__row_amount_, self.__col_amount_, self.__mines_amount_]


    def GetCurrentStep(self) -> int:
        return self.__step_


    def MakeStep(self, row, col) -> bool:
        cell = self.__map_[row][col]
        self.__field_[row][col] = cell
        self.__training_[row][col] = cell
        self.__step_ += 1
        self.__hide_cells_amount_ -= 1
        if cell == '*':
            self.__death_amount_ += 1
            return False
        if self.__step_ > self.__max_steps_:
            self.__max_steps_ = self.__step_
            self.__max_field_ = copy.deepcopy(self.__field_)
        if self.__step_ >= self.__milestone_step_ + self.__milestone_interval_:
            self.__milestone_field_ = copy.deepcopy(self.__field_)
            self.__milestone_hide_cells_amount_ = self.__hide_cells_amount_
            self.__milestone_step_ = self.__step_
        return True


    def GetTrainingForecast(self) -> str:
        # Return training data for field
        # Return: list of [row, col, predict]
        training = []
        for row in range(self.__row_amount_):
            for col in range(self.__col_amount_):
                if self.__field_[row][col] != '.':
                    continue
                # process closed cells only
                original_cell = self.__map_[row][col]
                if original_cell == '*':
                    training.append([row, col, '*'])
                else:
                    training.append([row, col, 'c'])
                # check alternate variant
                training_cell = self.__training_[row][col]
                # check another variant
                probe = '*'
                if original_cell == '*':
                    probe = ' '
                self.__training_[row][col] = probe
                alternate_valid = self.__ValidateTraining()
                self.__training_[row][col] = training_cell
                if alternate_valid:
                    training.append([row, col, '?'])
        return training

    def Display(self):
        print("Field: ", self.__row_amount_, "x", self.__col_amount_, ", Turn: ", self.__step_, ", Max turns: ", self.__max_steps_, sep='')
        for row in range(self.__row_amount_):
            print(" ".join(self.__field_[row]))
        print("")


    def DisplayCurrentAndMax(self):
        title = "Turn: " + str(self.__step_)
        space_amount = self.__col_amount_ * 2 - 1 + 5 - len(title)
        for counter in range(space_amount):
            title += ' '
        title += "Best turn: " + str(self.__max_steps_)
        print(title)
        for row in range(self.__row_amount_):
            print(" ".join(self.__field_[row]), "  |  ", " ".join(self.__max_field_[row]), sep='')
        print("")
        print("Death amount: ", self.__death_amount_, sep='')


    def GetDeathAmount(self):
        return self.__death_amount_


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

    def __ValidateTraining(self) -> bool:
        for row in range(self.__row_amount_):
            for col in range(self.__col_amount_):
                # Validate the cell
                if not self.__ValidateTrainingCell(row, col):
                    return False
        return True


    def __ValidateTrainingCell(self, row, col) -> bool:
        cell = self.__training_[row][col]
        if cell == ' ' or cell == '.' or cell == '*':
            # These cells are always valid :)1
            return True
        mines_amount = 0
        unknown_amount = 0
        for sub_row in range(row - 1, row + 2):
            for sub_col in range(col - 1, col + 2):
                if sub_row == row and sub_col == col:
                    continue
                if sub_row < 0 or sub_row >= self.__row_amount_ or sub_col < 0 or sub_col >= self.__col_amount_:
                    # out from field
                    continue
                cell = self.__training_[sub_row][sub_col]
                if cell == '*':
                    mines_amount += 1
                if cell == '.':
                    unknown_amount += 1
        target = int(self.__training_[row][col])
        return (target >= mines_amount) and (target <= (mines_amount + unknown_amount))


# main() - Base logic of gaming
sweeper = miner_dnn.TensorFlowSweeper()
answer = input("Make a training [y/n]:")
if answer == 'y' or answer == 'Y':
    files = [f for f in os.listdir('.') if (os.path.isfile(f) and len(f) >= 5 and f[0] == "t" and f[-4:] == ".txt")]
    for f in files:
        field = MineField(f)
        while not field.Completed():
            view = field.GetField()
            row, col = sweeper.GetStep(view)
            result = field.MakeStep(row, col)
            sweeper.Train(view, field.GetTrainingForecast())
            if not result:
                # Oops. Mine :(
                field.Display()
                # start from beginning
                field.Reset()
            # else:
            #     print("    Field ", f, ": death ", field.GetDeathAmount(), " step ", field.GetCurrentStep(), sep='')
        print("Field ", f, " de-mined with ", field.GetDeathAmount(), " death", sep='')
# Checking of AI
files = [f for f in os.listdir('.') if (os.path.isfile(f) and len(f) >= 5 and f[0] == "c" and f[-4:] == ".txt")]
for f in files:
    field_check = MineField(f)
    check_steps = 0
    while not field_check.Completed():
        view = field_check.GetField()
        row, col = sweeper.GetStep(view)
        result = field_check.MakeStep(row, col)
        if result:
            check_steps += 1
            print("    Field ", f, ": step ", field_check.GetCurrentStep(), sep='')
        else:
            # mine is found
            params = field_check.GetFieldParams()
            maximum_available_steps = params[0] * params[1] - params[2]
            print("Check in ", f, ": ", check_steps, " steps from ", maximum_available_steps, sep='')
            field_check.Display()
            break
    if field_check.Completed():
        print("++Check in ", f, ": successfull", sep='')
