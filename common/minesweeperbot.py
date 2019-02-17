#Author: Evgeny Kislov


def ReadField(file_name):
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
        raise Exception("Empty file")
    first_len = len(field[0])
    if first_len == 0:
        raise Exception("Narrow columns")
    for row in range(len(field)):
        if len(field[row]) != first_len:
            raise Exception("Not aligned data")
    return field
