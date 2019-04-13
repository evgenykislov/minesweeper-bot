# Author: Evgeny Kislov

import tensorflow as tf
import numpy as np
import copy

# hack for suppress warnings
tf.estimator.Estimator._validate_features_in_predict_input = lambda *args: None


class ExportHook(tf.train.SessionRunHook):
    def after_create_session(self, session, coord):
        # Layer 0
        kernel_0 = np.zeros((960, 960), dtype=np.float32)
        index = 36
        disp = index * 12
        kernel_0[disp][0] = 0.1
        kernel_0[disp + 1][0] = 0.2
        kernel_0[disp + 2][0] = 0.3
        kernel_0[disp + 3][0] = 0.4
        kernel_0[disp + 4][0] = 0.5
        kernel_0[disp + 5][0] = 0.6
        kernel_0[disp + 6][0] = 0.7
        kernel_0[disp + 7][0] = 0.8
        kernel_0[disp + 8][0] = 0.9
        kernel_0[disp + 9][0] = 1.0
        kernel_0[disp + 10][0] = 1.1
        kernel_0[disp + 11][0] = 1.2
        self.__ChangeHiddenLayer(session, 0, "kernel", kernel_0)
        self.__ChangeHiddenLayer(session, 0, "bias", np.zeros((960,), dtype=np.float32))
        # Layer 1
        kernel_1 = np.zeros((960, 140), dtype=np.float32)
        kernel_1[0][0] = 1.0
        self.__ChangeHiddenLayer(session, 1, "kernel", kernel_1)
        self.__ChangeHiddenLayer(session, 1, "bias", np.zeros((140,), dtype=np.float32))
        # Layer 2
        kernel_2 = np.zeros((140, 21), dtype=np.float32)
        kernel_2[0][0] = 1.0
        self.__ChangeHiddenLayer(session, 2, "kernel", kernel_2)
        self.__ChangeHiddenLayer(session, 2, "bias", np.zeros((21,), dtype=np.float32))
        # Logits
        kernel_l = np.zeros((21, 3), dtype=np.float32)
        kernel_l[0][0] = 1.0
        self.__ChangeLogitsLayer(session, "kernel", kernel_l)
        self.__ChangeLogitsLayer(session, "bias", np.zeros((3,), dtype=np.float32))


    def __ChangeHiddenLayer(self, session, layer_index, data_name, new_value):
        with tf.variable_scope("dnn/hiddenlayer_" + str(layer_index), reuse = True):
            value = tf.get_variable(data_name + "/part_0")
            value.load(new_value, session)
            np.savetxt("dnn_" + str(layer_index) + "_" + data_name + ".txt", new_value, fmt = "%.8f")


    def __ChangeLogitsLayer(self, session, data_name, new_value):
        with tf.variable_scope("dnn/logits", reuse = True):
            value = tf.get_variable(data_name + "/part_0")
            value.load(new_value, session)
            np.savetxt("dnn_logits_" + data_name + ".txt", new_value, fmt = "%.8f")


