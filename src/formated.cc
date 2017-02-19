// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// It is based on the eix::format class from eix
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "src/formated.h"
#include <config.h>

#include <cctype>  // isdigit

#include <cstdio>  // fwrite, fflush
#include <cstdlib>  // atoi, exit

#include <string>
#include <vector>

#include "src/m_likely.h"

using std::string;
using std::vector;

namespace format {

const FormatManip::ArgType
  FormatManip::kNone,
  FormatManip::kString,
  FormatManip::kDigit;

void Format::BadFormat() const {
  SayError("format: bad format specification \"%s\"") % text_;
  std::exit(EXIT_FAILURE);
}

#ifdef FORMAT_DEBUG
void Format::TooFewArguments() const {
  SayError("format: too few arguments passed for \"%s\"") % text_;
  std::exit(EXIT_FAILURE);
}

void Format::TooManyArguments() const {
  SayError("format: too many arguments passed for \"%s\"") % text_;
  std::exit(EXIT_FAILURE);
}
#endif

void Format::Init() {
  simple_ = false;
  current_ = 0;
  FormatManip::ArgCount imp(0);
  string::size_type i(0);
  while (i = text_.find('%', i), LIKELY(i != string::npos)) {
    string::size_type start(i), len(2);
    ++i;
    if (UNLIKELY(i == text_.size())) {
      BadFormat();
    }
    char c(text_[i]);
    if (c == '%') {
      text_.erase(start, len);
      if (LIKELY(start != text_.size())) {
        i = start;
        continue;
      }
      break;
    }
    ArgCount argnum(imp++);
    if (std::isdigit(c)) {
      string::size_type e(text_.find('$', i));
      if (UNLIKELY(e == string::npos)) {
        BadFormat();
      }
      len += e - start;
      string number(text_, i, e - i);
      int index = std::atoi(number.c_str());
      if (UNLIKELY(argnum <= 0)) {
        BadFormat();
      }
      argnum = static_cast<ArgCount>(index - 1);
      ++e;
      if (UNLIKELY(e == text_.size())) {
        BadFormat();
      }
      c = text_[e];
    }
    ArgType my_type;
#ifdef FORMAT_DEBUG
    switch (c) {
      case 's':
        my_type = FormatManip::kString;
        break;
      case 'd':
        my_type = FormatManip::kDigit;
        break;
      default:
        BadFormat();
        break;
    }
#else
    my_type = ((c == 's') ? FormatManip::kString : FormatManip::kDigit);
#endif
    if (LIKELY(argnum == wanted_.size())) {
      wanted_.push_back(my_type);
    } else if (argnum > wanted_.size()) {
      wanted_.insert(wanted_.end(), argnum - wanted_.size() + 1,
        FormatManip::kNone);
      wanted_[argnum] = my_type;
    } else {
      wanted_[argnum] |= my_type;
    }
    manip_.push_back(FormatManip(start, argnum, my_type));
    text_.erase(start, len);
    i = start;
    if (i == text_.size()) {
      break;
    }
  }
  if (UNLIKELY(wanted_.empty())) {
    Finalize();
  } else {
    args_.insert(args_.end(), wanted_.size(), FormatReplace());
  }
}

void Format::Finalize() {
  for (vector<FormatManip>::const_reverse_iterator it(manip_.rbegin());
    it != manip_.rend(); ++it) {
    text_.insert(it->index_, (it->type_ ?
      args_[it->argnum_].string_ : args_[it->argnum_].digit_));
  }
  manip_.clear();
  wanted_.clear();
  args_.clear();
  NewlineOutput();
}

void Format::NewlineOutput() {
  if (newline_) {
    text_.append(1, '\n');
  }
  if (output_ != nullptr) {
    if (LIKELY(!text_.empty())) {
      std::fwrite(text_.c_str(), sizeof(char), text_.size(), output_);
    }
    if (UNLIKELY(flush_)) {
      std::fflush(output_);
    }
  }
}

}/* namespace format */
