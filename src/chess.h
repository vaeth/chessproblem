// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CHESS_H_
#define SRC_CHESS_H_ 1

#include <config.h>

#include <deque>
#include <list>
#include <string>
#include <vector>

#include "src/m_assert.h"

namespace chess {
class Field;

typedef unsigned char Figure;
static constexpr Figure
  kEmpty = 0,
  kWhite = kEmpty,
  kBlack = 1,
  kNoFigure = kBlack,
  kPawn = 2,
  kKnight = 4,
  kBishop = 6,
  kRook = 8,
  kQueen = 10,
  kKing = 12,
  kMaxFigure = 13,
  kColor = kBlack,
  kFigure = 15 ^ kColor;

// Google style recommends to use inline if not used withing constexpr
// initializers, but some are really used in this way.
// For consistence and to avoid later rewrites, we make all constexpr
// (because "in mentality" they are part of the constants...)
// Moreover, if the compiler does not inline, maybe it takes it as a hint to
// produce separate functions for separate constant args...

constexpr const Figure FigureColor(const Figure f) {
  return (f & kColor);
}

constexpr const Figure UncoloredFigure(const Figure f) {
  return (f & kFigure);
}

constexpr const bool IsColor(const Figure color) {
  return ((color == kWhite) || (color == kBlack));
}

constexpr const Figure ColoredFigure(const Figure f, const Figure color) {
  return (f | color);
}

// Used for both, inverting a plain color or a color of the figure
constexpr const Figure InvertColor(const Figure figure) {
  return (figure ^ kColor);
}

// This is guaranteed to return 0 or 1. The argument _must_ be a plain color.
constexpr const int Color2Index(const Figure color) {
  return static_cast<int>(color);
}

static constexpr Figure
  kIndexMax = Color2Index(kWhite) + Color2Index(kBlack),
// the colored figures
  kWhitePawn = ColoredFigure(kPawn, kWhite),
  kWhiteKnight = ColoredFigure(kKnight, kWhite),
  kWhiteBishop = ColoredFigure(kBishop, kWhite),
  kWhiteRook = ColoredFigure(kRook, kWhite),
  kWhiteQueen = ColoredFigure(kQueen, kWhite),
  kWhiteKing = ColoredFigure(kKing, kWhite),
  kBlackPawn = ColoredFigure(kPawn, kBlack),
  kBlackKnight = ColoredFigure(kKnight, kBlack),
  kBlackBishop = ColoredFigure(kBishop, kBlack),
  kBlackRook = ColoredFigure(kRook, kBlack),
  kBlackQueen = ColoredFigure(kQueen, kBlack),
  kBlackKing = ColoredFigure(kKing, kBlack);

// The following arrays match the above constants
extern const char *uncolored_figure_name[kMaxFigure + 1],
  *colored_figure_name[kMaxFigure + 1],
  *color_name[kIndexMax + 1];

constexpr const Figure ColorValue(const char color) {
  return (((color == 'w') || (color == 'W')) ? kWhite : kBlack);
}

constexpr static Figure FigureValue(const char figure) {
  return ((figure == 'N') ? kKnight :
         ((figure == 'B') ? kBishop :
         ((figure == 'R') ? kRook :
         ((figure == 'Q') ? kQueen :
         ((figure == 'K') ? kKing :
         ((figure == 'P') ? kPawn : kNoFigure))))));
}

// Use the functions from Field to convert intto/from coordinate form.
// Google style does not recommend to use unsigned, but it is the only way to
// get a minimal compile time check...
typedef unsigned int Pos;
typedef signed char PosDelta;  // Differ from Pos to get a compile time check

constexpr const Pos AddDelta(const Pos pos, const PosDelta delta) {
  return static_cast<const Pos>(static_cast<const int>(pos) +
    static_cast<const int>(delta));
}

// EnPassant is either the EnPassant position (skipped field)
// or kNoEndPassant if none allowed.
typedef Pos EnPassant;
static constexpr EnPassant
  kNoEnPassant = 0,
  kUnknownEnPassant = 1;

typedef std::vector<EnPassant> EnPassantList;

// This is a bitfield:
typedef unsigned char Castling;
static constexpr Castling
  kNoCastling = 0,
  kWhiteShortCastling = 1,
  kWhiteLongCastling = 2,
  kBlackShortCastling = 4,
  kBlackLongCastling = 8,
  kWhiteCastling = (kWhiteShortCastling | kWhiteLongCastling),
  kBlackCastling = (kBlackShortCastling | kBlackLongCastling),
  kAllCastling = (kWhiteCastling | kBlackCastling),
  kUnknownCastling = kAllCastling + 1;

constexpr const Castling NegateCastling(const Castling c) {
  return (kAllCastling ^ c);
}

constexpr const Castling BlackToWhiteCastling(const Castling have) {
  return (have >> 2);
}

constexpr const bool HaveCastling(const Castling have,
    const Castling flags) {
  return ((have & flags) != kNoCastling);
}

constexpr const Castling SetCastling(const Castling have,
    const Castling flag) {
  return (have | flag);
}

// flag must already be negated!
constexpr const Castling UnsetCastling(const Castling have,
    const Castling flag) {
  return (have & flag);
}

static constexpr Castling
  kNoWhiteShortCastling = NegateCastling(kWhiteShortCastling),
  kNoWhiteLongCastling = NegateCastling(kWhiteLongCastling),
  kNoBlackShortCastling = NegateCastling(kBlackShortCastling),
  kNoBlackLongCastling = NegateCastling(kBlackLongCastling),
  kNoWhiteCastling = NegateCastling(kWhiteCastling),
  kNoBlackCastling = NegateCastling(kBlackCastling);

// A list of positions occupied by figures of a certain color
typedef std::list<Pos> PosList;

class Move {
 public:
  enum MoveType {
    kNormal,  // Normal move
    kDouble,  // Pawn double move
    kEnPassant,
    kShortCastling,
    kLongCastling,
    kQueen,  // Pawn transforms into Queen
    kKnight,  // Pawn transforms into Knight
    kRook,  // Pawn transforms into Rook
    kBishop  // Pawn transforms into Bishop
  };
  MoveType move_type_;
  Pos from_, to_;
  Move(MoveType move_type, Pos from, Pos to) :
    move_type_(move_type), from_(from), to_(to) {
  }

