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
      kTryOpenCell,
      kTryMineCell,
    } state_;
    std::set<CellPos> near_value_cells_;
    bool can_be_open_;
    bool can_be_mine_;

    BoundCell() {
      state_ = kTryOpenCell;
      can_be_open_ = false;
      can_be_mine_ = false;
    }
  };
  struct ValueCell { // Struct of opened cell near bound
    unsigned int value_; // Value of opened cell
    unsigned int mines_near_; // Amount of mines near cell
  };
  typedef std::vector<BoundCell> BoundCells;
  typedef std::map<CellPos, ValueCell> ValueCells;

  BoundCells bound_cells_; // The closed cells near opened cells
  ValueCells value_cells_; // The opened cells near closed cells
  unsigned int max_bound_mines_;
  unsigned int bound_mines_;

  void FormBoundValue(const Field& field);
  void FormValueCell(const Field& field, const CellPos& pos, ValueCell& info);
  bool NextCase();
  bool IsCaseRight();
  void StoreCase();
  void CheckFillOverflow();
  void FindStep(unsigned int& row, unsigned int& col, StepAction& step);
  void FindMaxMines(const Field& field, unsigned int mines_amount);
};

#endif // BRUTEFORCE_H
