// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <unistd.h>  // getopt

#include <cstdlib>  // atoi, exit

#include <iostream>  // std::cin
#include <string>
#include <vector>

#include "src/chess.h"
#include "src/chessproblem.h"
#include "src/formated.h"
#include "src/m_attribute.h"

using std::string;
using std::vector;

class ChessProblemDemo : public ChessProblem {
  bool Output() override;
  bool Progress(int level, const chess::MoveList& moves) override;
  bool Progress(int level, const chess::Move& my_move) override;

 public:
  int max_solutions_;
  FILE *progress_io_;

  explicit ChessProblemDemo(int max_solutions) :
    ChessProblem(), max_solutions_(max_solutions), progress_io_(nullptr) {
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
"-V   Output version and exit\n"
"-h   Output this help text and exit");
}

int main(int argc, char **argv) {
  char eparg('\0');
  chess::Castling castling(chess::kAllCastling);
  ChessProblemDemo chessproblem(2);
  bool get_stdin(false);
  int opt;
  while ((opt = getopt(argc, argv, "pPim:M:s:S:H:n:c:e:bwVh")) != -1) {
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
              exit(EXIT_FAILURE);
            }
          }
        }
        break;
      case 'e':
        eparg = optarg[0];
        if ((eparg < 'a') || (eparg > 'h')) {
          format::SayError("Argument %s of -e is not understood") % optarg;
          exit(EXIT_FAILURE);
        }
        break;
      case 'b':
        chessproblem.set_color(chess::kBlack);
        break;
      case 'w':
        chessproblem.set_color(chess::kWhite);
        break;
      case 'V':
        format::Say("%s %s") % PACKAGE_NAME % PACKAGE_VERSION;
        exit(EXIT_SUCCESS);
        break;
      case 'h':
        Help();
        exit(EXIT_SUCCESS);
        break;
      default:
        exit(EXIT_FAILURE);
        break;
    }
  }
  if (chessproblem.get_mode() == ChessProblem::kUnknown) {
    format::SayError("One of the options -M, -S, or -H has to be specified\n"
      "Use option -h for help");
    exit(EXIT_FAILURE);
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
      exit(EXIT_FAILURE);
    }
    if (get_stdin) {
      format::SayError("With option -i no arguments must be specified");
      exit(EXIT_FAILURE);
    }
    PlaceFigures(&chessproblem, chess::kWhite, argv[optind]);
    PlaceFigures(&chessproblem, chess::kBlack, argv[optind + 1]);
  } else {
    if (!get_stdin) {
      format::Say("Enter the white position in chess notation:");
    }
    if (std::cin.eof()) {
      exit(EXIT_FAILURE);
    }
    string line;
    std::getline(std::cin, line);
    PlaceFigures(&chessproblem, chess::kWhite, line);
    if (!get_stdin) {
      format::Say("Enter the black position in chess notation:");
    }
    if (std::cin.eof()) {
      exit(EXIT_FAILURE);
    }
    std::getline(std::cin, line);
    PlaceFigures(&chessproblem, chess::kBlack, line);
  }
  if (!chessproblem.HaveKings()) {
    format::SayError("There are not white and black kings on the board");
    exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
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
      exit(EXIT_FAILURE);
    }
    if (chessproblem->GetFigure(pos) != chess::kEmpty) {
      format::SayError("Figure was already on this field: %s") % s;
      exit(EXIT_FAILURE);
    }
    chessproblem->PlaceFigure(chess::ColoredFigure(figure, color), pos);
  }
}

static int CheckNum(const char *num, int min_value, char c) {
  int ret(atoi(num));
  if (ret < min_value) {
    format::SayError("Argument %s of -%s should be at least %d")
      % num
      % c
      % min_value;
    exit(EXIT_FAILURE);
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

bool ChessProblemDemo::Output() {
  auto num = num_solutions_found_;
  format::Say("Solution %s: %s")
    % num
    % get_move_stack();
  return ((max_solutions_ == 0) || (num < max_solutions_));
}

bool ChessProblemDemo::Progress(int level, const chess::MoveList& moves) {
  if ((progress_io_ == nullptr) || (level > 1)) {
    return true;
  }
  if (level != 0) {
    format::Format(progress_io_, "%s replies to check: %s", true, true)
      % moves.size()
      % str(moves);
    return true;
  }
  format::Format(progress_io_, "%s%s start moves to check: %s", true, true)
    % str()
    % moves.size()
    % str(moves);
  return true;
}

bool ChessProblemDemo::Progress(int level, const chess::Move& my_move) {
  if ((progress_io_ == nullptr) || (level > 1)) {
    return true;
  }
  if (level != 0) {
    format::Format(progress_io_, "Checking reply %s", true, true)
      % str(my_move);
    return true;
  }
  PushMove(&my_move);
  string board(*this);
  PopMove();
  format::Format(progress_io_, "Checking start move %s\n%s", false, true)
    % str(my_move)
    % board;
  return true;
}
