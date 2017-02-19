// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "src/chessproblem.h"
#include <config.h>

#include <cassert>

#ifndef NO_CHESSPROBLEM_THREADS
#include <thread>  // NOLINT(build/c++11)
#include <vector>
#endif

#include "src/m_likely.h"

const int
  ChessProblem::kMaxParallelDefault,
  ChessProblem::kMinHalfMovesDepth;

#ifndef NO_CHESSPROBLEM_THREADS
namespace chessproblem {
class Result {
 private:
  Result *parent_;
  volatile bool kill_signal_;
  volatile bool result_;

 public:
  explicit Result(Result *parent) :
    parent_(parent), kill_signal_(false) {
  }

  explicit Result(Result *parent, bool result) :
    parent_(parent), kill_signal_(false), result_(result) {
  }

  bool get_result() const {
    return result_;
  }

  void Win() {
    // No lock is necessary for setting a bool:
    // Except for initialization, we set it _only_ to true
    result_ = true;
  }

  // Signal all kill to current thread and all subthreads
  void Kill() {
    // No lock is necessary for setting a bool:
    // Except for initialization, we set it _only_ to true
    kill_signal_ = true;
  }

  // Has current thread or some of its parents received a signal?
  bool GotSignal() const {
    for (const Result *curr(this); LIKELY(curr != nullptr);
      curr = curr->parent_) {
      // No lock is necessary for reading a bool
      if (UNLIKELY(curr->kill_signal_)) {
        return true;
      }
    }
    return false;
  }

  // As GotSignal() but faster if we know that there are no parents
  bool TopSignal() const {
    assert(parent_ == nullptr);
    return kill_signal_;
  }
};
}  // namespace chessproblem
#endif  // NO_CHESSPROBLEM_THREADS

int ChessProblem::Solve() {
  assert(mode_ != kUnknown);
  assert(half_moves_ > 0);
  // Make sure to use defaults if the user of the class did not set them:
  set_default_color();
  default_color_ = false;
  assert(LegalValues());
  assert(LegalState());

  num_solutions_found_ = 0;
  switch (mode_) {
    case kMate:
      // If we are mate in the last move, we have lost
      mate_value_ = default_return_value_ = false;
      nomate_value_ = true;
      break;
    case kSelfMate:
      // If we are mate in the last move, we have won
      mate_value_ = true;
      nomate_value_ = default_return_value_ = false;
      break;
    default:
    // case kHelpMate:
      // All players "win" always so that we do not cut
      mate_value_ = nomate_value_ = default_return_value_ = true;
  }
  // format::Print() % str();
#ifndef NO_CHESSPROBLEM_THREADS
  thread_count_ = 0;
  have_running_threads_ = false;
  if (half_moves_ < min_half_moves_depth_) {
    max_threads_ = 0;
  } else {
    max_threads_ = max_parallel_ - 1;
    new_thread_depth_ = half_moves_ - min_half_moves_depth_;
  }
  chessproblem::Result kill_childs(nullptr);
  RecursiveSolver((cancel_ = &kill_childs), this);
#else
  cancel_ = false;
  RecursiveSolver();
#endif
  return num_solutions_found_;
}

#ifndef NO_CHESSPROBLEM_THREADS

#define OUTPUT_CANCEL(a) OutputCancel(a)
#define GENERATOR(a, b) a->Generator(b)
#define IS_IN_CHECK(a) a->IsInCheck()
#define IS_CHECK_MATE(a) a->IsCheckMate()

bool ChessProblem::OutputCancel(chess::Field *field) {
  if (HaveRunningThreads()) {
    LockGuard lock(io_mutex_);
    // Omit output if another thread canceled.
    // Note that user cancelation happens only with locked io_mutex_
    if (UNLIKELY(cancel_->TopSignal())) {
      return true;
    }
    ++num_solutions_found_;
    if (LIKELY(Output(field))) {
      return false;
    }
    // It is important to set cancel_ while we have the lock
    cancel_->Kill();
    return true;
  }
  if (UNLIKELY(cancel_->TopSignal())) {
    // Another thread has caused a cancel_, but meanwhile all threads finished
    return true;
  }
  ++num_solutions_found_;
  if (LIKELY(Output(field))) {
    return false;
  }
  cancel_->Kill();
  return true;
}