  // append a human readable form of the move
  void Readable(std::string *res, const Field &chess_field) const;

  void Readable(std::string *res, Figure from_figure, Figure to_figure) const;

  void Readable(std::string *res) const {
    Readable(res, kPawn, kPawn);
  }

  // return a human readable form of the move
  std::string Readable(const Field &chess_field) const {
    std::string r;
    Readable(&r, chess_field);
    return r;
  }

  std::string Readable(Figure from_figure, Figure to_figure) const {
    std::string r;
    Readable(&r, from_figure, to_figure);
    return r;
  }

  std::string Readable() const {
    return Readable(kPawn, kPawn);
  }
};

class MoveList : public std::vector<Move> {
 public:
  // append a human readable form of the list
  void Readable(std::string *res, const Field &chess_field) const;

  void Readable(std::string *res) const;

  // return a human readable form of the list
  std::string Readable(const Field &chess_field) const {
    std::string r;
    Readable(&r, chess_field);
    return r;
  }

  std::string Readable() const {
    std::string r;
    Readable(&r);
    return r;
  }
};

class MoveStore {
 public:
  const Move *move_;
  EnPassant ep_;
  Castling castling_;
  Figure from_figure_, to_figure_;
  MoveStore(const Move *m, EnPassant ep, Castling c,
      Figure from_figure, Figure to_figure) :
    move_(m), ep_(ep), castling_(c), from_figure_(from_figure),
    to_figure_(to_figure) {
  }

  // append a human readable form of the move
  void Readable(std::string *res) const {
    move_->Readable(res, from_figure_, to_figure_);
  }

