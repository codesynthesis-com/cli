// file      : cli/runtime-source.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// license   : MIT; see accompanying LICENSE file

#include <cli/runtime-source.hxx>

using namespace std;

void
generate_runtime_source (context& ctx, bool complete)
{
  ostream& os (ctx.os);

  os << "#include <map>" << endl
     << "#include <set>" << endl
     << "#include <string>" << endl
     << "#include <vector>" << endl
     << "#include <utility>" << endl // pair
     << "#include <ostream>" << endl
     << "#include <sstream>" << endl
     << "#include <cstring>" << endl;

  if (complete && ctx.options.generate_file_scanner ())
    os << "#include <fstream>" << endl;

  os << endl;

  ctx.ns_open (ctx.cli);

  if (complete)
  {
    string const& os_type (ctx.options.ostream_type ());

    const char* nothrow (ctx.options.std () < cxx_version::cxx11
                         ? "throw ()"
                         : "noexcept");

    // unknown_option
    //
    os << "// unknown_option" << endl
       << "//" << endl
       << "unknown_option::" << endl
       << "~unknown_option () " << nothrow
       << "{"
       << "}"

       << "void unknown_option::" << endl
       << "print (" << os_type << "& os) const"
       << "{"
       << "os << \"unknown option '\" << option ().c_str () << \"'\";"
       << "}"

       << "const char* unknown_option::" << endl
       << "what () const " << nothrow
       << "{"
       << "return \"unknown option\";"
       << "}";

    // unknown_argument
    //
    os << "// unknown_argument" << endl
       << "//" << endl
       << "unknown_argument::" << endl
       << "~unknown_argument () " << nothrow
       << "{"
       << "}"

       << "void unknown_argument::" << endl
       << "print (" << os_type << "& os) const"
       << "{"
       << "os << \"unknown argument '\" << argument ().c_str () << \"'\";"
       << "}"

       << "const char* unknown_argument::" << endl
       << "what () const " << nothrow
       << "{"
       << "return \"unknown argument\";"
       << "}";

    // missing_value
    //
    os << "// missing_value" << endl
       << "//" << endl
       << "missing_value::" << endl
       << "~missing_value () " << nothrow
       << "{"
       << "}"

       << "void missing_value::" << endl
       << "print (" << os_type << "& os) const"
       << "{"
       << "os << \"missing value for option '\" << option ().c_str () << \"'\";"
       << "}"

       << "const char* missing_value::" << endl
       << "what () const " << nothrow
       << "{"
       << "return \"missing option value\";"
       << "}";

    // invalid_value
    //
    os << "// invalid_value" << endl
       << "//" << endl
       << "invalid_value::" << endl
       << "~invalid_value () " << nothrow
       << "{"
       << "}"

       << "void invalid_value::" << endl
       << "print (" << os_type << "& os) const"
       << "{"
       << "os << \"invalid value '\" << value ().c_str () << \"' for " <<
      "option '\"" << endl
       << "   << option ().c_str () << \"'\";"
       << endl
       << "if (!message ().empty ())" << endl
       << "os << \": \" << message ().c_str ();"
       << "}"

       << "const char* invalid_value::" << endl
       << "what () const " << nothrow
       << "{"
       << "return \"invalid option value\";"
       << "}";

    // eos_reached
    //
    os << "// eos_reached" << endl
       << "//" << endl
       << "void eos_reached::" << endl
       << "print (" << os_type << "& os) const"
       << "{"
       << "os << what ();"
       << "}"

       << "const char* eos_reached::" << endl
       << "what () const " << nothrow
       << "{"
       << "return \"end of argument stream reached\";"
       << "}";

    if (ctx.options.generate_file_scanner ())
    {
      // file_io_failure
      //
      os << "// file_io_failure" << endl
         << "//" << endl
         << "file_io_failure::" << endl
         << "~file_io_failure () " << nothrow
         << "{"
         << "}"

         << "void file_io_failure::" << endl
         << "print (" << os_type << "& os) const"
         << "{"
         << "os << \"unable to open file '\" << file ().c_str () << " <<
        "\"' or read failure\";"
         << "}"

         << "const char* file_io_failure::" << endl
         << "what () const " << nothrow
         << "{"
         << "return \"unable to open file or read failure\";"
         << "}";

      // unmatched_argument
      //
      os << "// unmatched_quote" << endl
         << "//" << endl
         << "unmatched_quote::" << endl
         << "~unmatched_quote () " << nothrow
         << "{"
         << "}"

         << "void unmatched_quote::" << endl
         << "print (" << os_type << "& os) const"
         << "{"
         << "os << \"unmatched quote in argument '\" << " <<
        "argument ().c_str () << \"'\";"
         << "}"

         << "const char* unmatched_quote::" << endl
         << "what () const " << nothrow
         << "{"
         << "return \"unmatched quote\";"
         << "}";
    }

    if (ctx.options.generate_group_scanner ())
    {
      // unexpected_group
      //
      os << "// unexpected_group" << endl
         << "//" << endl

         << "unexpected_group::" << endl
         << "~unexpected_group () " << nothrow
         << "{"
         << "}"

         << "void unexpected_group::" << endl
         << "print (" << os_type << "& os) const"
         << "{"
         << "os << \"unexpected grouped argument '\" << group_ << \"' \"" << endl
         << "   << \"for argument '\" << argument_ << \"'\";"
         << "}"

         << "const char* unexpected_group::" << endl
         << "what () const " << nothrow
         << "{"
         << "return \"unexpected grouped argument\";"
         << "}";

        // group_separator
        //
        os << "// group_separator" << endl
           << "//" << endl

           << "group_separator::" << endl
           << "~group_separator () " << nothrow
           << "{"
           << "}"

           << "void group_separator::" << endl
           << "print (" << os_type << "& os) const"
           << "{"
           << "bool ex (!expected_.empty ());"
           << "bool en (!encountered_.empty ());"
           << endl
           << "if (ex)"
           << "{"
           << "os << \"expected group separator '\" << expected_ << \"'\";"
           << "if (en)" << endl
           << "os << \" instead of '\" << encountered_ << \"'\";"
           << "}"
           << "else" << endl
           << "os << \"unexpected group separator '\" << encountered_ << \"'\";"
           << endl
           << "if (en)" << endl
           << "os << \", use '\\\\\" << encountered_ << \"' to escape\";"
           << "}"

           << "const char* group_separator::" << endl
           << "what () const " << nothrow
           << "{"
           << "bool ex (!expected_.empty ());"
           << "bool en (!encountered_.empty ());"
           << endl
           << "return en" << endl
           << "  ? ex ? \"wrong group separator\"    : \"unexpected group separator\"" << endl
           << "  : ex ? \"expected group separator\" : \"\";"
           << "}";
    }

    // scanner
    //
    os << "// scanner" << endl
       << "//" << endl
       << "scanner::" << endl
       << "~scanner ()"
       << "{"
       << "}";

    // argv_scanner
    //
    // Note that due to the erase logic we cannot just add i_ to
    // start_position and so have to increment it instead. See also
    // argv_file_scanner that continues with this logic.
    //
    os << "// argv_scanner" << endl
       << "//" << endl

       << "bool argv_scanner::" << endl
       << "more ()"
       << "{"
       << "return i_ < argc_;"
       << "}"

       << "const char* argv_scanner::" << endl
       << "peek ()"
       << "{"
       << "if (i_ < argc_)" << endl
       << "return argv_[i_];"
       << "else" << endl
       << "throw eos_reached ();"
       << "}"

       << "const char* argv_scanner::" << endl
       << "next ()"
       << "{"
       << "if (i_ < argc_)"
       << "{"
       << "const char* r (argv_[i_]);"
       << endl
       << "if (erase_)"
       << "{"
       << "for (int i (i_ + 1); i < argc_; ++i)" << endl
       << "argv_[i - 1] = argv_[i];"
       << endl
       << "--argc_;"
       << "argv_[argc_] = 0;"
       << "}"
       << "else" << endl
       << "++i_;"
       << endl
       << "++start_position_;"
       << "return r;"
       << "}"
       << "else" << endl
       << "throw eos_reached ();"
       << "}"

       << "void argv_scanner::" << endl
       << "skip ()"
       << "{"
       << "if (i_ < argc_)"
       << "{"
       << "++i_;"
       << "++start_position_;"
       << "}"
       << "else" << endl
       << "throw eos_reached ();"
       << "}"

       << "std::size_t argv_scanner::" << endl
       << "position ()"
       << "{"
       << "return start_position_;"
       << "}";

    // vector_scanner
    //
    if (ctx.options.generate_vector_scanner ())
    {
      os << "// vector_scanner" << endl
         << "//" << endl

         << "bool vector_scanner::" << endl
         << "more ()"
         << "{"
         << "return i_ < v_.size ();"
         << "}"

         << "const char* vector_scanner::" << endl
         << "peek ()"
         << "{"
         << "if (i_ < v_.size ())" << endl
         << "return v_[i_].c_str ();"
         << "else" << endl
         << "throw eos_reached ();"
         << "}"

         << "const char* vector_scanner::" << endl
         << "next ()"
         << "{"
         << "if (i_ < v_.size ())" << endl
         << "return v_[i_++].c_str ();"
         << "else" << endl
         << "throw eos_reached ();"
         << "}"

         << "void vector_scanner::" << endl
         << "skip ()"
         << "{"
         << "if (i_ < v_.size ())" << endl
         << "++i_;"
         << "else" << endl
         << "throw eos_reached ();"
         << "}"

         << "std::size_t vector_scanner::" << endl
         << "position ()"
         << "{"
         << "return start_position_ + i_;"
         << "}";
    }

    // argv_file_scanner
    //
    // Note that we continue incrementing start_position like argv_scanner.
    //
    if (ctx.options.generate_file_scanner ())
    {
      bool sep (!ctx.opt_sep.empty ());

      const string& pfx (ctx.opt_prefix);
      size_t pfx_n (pfx.size ());
      bool comb_values (pfx_n != 0 && !ctx.options.no_combined_values ());

      os << "// argv_file_scanner" << endl
         << "//" << endl

         << "int argv_file_scanner::zero_argc_ = 0;"
         << "std::string argv_file_scanner::empty_string_;"
         << endl

         << "bool argv_file_scanner::" << endl
         << "more ()"
         << "{"
         << "if (!args_.empty ())" << endl
         << "return true;"
         << endl
         << "while (base::more ())"
         << "{"
         << "// See if the next argument is the file option." << endl
         << "//" << endl
         << "const char* a (base::peek ());"
         << "const option_info* oi = 0;"
         << "const char* ov = 0;"
         << endl;

      if (sep)
        os << "if (!skip_)"
           << "{";

      os << "if ((oi = find (a)) != 0)"
         << "{"
         << "base::next ();"
         << endl
         << "if (!base::more ())" << endl
         << "throw missing_value (a);"
         << endl
         << "ov = base::next ();"
         << "}";

      // Handle the combined option/value (--foo=bar). See the option parsing
      // implementation for details.
      //
      if (comb_values)
      {
        os << "else if (std::strncmp (a, \"" << pfx << "\", " <<
          pfx_n << ") == 0)" // It looks like an option.
           << "{"
           <<   "if ((ov = std::strchr (a, '=')) != 0)" // Has '='.
           <<   "{"
           <<     "std::string o (a, 0, ov - a);"
           <<     "if ((oi = find (o.c_str ())) != 0)"
           <<     "{"
           <<       "base::next ();"
           <<       "++ov;" // That's the value.
           <<     "}"
           <<   "}"
           << "}";
      }

      if (sep)
        os << "}";

      os << "if (oi != 0)"
         << "{"
         << "if (oi->search_func != 0)"
         << "{"
         << "std::string f (oi->search_func (ov, oi->arg));"
         << endl
         << "if (!f.empty ())" << endl
         << "load (f);"
         << "}"
         << "else" << endl
         << "load (ov);"
         << endl
         << "if (!args_.empty ())" << endl
         << "return true;"
         << "}"
         << "else"
         << "{";
      if (sep)
        os << "if (!skip_)" << endl
           << "skip_ = (std::strcmp (a, \"" << ctx.opt_sep << "\") == 0);"
           << endl;
      os << "return true;"
         << "}"
         << "}" // while
         << "return false;"
         << "}"

         << "const char* argv_file_scanner::" << endl
         << "peek ()"
         << "{"
         << "if (!more ())" << endl
         << "throw eos_reached ();"
         << endl
         << "return args_.empty () ? base::peek () : args_.front ().value.c_str ();"
         << "}"

         << "const std::string& argv_file_scanner::" << endl
         << "peek_file ()"
         << "{"
         << "if (!more ())" << endl
         << "throw eos_reached ();"
         << endl
         << "return args_.empty () ? empty_string_ : *args_.front ().file;"
         << "}"

         << "std::size_t argv_file_scanner::" << endl
         << "peek_line ()"
         << "{"
         << "if (!more ())" << endl
         << "throw eos_reached ();"
         << endl
         << "return args_.empty () ? 0 : args_.front ().line;"
         << "}"

         << "const char* argv_file_scanner::" << endl
         << "next ()"
         << "{"
         << "if (!more ())" << endl
         << "throw eos_reached ();"
         << endl
         << "if (args_.empty ())" << endl
         << "return base::next ();"
         << "else"
         << "{"
         << "hold_[i_ == 0 ? ++i_ : --i_].swap (args_.front ().value);"
         << "args_.pop_front ();"
         << "++start_position_;"
         << "return hold_[i_].c_str ();"
         << "}"
         << "}"

         << "void argv_file_scanner::" << endl
         << "skip ()"
         << "{"
         << "if (!more ())" << endl
         << "throw eos_reached ();"
         << endl
         << "if (args_.empty ())" << endl
         << "return base::skip ();"
         << "else"
         << "{"
         << "args_.pop_front ();"
         << "++start_position_;"
         << "}"
         << "}"

         << "const argv_file_scanner::option_info* argv_file_scanner::" << endl
         << "find (const char* a) const"
         << "{"
         << "for (std::size_t i (0); i < options_count_; ++i)" << endl
         << "if (std::strcmp (a, options_[i].option) == 0)" << endl
         << "return &options_[i];"
         << endl
         << "return 0;"
         << "}"

         << "std::size_t argv_file_scanner::" << endl
         << "position ()"
         << "{"
         << "return start_position_;"
         << "}"

         << "void argv_file_scanner::" << endl
         << "load (const std::string& file)"
         << "{"
         << "using namespace std;"
         << endl
         << "ifstream is (file.c_str ());"
         << endl
         << "if (!is.is_open ())" << endl
         << "throw file_io_failure (file);"
         << endl
         << "files_.push_back (file);"
         << endl
         << "arg a;"
         << "a.file = &*files_.rbegin ();"
         << endl
         << "for (a.line = 1; !is.eof (); ++a.line)"
         << "{"
         << "string line;"
         << "getline (is, line);"
         << endl
         << "if (is.fail () && !is.eof ())" << endl
         << "throw file_io_failure (file);"
         << endl
         << "string::size_type n (line.size ());"
         << endl
         << "// Trim the line from leading and trailing whitespaces." << endl
         << "//" << endl
         << "if (n != 0)"
         << "{"
         << "const char* f (line.c_str ());"
         << "const char* l (f + n);"
         << endl
         << "const char* of (f);"
         << "while (f < l && (*f == ' ' || *f == '\\t' || *f == '\\r'))" << endl
         << "++f;"
         << endl
         << "--l;"
         << endl
         << "const char* ol (l);"
         << "while (l > f && (*l == ' ' || *l == '\\t' || *l == '\\r'))" << endl
         << "--l;"
         << endl
         << "if (f != of || l != ol)" << endl
         << "line = f <= l ? string (f, l - f + 1) : string ();"
         << "}"
         << "// Ignore empty lines, those that start with #." << endl
         << "//" << endl
         << "if (line.empty () || line[0] == '#')" << endl
         << "continue;"
         << endl
         << "string::size_type p (string::npos);";

      // If we have the option prefix, then only consider lines that start
      // with that as options.
      //
      if (pfx_n != 0)
        os << "if (line.compare (0, " << pfx_n << ", \"" << pfx << "\") == 0)"
           << "{";

      os << "p = line.find (' ');";

      // Handle the combined option/value (--foo=bar). This is a bit tricky
      // since the equal sign can be part of the value (--foo bar=baz).
      //
      // Note that we cannot just pass it along as combined because of
      // quoting (--foo="'bar baz'").
      //
      if (comb_values)
      {
        os << endl
           << "string::size_type q (line.find ('='));"
           << "if (q != string::npos && q < p)" << endl
           <<   "p = q;";
      }

      if (pfx_n != 0)
        os << "}";
      else
        os << endl;

      os << "string s1;"
         << "if (p != string::npos)"
         << "{"
         <<   "s1.assign (line, 0, p);"
         << endl
         << "// Skip leading whitespaces in the argument." << endl
         << "//" << endl;
      if (comb_values)
        os << "if (line[p] == '=')" << endl // Keep whitespaces after '='.
           <<   "++p;"
           << "else"
           << "{";
      os <<     "n = line.size ();"
         <<     "for (++p; p < n; ++p)"
         <<     "{"
         <<       "char c (line[p]);"
         <<       "if (c != ' ' && c != '\\t' && c != '\\r')" << endl
         <<         "break;"
         <<     "}";
      if (comb_values)
        os << "}";
      os << "}";
      if (sep)
        os << "else if (!skip_)" << endl
           <<   "skip_ = (line == \"" << ctx.opt_sep << "\");"
           << endl;

      os << "string s2 (line, p != string::npos ? p : 0);"
         << endl
         << "// If the string (which is an option value or argument) is" << endl
         << "// wrapped in quotes, remove them." << endl
         << "//" << endl
         << "n = s2.size ();"
         << "char cf (s2[0]), cl (s2[n - 1]);"
         << endl
         << "if (cf == '\"' || cf == '\\'' || cl == '\"' || cl == '\\'')"
         << "{"
         <<   "if (n == 1 || cf != cl)" << endl
         <<     "throw unmatched_quote (s2);"
         << endl
         <<   "s2 = string (s2, 1, n - 2);"
         << "}";

      os << "if (!s1.empty ())"
         << "{"
         <<   "// See if this is another file option." << endl
         <<   "//" << endl
         <<   "const option_info* oi;"
         <<   "if (" << (sep ? "!skip_ && " : "") <<
        "(oi = find (s1.c_str ())))" << endl
         <<   "{"
         <<     "if (s2.empty ())" << endl
         <<       "throw missing_value (oi->option);"
         << endl
         <<     "if (oi->search_func != 0)"
         <<     "{"
         <<       "string f (oi->search_func (s2.c_str (), oi->arg));"
         <<       "if (!f.empty ())" << endl
         <<         "load (f);"
         <<     "}"
         <<     "else" << endl
         <<     "{"
         <<       "// If the path of the file being parsed is not simple and the" << endl
         <<       "// path of the file that needs to be loaded is relative, then" << endl
         <<       "// complete the latter using the former as a base." << endl
         <<       "//" << endl
         << "#ifndef _WIN32" << endl
         <<       "string::size_type p (file.find_last_of ('/'));"
         <<       "bool c (p != string::npos && s2[0] != '/');"
         << "#else" << endl
         <<       "string::size_type p (file.find_last_of (\"/\\\\\"));"
         <<       "bool c (p != string::npos && s2[1] != ':');"
         << "#endif" << endl
         <<       "if (c)" << endl
         <<         "s2.insert (0, file, 0, p + 1);"
         << endl
         <<       "load (s2);"
         <<     "}"
         <<     "continue;"
         <<   "}"
         <<   "a.value = s1;"
         <<   "args_.push_back (a);"
         << "}"
         << "a.value = s2;"
         << "args_.push_back (a);"
         << "}" // while
         << "}";
    }

    // group_scanner
    //
    if (ctx.options.generate_group_scanner ())
    {
      os << "// group_scanner" << endl
         << "//" << endl

         << "bool group_scanner::" << endl
         << "more ()"
         << "{"
         << "// We don't want to call scan_group() here since that"     << endl
         << "// would invalidate references to previous arguments."     << endl
         << "// But we do need to check that the previous group was"    << endl
         << "// handled."                                               << endl
         << "//"                                                        << endl
         << "if (state_ == scanned)"
         << "{"
         << "if (group_scan_.end () != group_.size ())" << endl
         << "throw unexpected_group (arg_[i_][j_], group_scan_.next ());"
         << "}"
         << "return j_ != 0 || scan_.more ();"
         << "}"

         << "const char* group_scanner::" << endl
         << "peek ()"
         << "{"
         << "if (state_ != peeked)"
         << "{"
         << "scan_group ();"
         << "state_ = peeked;"
         << "}"
         << "// Return unescaped." << endl
         << "return arg_[i_][j_ - 1].c_str ();"
         << "}"

         << "const char* group_scanner::" << endl
         << "next ()"
         << "{"
         << "if (state_ != peeked)" << endl
         << "scan_group ();"
         << "state_ = scanned;"
         << "// Return unescaped." << endl
         << "return arg_[i_][--j_].c_str ();"
         << "}"

         << "void group_scanner::" << endl
         << "skip ()"
         << "{"
         << "if (state_ != peeked)" << endl
         << "scan_group ();"
         << "state_ = skipped;"
         << "--j_;"
         << "}"

         << "std::size_t group_scanner::" << endl
         << "position ()"
         << "{"
         << "return j_ == 0 ? scan_.position () : pos_ + (arg_[i_].size () - j_);"
         << "}"

         << "void group_scanner::" << endl
         << "scan_group ()"
         << "{"
         << "// If the previous argument has been scanned, then make"   << endl
         << "// sure the group has been scanned (handled) as well."     << endl
         << "//"                                                        << endl
         << "if (state_ == scanned)"
         << "{"
         <<   "if (group_scan_.end () != group_.size ())" << endl
         <<     "throw unexpected_group (arg_[i_][j_], group_scan_.next ());"
         << "}"

         << "// If we still have arguments in the pack, rewind the group." << endl
         << "//"                                                           << endl
         << "if (j_ != 0)"
         << "{"
         <<   "group_scan_.reset ();"
         <<   "return;"
         << "}"

        // Position must remain the same from before the first call to peek()
        // (comes directly from the scanner) and until next().
        //
        // Note that while it may seem like a good idea to pass
        // scan_.position() to reset() below, the trailing group positions
        // will overlap with the argument's. So it seems best to start
        // positions of each argument in a group from 0.
        //
        // Note also that we try hard not to throw away allocated memory in
        // arg_[][0].
        //
         << "i_ += (i_ == 0 ? 1 : -1);"
         << "group_.clear ();"
         << "group_scan_.reset ();"
         << "pos_ = scan_.position ();"
         <<                                                                endl

         << "// Note: using group_ won't cover empty groups and using"  << endl
         << "// j_ won't cover single-argument packs."                  << endl
         << "//"                                                        << endl
         << "bool group (false), pack (false);"
         <<                                                                endl
         << "do"
         << "{"
         <<   "const char* a (scan_.next ());"
         <<   "size_t i (*a == '\\\\' ? 1 : 0);"
         <<   "separator s (sense (a + i));"
         <<                                                                endl
         <<   "if (s == none || i != 0)"
         <<   "{"
         <<     "if (arg_[i_].size () != 1)"                            << endl
         <<       "arg_[i_].resize (1);"
         <<                                                                endl
         <<     "arg_[i_][0] = a + (s != none ? i : 0);"
         <<     "j_ = 1;"
         <<     "break;"
         <<   "}"

         <<   "// Start of a leading group for the next argument or"    << endl
         <<   "// argument pack. We will only know which once we see"   << endl
         <<   "// the closing separator."                               << endl
         <<   "//"                                                      << endl
         <<   "if (s != open)"                                          << endl
         <<     "throw group_separator (a, \"\");"
         <<                                                                endl
         <<   "size_t n (group_.size ());"
         <<                                                                endl
         <<   "// Scan the group until the closing separator."          << endl
         <<   "//"                                                      << endl
         <<   "s = none;"
         <<   "while (s == none && scan_.more ())"
         <<   "{"
         <<     "a = scan_.next ();"
         <<     "i = (*a == '\\\\' ? 1 : 0);"
         <<     "s = sense (a + i);"
         <<                                                                endl
         <<     "if (s == none || i != 0)"
         <<     "{"
         <<       "group_.push_back (a + (s != none ? i : 0));"
         <<       "s = none;"
         <<     "}"
         <<   "}"

         <<   "if (s == close)"
         <<   "{"
         <<     "size_t m (group_.size ());"
         <<                                                                endl
         <<     "j_ = m - n;"
         <<     "if (j_ == 0)"                                          << endl
         <<       "throw group_separator (\"{\", \"\");"
         <<                                                                endl
         <<     "if (arg_[i_].size () != j_)" <<                           endl
         <<       "arg_[i_].resize (j_);"
         <<                                                                endl
         <<     "// Move from group_ to arg_. Add in reverse for ease " << endl
         <<     "// of iteration."                                      << endl
         <<     "//"                                                    << endl
         <<     "for (size_t j (0); j != j_; ++j)"                      << endl
         <<       "arg_[i_][j] = group_[m - j - 1];"
         <<     "group_.resize (n);"
         <<                                                                endl
         <<     "pack = true;"
         <<     "break;"
         <<   "}"
         <<   "else if (s == close_plus)" << endl
         <<     "group = true;"
         <<   "else"                                                    << endl
         <<     "throw group_separator ((s != none ? a : \"\"), \"}+\");"
         << "}"
         << "while (scan_.more ());"
         <<                                                                endl

         << "// Handle the case where we have seen the leading group"   << endl
         << "// but there are no more arguments."                       << endl
         << "//"                                                        << endl
         << "if (group && j_ == 0)"                                     << endl
         <<   "throw group_separator (\"{\", \"\");"
         <<                                                                endl
         << "// Handle trailing groups, if any."                        << endl
         << "//"                                                        << endl

         << "while (scan_.more ())"
         << "{"
         <<   "const char* a (scan_.peek ());"
         <<   "size_t i (*a == '\\\\' ? 1 : 0);"
         <<   "separator s (sense (a + i));"
         <<                                                                endl

         <<   "// Next argument, argument pack, or leading group."      << endl
         <<   "//"                                                      << endl
         <<   "if (s == none || s == open || i != 0)"                   << endl
         <<     "break;"
         <<                                                                endl
         <<   "if (s != open_plus)"                                     << endl
         <<     "throw group_separator (a, \"\");"
         <<                                                                endl
         <<   "group = true;"
         <<                                                                endl
         <<   "// Scan the group until the closing separator."          << endl
         <<   "//"                                                      << endl
         <<   "scan_.next ();"
         <<   "s = none;"
         <<   "while (s == none && scan_.more ())"
         <<   "{"
         <<     "a = scan_.next ();"
         <<     "i = (*a == '\\\\' ? 1 : 0);"
         <<     "s = sense (a + i);"
         <<                                                                endl
         <<     "if (s == none || i != 0)"
         <<     "{"
         <<       "group_.push_back (a + (s != none ? i : 0));"
         <<       "s = none;"
         <<     "}"
         <<   "}"

         <<   "if (s != close)" << endl
         <<     "throw group_separator ((s != none ? a : \"\"), \"}\");"
         << "}"

         << "// Handle the case where we have seen the argument pack"   << endl
         << "// without leading or trailing group."                     << endl
         << "//"                                                        << endl
         << "if (pack && !group)"                                       << endl
         <<   "throw group_separator (\"{\", \"\");"

         << "}";
    }

    // Option description.
    //
    if (ctx.options.generate_description ())
    {
      // options
      //
      os << "void options::" << endl
         << "push_back (const option& o)"
         << "{"
         << "container_type::size_type n (size ());"
         << "container_type::push_back (o);"
         << "map_[o.name ()] = n;"
         << endl
         << "for (option_names::const_iterator i (o.aliases ().begin ());" << endl
         << "i != o.aliases ().end (); ++i)" << endl
         << "map_[*i] = n;"
         << "}";
    }
  }

  // To reduce the number of standard headers we have to include in the
  // generated header file, we always generate the following templates
  // in the source file.
  //
  bool sp (ctx.gen_specifier);
  bool gen_merge (ctx.gen_merge);

  // parser class template & its specializations
  //
  os << "template <typename X>" << endl
     << "struct parser"
     << "{";

  os <<   "static void" << endl
     <<   "parse (X& x, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<   "{"
     <<     "using namespace std;"
     <<                                                                    endl
     <<     "const char* o (s.next ());"
     <<     "if (s.more ())"
     <<     "{"
     <<       "string v (s.next ());"
     <<       "istringstream is (v);"
     <<       "if (!(is >> x && is.peek () == istringstream::traits_type::eof ()))" << endl
     <<         "throw invalid_value (o, v);"
     <<     "}"
     <<     "else" << endl
     <<       "throw missing_value (o);";
  if (sp)
    os << endl
       <<   "xs = true;";
  os <<   "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (X& b, const X& a)"
       << "{"
       <<   "b = a;"
       << "}";

  os << "};";

  // parser<bool>
  //
  os << "template <>" << endl
     << "struct parser<bool>"
     << "{";

  os <<   "static void" << endl
     <<   "parse (bool& x, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<   "{"
     <<     "const char* o (s.next ());"
     <<                                                                    endl
     <<     "if (s.more ())"
     <<     "{"
     <<       "const char* v (s.next ());"
     <<                                                                    endl
     <<       "if (std::strcmp (v, \"1\")    == 0 ||" << endl
     <<           "std::strcmp (v, \"true\") == 0 ||" << endl
     <<           "std::strcmp (v, \"TRUE\") == 0 ||" << endl
     <<           "std::strcmp (v, \"True\") == 0)"   << endl
     <<             "x = true;"
     <<       "else if (std::strcmp (v, \"0\")     == 0 ||" << endl
     <<                "std::strcmp (v, \"false\") == 0 ||" << endl
     <<                "std::strcmp (v, \"FALSE\") == 0 ||" << endl
     <<                "std::strcmp (v, \"False\") == 0)"   << endl
     <<             "x = false;"
     <<       "else" << endl
     <<         "throw invalid_value (o, v);"
     <<     "}"
     <<     "else" << endl
     <<       "throw missing_value (o);";
  if (sp)
    os << endl
       <<   "xs = true;";
  os <<   "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (bool& b, const bool&)"
       << "{"
       <<   "b = true;" // We wouldn't be here if a is false.
       << "}";

  os << "};";

  // parser<string>
  //
  os << "template <>" << endl
     << "struct parser<std::string>"
     << "{";

  os <<   "static void" << endl
     <<   "parse (std::string& x, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<   "{"
     <<   "const char* o (s.next ());"
     <<                                                                    endl
     <<   "if (s.more ())" << endl
     <<     "x = s.next ();"
     <<   "else" << endl
     <<     "throw missing_value (o);";
  if (sp)
    os <<                                                                  endl
       << "xs = true;";
  os <<   "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (std::string& b, const std::string& a)"
       << "{"
       <<   "b = a;"
       << "}";

  os << "};";

  // parser<std::pair<X, std::size_t>>
  //
  os << "template <typename X>" << endl
     << "struct parser<std::pair<X, std::size_t> >"
     << "{";

  os <<   "static void" << endl
     <<   "parse (std::pair<X, std::size_t>& x, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<   "{"
     <<     "x.second = s.position ();"
     <<     "parser<X>::parse (x.first, " << (sp ? "xs, " : "") << "s);"
     <<   "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (std::pair<X, std::size_t>& b, const std::pair<X, std::size_t>& a)"
       << "{"
       <<   "b = a;"
       << "}";

  os << "};";

  // parser<std::vector<X>>
  //
  os << "template <typename X>" << endl
     << "struct parser<std::vector<X> >"
     << "{";

  os <<   "static void" << endl
     <<   "parse (std::vector<X>& c, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<   "{"
     <<     "X x;";
  if (sp)
    os <<   "bool dummy;";
  os <<     "parser<X>::parse (x, " << (sp ? "dummy, " : "") << "s);"
     <<     "c.push_back (x);";
  if (sp)
    os <<   "xs = true;";
  os <<   "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (std::vector<X>& b, const std::vector<X>& a)"
       << "{"
       <<   "b.insert (b.end (), a.begin (), a.end ());"
       << "}";

  os << "};";

  // parser<std::set<X,C>>
  //
  os << "template <typename X, typename C>" << endl
     << "struct parser<std::set<X, C> >"
     << "{";

  os <<  "static void" << endl
     <<  "parse (std::set<X, C>& c, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<  "{"
     <<    "X x;";
  if (sp)
    os <<  "bool dummy;";
  os <<    "parser<X>::parse (x, " << (sp ? "dummy, " : "") << "s);"
     <<    "c.insert (x);";
  if (sp)
    os <<  "xs = true;";
  os <<  "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (std::set<X, C>& b, const std::set<X, C>& a)"
       << "{"
       <<   "b.insert (a.begin (), a.end ());"
       << "}";

  os << "};";

  // parser<std::map<K,V,C>>
  //
  os << "template <typename K, typename V, typename C>" << endl
     << "struct parser<std::map<K, V, C> >"
     << "{";

  os <<   "static void" << endl
     <<   "parse (std::map<K, V, C>& m, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<   "{"
     <<     "const char* o (s.next ());"
     <<                                                                    endl
     <<     "if (s.more ())"
     <<     "{"
     <<       "std::size_t pos (s.position ());"
     <<       "std::string ov (s.next ());"
     <<       "std::string::size_type p = ov.find ('=');"
     <<                                                                    endl
     <<       "K k = K ();"
     <<       "V v = V ();"
     <<       "std::string kstr (ov, 0, p);"
     <<       "std::string vstr (ov, (p != std::string::npos ? p + 1 : ov.size ()));"
     <<                                                                    endl
     <<       "int ac (2);"
     <<       "char* av[] ="
     <<       "{"
     <<       "const_cast<char*> (o)," << endl
     <<       "0"
     <<       "};";
  if (sp)
    os <<     "bool dummy;";
  os <<       "if (!kstr.empty ())"
     <<       "{"
     <<         "av[1] = const_cast<char*> (kstr.c_str ());"
     <<         "argv_scanner s (0, ac, av, false, pos);"
     <<         "parser<K>::parse (k, " << (sp ? "dummy, " : "") << "s);"
     <<       "}"
     <<       "if (!vstr.empty ())"
     <<       "{"
     <<         "av[1] = const_cast<char*> (vstr.c_str ());"
     <<         "argv_scanner s (0, ac, av, false, pos);"
     <<         "parser<V>::parse (v, " << (sp ? "dummy, " : "") << "s);"
     <<       "}"
     <<       "m[k] = v;"
     <<     "}"
     <<     "else" << endl
     <<       "throw missing_value (o);";
  if (sp)
    os << endl
       <<   "xs = true;";
  os <<   "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (std::map<K, V, C>& b, const std::map<K, V, C>& a)"
       << "{"
       <<   "for (typename std::map<K, V, C>::const_iterator i (a.begin ()); " << endl
       <<         "i != a.end (); "                                            << endl
       <<         "++i)"                                                       << endl
       <<     "b[i->first] = i->second;"
       << "}";

  os << "};";

  // parser<std::multimap<K,V,C>>
  //
  os << "template <typename K, typename V, typename C>" << endl
     << "struct parser<std::multimap<K, V, C> >"
     << "{";

  os <<   "static void" << endl
     <<   "parse (std::multimap<K, V, C>& m, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     <<   "{"
     <<     "const char* o (s.next ());"
     <<                                                                    endl
     <<     "if (s.more ())"
     <<     "{"
     <<       "std::size_t pos (s.position ());"
     <<       "std::string ov (s.next ());"
     <<       "std::string::size_type p = ov.find ('=');"
     <<                                                                    endl
     <<       "K k = K ();"
     <<       "V v = V ();"
     <<       "std::string kstr (ov, 0, p);"
     <<       "std::string vstr (ov, (p != std::string::npos ? p + 1 : ov.size ()));"
     <<                                                                    endl
     <<       "int ac (2);"
     <<       "char* av[] ="
     <<       "{"
     <<       "const_cast<char*> (o)," << endl
     <<       "0"
     <<       "};";
  if (sp)
    os <<     "bool dummy;";
  os <<       "if (!kstr.empty ())"
     <<       "{"
     <<         "av[1] = const_cast<char*> (kstr.c_str ());"
     <<         "argv_scanner s (0, ac, av, false, pos);"
     <<         "parser<K>::parse (k, " << (sp ? "dummy, " : "") << "s);"
     <<       "}"
     <<       "if (!vstr.empty ())"
     <<       "{"
     <<         "av[1] = const_cast<char*> (vstr.c_str ());"
     <<         "argv_scanner s (0, ac, av, false, pos);"
     <<         "parser<V>::parse (v, " << (sp ? "dummy, " : "") << "s);"
     <<       "}"
     <<       "m.insert (typename std::multimap<K, V, C>::value_type (k, v));"
     <<     "}"
     <<     "else" << endl
     <<       "throw missing_value (o);";
  if (sp)
    os << endl
       <<   "xs = true;";
  os <<   "}";

  if (gen_merge)
    os << "static void" << endl
       << "merge (std::multimap<K, V, C>& b, const std::multimap<K, V, C>& a)"
       << "{"
       <<   "for (typename std::multimap<K, V, C>::const_iterator i (a.begin ()); " << endl
       <<         "i != a.end (); "                                            << endl
       <<         "++i)"                                                       << endl
       <<     "b.insert (typename std::multimap<K, V, C>::value_type (i->first," << endl
      <<                                                             "i->second));" << endl
       << "}";

  os << "};";

  // Parser thunk.
  //
  os << "template <typename X, typename T, T X::*M>" << endl
     << "void" << endl
     << "thunk (X& x, scanner& s)"
     << "{"
     << "parser<T>::parse (x.*M, s);"
     << "}";

  os << "template <typename X, bool X::*M>" << endl
     << "void" << endl
     << "thunk (X& x, scanner& s)"
     << "{"
     <<   "s.next ();"
     <<   "x.*M = true;"
     << "}";

  if (ctx.gen_specifier)
    os << "template <typename X, typename T, T X::*M, bool X::*S>" << endl
       << "void" << endl
       << "thunk (X& x, scanner& s)"
       << "{"
       << "parser<T>::parse (x.*M, x.*S, s);"
       << "}";

  ctx.ns_close (ctx.cli);
}