bool ChessProblem::ProgressCancel(const chess::MoveList *moves,
    chess::Field *field) {
  if (HaveRunningThreads()) {
    LockGuard lock(io_mutex_);
    // Omit output if another thread canceled.
    // Note that user cancelation happens only with locked io_mutex_
    if (UNLIKELY(cancel_->TopSignal())) {
      return true;
    }
    if (LIKELY(Progress(moves, field))) {
      return false;
    }
    // It is important to set cancel_ while we have the lock
    cancel_->Kill();
    return true;
  }
  if (UNLIKELY(cancel_->TopSignal())) {
    // Another thread has caused a cancel_, but meanwhile all threads finished
    return true;
  }
  if (LIKELY(Progress(moves, field))) {
    return false;
  }
  cancel_->Kill();
  return true;
}

bool ChessProblem::ProgressCancel(const chess::Move *my_move,
    chess::Field *field) {
  if (HaveRunningThreads()) {
    LockGuard lock(io_mutex_);
    // Omit output if another thread canceled.
    // Note that user cancelation happens only with locked io_mutex_
    if (UNLIKELY(cancel_->TopSignal())) {
      return true;
    }
    if (LIKELY(Progress(my_move, field))) {
      return false;
    }
    // It is important to set cancel_ while we have the lock
    cancel_->Kill();
    return true;
  }
  if (UNLIKELY(cancel_->TopSignal())) {
    // Another thread has caused a cancel_, but meanwhile all threads finished
    return true;
  }
  if (LIKELY(Progress(my_move, field))) {
    return false;
  }
  cancel_->Kill();
  return true;
}

#else  // defined(NO_CHESSPROBLEM_THREADS)

#define OUTPUT_CANCEL(a) OutputCancel()
#define GENERATOR(a, b) Generator(b)
#define IS_IN_CHECK(a) IsInCheck()
#define IS_CHECK_MATE(a) IsCheckMate()

inline bool ChessProblem::OutputCancel() {
  ++num_solutions_found_;
  if (LIKELY(Output(this))) {
    return false;
  }
  return ((cancel_ = true));
}

inline bool ChessProblem::ProgressCancel(const chess::MoveList *moves) {
  if (LIKELY(Progress(moves, this))) {
    return false;
  }
  return ((cancel_ = true));
}

inline bool ChessProblem::ProgressCancel(const chess::Move *my_move) {
  if (LIKELY(Progress(my_move, this))) {
    return false;
  }
  return ((cancel_ = true));
}

#endif  // NO_CHESSPROBLEM_THREADS