class TensorFlowSweeper:
    def __init__(self) -> None:
        self.__margin_size_ = 4
        self.__minimal_clear_probability_ = 0.6
        self.__max_probability_ = 1.0
        self.__signs_ = [' ', '.', '*', '0', '1', '2', '3', '4', '5', '6', '7', '8']
        self.__solver_ = tf.estimator.DNNClassifier(feature_columns = self.__CreateColumns(), hidden_units=[960, 140, 21], n_classes=3, model_dir="data/minesweeper-bot")

    def GetStep(self, field) -> int:
        step = self.__MineIteration(field)
        return step[0], step[1]


    def Train(self, field, training):
        self.__solver_.train(input_fn = lambda: self.__SolverInputData(field, training), steps=1)
        pass


    def ExportModelParameters(self):
        self.__ExportDnnLayer(0, "kernel")
        self.__ExportDnnLayer(0, "bias")
        self.__ExportDnnLayer(1, "kernel")
        self.__ExportDnnLayer(1, "bias")
        self.__ExportDnnLayer(2, "kernel")
        self.__ExportDnnLayer(2, "bias")
        self.__ExportDnnLogits("kernel")
        self.__ExportDnnLogits("bias")


    def ExportProbabilities(self, field):
        row_amount = len(field)
        col_amount = len(field[0])
        positions = []
        for row in range(row_amount):
            for col in range(col_amount):
                positions.append([row, col])
        if len(positions) == 0:
            raise ValueError("Empty list of cells for prediction")
        hook = [ExportHook()]
        predict = self.__solver_.predict(input_fn = lambda: self.__PredictInputData(field, positions), yield_single_examples = False)
        # Debug version with hook: predict = self.__solver_.predict(input_fn = lambda: self.__PredictInputData(field, positions), hooks = hook, yield_single_examples = False)
        forecast = next(predict)
        np.savetxt("test.txt", forecast["probabilities"], fmt = "%.5f")


    def __GetColumnName(self, row, col):
        if row == 0 and col == 0:
            raise ValueError("Logic error: request name for cel (0, 0)")
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

    def __SolverInputData(self, field, training):
        row_amount = len(field)
        col_amount = len(field[0])
        features = self.__CreateEmptyFeatures()
        labels = []
        for index in range(len(training)):
            target_row = training[index][0]
            target_col = training[index][1]
            target_forecast = training[index][2]
            for row in range(target_row - self.__margin_size_, target_row + self.__margin_size_ + 1):
                for col in range(target_col - self.__margin_size_, target_col + self.__margin_size_ + 1):
                    if row == target_row and col == target_col:
                        continue
                    symbol = ' '
                    if row >= 0 and row < row_amount and col >= 0 and col < col_amount:
                        symbol = field[row][col]
                    col_name = self.__GetColumnName(row - target_row, col - target_col)
                    features[col_name].append(symbol)
            if target_forecast == 'c':
                labels.append(0)
            elif target_forecast == '*':
                labels.append(1)
            else:
                labels.append(2)
        return features, labels


    def __GetPredictPositions(self, field):
        row_amount = len(field)
        col_amount = len(field[0])
        positions = []
        for row in range(row_amount):
            for col in range(col_amount):
                cell = field[row][col]
                if cell != '.':
                    continue
                positions.append([row, col])
        return positions


    def __PredictInputData(self, field, positions):
        features = self.__CreateEmptyFeatures()
        for pos in positions:
            self.__AddDnnData(field, pos[0], pos[1], features)
        return features


    def __MineIteration(self, field):
        positions = self.__GetPredictPositions(field)
        if len(positions) == 0:
            raise ValueError("Empty list of cells for prediction")
        predict = self.__solver_.predict(input_fn = lambda: self.__PredictInputData(field, positions), yield_single_examples = False)
        clear_cell = []
        clear_probability = self.__minimal_clear_probability_
        non_mine_cell = []
        mine_probability = self.__max_probability_
        forecast = next(predict)
        for index in range(len(positions)):
            clear_probe = forecast["probabilities"][index][0]
            mine_probe = forecast["probabilities"][index][1]
            if clear_probe > clear_probability:
                # predict clear cell. Select max probability
                clear_cell = positions[index]
                clear_probability = clear_probe
            if mine_probe < mine_probability:
                # predict unknown. Select minimal mine probability
                non_mine_cell = positions[index]
                mine_probability = mine_probe
        if len(clear_cell) != 0:
            # debug
            # print("Clear probability: ", clear_probability, sep='')
            return clear_cell
        if len(non_mine_cell) != 0:
            # debug
            # print("Mine probability: ", mine_probability, sep='')
            return non_mine_cell
        raise ValueError("can't find cell with max clear-state or min mine-state")


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


    def __ExportDnnLayer(self, layer_index, data_name):
        variable_name = "dnn/hiddenlayer_" + str(layer_index) + "/" + data_name
        file_name = "dnn_" + str(layer_index) + "_" + data_name + ".txt"
        np.savetxt(file_name, self.__solver_.get_variable_value(variable_name), fmt = "%.8f")


    def __ExportDnnLogits(self, data_name):
        variable_name = "dnn/logits/" + data_name
        file_name = "dnn_logits_" + data_name + ".txt"
        np.savetxt(file_name, self.__solver_.get_variable_value(variable_name), fmt = "%.8f")
