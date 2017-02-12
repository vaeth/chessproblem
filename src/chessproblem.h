// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CHESSPROBLEM_H_
#define SRC_CHESSPROBLEM_H_ 1

#include <config.h>

#include "src/m_assert.h"
#include "src/chess.h"

/*
This is a recursive solver for chess problems, based on the chess library.

In the current implementation, this class inherits from chess::Field,
thus providing a simple interface for usage.

Due to this inheriting, this class is currently not appropriate for
parallel processing: For parallel processing, we would have to provide
a separate copy if chess::Field for each thread.
Since this would make the interface more complicated (or requires to write a
lot of wrappers to keep the interface), this is currently not implemented:

I wanted to have a running project within 1-2 days, and for my purposes
the current version is fast enough anyway: No need to bug more simultaneous
processor power.

This class intentionally contains no I/O. All I/O (if at all) happens
over a virtual Output() function. To implement this function, just inherit
from the ChessProblem class and override the Output() function.

The Output() function is called for any found solution; if its return value
is false, the solver stops looking for further solutions.
The Output() function has access to the current stack of the chess library,
so it is up to this function to interpret that stack correspondingly.
The return value is the number of solutions found.

There are also similiar abstract Progress() functions which can optionally
be specialized to output the progress of the solver. Note that the Progress()
function can make use of str() of the inherited chess::Field
(possibly framed in PushMove() and PopMove() to show the effect of the move).
*/

class ChessProblem : public chess::Field {
  // This function is called for every found solution.
  // If it returns false, do not look for further solutions.
  // The default is to return true, so that by default Solve() just returns
  // the number of solutions found.
  // For HelpMate the full path is on the stack when Output() is called;
  // for Mate and SelfMate only the first move is on the stack.
  // The functions is non-const so that PushMove() and PopMove() can be used.
  virtual bool Output() {
    return true;
  }

  // If the progress value is true, this function is called
  // before we attack the specified list of moves.
  // Level = 0 means that this is the list of the first move.
  // The functions are non-const so that PushMove() and PopMove() can be used.
  virtual void Progress([[maybe_unused]] int level,
    [[maybe_unused]] const chess::MoveList& moves) {
  }

  // If the progress value is true, this function is called
  // before we attempt the move specified as parameter,
  // Level = 0 means that this is from the top-level list of moves.
  // The functions are non-const so that PushMove() and PopMove() can be used.
  virtual void Progress([[maybe_unused]] int level,
    [[maybe_unused]] const chess::Move& my_move) {
  }

 public:
  // Set this to true if the Progress() functions should be called
  enum Mode { kUnknown, kMate, kSelfMate, kHelpMate };

  ChessProblem() :
    chess::Field(), mode_(kUnknown), half_moves_(0), default_color_(true) {
  }

  ChessProblem(Mode mode, int moves) :
    chess::Field(), default_color_(true) {
    set_mode(mode, moves);
  }

  // Better make a virutal destructor, since we expect inheritance.
  // However, it is unlikely that this destructor is ever needed.
  virtual ~ChessProblem() {
  }

  void clear() {
    chess::Field::clear();
    mode_ = kUnknown;
    half_moves_ = 0;
    default_color_ = true;
  }

  // mode and moves must be legal.
  // half_moves_ are set.
  // Default color is pre-initialized if set_color() has not been called yet.
  void set_mode(Mode mode, int moves) {
    ASSERT((mode == kMate) || (mode == kSelfMate) || (mode == kHelpMate));
    ASSERT(moves > 0);
    mode_ = mode;
    half_moves_ = ((mode == kMate) ? (2 * moves - 1) : (2 * moves));
    set_default_color();
  }

  Mode get_mode() const {
    return mode_;
  }

  int get_half_moves() const {
    return half_moves_;
  }

  void set_color(chess::Figure color) {
    chess::Field::set_color(color);
    default_color_ = false;
  }

  // Set the default color according to mode_ if not already specified.
  void set_color() {
    ASSERT(mode_ != kUnknown);
    set_default_color();
    default_color_ = false;
  }

  int Solve();

 protected:
  // When Output() is called, it can access the already updated number
  int num_solutions_found_;

 private:
  Mode mode_;
  int half_moves_;
  bool default_color_;

  enum Result { kCancel, kWin, kLoose };

  // Avoid to put redundant information on the stack:
  int remaining_half_moves_;

  // These values are initialized to avoid recalculation during recursion:
  Result mate_value_, nomate_value_, nonsuccess_value_;
  int half_moves_minus_one_;

  void set_default_color() {
    if (default_color_) {
      chess::Field::set_color((mode_ == kHelpMate) ?
        chess::kBlack : chess::kWhite);
    }
  }

  ChessProblem::Result RecursiveSolver();
};

#endif  // SRC_CHESSPROBLEM_H_
