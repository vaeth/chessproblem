// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CHESSPROBLEM_H_
#define SRC_CHESSPROBLEM_H_ 1

#include <config.h>

#include <cassert>

#ifndef NO_CHESSPROBLEM_THREADS
#include <map>
#include <mutex>  // NOLINT(build/c++11)
#include <set>
#endif

#include "src/chess.h"
#include "src/m_attribute.h"

#ifndef NO_CHESSPROBLEM_THREADS
namespace chessproblem {
class Result;
}  // namespace chessproblem
#endif  // NO_CHESSPROBLEM_THREADS

/*
This is a recursive solver for chess problems, based on the chess library.

This class intentionally contains no I/O. All I/O (if at all) happens
over a virtual Output() function (and Process() functions).
To implement these functions, just inherit from the ChessProblem class
and override the corresponding functions.

The Output() function is called for any found solution; if its return value
is false, the solver is canceled and does not look for further solutions.
The Output() function gets passed the ChessField as the "field" argument.
(As an implicit argument it also inherits a ChessField as a base class.
In multi-threaded mode, the latter should not be used, since it does not
necessarily correspond to the currently running thread. In single-threaded
mode both are identical.)
It is up to the Output() function to interpret the passed ChessField and
its associated move stack for the user.
There are also similiar abstract Progress() functions which can optionally
be specialized to output the progress of the solver. For these the same
general remarks hold as for the Output() function.

While the Output() and Progress() functions are called, a mutex is locked
in multithreaded mode, so you can do any output without having to fear
that it is mixed with the output of other threads.
Note that these functions can output the passed "field" (and move-list/move).
Note that in order to show the position after a move, the function can use
field->PushMove (then output) followed by field->PopMove.
Note also that for a correct output of the move (including figure name),
the str() function from the passed field should be used.
*/

class ChessProblem : public chess::Field {
  // This function is called for every found solution.
  // If it returns false, the search process is canceled.
  // The default implementation just returns true, so that by default
  // Solve() just returns the number of solutions found and produces no I/O.
  // Output() should ignore the content of the enherited chess::Field
  // but just use the content of the passed argument &field instead.
  // (This is necessary for threading, because each thread receives a copy).
  // For HelpMate the full path is on the stack (of the passed &field)
  // when Output() is called;for Mate and SelfMate only the first move
  // is on the stack.
  ATTRIBUTE_NODISCARD ATTRIBUTE_NONNULL_ virtual bool Output(ATTRIBUTE_UNUSED
      chess::Field *field) const {
    UNUSED(field)
    return true;
  }

  // If the progress value is true, this function is called
  // before we attack the specified list of moves.
  // The size of field->get_move_stack() determines the current depth level.
  // Also this function can cancel the whole process by returning false.
  // The default implementation only returns true.
  ATTRIBUTE_NODISCARD ATTRIBUTE_NONNULL_ virtual bool Progress(
      ATTRIBUTE_UNUSED const chess::MoveList *moves,
      ATTRIBUTE_UNUSED chess::Field *field) const {
    UNUSED(level) UNUSED(moves) UNUSED(field)
    return true;
  }

  // If the progress value is true, this function is called
  // before we attempt the move specified as parameter,
  // The size of field->get_move_stack() determines the current depth level
  // (the passed move has not yet been pushed).
  // Also this function can cancel the whole process by returning false.
  // The default implementation only returns true.
  ATTRIBUTE_NODISCARD ATTRIBUTE_NONNULL_ virtual bool Progress(
      ATTRIBUTE_UNUSED const chess::Move *my_move,
      ATTRIBUTE_UNUSED chess::Field *field) const {
    UNUSED(level) UNUSED(my_move) UNUSED(field)
    return true;
  }

 public:
  enum Mode { kUnknown, kMate, kSelfMate, kHelpMate };

  constexpr static const int
    kMaxParallelDefault = 1,
    kMinHalfMovesDepth = 5;

  ChessProblem() :
    chess::Field(), mode_(kUnknown), half_moves_(0), default_color_(true) {
    set_max_parallel(kMaxParallelDefault);
    set_min_half_moves_depth(kMinHalfMovesDepth);
  }

  ChessProblem(Mode mode, int moves) :
    chess::Field(), default_color_(true) {
    set_mode(mode, moves);
    set_max_parallel(kMaxParallelDefault);
    set_min_half_moves_depth(kMinHalfMovesDepth);
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
    assert((mode == kMate) || (mode == kSelfMate) || (mode == kHelpMate));
    assert(moves > 0);
    mode_ = mode;
    half_moves_ = ((mode == kMate) ? (2 * moves - 1) : (2 * moves));
    set_default_color();
  }

  ATTRIBUTE_NODISCARD Mode get_mode() const {
    return mode_;
  }

  ATTRIBUTE_NODISCARD int get_half_moves() const {
    return half_moves_;
  }

  void set_color(chess::Figure color) {
    chess::Field::set_color(color);
    default_color_ = false;
  }

  // Set the default color according to mode_ if not already specified.
  void set_color() {
    assert(mode_ != kUnknown);
    set_default_color();
    default_color_ = false;
  }

#ifndef NO_CHESSPROBLEM_THREADS

  void set_max_parallel(int max_parallel) {
    max_parallel_ = max_parallel;
  }

