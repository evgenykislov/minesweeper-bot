# Author: Evgeny Kislov

import tensorflow as tf
import copy


class TensorFlowSweeper:
    def __init__(self) -> None:
        self.__margin_size_ = 4
        self.__mineset_iterations_ = 3
        self.__signs_ = [' ', '.', '*', '0', '1', '2', '3', '4', '5', '6', '7', '8']
        self.__solver_ = tf.estimator.DNNClassifier(feature_columns = self.__CreateColumns(), hidden_units=[1000, 100], n_classes=3, model_dir="data/minesweeper-bot")

    def GetStep(self, field) -> int:
        step = []
        predict_field = copy.deepcopy(field)
        for counter in range(self.__mineset_iterations_):
            step = self.__MineIteration(predict_field)
        return step[0], step[1]


    def Train(self, field, row, col, forecast):
        self.__solver_.train(input_fn = lambda: self.__SolverInputData(field, row, col, forecast), steps=1)
        pass

    def __GetColumnName(self, row, col):
        return "C" + str(row) + "." + str(col)


    def __CreateColumns(self):
        columns = []
        for row in range(-self.__margin_size_, self.__margin_size_ + 1):
            for col in range(-self.__margin_size_, self.__margin_size_ + 1):
                if row == 0 and col == 0:
                    continue
                sign_column = tf.feature_column.categorical_column_with_vocabulary_list(self.__GetColumnName(row, col), vocabulary_list = self.__signs_)
                sign_indicator_column = tf.feature_column.indicator_column(sign_column)
                columns.append(sign_indicator_column)
        return columns

    def __SolverInputData(self, field, target_row, target_col, forecast):
        row_amount = len(field)
        col_amount = len(field[0])
        features = {}
        for row in range(target_row - self.__margin_size_, target_row + self.__margin_size_ + 1):
            for col in range(target_col - self.__margin_size_, target_col + self.__margin_size_ + 1):
                if row == target_row and col == target_col:
                    continue
                symbol = ' '
                if row >= 0 and row < row_amount and col >= 0 and col < col_amount:
                    symbol = field[row][col]
                col_name = self.__GetColumnName(row - target_row, col - target_col)
                features[col_name] = [symbol]
        labels = [forecast]
        return features, labels


    def __GetPredictPositions(self, field):
        row_amount = len(field)
        col_amount = len(field[0])
        positions = []
        for row in range(row_amount):
            for col in range(col_amount):
                cell = field[row][col]
                if not (cell == '.' or cell == '*'):
                    continue
                positions.append([row, col])
        return positions


    def __PredictInputData(self, field, positions):
        features = self.__CreateEmptyFeatures()
        for pos in positions:
            cell = field[pos[0]][pos[1]]
            if not (cell == '.' or cell == '*'):
                continue
            self.__AddDnnData(field, pos[0], pos[1], features)
        return features


    def __MineIteration(self, field):
        positions = self.__GetPredictPositions(field)
        if len(positions) == 0:
            raise ValueError("Empty list of cells for prediction")
        predict = self.__solver_.predict(input_fn = lambda: self.__PredictInputData(field, positions))
        first_clear_cell = []
        first_unknown_cell = []
        row = 0
        for forecast in predict:
            if row >= len(positions):
                break
            forecast_value = forecast['class_ids'][0]
            if forecast_value == 0 and len(first_clear_cell) == 0:
                # predict first clear cell
                first_clear_cell = positions[row]
            if forecast_value == 1:
                # predict mine
                field[positions[row][0]][positions[row][1]] = '*'
            if forecast_value == 2 and len(first_unknown_cell) == 0:
                # predict unknown
                first_unknown_cell = positions[row]
            row += 1
        if len(first_clear_cell) == 0:
            if len(first_unknown_cell) == 0:
                return positions[0]
            return first_unknown_cell
        return first_clear_cell


    def __AddDnnData(self, field, target_row, target_col, features):
        row_amount = len(field)
        col_amount = len(field[0])
        for row in range(target_row - self.__margin_size_, target_row + self.__margin_size_ + 1):
            for col in range(target_col - self.__margin_size_, target_col + self.__margin_size_ + 1):
                if row == target_row and col == target_col:
                    continue
                symbol = ' '
                if row >= 0 and row < row_amount and col >= 0 and col < col_amount:
                    symbol = field[row][col]
                col_name = self.__GetColumnName(row - target_row, col - target_col)
                features[col_name].append(symbol)

    def __CreateEmptyFeatures(self):
        features = {}
        for row in range(-self.__margin_size_, self.__margin_size_ + 1):
            for col in range(-self.__margin_size_, self.__margin_size_ + 1):
                if row == 0 and col == 0:
                    continue
                col_name = self.__GetColumnName(row, col)
                features[col_name] = []
        return features