// We do a MinMax (or MaxMax for HalfMate) without pruning only when winning:
// Since there are only two states (win or loose, no even),
// alpha/beta pruning would happen only if "normal" pruning happens anyway...
#ifndef NO_CHESSPROBLEM_THREADS
bool ChessProblem::RecursiveSolver(chessproblem::Result *result,
    chess::Field *field) {
  int remaining_half_moves(static_cast<int>(field->get_move_stack().size())
    - half_moves_);
#else
bool ChessProblem::RecursiveSolver() {
  int remaining_half_moves(static_cast<int>(get_move_stack().size())
    - half_moves_);
#endif  // NO_CHESSPROBLEM_THREADS
  if (remaining_half_moves == 0) {
    if (UNLIKELY(IS_CHECK_MATE(field))) {
      if (mode_ == kHelpMate) {
        OUTPUT_CANCEL(field);
        return true;
      }
      return mate_value_;
    }
    return nomate_value_;
  }
  chess::MoveList moves;
  if (UNLIKELY(!GENERATOR(field, &moves))) {
    // Early mate or stalemate. This is hairy...
    if ((remaining_half_moves & 1) != 0) {
      // If we are not the party which needs to be mate in the last move,
      // we do not care whether this is mate or stalemate:
      // If mode_ is KMate we have not reached our goal.
      // If mode_ is kSelfMate we (as opponent) have reached our goal.
      // If mode_ is kHelpMate we simply ignore this failed leaf.
      return mate_value_;
    }
    // Now we do the same as in the above case (remaining_half_moves == 0):
    if (!IS_IN_CHECK(field)) {
      // Early stalemate
      return nomate_value_;
    }
    // Early mate
    if (LIKELY(mode_ != kHelpMate)) {
      return mate_value_;
    }
    // We get here only in case of ill-posed HelpMate problems
    // with a cook having less moves than the desired solution
    OUTPUT_CANCEL(field);
    return true;
  }
#ifndef NO_CHESSPROBLEM_THREADS
  if (UNLIKELY(ProgressCancel(&moves, field))) {
    return true;
  }
  // result might be modified several times, but only to "true" (if at all).
  // Therefore, it is safe to not even use any mutex
  chessproblem::Result child_result(result, default_return_value_);
  SolverThread(&child_result, &moves, moves.begin(), field);
  return child_result.get_result();
#else  // defined(NO_CHESSPROBLEM_THREADS)
  if (UNLIKELY(ProgressCancel(&moves))) {
    return true;
  }
  for (auto it = moves.begin(); it != moves.end(); ++it) {
    const chess::Move *current_move(&(*it));
    if (UNLIKELY(ProgressCancel(current_move))) {
      return true;
    }
    PushMove(current_move);
    int opponent(RecursiveSolver());
    chess::PushGuard push_guard(this);  // Postpone PopMove() to after Output
    if (UNLIKELY(cancel_)) {
      return true;
    }
    if (opponent) {
      // If opponent has reached his goal or if we are in helpmate do not prune
      continue;
    }
    // This is the second inconsistency with the multithreaded code:
    // We do not set the return value to "true" here, see below.

    // This is the only pruning we can do: We need not check after winning
    // (except when in the top level so that we find cooks).
    if (LIKELY(get_move_stack().size() != 1)) {
      // We are at top level (note that push_guard still exists!)
      return true;
    }
    if (UNLIKELY(OutputCancel())) {
      return true;
    }
  }
  // Note that in contrast to the multithreaded mode, we do not store any
  // return value. Instead, when we get here we know that we did not do the
  // pruning due to winning. In other words: We lost (except in kHelpMate),
  // except perhaps when we are in top level and cancel_ was never set.
  // In that case, the subsequent return value is "incorrect", but we throw
  // away the return value of the top level anyway.
  return default_return_value_;
#endif  // NO_CHESSPROBLEM_THREADS
}

#ifndef NO_CHESSPROBLEM_THREADS
void ChessProblem::SolverThread(chessproblem::Result *result,
    const chess::MoveList *moves,
    const chess::MoveList::const_iterator my_begin, chess::Field *field) {
  std::vector<std::thread> threads;
  bool partial_list(my_begin != moves->begin());
  const chess::MoveList::const_iterator moves_end(moves->end());
  for (auto it = my_begin; it != moves_end; ++it) {
    const chess::Move *current_move(&(*it));
    // Make sure each move from the current moves is processed at most once
    if (HaveRunningThreads()  // (a shortcut if no threads are running)
      && (partial_list  // We are a subthread of a loop
        || !threads.empty())) {  // or we already started a thread in the loop
      LockGuard lock(processed_moves_mutex_);
      if (!processed_moves_[moves].insert(current_move).second) {
        // the move was already processed
        continue;
      }
    }
    // Possibly start a new thread
    if (MultiThreadedMode()) {  // (quick test to do a shortcut)
      if (result->GotSignal()) {
        break;
      }
      if (field->get_move_stack().size() <= new_thread_depth_) {
        chess::MoveList::const_iterator next_it(it);
        ++next_it;
        if (next_it != moves_end) {  // shortcut if we would immediately end
          if (IncreaseThreads()) {
            // The new thread is just the current loop starting from ++it
            threads.emplace_back(&ChessProblem::SolverThread, this,
              result, moves, next_it, new chess::Field(*field));
          }
        }
      }
    }
    if (UNLIKELY(ProgressCancel(current_move, field))) {
      return;
    }
    field->PushMove(current_move);
    bool opponent(RecursiveSolver(result, field));
    chess::PushGuard push_guard(field);  // Postpone field->PopMove()
    if (result->GotSignal()) {
      break;
    }
    if (opponent == true) {
      // If opponent has reached his goal or if we are in helpmate do not prune
      continue;
    }
    result->Win();
    // This is the only pruning we can do: We need not check after winning
    // (except when in the top level so that we find cooks).
    if (LIKELY(field->get_move_stack().size() != 1)) {
      // We are at top level (note that push_guard still exists!)
      result->Kill();
      break;
    }
    if (UNLIKELY(OutputCancel(field))) {
      break;
    }
  }
  if (SingleThreadedMode()) {  // Only a shortcut
    return;
  }
  if (!threads.empty()) {  // We had started subthreads since my_begin
    for (auto& t : threads) {
      t.join();
    }
    if (!partial_list) {  // We are responsible for the whole list.
      // Then the whole list was processed, and we can free the resources
      LockGuard lock(processed_moves_mutex_);
      processed_moves_.erase(moves);
      return;  // A shortcut, because we are not at the end of a subthread
    }
  }
  if (partial_list) {  // We are at the end of a subthread
    delete field;
    DecreaseThreads();
  }
}
#endif  // NO_CHESSPROBLEM_THREADS
