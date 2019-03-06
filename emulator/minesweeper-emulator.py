# Author: Evgeny Kislov

import copy
import random
import miner_dnn


# Mines field
class MineField:
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


    def GetTrainingForecast(self, row, col) -> str:
        # Return training data for cell
        original_cell = self.__map_[row][col]
        training_cell = self.__training_[row][col]
        # check another variant
        probe = '*'
        if original_cell == '*':
            probe = ' '
        self.__training_[row][col] = probe
        alternate_valid = self.__ValidateTraining()
        self.__training_[row][col] = training_cell
        if alternate_valid and (random.random() < self.__unknown_probability_):
            return '?'
        # without altenatives
        if original_cell == '*':
            return '*'
        return 'c'


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
field = MineField("f.txt")
sweeper = miner_dnn.TensorFlowSweeper()
while not field.Completed():
    view = field.GetField()
    row, col = sweeper.GetStep(view)
    # Generate forecast
    forecast = field.GetTrainingForecast(row, col)
    sweeper.Train(view, row, col, forecast)
    forecast_str = ""
    if forecast == 'c':
        forecast_str = "Clear"
    if forecast == '*':
        forecast_str = "Mine"
    if forecast == '?':
        forecast_str = "Unknown 50/50"
    print("Sweeper step: ", row + 1, "x", col + 1, ". Training forecast: ", forecast_str, sep='')
    # step
    result = field.MakeStep(row, col)
    if not result:
        # mine is found
        print("Boom")
        print("***************")
        field.DisplayCurrentAndMax()
        # start from beginning
        field.Reset()
    else:
        field.DisplayCurrentAndMax()
