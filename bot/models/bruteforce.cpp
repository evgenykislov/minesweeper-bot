#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <exception>

#include "bruteforce.h"

using namespace std;

BruteForce::BruteForce() {
}

void BruteForce::GetStep(const Field& field, unsigned int mines_amount, unsigned int& step_row, unsigned int& step_col, StepAction& step) {
  bound_mines_ = 0;
  FormBoundValue(field);
  if (bound_cells_.empty()) {
    step_row = (unsigned int)(1.0 * field.size() * random() / RAND_MAX);
    if (step_row >= field.size()) {
      step_row = field.size() - 1;
    }
    step_col = (unsigned int)(1.0 * field[step_row].size() * random() / RAND_MAX);
    if (step_col >= field[step_row].size()) {
      step_col = field[step_row].size() - 1;
    }
    step = kOpenWithProbability;
    return;
  }
  FindMaxMines(field, mines_amount);
  do {
    if (IsCaseRight()) {
      StoreCase();
    }
    CheckFillOverflow();
  } while (NextCase());
  FindStep(step_row, step_col, step);
}

void BruteForce::FormBoundValue(const Field& field) {
  bound_cells_.clear();
  value_cells_.clear();
  for (unsigned int closed_row = 0; closed_row < field.size(); ++closed_row) {
    for (unsigned int closed_col = 0; closed_col < field[closed_row].size(); ++closed_col) {
      if (!IsClosedCell(field[closed_row][closed_col])) {
        continue;
      }
      // Process closed cells only
      BoundCell bound_cell;
      bound_cell.pos_ = CellPos{closed_row, closed_col};
      // See about neighbours
      for (int neigh_row = closed_row - 1; neigh_row < int(closed_row) + 2; ++neigh_row) {
        for (int neigh_col = closed_col - 1; neigh_col < int(closed_col) + 2; ++neigh_col) {
          if (neigh_row < 0 || neigh_row >= int(field.size())) {
            continue;
          }
          if (neigh_col < 0 || neigh_col >= int(field[neigh_row].size())) {
            continue;
          }
          unsigned int value;
          if (!IsValueCell(field[neigh_row][neigh_col], value)) {
            continue;
          }
          assert(neigh_row >= 0);
          assert(neigh_col >= 0);
          CellPos pos{(unsigned int)neigh_row, (unsigned int)neigh_col};
          bound_cell.near_value_cells_.insert(pos);
          if (value_cells_.find(pos) == value_cells_.end()) {
            // Need to add new value
            ValueCell info;
            FormValueCell(field, pos, info);
            value_cells_[pos] = info;
          }
        }
      }
      if (!bound_cell.near_value_cells_.empty()) {
        bound_cells_.push_back(bound_cell);
      }
    }
  }
}

void BruteForce::FormValueCell(const Field& field, const CellPos& pos, ValueCell& info) {
  info.mines_near_ = 0;
  if (!IsValueCell(field[pos.row_][pos.col_], info.value_)) {
    assert(false);
  }
  for (int row = pos.row_ - 1; row < (int)pos.row_ + 2; ++row) {
    for (int col = pos.col_ - 1; col < (int)pos.col_ + 2; ++col) {
      if (row < 0 || row >= (int)(field.size())) {
        continue;
      }
      if (col < 0 || col >= (int)(field[row].size())) {
        continue;
      }
      else if (IsMineCell(field[row][col])) {
        ++info.mines_near_;
      }
    }
  }
}

bool BruteForce::NextCase() {
  bool all_cells_are_mines = true;
  for (auto bound = bound_cells_.begin(); bound != bound_cells_.end(); ++ bound) {
    if (bound->state_ != BoundCell::kTryMineCell) {
      all_cells_are_mines = false;
      break;
    }
  }
  if (all_cells_are_mines) {
    return false;
  }
  for (auto bound = bound_cells_.begin(); bound != bound_cells_.end(); ++bound) {
    bool next_bound = false;
    switch (bound->state_) {
      case BoundCell::kTryOpenCell:
        for (auto iter = bound->near_value_cells_.begin(); iter != bound->near_value_cells_.end(); ++iter) {
          auto value = value_cells_.find(*iter);
          assert(value != value_cells_.end());
          value->second.mines_near_++;
        }
        bound->state_ = BoundCell::kTryMineCell;
        ++bound_mines_;
        break;
      case BoundCell::kTryMineCell:
        for (auto iter = bound->near_value_cells_.begin(); iter != bound->near_value_cells_.end(); ++iter) {
          auto value = value_cells_.find(*iter);
          assert(value != value_cells_.end());
          assert(value->second.mines_near_ > 0);
          value->second.mines_near_--;
        }
        bound->state_ = BoundCell::kTryOpenCell;
        next_bound = true;
        assert(bound_mines_ > 0);
        --bound_mines_;
        break;
      default:
        assert(false);
    }
    if (!next_bound) {
      break;
    }
  }
  return true;
}

bool BruteForce::IsCaseRight() {
  if (bound_mines_ > max_bound_mines_) {
    return false;
  }
  for (auto value = value_cells_.begin(); value != value_cells_.end(); ++value) {
    if (value->second.value_ != value->second.mines_near_) {
      return false;
    }
  }
  return true;
}

void BruteForce::StoreCase() {
  for (auto bound = bound_cells_.begin(); bound != bound_cells_.end(); ++bound) {
    if (bound->state_ == BoundCell::kTryOpenCell) {
      bound->can_be_open_ = true;
    }
    else if (bound->state_ == BoundCell::kTryMineCell) {
      bound->can_be_mine_ = true;
    }
  }
}

void BruteForce::CheckFillOverflow() {
  // TODO remove unused
}

void BruteForce::FindStep(unsigned int& row, unsigned int& col, StepAction& step) {
  bool open_only_exist = false;
  CellPos open_cell;
  bool mine_only_exist = false;
  CellPos mine_cell;
  for (auto bound = bound_cells_.begin(); bound != bound_cells_.end(); ++bound) {
    if (bound->can_be_open_ && !bound->can_be_mine_ && !open_only_exist) {
      open_only_exist = true;
      open_cell = bound->pos_;
    }
    if (!bound->can_be_open_ && bound->can_be_mine_ && !mine_only_exist) {
      mine_only_exist = true;
      mine_cell = bound->pos_;
    }
  }
  if (mine_only_exist) {
    row = mine_cell.row_;
    col = mine_cell.col_;
    step = kMarkAsMine;
  }
  else if (open_only_exist) {
    row = open_cell.row_;
    col = open_cell.col_;
    step = kOpenWithSure;
  }
  else {
    // There aren't exact defined cells
    unsigned int index = 0;
    if (bound_cells_.size() > 1) {
      // Select random cell
      index = min((unsigned int)(1.0 * bound_cells_.size() * random() / RAND_MAX), (unsigned int)(bound_cells_.size() - 1));
    }
    row = bound_cells_[index].pos_.row_;
    col = bound_cells_[index].pos_.col_;
    step = kOpenWithProbability;
  }
}

void BruteForce::FindMaxMines(const Field& field, unsigned int mines_amount) {
  unsigned int marked_mines = 0;
  for (unsigned int row = 0; row < field.size(); ++row) {
    for (unsigned int col = 0; col < field[row].size(); ++col) {
      if (IsMineCell(field[row][col])) {
        ++marked_mines;
      }
    }
  }
  if (mines_amount <= marked_mines) {
    throw std::out_of_range("Mines amount is less or equil marked mines");
  }
  max_bound_mines_ = mines_amount - marked_mines;
}
