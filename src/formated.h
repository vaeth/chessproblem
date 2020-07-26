// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
// SPDX-License-Identifier: GPL-2.0-only
//
// It is based on the eix::format class from eix
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net> (a previous version of eix::format)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_FORMATED_H_
#define SRC_FORMATED_H_ 1

#include <config.h>

#include <cstdio>  // FILE

#include <ostream>  // only used for overloading
#include <sstream>
#include <string>
#include <vector>

#include "src/m_attribute.h"
#include "src/m_likely.h"

#ifndef NDEBUG
#ifndef FORMAT_DEBUG
#define FORMAT_DEBUG 1
#endif
#endif

namespace format {

/**
printf-like, typesafe text formating that replaces tokens in the given
string with textual representations of values.

Specifier syntax is just like basic printf:
- @b %N is the specifier of type @b N
- @b %num$N is the specifier of type @b N for argument num
- @b %% is a literal @b %

Recognized specifiers are:
- @b s  convert argument to string by using the <<-operator of std::ostream.
- @b d  like %s, but for string::size_type, this will print <string::npos>
        if the argument equals to std::string::npos.

The constructor with an omitted format string acts like the format %s.
An exception is if the stream is specified last - after newline (and flush):
With this exceptional order an omitted format acts like the empty format.

The constructor with a newline (true) argument will add a newline at the end
The constructor with a stream will output the result to a stream.
The constructor with a stream and flush (true) will flush the stream.

The variants Print/PrintError use the default stream stdout.
The variants Say/SayError use the default stream stdout and set newline.
The variant SayError has flush set by default.
The variants PrintEmpty/PrintErrorEmpty/SayEmpty/SayErrorEmpty use
the empty format string.

Example usage:

\code
  std::string error_text("error description");
  int error_number = 17
  format::SayError("An error happened: %s (%s)") % error_text % error_number;
  std::string a = format::Format("first X in \"%s\"") % error_text;
  std::cout << format::Format("search %s: %d") % a % error_text.find('X');
\endcode
**/

class Format;

class FormatManip {
  friend class Format;

 private:
  typedef unsigned char ArgType;
  constexpr static const ArgType
    kNone   = 0x00,
    kString = 0x01,
    kDigit  = 0x02;

  typedef bool ManipType;
  typedef std::vector<ArgType>::size_type ArgCount;

  std::string::size_type index_;
  ArgCount argnum_;
  ManipType type_;  // true if string desired
  FormatManip(std::string::size_type index, ArgCount argnum, ArgType my_type)
    : index_(index), argnum_(argnum), type_((my_type & kDigit) == kNone) {
  }
};

class FormatReplace {
  friend class Format;

 private:
  std::string string_, digit_;
};

class Format {
 protected:
  typedef FormatManip::ArgType ArgType;
  typedef FormatManip::ArgCount ArgCount;

  bool simple_;  // true only if no formatstring given and no argument passed
  bool newline_, flush_;
  FILE *output_;

  // The currently parsed args
  ArgCount current_;
  std::vector<FormatReplace> args_;
  std::vector<ArgType> wanted_;

  // The format string or result
  std::string text_;
  std::vector<FormatManip> manip_;

  ATTRIBUTE_NORETURN void BadFormat() const;
#ifdef FORMAT_DEBUG
  ATTRIBUTE_NORETURN void TooFewArguments() const;
  ATTRIBUTE_NORETURN void TooManyArguments() const;
#endif

  // Write size_type or "<string::npos>" to stream
  static std::ostream& write_representation(std::ostream& s,
      const std::string::size_type& t) {
    if (t == std::string::npos) {
      return (s << "<string::npos>");
    }
    return (s << t);
  }

  // Write t into stream s
  template<typename T> static std::ostream& write_representation(
      std::ostream& s, const T& t) {
    return s << t;
  }

  void Finalize();

  void NewlineOutput();

  // Set the template string. Set simple_ = false
  void Init();

 public:
  Format(FILE *stream, const std::string& format, bool newline, bool flush) :
    newline_(newline), flush_(flush), output_(stream), text_(format) {
    Init();
  }