  // return a human readable form of the move
  std::string Readable() const {
    return move_->Readable(from_figure_, to_figure_);
  }
};

class MoveStack : public std::deque<MoveStore> {
 public:
  // append a human readable form of the stack
  void Readable(std::string *res) const;

  // return a human readable form of the move
  std::string Readable() const {
    std::string r;
    Readable(&r);
    return r;
  }
};

/*
Field is the main class to maintain the chess rules.

All coordinates are in the form "Pos". You can call CalcPos() or Readable()
to generate from/to human readable form.

Note that to initialize a field, you not only have to place the figures using
but you also have to set correctly the color of the party which is on move,
whether en-passant is possible in the first move, and which types of castling
are possible. There are auxiliary functions which help in this.
You are supposed to set this data before calling any other functions from
the library. By default (and after clear()) this data is made invalid.

Setting of the figures and of the color should happen with

PlaceFigure()
set_color()

Only after this, it is admissible (and required) to call

set_ep()
set_castling()

You are reliable that there are exactly two kings on the field and that
the castling and ep data are really possible in the first move of the
corresponding party.
To get the list of admissible values/flags for ep/castling, you can call
(after PlaceFigure() and set_color()):

CalcEnPassant()
CalcCastling()

The main library usage consists then usually of a sucessive call of:

Generator()  generates a list of all valid moves on the board
IsInCheck()  Check whether the moving party is in check
             (e.g. if Generator() returned an empty list)
PushMove()   execute a move, e.g. previously generated
PopMove()    undo the last pushed move and return it.

You can expect the currrent board with

GetFigure()  (or operator []);
get_ep()
get_castling()

and there are auxiliary functions

IsInCheck()
IsThreatened()

You can modify the board with

PlaceFigure()
RemoveFigure()
MoveFigure()
set_color()
set_ep()
set_castling()

but you are relable about keeping it consistent and not to interfere
with pushed moves. You can actually inspect the pushed moves with

get_move_stack()

This is a standard container of MoveStore entries with the last pushed
element being at the back.

*/
class Field {
  friend class Move;

 public:
  static constexpr Pos kColumns = 8;
  static constexpr Pos kRows = 8;

/*
In the standard case (columns = rows = 8) the internal field looks like this;
note that it is "on head" concerning the moves and mirrored concerning columns

     A  B  C  D  E  F  G  H
  * ,* ,* ,* ,* ,* ,* ,* ,* ,* ,
  * ,* ,* ,* ,* ,* ,* ,* ,* ,* ,
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 1
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 2
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 3
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 4
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 5
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 6
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 7
  * ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,_ ,* , // 8
  * ,* ,* ,* ,* ,* ,* ,* ,* ,* ,
  * ,* ,* ,* ,* ,* ,* ,* ,* ,*

*/

  static constexpr PosDelta
    kLeft = -1,
    kRight = 1,
    kUp = kColumns + 2,
    kDown = -kUp,
    kUpRight = kUp + kRight,
    kDownRight = kDown + kRight,
    kUpLeft = kUp + kLeft,
    kDownLeft = kDown + kLeft;

  static constexpr Pos
    kNpos = 0,
    kFieldSize = (kColumns + 2) * (kRows + 4),
    kFieldStart = (kColumns + 2) * 2 + 1,
    kLastRow = (kColumns + 2) * (kRows + 1) + 1,
    kFieldEnd = (kColumns + 2) * (kRows + 2) - 1,  // first nonaccessible

// En passant fields:
    kEndRow2 = (kColumns + 2) * 4,  // first after end of row 2
    kStartRow7 = (kColumns + 2) * kRows + 1,
    kStartRow3 = kEndRow2 + 1,
    kEndRow3 = kEndRow2 + kColumns + 2,
    kStartRow6 = kStartRow7 - kColumns - 2,
    kEndRow6 = kStartRow7 - 2,

// Castling fields:
    kPosWhiteKing = kFieldStart + (kColumns / 2),
    kPosWhiteLongRook = kFieldStart,
    kPosWhiteShortRook = kFieldStart + kColumns - 1,
    kPosBlackKing = kLastRow + (kColumns / 2),
    kPosBlackLongRook = kLastRow,
    kPosBlackShortRook = kLastRow + kColumns - 1;

