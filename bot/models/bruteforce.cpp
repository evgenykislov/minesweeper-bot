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
    // There aren't bound cells (There aren't opened cells with value (with number))
    FormRandomStep(field, step_row, step_col, step);
    return;
  }
  FindMaxMines(field, mines_amount);
  vector<CellPos> seque;
  FormSequence(seque);
  for (auto iter = seque.begin(); iter != seque.end(); ++iter) {
    unsigned int bomb_cell;
    unsigned int clear_cell;
    if (EnumerateCases(*iter, 0, bomb_cell, clear_cell)) {
      if (bomb_cell != kUnknownBoundCell) {
        step_row = bound_cells_[bomb_cell].pos_.row_;
        step_col = bound_cells_[bomb_cell].pos_.col_;
        step = kMarkAsMine;
        return;
      }
      if (clear_cell != kUnknownBoundCell) {
        step_row = bound_cells_[clear_cell].pos_.row_;
        step_col = bound_cells_[clear_cell].pos_.col_;
        step = kOpenWithSure;
        return;
      }
    }
  }
  // Uh, there aren't right variant/case
  // Random saves us
  unsigned int index = min((unsigned int)(1.0 * bound_cells_.size() * random() / RAND_MAX), (unsigned int)bound_cells_.size());
  step_row = bound_cells_[index].pos_.row_;
  step_col = bound_cells_[index].pos_.col_;
  step = kOpenWithSure;
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
          unsigned int value_cell_value;
          if (!IsValueCell(field[neigh_row][neigh_col], value_cell_value)) {
            continue;
          }
          assert(neigh_row >= 0);
          assert(neigh_col >= 0);
          CellPos pos{(unsigned int)neigh_row, (unsigned int)neigh_col};
          bound_cell.near_value_cells_.insert(pos);
          if (value_cells_.find(pos) == value_cells_.end()) {
            // Need to add new value
            ValueCell info;
            info.value_ = value_cell_value;
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
  // Fill value cells with bounds
  for (size_t index = 0; index != bound_cells_.size(); ++index) {
    auto& bound = bound_cells_[index];
    for (auto value = bound.near_value_cells_.begin(); value != bound.near_value_cells_.end(); ++value) {
      value_cells_[*value].near_bound_cells_.push_back(index);
    }
  }
}

void BruteForce::FormValueCell(const Field& field, const CellPos& pos, ValueCell& info) {
  info.fixed_mines_around_ = 0;
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
        ++info.fixed_mines_around_;
      }
    }
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

void BruteForce::FormSequence(std::vector<CellPos>& seque) {
  struct PosInfo {
    CellPos pos_;
    unsigned int bound_cells_amount_;
    bool operator<(const PosInfo& arg) const {
      return bound_cells_amount_ < arg.bound_cells_amount_;
    }
  };
  vector<PosInfo> poses;
  for (auto iter = value_cells_.begin(); iter != value_cells_.end(); ++iter) {
    PosInfo info;
    info.pos_ = iter->first;
    info.bound_cells_amount_ = iter->second.near_bound_cells_.size();
    poses.push_back(info);
  }
  sort(poses.begin(), poses.end());
  seque.clear();
  for (auto iter = poses.begin(); iter != poses.end(); ++iter) {
    seque.push_back(iter->pos_);
  }
}

