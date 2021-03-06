// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
// SPDX-License-Identifier: GPL-2.0-only
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef CHESSPROBLEM_CHESS_H_
#define CHESSPROBLEM_CHESS_H_ 1

#include <config.h>

#include <cassert>

#include <array>
#include <deque>
#include <list>
#include <string>
#include <utility>  // move
#include <vector>

#include "chessproblem/m_attribute.h"

namespace chess {
class Field;

// Use the functions from Field to convert intto/from coordinate form.
// Google style does not recommend to use unsigned, but it is the only way to
// get a minimal compile time check...
typedef unsigned int Pos;
typedef signed char PosDelta;  // Differ from Pos to get a compile time check

typedef unsigned char Figure;
constexpr static const Figure
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
constexpr const std::array<Pos, 2>::size_type Color2Index(const Figure color) {
  return static_cast<std::array<Pos, 2>::size_type>(color);
}

constexpr static const Figure
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

constexpr static const Figure FigureValue(const char figure) {
  return ((figure == 'N') ? kKnight :
         ((figure == 'B') ? kBishop :
         ((figure == 'R') ? kRook :
         ((figure == 'Q') ? kQueen :
         ((figure == 'K') ? kKing :
         ((figure == 'P') ? kPawn : kNoFigure))))));
}

constexpr const Pos AddDelta(const Pos pos, const PosDelta delta) {
  return static_cast<const Pos>(static_cast<const int>(pos) +
    static_cast<const int>(delta));
}

// EnPassant is either the EnPassant position (skipped field)
// or kNoEndPassant if none allowed.
typedef Pos EnPassant;
constexpr static const EnPassant
  kNoEnPassant = 0,
  kUnknownEnPassant = 1;

typedef std::vector<EnPassant> EnPassantList;

// This is a bitfield:
typedef unsigned char Castling;
constexpr static const Castling
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

constexpr static const Castling
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
  Move(MoveType move_type, Pos from, Pos to)
    : move_type_(move_type), from_(from), to_(to) {
  }

  // Append a human readable form of the move
  void Append(std::string *res, const Field &chess_field) const;

  void Append(std::string *res, Figure from_figure, Figure to_figure) const;

  // This is a poor man's version of Append (no figure name is contained)
  void AppendPoorMan(std::string *res) const {
    Append(res, kPawn, kPawn);
  }

  // AppendPoorMan with the natural name, but warn if used
  ATTRIBUTE_DEPRECATED("Move::Append(res) does not add the figure name")
      void Append(std::string *res) const {
    AppendPoorMan(res);
  }

  // Return a human readable form of the move
  ATTRIBUTE_NODISCARD std::string str(const Field &chess_field) const {
    std::string r;
    Append(&r, chess_field);
    return r;
  }

  ATTRIBUTE_NODISCARD std::string str(Figure from_figure, Figure to_figure)
      const {
    std::string r;
    Append(&r, from_figure, to_figure);
    return r;
  }

  // This is a poor man's version of str (no figure name is contained)
  ATTRIBUTE_NODISCARD std::string strPoorMan() const {
    return str(kPawn, kPawn);
  }

  // strPoorMan with the natural name, but warn if used
  ATTRIBUTE_DEPRECATED("Move::str() does not add the figure name")
      ATTRIBUTE_NODISCARD std::string str() const {
    return strPoorMan();
  }

  // Convenience wrapper for str()
  ATTRIBUTE_DEPRECATED("Move::operator string() does not add the figure name")
      operator std::string() {
    return strPoorMan();
  }
};

ATTRIBUTE_DEPRECATED("Output of Move does not add the figure name")
inline static std::ostream& operator<<(std::ostream& os, const Move& m);
inline static std::ostream& operator<<(std::ostream& os, const Move& m) {
  os << m.strPoorMan();
  return os;
}

class MoveList : public std::vector<Move> {
 public:
  // Append a human readable form of the list
  void Append(std::string *res, const Field &chess_field) const;

  // This is a poor man's version of Append (no figure names are contained)
  void AppendPoorMan(std::string *res) const;

  // AppendPoorMan with the natural name, but warn if used
  ATTRIBUTE_DEPRECATED("MoveList::Append(res) does not add the figure names")
      void Append(std::string *res) const {
      AppendPoorMan(res);
  }