  static constexpr PosDelta
    kWhitePawnHit1Delta = kUpLeft,
    kWhitePawnHit2Delta = kUpRight,
    kWhitePawnMoveDelta = kUp,
    kBlackPawnHit1Delta = kDownLeft,
    kBlackPawnHit2Delta = kDownRight,
    kBlackPawnMoveDelta = kDown;

  static const PosDelta knight_deltas[8];
  static const PosDelta bishop_deltas[4];
  static const PosDelta rook_deltas[4];
  static const PosDelta king_deltas[8];
  static const PosDelta white_pawn_hit_deltas[2];
  static const PosDelta black_pawn_hit_deltas[2];

  // For illegal arguments, the value kFieldEnd is returned
  static Pos CalcPos(char letter, char number);

  // For illegal arguments, the value kFieldEnd is returned
  static Pos CalcPos(const char *letter_number);

  // For illegal arguments, the value kFieldEnd is returned
  static Pos CalcPos(const std::string &letter_number);

  // A printable form of the field is appended
  void Readable(std::string *result) const;

  std::string Readable() const {
    std::string r;
    Readable(&r);
    return r;
  }

  static void Readable(char *letter, char *number, Pos pos);

  // The position is appended in a readable form
  static void Readable(std::string *letter_number, Pos pos);

  static std::string Readable(Pos pos) {
    std::string result;
    Readable(&result, pos);
    return result;
  }

  // The move is appended in a readable form
  void Readable(std::string *result, const Move& m) const {
    m.Readable(result, *this);
  }

  std::string Readable(const Move& m) const {
    return m.Readable(*this);
  }

  // The move_list is appended in a readable form
  void Readable(std::string *result, const MoveList& move_list) const {
    move_list.Readable(result, *this);
  }

  std::string Readable(const MoveList& move_list) const {
    return move_list.Readable(*this);
  }

  void PlaceFigure(Figure figure, Pos pos);

  // There must be a real figure to remove
  void RemoveFigure(Pos pos);

  // There must be a real figure to move
  void MoveFigure(Pos from, Pos to);

  // Add all possible ep values (except kNoEnPassand) for the position.
  // Requires that color has been set.
  void CalcEnPassant(EnPassantList *ep_values) const;

  // Return all those castling value for the position wich are set in c.
  // Thus, CalcCastling(kAllCastling) returns all possible castling values.
  Castling CalcCastling(Castling c) const;

  void set_color(Figure color) {
    ASSERT(IsColor(color));
    color_ = FigureColor(color);
  }

  // Return whether set_color was already called
  bool have_color() const {
    return (IsColor(color_));
  }

  void set_ep(EnPassant ep) {
    ASSERT(have_color());
    ASSERT((ep == kNoEnPassant) || ((color_ == kWhite) ?
      ((ep >= kStartRow6) && (ep < kEndRow6)) :
      ((ep >= kStartRow3) && (ep < kEndRow3))));
    ep_ = ep;
  }

  void set_castling(Castling c) {
    ASSERT(c < kUnknownCastling);
    castling_ = c;
  }

  // Return true if (at least) one black and white king are on the board
  bool HaveKings() const;

  // Return true if the castling value makes sense for the position
  bool IsCastlingValid(Castling c) const {
    return (CalcCastling(c) == c);
  }

  // Return true if the ep value is valid (i.e. looks like a double move).
  // Requires that color has been set.
  // If opponent_test is true, check not only whether the value is valid but
  // also whether it useful, that is whether there is a pawn which could hit
  // the moved pawn.
  bool IsEnPassantValid(EnPassant ep, bool opponent_test) const;

  // Return true if set_color(), set_ep() and set_castling() have been called
  bool HaveData() const;

  // Return true if all values have been set and appear to be complete/legal.
  // This is essentially a shortcut for the above validity check functions.
  bool LegalValues() const {
    return (HaveData() && HaveKings() && IsEnPassantValid(ep_, false) &&
      IsCastlingValid(castling_));
  }

