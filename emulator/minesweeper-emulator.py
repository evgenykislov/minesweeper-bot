# Author: Evgeny Kislov

import copy
import socket

# Mines field
class MineField:
    def __init__(self, field_file_name: str) -> None:
        self.map_ = self.ReadField(field_file_name)
        self.step_ = 0
        self.max_steps_ = 0
        self.row_ = len(self.map_)
        self.col_ = len(self.map_[0])
        self.field_ = copy.deepcopy(self.map_)
        for row_index in range(self.row_):
            for col_index in range(self.col_):
                if self.field_[row_index][col_index] != ' ':
                    self.field_[row_index][col_index] = '.'

    def Completed(self) -> bool:
        raise NotImplementedError
        pass

    def GetField(self) -> [[str]]:
        raise NotImplementedError
        pass

    def Display(self):
        print("Поле ", self.row_, "x", self.col_, ", Шаг ", self.step_, " (Макс ", self.max_steps_, ")", sep='')
        for row in range(self.row_):
            print("".join(self.field_[row]))
        print("---")

    def ReadField(self, file_name):
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


# Proxy for external solver
class Sweeper:
    def __init__(self, port: int) -> None:
        self.socket = socket.socket()
        self.socket.connect(('localhost', port))


# Description of sweeper step
class Step:
    def IsUnknown(self) -> bool:
        return True

    def ShouldOpen(self) -> bool:
        return False

    def MarkMine(self) -> bool:
        return True

# main() - Base logic of gaming
field = MineField("f.txt")
field.Display()
sweeper = Sweeper(5000)
while not field.Completed():
    view = field.GetField()
    step = sweeper.GetStepForField(view)
    if step.IsUnknown():
        if field.HasOpenedCells():
            # There were opened cells from last clearing
            field.ClearBombMarks()
        else:
            field.OpenRandom()
            opened_cells = True
    if field.MarkBomb():
        field.MarkBomb(step)
    if step.OpenCell():
        field.OpenCell(step)
        opened_cells = True
    if field.Bombed():
        break
if field.Bombed():
    print("Boom-boom")
else:
    print("!!! Congratulation !!!")
