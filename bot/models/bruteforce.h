#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

#include <map>
#include <set>
#include <vector>

#include "classifier.h"

class BruteForce : public Classifier
{
public:
  BruteForce();

  virtual bool LoadModel(std::vector<uint8_t>&&) override { return true; }
  virtual void GetStep(const Field& field, unsigned int mines_amount, unsigned int& step_row, unsigned int& step_col, StepAction& step) override;


 private:
  const unsigned int kMaxLevel = 3;
  const unsigned int kUnknownBoundCell = (unsigned int)(-1);
  struct CellPos {
    unsigned int row_;
    unsigned int col_;
    bool operator<(const CellPos& arg) const {
      if (row_ < arg.row_) return true;
      if (row_ > arg.row_) return false;
      return col_ < arg.col_;
    }
  };

  const unsigned int kMaxNeighbourCells = 8;
  struct BoundCell { // Struct of closed cell near opened
    CellPos pos_;
    enum {
      kClosedCell,
      kTryOpenCell_1,
      kTryMineCell_1,
    } state_;
    std::set<CellPos> near_value_cells_;

    BoundCell() {
      state_ = kClosedCell;
    }
  };
  struct ValueCell { // Struct of opened cell near bound
    unsigned int value_; // Value of opened cell
    unsigned int fixed_mines_around_;
    std::vector<size_t> near_bound_cells_;
  };
  struct CaseClosedCell { // Information about cell, closed for the enumeration
    unsigned int bound_index_;
    bool marked_mine_; // Flag closed cell will be marked as mine. In false, cell wil be open
    bool can_be_open_;
    bool can_be_mine_;
  };
  typedef std::vector<BoundCell> BoundCells;
  typedef std::map<CellPos, ValueCell> ValueCells;
  typedef std::vector<CaseClosedCell> CaseCells;

  BoundCells bound_cells_; // The closed cells near opened cells
  ValueCells value_cells_; // The opened cells near closed cells
  unsigned int max_bound_mines_;
  unsigned int bound_mines_;

  void FormBoundValue(const Field& field);
  void FormValueCell(const Field& field, const CellPos& pos, ValueCell& info);
  void FindMaxMines(const Field& field, unsigned int mines_amount);
  void FormSequence(std::vector<CellPos>& seque); // Form sequence of value cells for calculation
  bool EnumerateCases(const CellPos& value_pos, unsigned int level, unsigned int& bomb_bound_cell, unsigned int& clear_bound_cell);
  void FormRandomStep(const Field& field, unsigned int& step_row, unsigned int& step_col);
  unsigned int GetMinesInCaseCells(const CaseCells& cells);
  void SetCaseCells(const CaseCells& cells);
  void ResetCaseCells(const CaseCells& cells);
  bool NextCombinationOfCaseCells(CaseCells& cells);
};

#endif // BRUTEFORCE_H
