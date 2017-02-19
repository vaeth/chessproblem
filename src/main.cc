// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <unistd.h>  // getopt

#include <cstdlib>  // atoi, exit

#include <iostream>  // cin, getline
#include <string>
#include <vector>

#include "src/chess.h"
#include "src/chessproblem.h"
#include "src/formated.h"
#include "src/m_attribute.h"
#include "src/m_likely.h"

using std::string;
using std::vector;

class ChessProblemDemo : public ChessProblem {
  ATTRIBUTE_NONNULL_ bool Output(chess::Field *field) const override;
  ATTRIBUTE_NONNULL_ bool Progress(const chess::MoveList *moves,
      chess::Field *field) const override;
  ATTRIBUTE_NONNULL_ bool Progress(const chess::Move *my_move,
      chess::Field *field) const override;

 public:
  int max_solutions_;
  bool verbose;
  FILE *progress_io_;

  explicit ChessProblemDemo(int max_solutions) :
    ChessProblem(), max_solutions_(max_solutions), verbose(false),
    progress_io_(nullptr) {
  }
};

static void Help();
ATTRIBUTE_NONNULL_ static void PlaceFigures(ChessProblem *chessproblem,
    chess::Figure color, const string &str);
ATTRIBUTE_NONNULL_ static int CheckNum(const char *num,
    int min_value, char c);
ATTRIBUTE_NONNULL_ static void SplitString(vector<string> *res,
    const string& str);

static void Help() {
  format::Say("Usage: chessproblem [options] white-pieces black-pieces\n"
"Output solutions of a chess problem, including possible cooks.\n"
"\n"
"The pieces must be a single string, separated by commas or spaces\n"
"(when using spaces, do not forget quoting when calling from shell)\n"
"in usual chess notation, e.g. \"Ke1,Qd1,Ra1,Rh1,Nc1,Bb1,a2,Pb2\"\n"
"(the \"P\" can be omitted as in this example string).\n"
"\n"
"When no pieces are specified, the input of pieces is interactive.\n"
"Optionally, the position can be read from standard input.\n"
"\n"
"The return value is 0 if there is a unique solution\n"
"\n"
"Options:\n"
"-i   Read position from standard input\n"
"-j X Use up to X parallel threads %s\n"
"-J X For a new thread require at least X half moves depth %s\n"
"-M X Mate in X moves (2X - 1 half moves)\n"
"-S X Selfmate in X moves (2X half moves)\n"
"-H X Helpmate in X moves (2X half moves)\n"
"-n X Print at most X solutions. Default value is 2. X=0 means to print all.\n"
"-c X Exclude certain castling. X is the field (or list of fields,\n"
"     separated by commas) of relevant figures which had been moved.\n"
"     For instance \"e1,a8\" excludes all castling from white (even if the\n"
"     white king should be on e1, the program will assume that it has been\n"
"     moved earlier in the game), and it excludes long castling of black.\n"
"-e X Allow en passant as the first move in column X (a...h)\n"
"-b   First move is from black (default only for helpmate)\n"
"-w   First move is from white (default for mate or selfmate)\n"
"-p   Output progress on stdout\n"
"-P   Output progress on stderr\n"
"-v   Progress output is extremely verbose\n"
"-V   Output version and exit\n"
"-h   Output this help text and exit") %
#ifndef NO_CHESSPROBLEM_THREADS
(format::Format(" (default is %s)") % ChessProblemDemo::kMaxParallelDefault) %
(format::Format(" (default is %s)") % ChessProblemDemo::kMinHalfMovesDepth);
#else
" (ignored:\n"
"     program is compiled without threading support)" %
" (ignored:\n"
"     program is compiled without threading support)";
#endif
}