bool BruteForce::EnumerateCases(const CellPos& value_pos, unsigned int level, unsigned int& bomb_bound_cell, unsigned int& clear_bound_cell) {
  bomb_bound_cell = kUnknownBoundCell;
  clear_bound_cell = kUnknownBoundCell;
  auto& info = value_cells_.find(value_pos)->second;
  // Get list of mutable cells
  CaseCells cells;
  unsigned int closed_cells = 0; // Amount of bound cells, closed as this case
  unsigned int mine_cells = 0; // Amount of bound cells, marked by mine (by previous cases)
  for (auto index: info.near_bound_cells_) {
    if (bound_cells_[index].state_ == BoundCell::kClosedCell) {
      CaseClosedCell cell_info;
      cell_info.bound_index_ = index;
      cell_info.marked_mine_ = false;
      cell_info.can_be_open_ = false;
      cell_info.can_be_mine_ = false;
      cells.push_back(cell_info);
      ++closed_cells;
    }
    else if (bound_cells_[index].state_ == BoundCell::kTryMineCell_1) {
      ++mine_cells;
    }
  }
  unsigned int mines_around = info.fixed_mines_around_ + mine_cells; // Fixed mines (with game field) + closed cells, marked as mines
  unsigned int max_mines = mines_around + closed_cells;
  if (info.value_ < mines_around || info.value_ > max_mines) {
    return false;
  }
  if (closed_cells == 0) {
    // All around cells are defined. And mines amount is equils cell value
    return true;
  }
  if (level > kMaxLevel) {
    return true;
  }
  // Enumeration of cases
  bool cases_exist = false;
  do {
    unsigned int new_marked_mines = GetMinesInCaseCells(cells);
    if (info.value_ != mines_around + new_marked_mines) {
      continue;
    }
    SetCaseCells(cells);
    bool check_fault = false;
    for (auto& cell: cells) {
      auto& bound_cell = bound_cells_[cell.bound_index_];
      for (auto& pos: bound_cell.near_value_cells_) {
        unsigned int mine_arg;
        unsigned int clear_arg;
        if (!EnumerateCases(pos, level + 1, mine_arg, clear_arg)) {
          check_fault = true;
          break;
        }
      }
      if (check_fault) {
        break;
      }
    }
    if (!check_fault) {
      cases_exist = true;
      // Store variant
      for (auto& cell: cells) {
        if (cell.marked_mine_) {
          cell.can_be_mine_ = true;
        }
        else {
          cell.can_be_open_ = true;
        }
      }
    }
    ResetCaseCells(cells);
  } while (NextCombinationOfCaseCells(cells));
  // Form result if there are valid cases
  if (cases_exist) {
    for (auto& cell: cells) {
      // Check bomb only
      if (!cell.can_be_open_ && cell.can_be_mine_ && bomb_bound_cell == kUnknownBoundCell) {
        bomb_bound_cell = cell.bound_index_;
      }
      // Check clear only
      if (cell.can_be_open_ && !cell.can_be_mine_ && clear_bound_cell == kUnknownBoundCell) {
        clear_bound_cell = cell.bound_index_;
      }
    }
    return true;
  }
  return false;
}

void BruteForce::FormRandomStep(const Field& field, unsigned int& step_row, unsigned int& step_col, StepAction& step) {
  step_row = (unsigned int)(1.0 * field.size() * random() / RAND_MAX);
  if (step_row >= field.size()) {
    step_row = field.size() - 1;
  }
  step_col = (unsigned int)(1.0 * field[step_row].size() * random() / RAND_MAX);
  if (step_col >= field[step_row].size()) {
    step_col = field[step_row].size() - 1;
  }
  step = kOpenWithProbability;
}

unsigned int BruteForce::GetMinesInCaseCells(const CaseCells& cells) {
  unsigned int mines = 0;
  for (auto& cell: cells) {
    if (cell.marked_mine_) {
      ++mines;
    }
  }
  return mines;
}

void BruteForce::SetCaseCells(const CaseCells& cells) {
  for (const auto& cell: cells) {
    if (cell.marked_mine_) {
      bound_cells_[cell.bound_index_].state_ = BoundCell::kTryMineCell_1;
    }
    else {
      bound_cells_[cell.bound_index_].state_ = BoundCell::kTryOpenCell_1;
    }
  }
}

void BruteForce::ResetCaseCells(const CaseCells& cells) {
  for (auto& cell: cells) {
    bound_cells_[cell.bound_index_].state_ = BoundCell::kClosedCell;
  }
}

bool BruteForce::NextCombinationOfCaseCells(CaseCells& cells) {
  for (auto iter = cells.begin(); iter != cells.end(); ++iter) {
    iter->marked_mine_ = !iter->marked_mine_;
    if (iter->marked_mine_) {
      return true;
    }
  }
  return false;
}
