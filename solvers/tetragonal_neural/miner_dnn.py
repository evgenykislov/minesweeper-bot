# Author: Evgeny Kislov

import tensorflow as tf
import numpy as np

margin_size = 4

signs = [' ', '.', '*', '0', '1', '2', '3', '4', '5', '6', '7', '8']


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


def GetColumnName(row, col):
    return "C" + str(row) + "." + str(col)


def CreateColumns():
    columns = []
    for row in range(-margin_size, margin_size + 1):
        for col in range(-margin_size, margin_size + 1):
            if row == 0 and col == 0:
                continue
            sign_column = tf.feature_column.categorical_column_with_vocabulary_list(GetColumnName(row, col),
                                                                                    vocabulary_list=signs)
            sign_indicator_column = tf.feature_column.indicator_column(sign_column)
            columns.append(sign_indicator_column)
    return columns


def FormInputData(field, cell_row, cell_col, features, labels):
    for row in range(cell_row - margin_size, cell_row + margin_size + 1):
        for col in range(cell_col - margin_size, cell_col + margin_size + 1):
            symbol = ' '
            if cell_row >= 0 and cell_row < len(field) and cell_col >= 0 and cell_col < len(field[0]):
                symbol = field[cell_row][cell_col]
            col_name = GetColumnName(cell_row - row, cell_col - col)
            if row == cell_row and col == cell_col:
                continue
            features[col_name].append(symbol)
    # get label
    result = 0
    if field[cell_row][cell_col] == '*':
        result = 1
    # TODO sometimes it should be '?/2'
    labels.append(result)


solver = tf.estimator.DNNClassifier(feature_columns=CreateColumns(), hidden_units=[1000, 100], n_classes=3, model_dir="data/minesweeper-bot")


def solver_input_data(file_name):
    field = ReadField(file_name)
    row_amount = len(field)
    col_amount = len(field[0])
    features = {}
    for row in range(-margin_size, margin_size + 1):
        for col in range(-margin_size, margin_size + 1):
            features[GetColumnName(row, col)] = []
    labels = []
    for cell_row in range(row_amount):
        for cell_col in range(col_amount):
            FormInputData(field, cell_row, cell_col, features, labels)
    return features, labels


solver.train(input_fn=lambda: solver_input_data("f.txt"), steps=10)