int main(int argc, char **argv) {
  char eparg('\0');
  chess::Castling castling(chess::kAllCastling);
  ChessProblemDemo chessproblem(2);
  bool get_stdin(false);
  int opt;
  while ((opt = getopt(argc, argv, "pPij:J:m:M:s:S:H:n:c:e:bwvVh")) != -1) {
    switch (opt) {
      case 'p':
        chessproblem.progress_io_ = stdout;
        break;
      case 'P':
        chessproblem.progress_io_ = stderr;
        break;
      case 'i':
        get_stdin = true;
        break;
      case 'j':
        chessproblem.set_max_parallel(CheckNum(optarg, 1, 'j'));
        break;
      case 'J':
        chessproblem.set_min_half_moves_depth(CheckNum(optarg, 1, 'J'));
        break;
      case 'm':
      case 'M':
        chessproblem.set_mode(ChessProblem::kMate,
          CheckNum(optarg, 1, 'm'));
        break;
      case 's':
      case 'S':
        chessproblem.set_mode(ChessProblem::kSelfMate,
          CheckNum(optarg, 1, 's'));
        break;
      case 'H':
        chessproblem.set_mode(ChessProblem::kHelpMate,
          CheckNum(optarg, 1, 'h'));
        break;
      case 'n':
        chessproblem.max_solutions_ = CheckNum(optarg, 0, 'n');
        break;
      case 'c': {
          vector<string> s;
          SplitString(&s, optarg);
          for (auto c : s) {
            if (c == "e1") {
              castling = chess::UnsetCastling(castling,
                chess::kNoWhiteCastling);
            } else if (c == "a1") {
              castling = chess::UnsetCastling(castling,
                chess::kNoWhiteLongCastling);
            } else if (c == "h1") {
              castling = chess::UnsetCastling(castling,
                chess::kNoWhiteShortCastling);
            } else if (c == "e8") {
              castling = chess::UnsetCastling(castling,
                chess::kNoBlackCastling);
            } else if (c == "a8") {
              castling = chess::UnsetCastling(castling,
                chess::kNoBlackLongCastling);
            } else if (c == "h8") {
              castling = chess::UnsetCastling(castling,
                chess::kNoBlackShortCastling);
            } else {
              format::SayError("Argument %s of -c is not understood") % c;
              std::exit(EXIT_FAILURE);
            }
          }
        }
        break;
      case 'e':
        eparg = optarg[0];
        if ((eparg < 'a') || (eparg > 'h')) {
          format::SayError("Argument %s of -e is not understood") % optarg;
          std::exit(EXIT_FAILURE);
        }
        break;
      case 'b':
        chessproblem.set_color(chess::kBlack);
        break;
      case 'w':
        chessproblem.set_color(chess::kWhite);
        break;
      case 'v':
        chessproblem.verbose = true;
        break;
      case 'V':
        format::Say("%s %s") % PACKAGE_NAME % PACKAGE_VERSION;
        std::exit(EXIT_SUCCESS);
        break;
      case 'h':
        Help();
        std::exit(EXIT_SUCCESS);
        break;
      default:
        std::exit(EXIT_FAILURE);
        break;
    }
  }
  if (chessproblem.get_mode() == ChessProblem::kUnknown) {
    format::SayError("One of the options -M, -S, or -H has to be specified\n"
      "Use option -h for help");
    std::exit(EXIT_FAILURE);
  }
  chessproblem.set_color();
  chess::EnPassant ep(chess::kNoEnPassant);
  if (eparg != '\0') {
    ep = chess::Field::CalcPos(eparg,
      (chessproblem.get_color() == chess::kWhite) ? '6' : '3');
  }
  if (optind < argc) {
    if (optind + 2 != argc) {
      format::SayError(
        "Only 0 or 2 arguments are admissible, but %d are specified")
        % (argc - optind);
      std::exit(EXIT_FAILURE);
    }
    if (get_stdin) {
      format::SayError("With option -i no arguments must be specified");
      std::exit(EXIT_FAILURE);
    }
    PlaceFigures(&chessproblem, chess::kWhite, argv[optind]);
    PlaceFigures(&chessproblem, chess::kBlack, argv[optind + 1]);
  } else {
    if (!get_stdin) {
      format::Say("Enter the white position in chess notation:");
    }
    if (std::cin.eof()) {
      std::exit(EXIT_FAILURE);
    }
    string line;
    std::getline(std::cin, line);
    PlaceFigures(&chessproblem, chess::kWhite, line);
    if (!get_stdin) {
      format::Say("Enter the black position in chess notation:");
    }
    if (std::cin.eof()) {
      std::exit(EXIT_FAILURE);
    }
    std::getline(std::cin, line);
    PlaceFigures(&chessproblem, chess::kBlack, line);
  }
  if (!chessproblem.HaveKings()) {
    format::SayError("There are not white and black kings on the board");
    std::exit(EXIT_FAILURE);
  }
  if (!chessproblem.IsEnPassantValid(ep, true)) {
    format::SayError("Invalid or useless en passant field specified");
    chess::EnPassantList eps;
    chessproblem.CalcEnPassant(&eps);
    if (eps.empty()) {
      format::SayError("In the given position no en passant move is possible");
    } else {
      string admissible_ep;
      for (auto e : eps) {
        char letter, number;
        chess::Field::LetterNumber(&letter, &number, e);
        if (!admissible_ep.empty()) {
          admissible_ep.append(", ");
        }
        admissible_ep.append(1, letter);
      }
      format::SayError("Admissible value(s) would be: %s") % admissible_ep;
    }
    std::exit(EXIT_FAILURE);
  }
  chessproblem.set_ep(ep);
  chess::Castling new_castling(chessproblem.CalcCastling(castling));
  chessproblem.set_castling(new_castling);
  int num(chessproblem.Solve());
  if (num == 0) {
    format::Say("No solution exists");
  }
  return (num == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void PlaceFigures(ChessProblem *chessproblem, chess::Figure color,
    const string &str) {
  vector<string> figures;
  SplitString(&figures, str);
  if (figures.empty()) {
    format::SayError("No figures of color %s specified")
      % chess::color_name[color];
    std::exit(EXIT_FAILURE);
  }
  for (auto s : figures) {
    chess::Figure figure(chess::kNoFigure);
    chess::Pos pos(chess::Field::kFieldEnd);
    if (s.length() == 2) {
      figure = chess::kPawn;
      pos = chess::Field::CalcPos(s);
    } else if (s.length() == 3) {
      figure = chess::FigureValue(s[0]);
      pos = chess::Field::CalcPos(s.substr(1));
    }
    if ((figure == chess::kNoFigure) || (pos == chess::Field::kFieldEnd)) {
      format::SayError("Figure or placement not understood: %s") % s;
      std::exit(EXIT_FAILURE);
    }
    if (chessproblem->GetFigure(pos) != chess::kEmpty) {
      format::SayError("Figure was already on this field: %s") % s;
      std::exit(EXIT_FAILURE);
    }
    chessproblem->PlaceFigure(chess::ColoredFigure(figure, color), pos);
  }
}

static int CheckNum(const char *num, int min_value, char c) {
  int ret(std::atoi(num));
  if (ret < min_value) {
    format::SayError("Argument %s of -%s should be at least %d")
      % num
      % c
      % min_value;
    std::exit(EXIT_FAILURE);
  }
  return ret;
}

static void SplitString(vector<string> *res, const string& str) {
  string::size_type last_pos(0);
  for (string::size_type pos(0);
      (pos = str.find_first_of("\t\r\n ,.:;!?_-", pos)) != string::npos;
      last_pos = ++pos) {
    if (pos > last_pos) {
      res->emplace_back(str, last_pos, pos - last_pos);
    }
  }
  if (str.size() > last_pos) {
    res->emplace_back(str, last_pos);
  }
}

bool ChessProblemDemo::Output(chess::Field *field) const {
  auto num = num_solutions_found_;
  format::Say("Solution %s: %s")
    % num
    % field->get_move_stack();
  return ((max_solutions_ == 0) || (num < max_solutions_));
}

bool ChessProblemDemo::Progress(const chess::MoveList *moves,
    chess::Field *field) const {
  // Make no I/O the quick case, because with I/O time does not matter so much
  if (LIKELY(progress_io_ == nullptr)) {
    return true;
  }
  auto& move_stack = field->get_move_stack();
  auto level = move_stack.size();
  if (LIKELY(level > 1)) {
    if (LIKELY(!verbose)) {
      return true;
    }
  }
  format::Format(progress_io_, "%s%s %s to check: %s", true, true)
    % (*field)
    % moves->size()
    % ((level == 0) ? std::string("start moves") :
       (format::Format("replies to %s") % move_stack).str())
    % field->str(*moves);
  return true;
}

bool ChessProblemDemo::Progress(const chess::Move *my_move,
    chess::Field *field) const {
  // Make no I/O the quick case, because with I/O time does not matter so much
  if (LIKELY(progress_io_ == nullptr)) {
    return true;
  }
  auto& move_stack = field->get_move_stack();
  auto level = move_stack.size();
  if (UNLIKELY(verbose)) {
    if (UNLIKELY(level == 0)) {
      format::Format(progress_io_, "Checking %s", true, true)
        % field->str(*my_move);
        return true;
    }
    format::Format(progress_io_, "Checking %s %s", true, true)
      % move_stack
      % field->str(*my_move);
    return true;
  }
  if (LIKELY(level > 1)) {
    return true;
  }
  if (level != 0) {
    format::Format(progress_io_, "Checking %s %s", true, true)
      % move_stack
      % field->str(*my_move);
    return true;
  }
  field->PushMove(my_move);
  string board(*field);
  field->PopMove();
  format::Format(progress_io_, "Checking %s\n%s", false, true)
    % field->str(*my_move)
    % board;
  return true;
}