  // Add all valid moves. If moves is nullptr, only return value is produced.
  // The return value is true if there is at least one valid move.
  bool Generator(MoveList *moves) const;

  // Do the move. The pointer must be kept until corresponding PopMove()
  void PushMove(const Move *my_move);

  // Undo the last pushed move.
  const Move *PopMove();

  // Is moving party in check?
  bool IsInCheck() const {
    return IsThreatened(kings_[color_]);
  }

  // Is moving party checkmate? This takes quite a while...
  bool IsCheckMate() const {
    return (IsInCheck() && !Generator(nullptr));
  }

  Castling get_castling() const {
    ASSERT(castling_ < kUnknownCastling);
    return castling_;
  }

  EnPassant get_ep_() const {
    ASSERT(ep_ != kUnknownEnPassant);
    return ep_;
  }

  Figure get_color() const {
    ASSERT(have_color());
    return color_;
  }

  Figure GetFigure(Pos pos) const {
    ASSERT((pos >= kFieldStart) && (pos < kFieldEnd) &&
      (field_[pos] != kNoFigure));
    return field_[pos];
  }

  Figure operator[](Pos pos) const {
    return GetFigure(pos);
  }

  const PosList& GetPosList(Figure color) const {
    ASSERT(UncoloredFigure(color) == kEmpty);
    return pos_lists_[color];
  }

  // Would piece of color threatened at pos?
  bool IsThreatened(Pos pos, Figure color) const;

  // Would piece of moving color be threatened at pos?
  bool IsThreatened(Pos pos) const {
    return IsThreatened(pos, color_);
  }

  // Is color in check?
  bool IsInCheck(Figure color) const {
    ASSERT(UncoloredFigure(color) == kEmpty);
    return IsThreatened(kings_[color], color);
  }

  Pos LongAddDelta(Pos pos, PosDelta delta) const {
    do {
      pos = AddDelta(pos, delta);
    } while (field_[pos] == kEmpty);
    return pos;
  }

  void clear();

  // After the following, no PopMove() is necessary/possible anymore.
  void ClearStack() {
    move_stack_.clear();
  }

  const MoveStack& get_move_stack() const {
    return move_stack_;
  }

  Field() {
    ClearField();
  }

 private:
  typedef PosList::iterator Pointer;

  // Might leave invalid data
  void ClearField();

  // in_check must be 1/-1/0 if king is in check/not in check/unknown
  // pos is position of the king, dir is +1/-1 for short/long castling.
  // Return is position of rook or kNpos if castling is not valid.
  Pos CastlingRook(int *in_check, Pos pos, const PosDelta dir) const;

  // Return true if move of single figure does not leave moving party in check
  bool IsValidMove(Pos from, Pos to) const;

  // Generate moves of long moving figure.
  // Return true if moves is nullptr and move could be generated
  bool GenerateLong(MoveList *moves, Pos from, PosDelta dir) const;

  // Generate moves of short moving figure.
  // Return true if moves is nullptr and move could be generated
  bool GenerateShort(MoveList *moves, Pos from, PosDelta dir) const;

  // Generate moves of white pawn.
  // Return true if moves is nullptr and move could be generated
  bool GenerateWhitePawn(MoveList *moves, Pos from) const;

  // Generate moves of black pawn.
  // Return true if moves is nullptr and move could be generated
  bool GenerateBlackPawn(MoveList *moves, Pos from) const;

  // Only used for debugging: Check whether pos_lists_ are reasonable
  bool ValidatePosLists() const;

  static void GenerateTransform(MoveList *moves, Pos from, Pos to);

  // mutable, because functions like generator() modify it temporarily:
  mutable Figure field_[kFieldSize];
  Pointer refs_[kFieldSize];  // pointers to pos_lists
  Figure color_;
  EnPassant ep_;
  Castling castling_;
  PosList pos_lists_[kIndexMax + 1];
  Pos kings_[kIndexMax + 1];
  MoveStack move_stack_;
};

}  // namespace chess

#endif  // SRC_CHESS_H_