  ATTRIBUTE_NONNULL((3)) Format(FILE *stream, const char *format, bool newline,
      bool flush) :
    newline_(newline), flush_(flush), output_(stream), text_(format) {
    Init();
  }

  Format(FILE *stream, char format, bool newline, bool flush) :
    simple_(false), newline_(newline), flush_(flush), output_(stream),
    text_(1, format) {
    NewlineOutput();
  }

  Format(FILE *stream, const std::string& format, bool newline) :
    newline_(newline), flush_(false), output_(stream), text_(format) {
    Init();
  }

  ATTRIBUTE_NONNULL((3)) Format(FILE *stream, const char *format,
      bool newline) :
    newline_(newline), flush_(false), output_(stream), text_(format) {
    Init();
  }

  Format(FILE *stream, char format, bool newline) :
    simple_(false), newline_(newline), flush_(false), output_(stream),
    text_(1, format) {
    NewlineOutput();
  }

  Format(FILE *stream, const std::string& format) :
    newline_(false), flush_(false), output_(stream), text_(format) {
    Init();
  }

  Format(FILE *stream, const char *format) :
    newline_(false), flush_(false), output_(stream), text_(format) {
    Init();
  }

  Format(FILE *stream, char format) :
    simple_(false), newline_(false), flush_(false), output_(stream),
    text_(1, format) {
    NewlineOutput();
  }

  Format(const std::string& format, bool newline) :
    newline_(newline), output_(nullptr), text_(format) {
    Init();
  }

  ATTRIBUTE_NONNULL_ Format(const char *format, bool newline) :
    newline_(newline), output_(nullptr), text_(format) {
    Init();
  }

  Format(char format, bool newline) :
    simple_(false), newline_(newline), output_(nullptr), text_(1, format) {
    NewlineOutput();
  }

  explicit Format(const std::string& format) :
    newline_(false), output_(nullptr), text_(format) {
    Init();
  }

  ATTRIBUTE_NONNULL_ explicit Format(const char *format) :
    newline_(false), output_(nullptr), text_(format) {
    Init();
  }

  explicit Format(char format) :
    simple_(false), newline_(false), output_(nullptr), text_(1, format) {
  }

  Format(FILE *stream, bool newline, bool flush) :
    simple_(true), newline_(newline), flush_(flush), output_(stream) {
  }

  Format(FILE *stream, bool newline) :
    simple_(true), newline_(newline), flush_(false), output_(stream) {
  }

  // Exceptional args order in which case we use the empty string as default
  Format(bool newline, bool flush, FILE *stream) :
    simple_(false), newline_(newline), flush_(flush), output_(stream) {
    NewlineOutput();
  }

  // Exceptional args order in which case we use the empty string as default
  Format(bool newline, FILE *stream) :
    simple_(false), newline_(newline), flush_(false), output_(stream) {
    NewlineOutput();
  }

  explicit Format(FILE *stream) :
    simple_(true), newline_(false), flush_(false), output_(stream) {
  }

  explicit Format(bool newline) :
    simple_(true), newline_(newline), output_(nullptr) {
  }

  Format() : simple_(true), newline_(false), output_(nullptr) {
  }

  // Insert the value for the next placeholder
  template<typename T> Format& operator%(const T& s) {
    if (simple_) {
      simple_ = false;
      std::ostringstream os;
      os << s;
      text_.append(os.str());
      NewlineOutput();
      return *this;
    }
    if (UNLIKELY(manip_.empty())) {
#ifdef FORMAT_DEBUG
      TooManyArguments();
#else
      return *this;
#endif
    }
    ArgType c(wanted_[current_]);
    if ((c & FormatManip::kString) != FormatManip::kNone) {
      std::ostringstream os;
      os << s;
      args_[current_].string_ = os.str();
    }
    if ((c & FormatManip::kDigit) != FormatManip::kNone) {
      std::ostringstream os;
      write_representation(os, s);
      args_[current_].digit_ = os.str();
    }
    if (UNLIKELY(++current_ == wanted_.size())) {
      Finalize();
    }
    return *this;
  }

