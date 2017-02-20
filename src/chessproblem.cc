// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "src/chessproblem.h"
#include <config.h>

#include <cassert>

#ifndef NO_CHESSPROBLEM_THREADS
#include <atomic>
#include <mutex>  // NOLINT(build/c++11)
#include <thread>  // NOLINT(build/c++11)
#include <vector>
#endif

#include "src/m_attribute.h"
#include "src/m_likely.h"

const int
  ChessProblem::kMaxParallelDefault,
  ChessProblem::kMinHalfMovesDepth;

#ifndef NO_CHESSPROBLEM_THREADS
namespace chessproblem {

// For each MoveList, one Communicate object is created.
// All threads "testing" lists from this MoveList as their first move
// share this objects.
// In order to receive a signal from parent MoveLists, we collect all objects
// as a tree (with "inverse" pointers from childs to parents only).
// The "root" of that tree (to which all nodes eventually point to) is *cancel_

class Communicate {
 private:
  Communicate *parent_;
  bool equal_level_threads_;
  std::atomic_bool kill_signal_;
  const chess::MoveList *moves_;
  std::atomic<chess::MoveList::const_iterator> current_;
  chess::MoveList::const_iterator end_;
  std::atomic_bool result_;
  std::mutex current_mutex_;
  typedef std::lock_guard<std::mutex> LockGuard;

 public:
  explicit Communicate(Communicate *parent) :
    parent_(parent), equal_level_threads_(false), kill_signal_(false) {
  }

  ATTRIBUTE_NONNULL_ explicit Communicate(Communicate *parent,
      const chess::MoveList *moves, bool result) :
    parent_(parent), equal_level_threads_(false), kill_signal_(false),
    moves_(moves), current_(moves->begin()), end_(moves->end()),
    result_(result) {
  }

  ATTRIBUTE_NODISCARD bool get_equal_level_threads() {
    return equal_level_threads_;
  }

  // This must be called before starting any subthread of equal level:
  // There is no atomic signaling mechanism used.
  void set_equal_level_threads() {
    equal_level_threads_ = true;
  }

  // Return true if GetIncreasing() would probably get a new move.
  // By its very nature, the data can already be outdated at the return.
  bool HaveNextUnsafe() const {
    return (current_.load(std::memory_order_consume) != end_);
  }

  // Get the next move. Return true if there is some. Not thread-safe
  ATTRIBUTE_NONNULL_ bool GetIncreasingUnsafe(const chess::Move **my_move) {
    auto current = current_.load(std::memory_order_relaxed);
    if (UNLIKELY(current == end_)) {
      return false;
    }
    *my_move = &(*current);
    ++current;
    current_.store(current, std::memory_order_relaxed);
    return true;
  }

  // Get the next move. Return true if there is some. Thread-safe and slow
  ATTRIBUTE_NONNULL_ bool GetIncreasing(const chess::Move **my_move) {
    LockGuard lock(current_mutex_);
    auto current = current_.load(std::memory_order_consume);
    if (UNLIKELY(current == end_)) {
      return false;
    }
    *my_move = &(*current);
    ++current;
    current_.store(current, std::memory_order_release);
    return true;
  }

  // Get the next move. Return true if there is some
  ATTRIBUTE_NONNULL_ bool GetIncreasing(const chess::Move **my_move,
      bool threadsafe) {
    if (threadsafe) {
      return GetIncreasing(my_move);
    }
    return GetIncreasingUnsafe(my_move);
  }

  // Return the signaled result
  bool get_result() const {
    return result_.load(std::memory_order_consume);
  }

  // Signal success
  void Win() {
    result_.store(true, std::memory_order_release);
  }

  // Signal kill to current thread and all descendants
  void Kill() {
    kill_signal_.store(true, std::memory_order_release);
  }

  // Has current thread or some of its parents received a signal?
  bool GotSignal() const {
    for (const Communicate *curr(this); LIKELY(curr != nullptr);
      curr = curr->parent_) {
      if (UNLIKELY(curr->kill_signal_.load(std::memory_order_consume))) {
        return true;
      }
    }
    return false;
  }

  // As GotSignal() but faster if we know that there are no parents
  bool TopSignal() const {
    assert(parent_ == nullptr);
    return kill_signal_.load(std::memory_order_consume);
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
  num_solutions_found_.store(0, std::memory_order_release);
  thread_count_ = 0;
  if (half_moves_ < min_half_moves_depth_) {
    max_threads_ = 0;
  } else {
    max_threads_ = max_parallel_ - 1;
    new_thread_depth_ = half_moves_ - min_half_moves_depth_;
  }
  chessproblem::Communicate kill_childs(nullptr);
  RecursiveSolver((cancel_ = &kill_childs), this);
  return num_solutions_found_.load(std::memory_order_acquire);
#else
  num_solutions_found_ = 0;
  cancel_ = false;
  RecursiveSolver();
  return num_solutions_found_;
#endif
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
bool ChessProblem::RecursiveSolver(chessproblem::Communicate *parent,
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
  chessproblem::Communicate communicate(parent, &moves, default_return_value_);
  SolverThread(&communicate, field);
  return communicate.get_result();
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
void ChessProblem::SolverThread(chessproblem::Communicate *communicate,
    chess::Field *field) {
  bool subthread(communicate->get_equal_level_threads());
  std::vector<std::thread> threads;
  const chess::Move *current_move;
  while (communicate->GetIncreasing(&current_move, HaveRunningThreads())) {
    // Possibly start a new thread
    if (MultiThreadedMode()) {  // (quick test to do a shortcut)
      if (communicate->GotSignal()) {
        break;
      }
      if (field->get_move_stack().size() <= new_thread_depth_) {
        if (communicate->HaveNextUnsafe()) {
          if (IncreaseThreads()) {
            communicate->set_equal_level_threads();
            threads.emplace_back(&ChessProblem::SolverThread, this,
              communicate, new chess::Field(*field));
          }
        }
      }
    }
    if (UNLIKELY(ProgressCancel(current_move, field))) {
      return;
    }
    field->PushMove(current_move);
    bool opponent(RecursiveSolver(communicate, field));
    chess::PushGuard push_guard(field);  // Postpone field->PopMove()
    if (cancel_->TopSignal()) {
      break;
    }
    if (opponent == true) {
      // If opponent has reached his goal or if we are in helpmate do not prune
      continue;
    }
    communicate->Win();
    // This is the only pruning we can do: We need not check after winning
    // (except when in the top level so that we find cooks).
    if (LIKELY(field->get_move_stack().size() != 1)) {
      // We are at top level (note that push_guard still exists!)
      communicate->Kill();
      break;
    }
    if (UNLIKELY(OutputCancel(field))) {
      break;
    }
  }
  if (SingleThreadedMode()) {  // Only a shortcut
    return;
  }
  for (auto& t : threads) {
    t.join();
  }
  if (subthread) {  // We are at the end of a subthread
    delete field;
    DecreaseThreads();
  }
}
#endif  // NO_CHESSPROBLEM_THREADS
