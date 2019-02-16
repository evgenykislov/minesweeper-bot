# Author: Evgeny Kislov

import random
import numpy

# Generate mines field
# cell values:
# space - out from playing ground
# * - mine
# 0 - 8 - number of mines around (if we are not on a mine)
def GenerateField(rowAmount, colAmount, minesAmount):
    minesList = random.sample(range(rowAmount * colAmount), minesAmount)
    field = numpy.zeros((rowAmount, colAmount), numpy.int)
    for index in minesList:
        row = index // colAmount
        col = index % colAmount
        field[row][col] = -1;
    for row in range(rowAmount):
        for col in range(colAmount):
            if field[row][col] ==  -1:
                continue
            amount = 0;
            for r in range(row - 1, row + 2):
                for c in range(col -1, col + 2):
                    if (r < 0 or r >= rowAmount):
                        continue
                    if (c < 0 or c >= colAmount):
                        continue
                    if field[r][c] == -1:
                        amount += 1
            field[row][col] = amount
    # store field as symbols
    rows = []
    for row in range(rowAmount):
        line = []
        for col in range(colAmount):
            if field[row][col] == -1:
                line.append('*')
            else:
                line.append(str(field[row][col]))
        rows.append(''.join(line))
    return rows

# main
print("Mines field generator")
# input user parameters
print("How many rows: ")
row = int(input())
print("How many columns: ")
col = int(input())
print("How many mines: ")
mines = int(input())
print("How many field instances: ")
fields = int(input())
print("Field start index: ")
field_start_index = int(input())

for field_index in range(fields):
    field = GenerateField(row, col, mines)
    file_name = "field_{row}x{col}_{index:05d}.txt".format(row = row, col = col, index = field_index + field_start_index)
    with open(file_name, 'x') as storage:
        for line in field:
            print(line, file = storage)

print("Completed")