  // return the formatted string
  std::string str() const {
#ifdef FORMAT_DEBUG
    if (UNLIKELY(simple_ || !manip_.empty())) {
      TooFewArguments();
    }
#endif
    return text_;
  }

  // Convenience wrapper for str()
  operator std::string() {
    return str();
  }

  // Write formated string to ostream os
  friend std::ostream& operator<<(std::ostream& os, const Format& formater) {
    return os << formater.str();
  }
};

class Print : public Format {
 public:
  Print(const std::string& format, bool flush) :
    Format(stdout, format, false, flush) {
  }

  ATTRIBUTE_NONNULL_ Print(const char *format, bool flush) :
    Format(stdout, format, false, flush) {
  }

  Print(char format, bool flush) : Format(stdout, format, false, flush) {
  }

  explicit Print(const std::string& format) : Format(stdout, format) {
  }

  ATTRIBUTE_NONNULL_ explicit Print(const char *format) :
    Format(stdout, format) {
  }

  explicit Print(char format) : Format(stdout, format) {
  }

  explicit Print(bool flush) : Format(stdout, false, flush) {
  }

  Print() : Format(stdout) {
  }
};

class PrintEmpty : public Format {
 public:
  explicit PrintEmpty(bool flush) : Format(false, flush, stdout) {
  }

  PrintEmpty() : Format(false, stdout) {
  }
};

class PrintError : public Format {
 public:
  PrintError(const std::string& format, bool flush) :
    Format(stderr, format, false, flush) {
  }

  PrintError(const char *format, bool flush) :
    Format(stderr, format, false, flush) {
  }

  PrintError(char format, bool flush) : Format(stderr, format, false, flush) {
  }

  explicit PrintError(const std::string& format) : Format(stderr, format) {
  }

  ATTRIBUTE_NONNULL_ explicit PrintError(const char *format) :
    Format(stderr, format) {
  }

  explicit PrintError(char format) : Format(stderr, format) {
  }

  explicit PrintError(bool flush) : Format(stderr, false, flush) {
  }

  PrintError() : Format(stderr) {
  }
};

class PrintErrorEmpty : public Format {
 public:
  explicit PrintErrorEmpty(bool flush) : Format(false, flush, stderr) {
  }

  PrintErrorEmpty() : Format(false, stderr) {
  }
};


class Say : public Format {
 public:
  Say(const std::string& format, bool flush) :
    Format(stdout, format, true, flush) {
  }

  Say(const char *format, bool flush) : Format(stdout, format, true, flush) {
  }

  Say(char format, bool flush) : Format(stdout, format, true, flush) {
  }

  explicit Say(const std::string& format) : Format(stdout, format, true) {
  }

  ATTRIBUTE_NONNULL_ explicit Say(const char *format) :
    Format(stdout, format, true) {
  }

  explicit Say(char format) : Format(stdout, format, true) {
  }

  explicit Say(bool flush) : Format(stdout, true, flush) {
  }

  Say() : Format(stdout, true) {
  }
};

class SayEmpty : public Format {
 public:
  explicit SayEmpty(bool flush) : Format(true, flush, stdout) {
  }

  SayEmpty() : Format(true, stdout) {
  }
};

class SayError : public Format {
 public:
  SayError(const std::string& format, bool flush) :
    Format(stderr, format, true, flush) {
  }

  SayError(const char *format, bool flush) :
    Format(stderr, format, true, flush) {
  }

  SayError(char format, bool flush) : Format(stderr, format, true, flush) {
  }

  explicit SayError(const std::string& format) :
    Format(stderr, format, true, true) {
  }

  ATTRIBUTE_NONNULL_ explicit SayError(const char *format) :
    Format(stderr, format, true, true) {
  }

  explicit SayError(char format) : Format(stderr, format, true, true) {
  }

  explicit SayError(bool flush) : Format(stderr, true, flush) {
  }

  SayError() : Format(stderr, true, true) {
  }
};

class SayErrorEmpty : public Format {
 public:
  explicit SayErrorEmpty(bool flush) : Format(true, flush, stderr) {
  }

  SayErrorEmpty() : Format(true, stderr) {
  }
};

}/* namespace format */

#endif  // SRC_FORMATED_H_