  ATTRIBUTE_NODISCARD std::string str(const Field &chess_field) const {
    std::string r;
    Append(&r, chess_field);
    return r;
  }

  // This is a poor man's version of str (no figure names are contained)
  ATTRIBUTE_NODISCARD std::string strPoorMan() const {
    std::string r;
    AppendPoorMan(&r);
    return r;
  }

  // strPoorMan with the natural name, but warn if used
  ATTRIBUTE_DEPRECATED("MoveList::str() does not add the figure names")
  ATTRIBUTE_NODISCARD std::string str() const {
    return strPoorMan();
  }

  // Convenience wrapper for str()
  ATTRIBUTE_DEPRECATED("MoveList::operator string() does not add the "
      "figure names") operator std::string() {
    return strPoorMan();
  }
};

ATTRIBUTE_DEPRECATED("Output of MoveList does not add the figure names")
inline static std::ostream& operator<<(std::ostream& os, const MoveList& l);
inline static std::ostream& operator<<(std::ostream& os, const MoveList& l) {
  os << l.strPoorMan();
  return os;
}

class MoveStore {
 public:
  const Move *move_;
  EnPassant ep_;
  Castling castling_;
  Figure from_figure_, to_figure_;
  MoveStore(const Move *m, EnPassant ep, Castling c,
      Figure from_figure, Figure to_figure)
    : move_(m), ep_(ep), castling_(c), from_figure_(from_figure),
    to_figure_(to_figure) {
  }

  // append a human readable form of the move
  void Append(std::string *res) const {
    move_->Append(res, from_figure_, to_figure_);
  }

  // return a human readable form of the move
  ATTRIBUTE_NODISCARD std::string str() const {
    return move_->str(from_figure_, to_figure_);
  }

  // Convenience wrapper for str()
  ATTRIBUTE_NODISCARD operator std::string() const {
    return str();
  }
};

inline static std::ostream& operator<<(std::ostream& os, const MoveStore& m);
inline static std::ostream& operator<<(std::ostream& os, const MoveStore& m) {
  os << m.str();
  return os;
}


class MoveStack : public std::deque<MoveStore> {
 public:
  // append a human readable form of the stack
  void Append(std::string *res) const;

  // return a human readable form of the move
  ATTRIBUTE_NODISCARD std::string str() const {
    std::string r;
    Append(&r);
    return r;
  }

  // Convenience wrapper for str()
  operator std::string() const {
    return str();
  }
};

inline static std::ostream& operator<<(std::ostream& os, const MoveStack& s);
inline static std::ostream& operator<<(std::ostream& os, const MoveStack& s) {
  os << s.str();
  return os;
}

