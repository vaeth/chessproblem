// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "src/chessproblem.h"
#include <config.h>

#include "src/m_assert.h"
#include "src/m_likely.h"

int ChessProblem::Solve() {
  ASSERT(mode_ != kUnknown);
  ASSERT(half_moves_ > 0);
  // Make sure to use defaults if the user of the class did not:
  set_default_color();
  default_color_ = false;

  num_solutions_found_ = 0;
  remaining_half_moves_ = half_moves_;
  half_moves_minus_one_ = half_moves_ - 1;
  switch (mode_) {
    case kMate:
      // If we are mate in the last move, we have lost
      mate_value_ = nonsuccess_value_ = kLoose;
      nomate_value_ = kWin;
      break;
    case kSelfMate:
      // If we are mate in the last move, we have won
      mate_value_ = kWin;
      nomate_value_ = nonsuccess_value_ = kLoose;
      break;
    default:
    // case kHelpMate:
      // All players "win" always so that we do not cut
      mate_value_ = nomate_value_ = nonsuccess_value_ = kWin;
  }
  // format::Print() % Readable();
  RecursiveSolver();
  return num_solutions_found_;
}

// We do a MinMax (or MaxMax for HalfMate) without pruning only when winning:
// Since there are only two states (win or loose, no even),
// alpha/beta pruning would happen only if "normal" pruning happens anyway...
ChessProblem::Result ChessProblem::RecursiveSolver() {
  if (remaining_half_moves_ == 0) {
    if (UNLIKELY(IsCheckMate())) {
      if (mode_ == kHelpMate) {
        ++num_solutions_found_;
        return (Output() ? kWin : kCancel);
      }
      return mate_value_;
    }
    return nomate_value_;
  }
  chess::MoveList moves;
  if (UNLIKELY(!Generator(&moves))) {
    // Early mate or stalemate. This is hairy...
    if ((remaining_half_moves_ & 1) != 0) {
      // If we are not the party which needs to be mate in the last move,
      // we do not care whether this is mate or stalemate:
      // If mode_ is KMate we have not reached our goal.
      // If mode_ is kSelfMate we (as opponent) have reached our goal.
      // If mode_ is kHelpMate we simply ignore this failed leaf.
      return mate_value_;
    }
    // Now we do the same as in the above case (remaining_half_moves_ == 0):
    if (!IsInCheck()) {
      // Early stalemate
      return nomate_value_;
    }
    // Early mate
    if (LIKELY(mode_ != kHelpMate)) {
      return mate_value_;
    }
    // We get here only in case of ill-posed HelpMate problems
    // with a cook having less moves than the desired solution
    ++num_solutions_found_;
    return (Output() ? kWin : kCancel);
  }
  Progress(half_moves_ - remaining_half_moves_, moves);

  // Do not forget to re-increase the value before returning!
  // (This is not necessary when returning due to Output() cancelation...)
  --remaining_half_moves_;
  for (chess::MoveList::const_iterator it(moves.begin());
    it != moves.end(); PopMove(), ++it) {
    Progress(half_moves_minus_one_ - remaining_half_moves_, *it);
    PushMove(&(*it));
    int opponent(RecursiveSolver());
    // Normally, we should PopMove() here, but we must postpone for Output().
    // So we must not forget to PopMove() at every exit of this block.
    // We do this in the "for" clause above, but we must take care of this
    // in "break" or "return" statements...
    if (UNLIKELY(opponent == kCancel)) {
      PopMove();
      return kCancel;
    }
    if (opponent == kWin) {
      // If opponent has reached his goal or if we are in helpmate do not prune
      continue;
    }
    // This is the only pruning we can do: We need not check after winning
    // (except when in the top level so that we find cooks).
    if (LIKELY(remaining_half_moves_ != half_moves_minus_one_)) {
      ++remaining_half_moves_;
      PopMove();
      return kWin;
    }
    ++num_solutions_found_;
    if (UNLIKELY(!Output())) {
      PopMove();
      return kCancel;
    }
  }
  ++remaining_half_moves_;
  // At the top level, this return value might be wrong, but we do not care...
  return nonsuccess_value_;
}
