// file      : cli/runtime-source.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2011 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include "runtime-source.hxx"

using namespace std;

void
generate_runtime_source (context& ctx)
{
  ostream& os (ctx.os);

  os << "#include <map>" << endl
     << "#include <set>" << endl
     << "#include <string>" << endl
     << "#include <vector>" << endl
     << "#include <ostream>" << endl
     << "#include <sstream>" << endl;

  if (ctx.options.generate_file_scanner ())
    os << "#include <cstring>" << endl
       << "#include <fstream>" << endl;

  os << endl;

  ctx.cli_open ();

  // unknown_option
  //
  os << "// unknown_option" << endl
     << "//" << endl
     << "unknown_option::" << endl
     << "~unknown_option () throw ()"
     << "{"
     << "}"

     << "void unknown_option::" << endl
     << "print (std::ostream& os) const"
     << "{"
     << "os << \"unknown option '\" << option () << \"'\";"
     << "}"

     << "const char* unknown_option::" << endl
     << "what () const throw ()"
     << "{"
     << "return \"unknown option\";"
     << "}";

  // unknown_argument
  //
  os << "// unknown_argument" << endl
     << "//" << endl
     << "unknown_argument::" << endl
     << "~unknown_argument () throw ()"
     << "{"
     << "}"

     << "void unknown_argument::" << endl
     << "print (std::ostream& os) const"
     << "{"
     << "os << \"unknown argument '\" << argument () << \"'\";"
     << "}"

     << "const char* unknown_argument::" << endl
     << "what () const throw ()"
     << "{"
     << "return \"unknown argument\";"
     << "}";

  // missing_value
  //
  os << "// missing_value" << endl
     << "//" << endl
     << "missing_value::" << endl
     << "~missing_value () throw ()"
     << "{"
     << "}"

     << "void missing_value::" << endl
     << "print (std::ostream& os) const"
     << "{"
     << "os << \"missing value for option '\" << option () << \"'\";"
     << "}"

     << "const char* missing_value::" << endl
     << "what () const throw ()"
     << "{"
     << "return \"missing option value\";"
     << "}";

  // invalid_value
  //
  os << "// invalid_value" << endl
     << "//" << endl
     << "invalid_value::" << endl
     << "~invalid_value () throw ()"
     << "{"
     << "}"

     << "void invalid_value::" << endl
     << "print (std::ostream& os) const"
     << "{"
     << "os << \"invalid value '\" << value () << \"' for option '\"" << endl
     << "   << option () << \"'\";"
     << "}"

     << "const char* invalid_value::" << endl
     << "what () const throw ()"
     << "{"
     << "return \"invalid option value\";"
     << "}";

  // eos_reached
  //
  os << "// eos_reached" << endl
     << "//" << endl
     << "void eos_reached::" << endl
     << "print (std::ostream& os) const"
     << "{"
     << "os << what ();"
     << "}"

     << "const char* eos_reached::" << endl
     << "what () const throw ()"
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
       << "~file_io_failure () throw ()"
       << "{"
       << "}"

       << "void file_io_failure::" << endl
       << "print (std::ostream& os) const"
       << "{"
       << "os << \"unable to open file '\" << file () << \"' or read failure\";"
       << "}"

       << "const char* file_io_failure::" << endl
       << "what () const throw ()"
       << "{"
       << "return \"unable to open file or read failure\";"
       << "}";

    // unmatched_argument
    //
    os << "// unmatched_quote" << endl
       << "//" << endl
       << "unmatched_quote::" << endl
       << "~unmatched_quote () throw ()"
       << "{"
       << "}"

       << "void unmatched_quote::" << endl
       << "print (std::ostream& os) const"
       << "{"
       << "os << \"unmatched quote in argument '\" << argument () << \"'\";"
       << "}"

       << "const char* unmatched_quote::" << endl
       << "what () const throw ()"
       << "{"
       << "return \"unmatched quote\";"
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
     << "return r;"
     << "}"
     << "else" << endl
     << "throw eos_reached ();"
     << "}"

     << "void argv_scanner::" << endl
     << "skip ()"
     << "{"
     << "if (i_ < argc_)" << endl
     << "++i_;"
     << "else" << endl
     << "throw eos_reached ();"
     << "}";

  // argv_file_scanner
  //
  if (ctx.options.generate_file_scanner ())
  {
    bool sep (!ctx.opt_sep.empty ());

    os << "// argv_file_scanner" << endl
       << "//" << endl

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
       << "const option_info* oi;"
       << endl
       << "if (" << (sep ? "!skip_ && " : "") << "(oi = find (a)))"
       << "{"
       << "base::next ();"
       << endl
       << "if (!base::more ())" << endl
       << "throw missing_value (oi->option);"
       << endl
       << "if (oi->search_func != 0)" << endl
       << "load (oi->search_func (base::next (), oi->arg));"
       << "else" << endl
       << "load (base::next ());"
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
       << "return args_.empty () ? base::peek () : args_.front ().c_str ();"
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
       << "hold_.swap (args_.front ());"
       << "args_.pop_front ();"
       << "return hold_.c_str ();"
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
       << "else" << endl
       << "args_.pop_front ();"
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
       << "while (!is.eof ())"
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
       << "string::size_type p (line.find (' '));"
       << endl
       << "if (p == string::npos)"
       << "{";
    if (sep)
      os << "if (!skip_)" << endl
         << "skip_ = (line == \"" << ctx.opt_sep << "\");"
         << endl;
    os << "args_.push_back (line);"
       << "}"
       << "else"
       << "{"
       << "string s1 (line, 0, p);"
       << endl
       << "// Skip leading whitespaces in the argument." << endl
       << "//" << endl
       << "n = line.size ();"
       << "for (++p; p < n; ++p)"
       << "{"
       << "char c (line[p]);"
       << endl
       << "if (c != ' ' && c != '\\t' && c != '\\r')" << endl
       << "break;"
       << "}"
       << "string s2 (line, p);"
       << endl
       << "// If the string is wrapped in quotes, remove them." << endl
       << "//" << endl
       << "n = s2.size ();"
       << "char cf (s2[0]), cl (s2[n - 1]);"
       << endl
       << "if (cf == '\"' || cf == '\\'' || cl == '\"' || cl == '\\'')"
       << "{"
       << "if (n == 1 || cf != cl)" << endl
       << "throw unmatched_quote (s2);"
       << endl
       << "s2 = string (s2, 1, n - 2);"
       << "}"
       << "const option_info* oi;"
       << "if (" << (sep ? "!skip_ && " : "") << "(oi = find (s1.c_str ())))" << endl
       << "{"
       << "if (s2.empty ())" << endl
       << "throw missing_value (oi->option);"
       << endl
       << "if (oi->search_func != 0)" << endl
       << "load (oi->search_func (s2.c_str (), oi->arg));"
       << "else" << endl
       << "load (s2);"
       << "}"
       << "else"
       << "{"
       << "args_.push_back (s1);"
       << "args_.push_back (s2);"
       << "}"
       << "}"
       << "}" // while
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

  bool sp (ctx.specifier);

  // parser class template & its specializations
  //
  os << "template <typename X>" << endl
     << "struct parser"
     << "{"
     << "static void" << endl
     << "parse (X& x, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     << "{"
     << "const char* o (s.next ());"
     << endl
     << "if (s.more ())"
     << "{"
     << "const char* v (s.next ());"
     << "std::istringstream is (v);"
     << "if (!(is >> x && is.eof ()))" << endl
     << "throw invalid_value (o, v);"
     << "}"
     << "else" << endl
     << "throw missing_value (o);";

  if (sp)
    os << endl
       << "xs = true;";

  os << "}"
     << "};";

  // parser<bool>
  //
  os << "template <>" << endl
     << "struct parser<bool>"
     << "{"
     << "static void" << endl
     << "parse (bool& x, scanner& s)"
     << "{"
     << "s.next ();"
     << "x = true;"
     << "}"
     << "};";

  // parser<string>
  //
  os << "template <>" << endl
     << "struct parser<std::string>"
     << "{"
     << "static void" << endl
     << "parse (std::string& x, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     << "{"
     << "const char* o (s.next ());"
     << endl
     << "if (s.more ())" << endl
     << "x = s.next ();"
     << "else" << endl
     << "throw missing_value (o);";

  if (sp)
    os << endl
       << "xs = true;";

  os << "}"
     << "};";

  // parser<std::vector<X>>
  //
  os << "template <typename X>" << endl
     << "struct parser<std::vector<X> >"
     << "{"
     << "static void" << endl
     << "parse (std::vector<X>& c, " << (sp ? "bool& xs, " : "") <<
    "scanner& s)"
     << "{"
     << "X x;";

  if (sp)
    os << "bool dummy;";

  os << "parser<X>::parse (x, " << (sp ? "dummy, " : "") << "s);"
     << "c.push_back (x);";

  if (sp)
    os << "xs = true;";

  os << "}"
     << "};";

  // parser<std::set<X>>
  //
  os << "template <typename X>" << endl
     << "struct parser<std::set<X> >"
     << "{"
     << "static void" << endl
     << "parse (std::set<X>& c, " << (sp ? "bool& xs, " : "") << "scanner& s)"
     << "{"
     << "X x;";

  if (sp)
    os << "bool dummy;";

  os << "parser<X>::parse (x, " << (sp ? "dummy, " : "") << "s);"
     << "c.insert (x);";

  if (sp)
    os << "xs = true;";

  os << "}"
     << "};";

  // parser<std::map<K,V>>
  //
  os << "template <typename K, typename V>" << endl
     << "struct parser<std::map<K, V> >"
     << "{"
     << "static void" << endl
     << "parse (std::map<K, V>& m, " << (sp ? "bool& xs, " : "") <<
    "scanner& s)"
     << "{"
     << "const char* o (s.next ());"
     << endl
     << "if (s.more ())"
     << "{"
     << "std::string ov (s.next ());"
     << "std::string::size_type p = ov.find ('=');"
     << endl
     << "if (p == std::string::npos)"
     << "{"
     << "K k = K ();"
     << endl
     << "if (!ov.empty ())"
     << "{"
     << "std::istringstream ks (ov);"
     << endl
     << "if (!(ks >> k && ks.eof ()))" << endl
     << "throw invalid_value (o, ov);"
     << "}"
     << "m[k] = V ();"
     << "}"
     << "else"
     << "{"
     << "K k = K ();"
     << "V v = V ();"
     << "std::string kstr (ov, 0, p);"
     << "std::string vstr (ov, p + 1);"
     << endl
     << "if (!kstr.empty ())"
     << "{"
     << "std::istringstream ks (kstr);"
     << endl
     << "if (!(ks >> k && ks.eof ()))" << endl
     << "throw invalid_value (o, ov);"
     << "}"
     << "if (!vstr.empty ())"
     << "{"
     << "std::istringstream vs (vstr);"
     << endl
     << "if (!(vs >> v && vs.eof ()))" << endl
     << "throw invalid_value (o, ov);"
     << "}"
     << "m[k] = v;"
     << "}"
     << "}"
     << "else" << endl
     << "throw missing_value (o);";

  if (sp)
    os << endl
       << "xs = true;";

  os << "}"
     << "};";

  // Parser thunk.
  //
  os << "template <typename X, typename T, T X::*M>" << endl
     << "void" << endl
     << "thunk (X& x, scanner& s)"
     << "{"
     << "parser<T>::parse (x.*M, s);"
     << "}";

  if (ctx.specifier)
    os << "template <typename X, typename T, T X::*M, bool X::*S>" << endl
       << "void" << endl
       << "thunk (X& x, scanner& s)"
       << "{"
       << "parser<T>::parse (x.*M, x.*S, s);"
       << "}";

  ctx.cli_close ();
}