/*
Field is the main class to maintain the chess rules.

All coordinates are in the form "Pos".
You can call CalcPos() or LetterNumber() (or Append()/str())
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
  constexpr static const Pos
    kColumns = 8,
    kRows = 8;

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

  constexpr static const PosDelta
    kLeft = -1,
    kRight = 1,
    kUp = kColumns + 2,
    kDown = -kUp,
    kUpRight = kUp + kRight,
    kDownRight = kDown + kRight,
    kUpLeft = kUp + kLeft,
    kDownLeft = kDown + kLeft;

  constexpr static const Pos
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

  constexpr static const PosDelta
    kWhitePawnHit1 = kUpLeft,
    kWhitePawnHit2 = kUpRight,
    kWhitePawnMove = kUp,
    kBlackPawnHit1 = kDownLeft,
    kBlackPawnHit2 = kDownRight,
    kBlackPawnMove = kDown;

  static const PosDelta knight_deltas[8];
  static const PosDelta bishop_deltas[4];
  static const PosDelta rook_deltas[4];
  static const PosDelta king_deltas[8];
  static const PosDelta white_pawn_hit_deltas[2];
  static const PosDelta black_pawn_hit_deltas[2];

  // For illegal arguments, the value kFieldEnd is returned
  ATTRIBUTE_CONST static Pos CalcPos(char letter, char number);

  // For illegal arguments, the value kFieldEnd is returned
  ATTRIBUTE_PURE static Pos CalcPos(const char *letter_number);

  // For illegal arguments, the value kFieldEnd is returned
  ATTRIBUTE_PURE static Pos CalcPos(const std::string &letter_number);

  // Append a printable form of the field
  void Append(std::string *result) const;

  // return a printable form of the field
  ATTRIBUTE_NODISCARD std::string str() const {
    std::string r;
    Append(&r);
    return r;
  }

  // Convenience wrapper for str()
  operator std::string() const {
    return str();
  }

  static void LetterNumber(char *letter, char *number, Pos pos);

  // Append position name in a human readable form
  static void Append(std::string *letter_number, Pos pos);

  ATTRIBUTE_NODISCARD static std::string str(Pos pos) {
    std::string result;
    Append(&result, pos);
    return result;
  }

  // Append Move is appended in a human readable form
  void Append(std::string *result, const Move& m) const {
    m.Append(result, *this);
  }

  // Return Move in a human readable form
  ATTRIBUTE_NODISCARD std::string str(const Move& m) const {
    return m.str(*this);
  }

  // Append MoveList in a human readable form
  void Append(std::string *result, const MoveList& move_list) const {
    move_list.Append(result, *this);
  }

  // Return MoveList in a human readable form
  ATTRIBUTE_NODISCARD std::string str(const MoveList& move_list) const {
    return move_list.str(*this);
  }

  void PlaceFigure(Figure figure, Pos pos);

  // There must be a real figure to remove
  void RemoveFigure(Pos pos);

  // There must be a real figure to move
  void MoveFigure(Pos from, Pos to);

  // Add all possible ep values (except kNoEnPassand) for the position.
  // Requires that color has been set.
  ATTRIBUTE_NONNULL_ void CalcEnPassant(EnPassantList *ep_values) const;

  // Return all those castling value for the position wich are set in c.
  // Thus, CalcCastling(kAllCastling) returns all possible castling values.
  ATTRIBUTE_PURE Castling CalcCastling(Castling c) const;

  void set_color(Figure color) {
    assert(IsColor(color));
    color_ = FigureColor(color);
  }

  // Return whether set_color was already called
  ATTRIBUTE_NODISCARD bool have_color() const {
    return (IsColor(color_));
  }

  void set_ep(EnPassant ep) {
    assert(have_color());
    assert((ep == kNoEnPassant) || ((color_ == kWhite) ?
      ((ep >= kStartRow6) && (ep < kEndRow6)) :
      ((ep >= kStartRow3) && (ep < kEndRow3))));
    ep_ = ep;
  }

  void set_castling(Castling c) {
    assert(c < kUnknownCastling);
    castling_ = c;
  }

  // Return true if (at least) one black and white king are on the board
  ATTRIBUTE_NODISCARD bool HaveKings() const;

  // Return true if the castling value makes sense for the position
  ATTRIBUTE_NODISCARD bool IsCastlingValid(Castling c) const {
    return (CalcCastling(c) == c);
  }

  // Return true if the ep value is valid (i.e. looks like a double move).
  // Requires that color has been set.
  // If opponent_test is true, check not only whether the value is valid but
  // also whether it useful, that is whether there is a pawn which could hit
  // the moved pawn.
  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE  bool IsEnPassantValid(EnPassant ep,
      bool opponent_test) const;

  // Return true if set_color(), set_ep() and set_castling() have been called
  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE bool HaveData() const;

  // Return true if all values have been set and appear to be complete/legal.
  // This is essentially a shortcut for the above validity check functions.
  ATTRIBUTE_NODISCARD bool LegalValues() const {
    return (HaveData() && HaveKings() && IsEnPassantValid(ep_, false) &&
      IsCastlingValid(castling_));
  }

  // Return true if internal state is legal. This is mainly for debugging.
  // Calling this is time consuming and should not be used in any loop.
  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE bool LegalState() const;

  // Add all valid moves. If moves is nullptr, only return value is produced.
  // The return value is true if there is at least one valid move.
  bool Generator(MoveList *moves) const;

  // Do the move. The pointer must be kept until corresponding PopMove()
  ATTRIBUTE_NONNULL_ void PushMove(const Move *my_move);

  // Undo the last pushed move.
  const Move *PopMove();

  // Is moving party in check?
  bool IsInCheck() const {
    return IsThreatened(kings_[color_]);
  }

  // Is moving party checkmate? This takes quite a while...
  ATTRIBUTE_NODISCARD bool IsCheckMate() const {
    return (IsInCheck() && !Generator(nullptr));
  }

  ATTRIBUTE_NODISCARD Castling get_castling() const {
    assert(castling_ < kUnknownCastling);
    return castling_;
  }

  ATTRIBUTE_NODISCARD EnPassant get_ep_() const {
    assert(ep_ != kUnknownEnPassant);
    return ep_;
  }

  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE Figure get_color() const {
    assert(have_color());
    return color_;
  }

  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE Figure GetFigure(Pos pos) const {
    assert((pos >= kFieldStart) && (pos < kFieldEnd) &&
      (field_[pos] != kNoFigure));
    return field_[pos];
  }

  ATTRIBUTE_NODISCARD Figure operator[](Pos pos) const {
    return GetFigure(pos);
  }

  ATTRIBUTE_NODISCARD const PosList& GetPosList(Figure color) const {
    assert(UncoloredFigure(color) == kEmpty);
    return pos_lists_[color];
  }

  // Would piece of color threatened at pos?
  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE bool IsThreatened(Pos pos, Figure color)
      const;

  // Would piece of moving color be threatened at pos?
  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE bool IsThreatened(Pos pos) const {
    return IsThreatened(pos, color_);
  }

  // Is color in check?
  ATTRIBUTE_NODISCARD bool IsInCheck(Figure color) const {
    assert(UncoloredFigure(color) == kEmpty);
    return IsThreatened(kings_[color], color);
  }

  ATTRIBUTE_NODISCARD ATTRIBUTE_PURE  Pos LongAddDelta(Pos pos, PosDelta delta)
      const {
    do {
      pos = AddDelta(pos, delta);
    } while (field_[pos] == kEmpty);
    return pos;
  }

  // After the following, no PopMove() is necessary/possible anymore.
  void ClearStack() {
    move_stack_.clear();
  }

  ATTRIBUTE_NODISCARD const MoveStack& get_move_stack() const {
    return move_stack_;
  }

  void clear();

  void assign(const Field& f) noexcept;

  void assign(Field&& f) noexcept;

  Field() {
    ClearField();
  }

  // No destructor is needed, but we must take care when copying/moving,
  // because we need to fixup refs_ to point to the copied/moved iterators.

  Field(const Field& f) noexcept {
    assign(f);
  }

  Field(Field&& f) noexcept {
    assign(std::move(f));
  }

  Field& operator=(const Field& f) noexcept {
    assign(f);
    return *this;
  }

  Field& operator=(Field&& f) noexcept {
    assign(std::move(f));
    return *this;
  }

 private:
  typedef PosList::iterator Pointer;
  typedef std::array<PosList, kIndexMax + 1> PosLists;
  typedef std::array<Pos, kIndexMax + 1> KingsPos;

  // Needed only inline for the copy/move assignment operator
  void RecreateRefs() noexcept {
    for (auto& l : pos_lists_) {
      for (PosList::iterator it(l.begin()); it != l.end(); ++it) {
        refs_[*it] = it;
      }
    }
  }

  // Might leave invalid data
  void ClearField();

  // in_check must be 1/-1/0 if king is in check/not in check/unknown
  // pos is position of the king, dir is +1/-1 for short/long castling.
  // Return is position of rook or kNpos if castling is not valid.
  ATTRIBUTE_NONNULL_ Pos CastlingRook(int *in_check, Pos pos,
      const PosDelta dir) const;

  // Return true if move of single figure does not leave moving party in check
  ATTRIBUTE_NODISCARD bool IsValidMove(Pos from, Pos to) const;

  // Generate moves of long moving figure.
  // Return true if moves is nullptr and move could be generated
  ATTRIBUTE_NODISCARD bool GenerateLong(MoveList *moves, Pos from,
      PosDelta dir) const;

  // Generate moves of short moving figure.
  // Return true if moves is nullptr and move could be generated
  bool GenerateShort(MoveList *moves, Pos from, PosDelta dir) const;

  // Generate moves of white pawn.
  // Return true if moves is nullptr and move could be generated
  bool GenerateWhitePawn(MoveList *moves, Pos from) const;

  // Generate moves of black pawn.
  // Return true if moves is nullptr and move could be generated
  bool GenerateBlackPawn(MoveList *moves, Pos from) const;

  static void GenerateTransform(MoveList *moves, Pos from, Pos to);

  // mutable, because functions like generator() modify it temporarily:
  mutable std::array<Figure, kFieldSize> field_;
  std::array<Pointer, kFieldSize> refs_;  // pointers to pos_lists_
  Figure color_;
  EnPassant ep_;
  Castling castling_;
  PosLists pos_lists_;
  KingsPos kings_;
  MoveStack move_stack_;
};

inline static std::ostream& operator<<(std::ostream& os, const Field& f);
inline static std::ostream& operator<<(std::ostream& os, const Field& f) {
  os << f.str();
  return os;
}

// A rather unexpensive convience wrapper to ensure PopMove() is not missed:
//
// When the object is initialized, the move is pushed.
// (You may also initialize it without actually pushing).
// When the object goes out of scope the move is popped.
// It is the user's responsibility to ensure that the field is available in
// the latter moment!
// The object is non-copyable/movable (though you might create a copy by using
// get() to read the original field. But this would mean that two moves
// are popped when both objects go out of scope).
// For special usage it is also possible to move the underlying field with
// set(field), but field must not be nullptr.
// When you want a possibility to "disarm" the Guard where field may be nullptr
// use the slightly more expensive subsequent class.

class push_guard {
 public:
  ATTRIBUTE_NONNULL_ push_guard(Field *field, const Move *my_move)
    : field_(field) {
    field->PushMove(my_move);
  }

  ATTRIBUTE_NONNULL_ explicit push_guard(Field *field)
    : field_(field) {
  }

  ATTRIBUTE_NODISCARD Field *get() const {
    return field_;
  }

  ATTRIBUTE_NONNULL_ void set(Field *field) {
    field_ = field;
  }

  ~push_guard() {
    field_->PopMove();
  }

  push_guard(const push_guard& g) = delete;
  push_guard& operator=(const push_guard& g) = delete;
  push_guard(push_guard&& g) = delete;
  push_guard& operator=(push_guard&& g) = delete;

 private:
  Field *field_;
};

// This is a more expensive disarmable variant of the previous class.
// It is non-copyable but movable.
// For this class field can be a nullptr, and in this case no PopMove is called
// when the object leaves the scope.
// The pointer can be set explicitly or implicitly using reset() and release()
// analogously to std::unique_ptr.
// There is also a default constructor (without arguments) which produces the
// object in the disarmed state. You can arm it later by calling
// set(field) or (slower) reset(field) (possibly followed by PushMove(move))
// or by PushMove(field, move) (to do both simultaneously).

class unique_push {
 public:
  unique_push()
    : field_(nullptr) {
  }

  ATTRIBUTE_NONNULL_ unique_push(Field *field, const Move *my_move)
    : field_(field) {
    field->PushMove(my_move);
  }

  explicit unique_push(Field *field)
    : field_(field) {
  }

  ATTRIBUTE_NONNULL_ void PushMove(Field *field, const Move *my_move) {
    field_ = field;
    field->PushMove(my_move);
  }

  ATTRIBUTE_NONNULL_ void PushMove(const Move *my_move) {
    assert(field_ != nullptr);
    field_->PushMove(my_move);
  }

  ATTRIBUTE_NODISCARD Field *get() const {
    return field_;
  }

  void set(Field *field) {
    field_ = field;
  }

  Field *release() noexcept {
    Field *ret(field_);
    field_ = nullptr;
    return ret;
  }

  void reset(Field *field) {
    if (field_ != nullptr) {
      field_->PopMove();
    }
    field_ = field;
  }

  ~unique_push() {
    if (field_ != nullptr) {
      field_->PopMove();
    }
  }

  unique_push(const unique_push& g) = delete;
  unique_push& operator=(const unique_push& g) = delete;

  unique_push(unique_push&& g) noexcept {
    field_ = g.release();
  }

  unique_push& operator=(unique_push&& g) noexcept {
    field_ = g.release();
    return *this;
  }

 private:
  Field *field_;
};

}  // namespace chess

#endif  // CHESSPROBLEM_CHESS_H_
