// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
// SPDX-License-Identifier: GPL-2.0-only
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "chessproblem/chess.h"
#include <config.h>

#include <cassert>

#include <string>
#include <utility>  // move

#include "chessproblem/m_attribute.h"
#include "chessproblem/m_likely.h"

using std::string;

namespace chess {

const char *uncolored_figure_name[] = {
  "_", "?",
  "", "", "N", "N", "B", "B", "R", "R", "Q", "Q", "K", "K"
};

const char *colored_figure_name[] = {
  "__", "??",
  "wP", "bP", "wN", "bN", "wB", "bB", "wR", "bR", "wQ", "bQ", "wK", "bK"
};

const char *color_name[] = {
  "white", "black"
};

const Pos
  Field::kColumns,
  Field::kRows,
  Field::kNpos,
  Field::kFieldSize,
  Field::kFieldStart,
  Field::kLastRow,
  Field::kFieldEnd,
  Field::kEndRow2,
  Field::kStartRow7,
  Field::kStartRow3,
  Field::kEndRow3,
  Field::kStartRow6,
  Field::kEndRow6,
  Field::kPosWhiteKing,
  Field::kPosWhiteLongRook,
  Field::kPosWhiteShortRook,
  Field::kPosBlackKing,
  Field::kPosBlackLongRook,
  Field::kPosBlackShortRook;

const PosDelta
  Field::kLeft,
  Field::kRight,
  Field::kUp,
  Field::kDown,
  Field::kUpRight,
  Field::kDownRight,
  Field::kUpLeft,
  Field::kDownLeft,
  Field::kWhitePawnHit1,
  Field::kWhitePawnHit2,
  Field::kWhitePawnMove,
  Field::kBlackPawnHit1,
  Field::kBlackPawnHit2,
  Field::kBlackPawnMove;

const PosDelta Field::bishop_deltas[] = {
  kUpLeft, kUpRight, kDownLeft, kDownRight
};

const PosDelta Field::rook_deltas[] = {
  kUp, kDown, kLeft, kRight
};

const PosDelta Field::king_deltas[] = {
  kUp, kDown, kLeft, kRight,
  kUpLeft, kUpRight, kDownLeft, kDownRight
};

const PosDelta Field::knight_deltas[] = {
  kUp + kUpLeft, kUp + kUpRight, kUpLeft + kLeft, kUpRight + kRight,
  kDown + kDownLeft, kDown + kDownRight, kDownLeft + kLeft, kDownRight + kRight
};

const PosDelta Field::white_pawn_hit_deltas[] = {
  kWhitePawnHit1,
  kWhitePawnHit2
};

const PosDelta Field::black_pawn_hit_deltas[] = {
  kBlackPawnHit1,
  kBlackPawnHit2
};

void Move::Append(string *res, Figure from_figure, Figure to_figure) const {
  switch (move_type_) {
    case Move::kShortCastling:
      res->append("0-0");
      return;
    case Move::kLongCastling:
      res->append("0-0-0");
      return;
    case Move::kEnPassant:
      to_figure = kEmpty + 1;  // Display a hit
    default:
      break;
  }
  res->append(uncolored_figure_name[from_figure]);
  Field::Append(res, from_);
  res->append(1, (to_figure == kEmpty) ? '-' : '*');
  // Use this if you want syntax Ra1*Qa8:
  // res->append(uncolored_figure_name[to_figure]);
  Field::Append(res, to_);
  switch (move_type_) {
    case Move::kEnPassant:
      res->append("ep");
      break;
    case Move::kQueen:
      res->append("=Q");
      break;
    case Move::kKnight:
      res->append("=N");
      break;
    case Move::kRook:
      res->append("=R");
      break;
    case Move::kBishop:
      res->append("=B");
      break;
    default:
      break;
  }
}

void Move::Append(std::string *res, const Field &chess_field) const {
  Append(res, chess_field.field_[from_], chess_field.field_[to_]);
}

void MoveList::Append(string *res, const Field &chess_field) const {
  for (auto m : *this) {
    if (!res->empty()) {
      res->append(1, ' ');
    }
    m.Append(res, chess_field);
  }
}

void MoveList::AppendPoorMan(string *res) const {
  for (auto m : *this) {
    if (!res->empty()) {
      res->append(1, ' ');
    }
    m.AppendPoorMan(res);
  }
}

void MoveStack::Append(string *res) const {
  for (auto m : *this) {
    if (!res->empty()) {
      res->append(1, ' ');
    }
    m.Append(res);
  }
}

void Field::Append(std::string *result) const {
  string columns("  ");
  for (Pos pos(0); pos < kColumns; ++pos) {
    columns.append(1, ' ');
    columns.append(1, 'a' + static_cast<char>(pos));
  }
  columns.append("  \n");
  result->append(columns);
  PosDelta row(kRows * kUp);
  while (row != 0) {
    row += kDown;
    Pos pos(AddDelta(kFieldStart, row));
    char letter, number;
    LetterNumber(&letter, &number, pos);
    result->append(1, number);
    result->append(1, ' ');
    for (Pos column(0); column < kColumns; ++column) {
      result->append(colored_figure_name[field_[pos++]]);
    }
    result->append(1, ' ');
    result->append(1, number);
    result->append(1, '\n');
  }
  result->append(columns);
}

void Field::LetterNumber(char *letter, char *number, Pos pos) {
  if (UNLIKELY((pos < kFieldStart) && (pos >= kFieldEnd))) {
    *letter = *number = '?';
    return;
  }
  pos -= kFieldStart;
  Pos num(pos / (kColumns + 2));
  pos -= num * (kColumns + 2);
  if (UNLIKELY(pos >= kColumns)) {
    *letter = *number = '?';
    return;
  }
  *letter = (static_cast<char>(pos) + 'a');
  *number = (static_cast<char>(num) + '1');
}

void Field::Append(string *letter_number, Pos pos) {
  char result[3];
  LetterNumber(&(result[0]), &(result[1]), pos);
  result[2] = '\0';
  letter_number->append(result);
}

Pos Field::CalcPos(char letter, char number) {
  letter -= 'a';
  if (UNLIKELY((letter < 0) || (letter > kRows))) {
    return kFieldEnd;
  }
  number -= '1';
  if (UNLIKELY((number < 0) || (number > kColumns))) {
    return kFieldEnd;
  }
  return kFieldStart + (number * (kColumns + 2)) + letter;
}

Pos Field::CalcPos(const char *letter_number) {
  if (UNLIKELY(*letter_number == '\0')) {
    return kFieldEnd;
  }
  return CalcPos(letter_number[0], letter_number[1]);
}

Pos Field::CalcPos(const string& letter_number) {
  if (UNLIKELY(letter_number.size() < 2)) {
    return kFieldEnd;
  }
  return CalcPos(letter_number[0], letter_number[1]);
}

void Field::ClearField() {
  color_ = kWhiteKing;
  ep_ = kUnknownEnPassant;
  castling_ = kUnknownCastling;
  kings_[Color2Index(kWhite)] = kings_[Color2Index(kBlack)] = kNpos;
  Pos i(0);
  while (i < kFieldStart) {
    field_[i++] = kNoFigure;
  }
  while (i < kFieldEnd) {
    for (Pos j(kColumns); j > 0; --j) {
      field_[i++] = kEmpty;
    }
    field_[i++] = kNoFigure;
    field_[i++] = kNoFigure;
  }
  while (i < kFieldSize) {
    field_[i++] = kNoFigure;
  }
}

void Field::clear() {
  ClearField();
  pos_lists_[Color2Index(kWhite)].clear();
  pos_lists_[Color2Index(kBlack)].clear();
  ClearStack();
}

void Field::assign(const Field& f) noexcept {
  field_ = f.field_;
  color_ = f.color_;
  ep_ = f.ep_;
  castling_ = f.castling_;
  pos_lists_ = f.pos_lists_;
  kings_ = f.kings_;
  move_stack_ = f.move_stack_;
  RecreateRefs();
}

void Field::assign(Field&& f) noexcept {
  field_ = std::move(f.field_);
  color_ = std::move(f.color_);
  ep_ = std::move(f.ep_);
  castling_ = std::move(f.castling_);
  pos_lists_ = std::move(f.pos_lists_);
  kings_ = std::move(f.kings_);
  move_stack_ = std::move(f.move_stack_);
  RecreateRefs();
}

void Field::PlaceFigure(Figure figure, Pos pos) {
  assert((pos >= kFieldStart) && (pos < kFieldEnd));
  auto i(FigureColor(figure));
  if (UNLIKELY(UncoloredFigure(figure) == kKing)) {
    kings_[Color2Index(i)] = pos;
  }
  PosList& pos_list = pos_lists_[i];
  pos_list.push_front(pos);
  Figure& field = field_[pos];
  Pointer& ref = refs_[pos];
  assert(field != kNoFigure);
  if (UNLIKELY(field != kEmpty)) {
    pos_lists_[Color2Index(FigureColor(field))].erase(ref);
  }
  field = figure;
  ref = pos_list.begin();
}

void Field::RemoveFigure(Pos pos) {
  assert((pos >= kFieldStart) && (pos < kFieldEnd));
  Figure& field = field_[pos];
  assert((field != kEmpty) && (field != kNoFigure));
  pos_lists_[Color2Index(FigureColor(field))].erase(refs_[pos]);
  field = kEmpty;
}

void Field::MoveFigure(Pos from, Pos to) {
  assert((from >= kFieldStart) && (from < kFieldEnd));
  assert((to >= kFieldStart) && (to < kFieldEnd));
  Figure& from_field = field_[from];
  Figure figure(from_field);
  assert((figure != kEmpty) && (figure != kNoFigure));
  if (UNLIKELY(UncoloredFigure(figure) == kKing)) {
    kings_[Color2Index(FigureColor(figure))] = to;
  }
  Figure& to_field = field_[to];
  Pointer& to_ref = refs_[to];
  assert(to_field != kNoFigure);
  if (UNLIKELY(to_field != kEmpty)) {
    pos_lists_[Color2Index(FigureColor(to_field))].erase(to_ref);
  }
  from_field = kEmpty;
  to_field = figure;
  Pointer from_ref(refs_[from]);
  *from_ref = to;
  to_ref = from_ref;
}

bool Field::LegalState() const {
  for (Figure color(kWhite); ; color = kBlack) {
    const PosList& l = pos_lists_[Color2Index(color)];
    for (PosList::const_iterator it(l.begin()); it != l.end(); ++it) {
      auto i = *it;
      Figure figure(field_[i]);
      if ((figure == kEmpty) || (figure == kNoFigure) ||
        (FigureColor(figure) != color)) {
        return false;
      }
      if (refs_[i] != it) {
        return false;
      }
    }
    if (color == kBlack) {
      break;
    }
  }
  PosList::size_type count[kIndexMax + 1];
  count[Color2Index(kWhite)] = count[Color2Index(kBlack)] = 0;
  for (Pos pos(kFieldStart); pos < kFieldEnd; ++pos) {
    Figure figure(field_[pos]);
    if ((figure != kEmpty) && (figure != kNoFigure)) {
      ++count[Color2Index(FigureColor(figure))];
    }
  }
  return (count[Color2Index(kWhite)] == pos_lists_[Color2Index(kWhite)].size())
      && (count[Color2Index(kBlack)] == pos_lists_[Color2Index(kBlack)].size());
}

void Field::CalcEnPassant(EnPassantList *ep_values) const {
  if (color_ == kWhite) {
    for (Pos pos(kStartRow6); pos < kEndRow6; ++pos) {
      if (IsEnPassantValid(pos, true)) {
        ep_values->push_back(pos);
      }
    }
  } else {
    for (Pos pos(kStartRow3); pos < kEndRow3; ++pos) {
      if (IsEnPassantValid(pos, true)) {
        ep_values->push_back(pos);
      }
    }
  }
}

bool Field::IsEnPassantValid(EnPassant ep, bool opponent_test) const {
  assert(UncoloredFigure(color_) == kEmpty);
  if (ep == kNoEnPassant) {
    return true;
  }
  if (color_ == kWhite) {
    if ((ep >= kStartRow6) && (ep < kEndRow6) &&
        (field_[AddDelta(ep, kBlackPawnMove)] == kBlackPawn) &&
        (field_[ep] == kEmpty) &&
        (field_[AddDelta(ep, kWhitePawnMove)] == kEmpty) && (
          (!opponent_test) ||
          (field_[AddDelta(ep, kBlackPawnHit1)] == kWhitePawn) ||
          (field_[AddDelta(ep, kBlackPawnHit2)] == kWhitePawn))) {
        return true;
    }
  } else if ((ep >= kStartRow3) && (ep < kEndRow3) &&
        (field_[AddDelta(ep, kWhitePawnMove)] == kWhitePawn) &&
        (field_[ep] == kEmpty) &&
        (field_[AddDelta(ep, kBlackPawnMove)] == kEmpty) && (
          (!opponent_test) ||
          (field_[AddDelta(ep, kWhitePawnHit1)] == kBlackPawn) ||
          (field_[AddDelta(ep, kWhitePawnHit2)] == kBlackPawn))) {
        return true;
  }
  return false;
}

Castling Field::CalcCastling(Castling c) const {
  if (HaveCastling(c, kWhiteCastling)) {
    if (field_[kPosWhiteKing] != kWhiteKing) {
      c = UnsetCastling(c, kNoWhiteCastling);
    } else {
      if (field_[kPosWhiteShortRook] != kWhiteRook) {
        c = UnsetCastling(c, kNoWhiteShortCastling);
      }
      if (field_[kPosWhiteLongRook] != kWhiteRook) {
        c = UnsetCastling(c, kNoWhiteLongCastling);
      }
    }
  }
  if (HaveCastling(c, kBlackCastling)) {
    if (field_[kPosBlackKing] != kBlackKing) {
      c = UnsetCastling(c, kNoBlackCastling);
    } else {
      if (field_[kPosBlackShortRook] != kBlackRook) {
        c = UnsetCastling(c, kNoBlackShortCastling);
      }
      if (field_[kPosBlackLongRook] != kBlackRook) {
        c = UnsetCastling(c, kNoBlackLongCastling);
      }
    }
  }
  return c;
}

bool Field::HaveData() const {
  return ((UncoloredFigure(color_) == kEmpty) &&
    (castling_ < kUnknownCastling) &&
    (ep_ != kUnknownEnPassant));
}

bool Field::HaveKings() const {
  return ((field_[kings_[Color2Index(kWhite)]] = kWhiteKing) &&
    (field_[kings_[Color2Index(kBlack)]] = kBlackKing));
}

void Field::PushMove(const Move *my_move) {
  assert(LegalValues());
  Castling castling(castling_);
  Figure color(color_);
  Pos from(my_move->from_), to(my_move->to_);
  move_stack_.emplace_back(my_move, ep_, castling, field_[from], field_[to]);
  ep_ = 0;
  Move::MoveType move_type(my_move->move_type_);
  switch (move_type) {
    case Move::kEnPassant:
      RemoveFigure(AddDelta(to,
        ((color == kWhite) ? kBlackPawnMove : kWhitePawnMove)));
      MoveFigure(from, to);
      break;
    case Move::kDouble:
      MoveFigure(from, to);
      ep_ = AddDelta(to,
        (color == kWhite) ? kBlackPawnMove : kWhitePawnMove);
      break;
    case Move::kQueen:
      MoveFigure(from, to);
      field_[to] = ColoredFigure(kQueen, color);
      break;
    case Move::kKnight:
      MoveFigure(from, to);
      field_[to] = ColoredFigure(kKnight, color);
      break;
    case Move::kRook:
      MoveFigure(from, to);
      field_[to] = ColoredFigure(kRook, color);
      break;
    case Move::kBishop:
      MoveFigure(from, to);
      field_[to] = ColoredFigure(kBishop, color);
      break;
    case Move::kShortCastling:
      MoveFigure(to, AddDelta(from, kRight));
      MoveFigure(from, AddDelta(from, kRight + kRight));
      castling_ = UnsetCastling(castling,
        (color == kWhite) ? kNoWhiteCastling : kNoBlackCastling);
      break;
    case Move::kLongCastling:
      MoveFigure(to, AddDelta(from, kLeft));
      MoveFigure(from, AddDelta(from, kLeft + kLeft));
      castling_ = UnsetCastling(castling,
        (color == kWhite) ? kNoWhiteCastling : kNoBlackCastling);
      break;
    default:
    // case Move::kNormal:
      switch (from) {
        case kPosWhiteShortRook:
          if (color == kWhite) {
            castling_ = UnsetCastling(castling, kNoWhiteShortCastling);
          }
          break;
        case kPosWhiteLongRook:
          if (color == kWhite) {
            castling_ = UnsetCastling(castling, kNoWhiteLongCastling);
          }
          break;
        case kPosWhiteKing:
          if (color == kWhite) {
            castling_ = UnsetCastling(castling, kNoWhiteCastling);
          }
        case kPosBlackShortRook:
          if (color != kWhite) {
            castling_ = UnsetCastling(castling, kNoBlackShortCastling);
          }
          break;
        case kPosBlackLongRook:
          if (color != kWhite) {
            castling_ = UnsetCastling(castling, kNoBlackLongCastling);
          }
          break;
        case kPosBlackKing:
          if (color != kWhite) {
            castling_ = UnsetCastling(castling, kNoBlackCastling);
          }
          break;
        default:
          break;
      }
      MoveFigure(from, to);
      break;
  }
  color_ = InvertColor(color);
}

const Move *Field::PopMove() {
  const MoveStore& save_move = move_stack_.back();
  const Move *my_move(save_move.move_);
  ep_ = save_move.ep_;
  castling_ = save_move.castling_;
  Figure to_figure(save_move.to_figure_);
  Pos from(my_move->from_), to(my_move->to_);
  Figure color(InvertColor(color_));
  Move::MoveType move_type(my_move->move_type_);
  switch (move_type) {
    case Move::kEnPassant:
      MoveFigure(to, from);
      if (color == kWhite) {
        PlaceFigure(kBlackPawn, AddDelta(to, kBlackPawnMove));
      } else {
        PlaceFigure(kWhitePawn, AddDelta(to, kWhitePawnMove));
      }
      break;
    case Move::kShortCastling:
      MoveFigure(AddDelta(from, kRight), to);
      MoveFigure(AddDelta(from, kRight + kRight), from);
      break;
    case Move::kLongCastling:
      MoveFigure(AddDelta(from, kLeft), to);
      MoveFigure(AddDelta(from, kLeft + kLeft), from);
      break;
    case Move::kQueen:
    case Move::kKnight:
    case Move::kRook:
    case Move::kBishop:
      field_[to] = ColoredFigure(kPawn, color);
      ATTRIBUTE_FALLTHROUGH
    default:
    // case Move::kNormal:
    // case Move::kDouble:
      MoveFigure(to, from);
      if (to_figure != kEmpty) {
        PlaceFigure(to_figure, to);
      }
      break;
  }
  color_ = color;
  move_stack_.pop_back();
  return my_move;
}

bool Field::IsThreatened(Pos pos, Figure color) const {
  Figure invert_color(InvertColor(color));
  Figure check_queen(ColoredFigure(kQueen, invert_color));
  Figure check_king(ColoredFigure(kKing, invert_color));
  Figure check_figure(ColoredFigure(kBishop, invert_color));
  for (auto curr_delta : bishop_deltas) {
    Pos destpos(LongAddDelta(pos, curr_delta));
    Figure figure(field_[destpos]);
    if ((figure == check_figure) || (figure == check_queen) ||
      ((figure == check_king) &&
        (destpos == AddDelta(pos, curr_delta)))) {
      return true;
    }
  }
  check_figure = ColoredFigure(kRook, invert_color);
  for (auto curr_delta : rook_deltas) {
    Pos destpos(LongAddDelta(pos, curr_delta));
    Figure figure(field_[destpos]);
    if ((figure == check_figure) || (figure == check_queen) ||
      ((figure == check_king) &&
        (destpos == AddDelta(pos, curr_delta)))) {
      return true;
    }
  }
  check_figure = ColoredFigure(kKnight, invert_color);
  for (auto curr_delta : knight_deltas) {
    Figure figure(field_[AddDelta(pos, curr_delta)]);
    if (figure == check_figure) {
      return true;
    }
  }
  if (color == kWhite) {
    if ((field_[AddDelta(pos, kWhitePawnHit1)] == kBlackPawn) ||
        (field_[AddDelta(pos, kWhitePawnHit2)] == kBlackPawn)) {
      return true;
    }
  } else {
    if ((field_[AddDelta(pos, kBlackPawnHit1)] == kWhitePawn) ||
        (field_[AddDelta(pos, kBlackPawnHit2)] == kWhitePawn)) {
      return true;
    }
  }
  return false;
}

Pos Field::CastlingRook(int *in_check, Pos pos, const PosDelta dir) const {
  if (*in_check > 0) {
    return kNpos;
  }
  // Rely on castling variable: assume that king and rook are correct.
  // However, there must not be any figure in between:
  Pos rook_pos(LongAddDelta(pos, dir));
  if (field_[AddDelta(rook_pos, dir)] != kNoFigure) {
    return kNpos;
  }
  Figure color(color_);
  // The king must not be threatened (in check) and neither the two next fields
  if (*in_check == 0) {
    if (IsThreatened(pos, color)) {
      *in_check = 1;
      return kNpos;
    }
    *in_check = -1;  // no need to test for in_check for opposite dir value
  }
  pos = AddDelta(pos, dir);
  if (IsThreatened(pos, color)) {
    return kNpos;
  }
  pos = AddDelta(pos, dir);
  if (IsThreatened(pos, color)) {
    return kNpos;
  }
  return rook_pos;
}

bool Field::IsValidMove(const Pos from, const Pos to) const {
  Figure& field_from = field_[from];
  Figure figure_from(field_from);
  Figure& field_to = field_[to];
  Figure figure_to(field_to);
  field_from = kEmpty;
  field_to = figure_from;
  Pos king_field(kings_[Color2Index(color_)]);
  if (king_field == from) {
    king_field = to;
  }
  bool result(!IsThreatened(king_field));
  field_from = figure_from;
  field_to = figure_to;
  return result;
}

bool Field::GenerateLong(MoveList *moves, Pos from, PosDelta dir) const {
  Figure color(color_);
  for (Pos to(AddDelta(from, dir)); ; to = AddDelta(to, dir)) {
    Figure figure(field_[to]);
    if ((figure == kNoFigure) ||
      ((figure != kEmpty) && FigureColor(figure) == color)) {
      return false;
    }
    if (LIKELY(IsValidMove(from, to))) {
      if (moves == nullptr) {
        return true;
      }
      moves->emplace_back(Move::kNormal, from, to);
    }
    if (figure != kEmpty) {
      return false;
    }
  }
  return false;
}

bool Field::GenerateShort(MoveList *moves, Pos from, PosDelta dir) const {
  Pos to(AddDelta(from, dir));
  Figure figure(field_[to]);
    if ((figure == kNoFigure) ||
      ((figure != kEmpty) && FigureColor(figure) == color_)) {
    return false;
  }
  if (LIKELY(IsValidMove(from, to))) {
    if (moves == nullptr) {
      return true;
    }
    moves->emplace_back(Move::kNormal, from, to);
  }
  return false;
}

void Field::GenerateTransform(MoveList *moves, Pos from, Pos to) {
  moves->emplace_back(Move::kQueen, from, to);
  moves->emplace_back(Move::kKnight, from, to);
  moves->emplace_back(Move::kRook, from, to);
  moves->emplace_back(Move::kBishop, from, to);
}

bool Field::GenerateWhitePawn(MoveList *moves, Pos from) const {
  Pos to(AddDelta(from, kWhitePawnMove));
  if (field_[to] == kEmpty) {
    if (LIKELY(IsValidMove(from, to))) {
      if (moves == nullptr) {
        return true;
      }
      if (LIKELY(from < kStartRow7)) {
        moves->emplace_back(Move::kNormal, from, to);
      } else {
        GenerateTransform(moves, from, to);
      }
      if (UNLIKELY(from <= kEndRow2)) {
        to = AddDelta(to, kWhitePawnMove);
        if ((field_[to] == kEmpty) && LIKELY(IsValidMove(from, to))) {
          moves->emplace_back(Move::kDouble, from, to);
        }
      }
    }
  }
  for (auto delta : white_pawn_hit_deltas) {
    to = AddDelta(from, delta);
    if (UNLIKELY(to == ep_)) {
      Figure &pawn = field_[AddDelta(to, kBlackPawnMove)];
      pawn = kEmpty;
      bool is_valid(IsValidMove(from, to));
      pawn = kBlackPawn;
      if (LIKELY(is_valid)) {
        if (moves == nullptr) {
          return true;
        }
        moves->emplace_back(Move::kEnPassant, from, to);
      }
    } else {
      Figure figure(field_[to]);
      if ((figure != kNoFigure)
        && (figure != kEmpty) && (FigureColor(figure) != kWhite)
        && LIKELY(IsValidMove(from, to))) {
        if (moves == nullptr) {
          return true;
        }
        if (LIKELY(from < kStartRow7)) {
          moves->emplace_back(Move::kNormal, from, to);
        } else {
          GenerateTransform(moves, from, to);
        }
      }
    }
  }
  return false;
}

bool Field::GenerateBlackPawn(MoveList *moves, Pos from) const {
  Pos to(AddDelta(from, kBlackPawnMove));
  if (field_[to] == kEmpty) {
    if (LIKELY(IsValidMove(from, to))) {
      if (moves == nullptr) {
        return true;
      }
      if (LIKELY(from > kEndRow2)) {
        moves->emplace_back(Move::kNormal, from, to);
      } else {
        GenerateTransform(moves, from, to);
      }
      if (UNLIKELY(from >= kStartRow7)) {
        to = AddDelta(to, kBlackPawnMove);
        if ((field_[to] == kEmpty) && LIKELY(IsValidMove(from, to))) {
          moves->emplace_back(Move::kDouble, from, to);
        }
      }
    }
  }
  for (auto delta : black_pawn_hit_deltas) {
    to = AddDelta(from, delta);
    if (UNLIKELY(to == ep_)) {
      Figure &pawn = field_[AddDelta(to, kWhitePawnMove)];
      pawn = kEmpty;
      bool is_valid(IsValidMove(from, to));
      pawn = kWhitePawn;
      if (LIKELY(is_valid)) {
        if (moves == nullptr) {
          return true;
        }
        moves->emplace_back(Move::kEnPassant, from, to);
      }
    } else {
      Figure figure(field_[to]);
      if ((figure != kNoFigure)
        && (figure != kEmpty) && (FigureColor(figure) == kWhite)
        && LIKELY(IsValidMove(from, to))) {
        if (moves == nullptr) {
          return true;
        }
        if (LIKELY(from > kEndRow2)) {
          moves->emplace_back(Move::kNormal, from, to);
        } else {
          GenerateTransform(moves, from, to);
        }
      }
    }
  }
  return false;
}

bool Field::Generator(MoveList *moves) const {
  assert(LegalValues());
  Figure color(color_);
  if (UNLIKELY(castling_ != kNoCastling)) {
    Castling castling((color_ == kWhite) ? castling_ :
      BlackToWhiteCastling(castling_));
    Pos king_pos(kings_[Color2Index(color_)]);
    int in_check(0);
    if (HaveCastling(castling, kWhiteShortCastling)) {
      Pos rook_pos(CastlingRook(&in_check, king_pos, 1));
      if (rook_pos != kNpos) {
        if (moves == nullptr) {
          return true;
        }
        moves->emplace_back(Move::kShortCastling, king_pos, rook_pos);
      }
    }
    if (HaveCastling(castling, kWhiteLongCastling)) {
      Pos rook_pos(CastlingRook(&in_check, king_pos, -1));
      if (rook_pos != kNpos) {
        if (moves == nullptr) {
          moves->emplace_back(Move::kLongCastling, king_pos, rook_pos);
        }
      }
    }
  }
  for (Pos from : pos_lists_[Color2Index(color)]) {
    Figure figure(field_[from]);
    switch (UncoloredFigure(figure)) {
      case kBishop:
        for (auto dir : bishop_deltas) {
          if (GenerateLong(moves, from, dir)) {
            return true;
          }
        }
        break;
      case kRook:
        for (auto dir : rook_deltas) {
          if (GenerateLong(moves, from, dir)) {
            return true;
          }
        }
        break;
      case kQueen:
        for (auto dir : king_deltas) {
          if (GenerateLong(moves, from, dir)) {
            return true;
          }
        }
        break;
      case kKing:
        for (auto dir : king_deltas) {
          if (GenerateShort(moves, from, dir)) {
            return true;
          }
        }
        break;
      case kKnight:
        for (auto dir : knight_deltas) {
          if (GenerateShort(moves, from, dir)) {
            return true;
          }
        }
        break;
      case kPawn:
        if (color == kWhite) {
          if (GenerateWhitePawn(moves, from)) {
            return true;
          }
        } else {
          if (GenerateBlackPawn(moves, from)) {
            return true;
          }
        }
        break;
      default:
        assert(false);
        break;
    }
  }
  if (moves == nullptr) {
    return false;
  }
  return !moves->empty();
}

}  // namespace chess