  ATTRIBUTE_NODISCARD int get_max_parallel() {
    return max_parallel_;
  }

  void set_min_half_moves_depth(int min_half_moves_depth) {
    min_half_moves_depth_ = min_half_moves_depth;
  }

  ATTRIBUTE_NODISCARD int get_min_half_moves_depth() const {
    return min_half_moves_depth_;
  }

#else  // defined(NO_CHESSPROBLEM_THREADS)

  void set_max_parallel(ATTRIBUTE_UNUSED int max_parallel) {
    UNUSED(max_parallel)
  }

  ATTRIBUTE_NODISCARD int get_max_parallel() {
    return 1;
  }

  void set_min_half_moves_depth(ATTRIBUTE_UNUSED int min_half_moves_depth) {
    UNUSED(min_half_moves_depth)
  }

  ATTRIBUTE_NODISCARD int get_min_half_moves_depth() const {
    return kMinHalfMovesDepth;
  }

#endif  // NO_CHESSPROBLEM_THREADS

  // This is the main function which should be called when all is set up.
  // The return value is the number of solutions found.
  int Solve();

 protected:
  // When Output() is called, it can access the already updated number
#ifndef NO_CHESSPROBLEM_THREADS
  volatile
#endif
  int num_solutions_found_;

 private:
  Mode mode_;
  int half_moves_;
  bool default_color_;
#ifndef NO_CHESSPROBLEM_THREADS
  chessproblem::Result *cancel_;
#else
  bool cancel_;
#endif

  // These values are initialized to avoid recalculation during recursion:
  bool mate_value_, nomate_value_, default_return_value_;

#ifndef NO_CHESSPROBLEM_THREADS
  int max_parallel_, min_half_moves_depth_;
  int max_threads_, new_thread_depth_;
  volatile int thread_count_;
  volatile bool have_running_threads_;
  typedef std::set<const chess::Move *> ProcessedMoves;
  // This should actually be volatile, but STL does not provide this.
  // Let us hope that the compiler does not optimize container access away...
  std::map<const chess::MoveList *, ProcessedMoves> processed_moves_;
  std::mutex io_mutex_, thread_count_mutex_, processed_moves_mutex_;
  typedef std::lock_guard<std::mutex> LockGuard;
#endif  // NO_CHESSPROBLEM_THREADS

  void set_default_color() {
    if (default_color_) {
      chess::Field::set_color((mode_ == kHelpMate) ?
        chess::kBlack : chess::kWhite);
    }
  }

#ifndef NO_CHESSPROBLEM_THREADS

  // Increase num_solutions_found, call Output() thread-safe.
  // Possibly set cancel_ and return true if cancel_
  ATTRIBUTE_NONNULL_ bool OutputCancel(chess::Field *field);

  // Call Progress() thread-safe.
  // Possibly set cancel_ and return true if cancel_
  ATTRIBUTE_NONNULL_ bool ProgressCancel(const chess::MoveList *moves,
      chess::Field *field);

  // Call Progress() thread-sade.
  // Possibly set cancel_ and return true if cancel_
  ATTRIBUTE_NONNULL_ bool ProgressCancel(const chess::Move *my_move,
      chess::Field *field);

  ATTRIBUTE_NONNULL_ bool RecursiveSolver(chessproblem::Result *result,
      chess::Field *field);

  ATTRIBUTE_NONNULL_ void SolverThread(chessproblem::Result *result,
    const chess::MoveList *moves,
    const chess::MoveList::const_iterator my_begin, chess::Field *field);

  // If we can produce a new thread, increase number and return true:
  // True means that we _must_ produce a new thread afterwards (or otherwise
  // call DecareaseThreads()).
  // Locks already before reading to make sure to not increase too much!
  ATTRIBUTE_NODISCARD bool IncreaseThreads() {
    LockGuard lock(thread_count_mutex_);
    auto curr_count = thread_count_;  // local variable to omit volatile access
    if (curr_count >= max_threads_) {
      return false;
    }
    thread_count_ = curr_count + 1;
    return ((have_running_threads_ = true));
  }

  void DecreaseThreads() {
    LockGuard lock(thread_count_mutex_);
    if (--thread_count_ == 0) {
      have_running_threads_ = false;
    }
  }

  // Return true if currently threads may be running.
  // By its very nature, this information can be outdated.
  // It is guaranteed that if false is returned then no thread is running
  // (or at least not modifying volatile data or producing any output).
  bool HaveRunningThreads() {
    // No lock is necessary for reading a bool
    return have_running_threads_;
  }

  bool SingleThreadedMode() {
    return (max_threads_ == 0);
  }

  bool MultiThreadedMode() {
    return (max_threads_ != 0);
  }

#else  // defined(NO_CHESSPROBLEM_THREADS)

  // Increase num_solutions_found, call Output().
  // Possibly set cancel_ and return true if cancel_
  inline bool OutputCancel();

  // Call Progress(). Possibly set cancel_ and return true if cancel_
  ATTRIBUTE_NONNULL_ bool ProgressCancel(const chess::MoveList *moves);

  // Call Progress(). Possibly set cancel_ and return true if cancel_
  ATTRIBUTE_NONNULL_ bool ProgressCancel(const chess::Move *my_move);

  bool RecursiveSolver();

#endif  // NO_CHESSPROBLEM_THREADS
};

#endif  // SRC_CHESSPROBLEM_H_
