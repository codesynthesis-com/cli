// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

// Begin prologue.
//
//
// End prologue.

#include "options.hxx"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <cstring>
#include <fstream>

namespace cli
{
  // unknown_option
  //
  unknown_option::
  ~unknown_option () throw ()
  {
  }

  void unknown_option::
  print (::std::ostream& os) const
  {
    os << "unknown option '" << option ().c_str () << "'";
  }

  const char* unknown_option::
  what () const throw ()
  {
    return "unknown option";
  }

  // unknown_argument
  //
  unknown_argument::
  ~unknown_argument () throw ()
  {
  }

  void unknown_argument::
  print (::std::ostream& os) const
  {
    os << "unknown argument '" << argument ().c_str () << "'";
  }

  const char* unknown_argument::
  what () const throw ()
  {
    return "unknown argument";
  }

  // missing_value
  //
  missing_value::
  ~missing_value () throw ()
  {
  }

  void missing_value::
  print (::std::ostream& os) const
  {
    os << "missing value for option '" << option ().c_str () << "'";
  }

  const char* missing_value::
  what () const throw ()
  {
    return "missing option value";
  }

  // invalid_value
  //
  invalid_value::
  ~invalid_value () throw ()
  {
  }

  void invalid_value::
  print (::std::ostream& os) const
  {
    os << "invalid value '" << value ().c_str () << "' for option '"
       << option ().c_str () << "'";
  }

  const char* invalid_value::
  what () const throw ()
  {
    return "invalid option value";
  }

  // eos_reached
  //
  void eos_reached::
  print (::std::ostream& os) const
  {
    os << what ();
  }

  const char* eos_reached::
  what () const throw ()
  {
    return "end of argument stream reached";
  }

  // file_io_failure
  //
  file_io_failure::
  ~file_io_failure () throw ()
  {
  }

  void file_io_failure::
  print (::std::ostream& os) const
  {
    os << "unable to open file '" << file ().c_str () << "' or read failure";
  }

  const char* file_io_failure::
  what () const throw ()
  {
    return "unable to open file or read failure";
  }

  // unmatched_quote
  //
  unmatched_quote::
  ~unmatched_quote () throw ()
  {
  }

  void unmatched_quote::
  print (::std::ostream& os) const
  {
    os << "unmatched quote in argument '" << argument ().c_str () << "'";
  }

  const char* unmatched_quote::
  what () const throw ()
  {
    return "unmatched quote";
  }

  // scanner
  //
  scanner::
  ~scanner ()
  {
  }

  // argv_scanner
  //
  bool argv_scanner::
  more ()
  {
    return i_ < argc_;
  }

  const char* argv_scanner::
  peek ()
  {
    if (i_ < argc_)
      return argv_[i_];
    else
      throw eos_reached ();
  }

  const char* argv_scanner::
  next ()
  {
    if (i_ < argc_)
    {
      const char* r (argv_[i_]);

      if (erase_)
      {
        for (int i (i_ + 1); i < argc_; ++i)
          argv_[i - 1] = argv_[i];

        --argc_;
        argv_[argc_] = 0;
      }
      else
        ++i_;

      return r;
    }
    else
      throw eos_reached ();
  }

  void argv_scanner::
  skip ()
  {
    if (i_ < argc_)
      ++i_;
    else
      throw eos_reached ();
  }

  // argv_file_scanner
  //
  bool argv_file_scanner::
  more ()
  {
    if (!args_.empty ())
      return true;

    while (base::more ())
    {
      // See if the next argument is the file option.
      //
      const char* a (base::peek ());
      const option_info* oi;

      if (!skip_ && (oi = find (a)))
      {
        base::next ();

        if (!base::more ())
          throw missing_value (oi->option);

        if (oi->search_func != 0)
        {
          std::string f (oi->search_func (base::next (), oi->arg));

          if (!f.empty ())
            load (f);
        }
        else
          load (base::next ());

        if (!args_.empty ())
          return true;
      }
      else
      {
        if (!skip_)
          skip_ = (std::strcmp (a, "--") == 0);

        return true;
      }
    }

    return false;
  }

  const char* argv_file_scanner::
  peek ()
  {
    if (!more ())
      throw eos_reached ();

    return args_.empty () ? base::peek () : args_.front ().c_str ();
  }

  const char* argv_file_scanner::
  next ()
  {
    if (!more ())
      throw eos_reached ();

    if (args_.empty ())
      return base::next ();
    else
    {
      hold_.swap (args_.front ());
      args_.pop_front ();
      return hold_.c_str ();
    }
  }

  void argv_file_scanner::
  skip ()
  {
    if (!more ())
      throw eos_reached ();

    if (args_.empty ())
      return base::skip ();
    else
      args_.pop_front ();
  }

  const argv_file_scanner::option_info* argv_file_scanner::
  find (const char* a) const
  {
    for (std::size_t i (0); i < options_count_; ++i)
      if (std::strcmp (a, options_[i].option) == 0)
        return &options_[i];

    return 0;
  }

  void argv_file_scanner::
  load (const std::string& file)
  {
    using namespace std;

    ifstream is (file.c_str ());

    if (!is.is_open ())
      throw file_io_failure (file);

    while (!is.eof ())
    {
      string line;
      getline (is, line);

      if (is.fail () && !is.eof ())
        throw file_io_failure (file);

      string::size_type n (line.size ());

      // Trim the line from leading and trailing whitespaces.
      //
      if (n != 0)
      {
        const char* f (line.c_str ());
        const char* l (f + n);

        const char* of (f);
        while (f < l && (*f == ' ' || *f == '\t' || *f == '\r'))
          ++f;

        --l;

        const char* ol (l);
        while (l > f && (*l == ' ' || *l == '\t' || *l == '\r'))
          --l;

        if (f != of || l != ol)
          line = f <= l ? string (f, l - f + 1) : string ();
      }

      // Ignore empty lines, those that start with #.
      //
      if (line.empty () || line[0] == '#')
        continue;

      string::size_type p (line.find (' '));

      if (p == string::npos)
      {
        if (!skip_)
          skip_ = (line == "--");

        args_.push_back (line);
      }
      else
      {
        string s1 (line, 0, p);

        // Skip leading whitespaces in the argument.
        //
        n = line.size ();
        for (++p; p < n; ++p)
        {
          char c (line[p]);

          if (c != ' ' && c != '\t' && c != '\r')
            break;
        }

        string s2 (line, p);

        // If the string is wrapped in quotes, remove them.
        //
        n = s2.size ();
        char cf (s2[0]), cl (s2[n - 1]);

        if (cf == '"' || cf == '\'' || cl == '"' || cl == '\'')
        {
          if (n == 1 || cf != cl)
            throw unmatched_quote (s2);

          s2 = string (s2, 1, n - 2);
        }

        const option_info* oi;
        if (!skip_ && (oi = find (s1.c_str ())))
        {
          if (s2.empty ())
            throw missing_value (oi->option);

          if (oi->search_func != 0)
          {
            std::string f (oi->search_func (s2.c_str (), oi->arg));

            if (!f.empty ())
              load (f);
          }
          else
            load (s2);
        }
        else
        {
          args_.push_back (s1);
          args_.push_back (s2);
        }
      }
    }
  }

  template <typename X>
  struct parser
  {
    static void
    parse (X& x, scanner& s)
    {
      std::string o (s.next ());

      if (s.more ())
      {
        std::string v (s.next ());
        std::istringstream is (v);
        if (!(is >> x && is.eof ()))
          throw invalid_value (o, v);
      }
      else
        throw missing_value (o);
    }
  };

  template <>
  struct parser<bool>
  {
    static void
    parse (bool& x, scanner& s)
    {
      s.next ();
      x = true;
    }
  };

  template <>
  struct parser<std::string>
  {
    static void
    parse (std::string& x, scanner& s)
    {
      const char* o (s.next ());

      if (s.more ())
        x = s.next ();
      else
        throw missing_value (o);
    }
  };

  template <typename X>
  struct parser<std::vector<X> >
  {
    static void
    parse (std::vector<X>& c, scanner& s)
    {
      X x;
      parser<X>::parse (x, s);
      c.push_back (x);
    }
  };

  template <typename X>
  struct parser<std::set<X> >
  {
    static void
    parse (std::set<X>& c, scanner& s)
    {
      X x;
      parser<X>::parse (x, s);
      c.insert (x);
    }
  };

  template <typename K, typename V>
  struct parser<std::map<K, V> >
  {
    static void
    parse (std::map<K, V>& m, scanner& s)
    {
      const char* o (s.next ());

      if (s.more ())
      {
        std::string ov (s.next ());
        std::string::size_type p = ov.find ('=');

        K k = K ();
        V v = V ();
        std::string kstr (ov, 0, p);
        std::string vstr (ov, (p != std::string::npos ? p + 1 : ov.size ()));

        int ac (2);
        char* av[] = 
        {
          const_cast<char*> (o), 0
        };

        if (!kstr.empty ())
        {
          av[1] = const_cast<char*> (kstr.c_str ());
          argv_scanner s (0, ac, av);
          parser<K>::parse (k, s);
        }

        if (!vstr.empty ())
        {
          av[1] = const_cast<char*> (vstr.c_str ());
          argv_scanner s (0, ac, av);
          parser<V>::parse (v, s);
        }

        m[k] = v;
      }
      else
        throw missing_value (o);
    }
  };

  template <typename X, typename T, T X::*M>
  void
  thunk (X& x, scanner& s)
  {
    parser<T>::parse (x.*M, s);
  }
}

#include <map>
#include <cstring>

// options
//

options::
options ()
: help_ (),
  version_ (),
  include_path_ (),
  output_dir_ (),
  generate_modifier_ (),
  generate_specifier_ (),
  generate_parse_ (),
  generate_description_ (),
  generate_file_scanner_ (),
  suppress_inline_ (),
  cli_namespace_ ("::cli"),
  ostream_type_ ("::std::ostream"),
  generate_cxx_ (),
  generate_man_ (),
  generate_html_ (),
  stdout__ (),
  suppress_undocumented_ (),
  suppress_usage_ (),
  long_usage_ (),
  short_usage_ (),
  option_length_ (0),
  ansi_color_ (),
  exclude_base_ (),
  class__ (),
  docvar_ (),
  hxx_prologue_ (),
  ixx_prologue_ (),
  cxx_prologue_ (),
  man_prologue_ (),
  html_prologue_ (),
  hxx_epilogue_ (),
  ixx_epilogue_ (),
  cxx_epilogue_ (),
  man_epilogue_ (),
  html_epilogue_ (),
  hxx_prologue_file_ (),
  ixx_prologue_file_ (),
  cxx_prologue_file_ (),
  man_prologue_file_ (),
  html_prologue_file_ (),
  hxx_epilogue_file_ (),
  ixx_epilogue_file_ (),
  cxx_epilogue_file_ (),
  man_epilogue_file_ (),
  html_epilogue_file_ (),
  hxx_suffix_ (".hxx"),
  ixx_suffix_ (".ixx"),
  cxx_suffix_ (".cxx"),
  man_suffix_ (".1"),
  html_suffix_ (".html"),
  option_prefix_ ("-"),
  option_separator_ ("--"),
  include_with_brackets_ (),
  include_prefix_ (),
  guard_prefix_ (),
  reserved_name_ (),
  options_file_ ()
{
}

options::
options (int& argc,
         char** argv,
         bool erase,
         ::cli::unknown_mode opt,
         ::cli::unknown_mode arg)
: help_ (),
  version_ (),
  include_path_ (),
  output_dir_ (),
  generate_modifier_ (),
  generate_specifier_ (),
  generate_parse_ (),
  generate_description_ (),
  generate_file_scanner_ (),
  suppress_inline_ (),
  cli_namespace_ ("::cli"),
  ostream_type_ ("::std::ostream"),
  generate_cxx_ (),
  generate_man_ (),
  generate_html_ (),
  stdout__ (),
  suppress_undocumented_ (),
  suppress_usage_ (),
  long_usage_ (),
  short_usage_ (),
  option_length_ (0),
  ansi_color_ (),
  exclude_base_ (),
  class__ (),
  docvar_ (),
  hxx_prologue_ (),
  ixx_prologue_ (),
  cxx_prologue_ (),
  man_prologue_ (),
  html_prologue_ (),
  hxx_epilogue_ (),
  ixx_epilogue_ (),
  cxx_epilogue_ (),
  man_epilogue_ (),
  html_epilogue_ (),
  hxx_prologue_file_ (),
  ixx_prologue_file_ (),
  cxx_prologue_file_ (),
  man_prologue_file_ (),
  html_prologue_file_ (),
  hxx_epilogue_file_ (),
  ixx_epilogue_file_ (),
  cxx_epilogue_file_ (),
  man_epilogue_file_ (),
  html_epilogue_file_ (),
  hxx_suffix_ (".hxx"),
  ixx_suffix_ (".ixx"),
  cxx_suffix_ (".cxx"),
  man_suffix_ (".1"),
  html_suffix_ (".html"),
  option_prefix_ ("-"),
  option_separator_ ("--"),
  include_with_brackets_ (),
  include_prefix_ (),
  guard_prefix_ (),
  reserved_name_ (),
  options_file_ ()
{
  ::cli::argv_scanner s (argc, argv, erase);
  _parse (s, opt, arg);
}

options::
options (int start,
         int& argc,
         char** argv,
         bool erase,
         ::cli::unknown_mode opt,
         ::cli::unknown_mode arg)
: help_ (),
  version_ (),
  include_path_ (),
  output_dir_ (),
  generate_modifier_ (),
  generate_specifier_ (),
  generate_parse_ (),
  generate_description_ (),
  generate_file_scanner_ (),
  suppress_inline_ (),
  cli_namespace_ ("::cli"),
  ostream_type_ ("::std::ostream"),
  generate_cxx_ (),
  generate_man_ (),
  generate_html_ (),
  stdout__ (),
  suppress_undocumented_ (),
  suppress_usage_ (),
  long_usage_ (),
  short_usage_ (),
  option_length_ (0),
  ansi_color_ (),
  exclude_base_ (),
  class__ (),
  docvar_ (),
  hxx_prologue_ (),
  ixx_prologue_ (),
  cxx_prologue_ (),
  man_prologue_ (),
  html_prologue_ (),
  hxx_epilogue_ (),
  ixx_epilogue_ (),
  cxx_epilogue_ (),
  man_epilogue_ (),
  html_epilogue_ (),
  hxx_prologue_file_ (),
  ixx_prologue_file_ (),
  cxx_prologue_file_ (),
  man_prologue_file_ (),
  html_prologue_file_ (),
  hxx_epilogue_file_ (),
  ixx_epilogue_file_ (),
  cxx_epilogue_file_ (),
  man_epilogue_file_ (),
  html_epilogue_file_ (),
  hxx_suffix_ (".hxx"),
  ixx_suffix_ (".ixx"),
  cxx_suffix_ (".cxx"),
  man_suffix_ (".1"),
  html_suffix_ (".html"),
  option_prefix_ ("-"),
  option_separator_ ("--"),
  include_with_brackets_ (),
  include_prefix_ (),
  guard_prefix_ (),
  reserved_name_ (),
  options_file_ ()
{
  ::cli::argv_scanner s (start, argc, argv, erase);
  _parse (s, opt, arg);
}

options::
options (int& argc,
         char** argv,
         int& end,
         bool erase,
         ::cli::unknown_mode opt,
         ::cli::unknown_mode arg)
: help_ (),
  version_ (),
  include_path_ (),
  output_dir_ (),
  generate_modifier_ (),
  generate_specifier_ (),
  generate_parse_ (),
  generate_description_ (),
  generate_file_scanner_ (),
  suppress_inline_ (),
  cli_namespace_ ("::cli"),
  ostream_type_ ("::std::ostream"),
  generate_cxx_ (),
  generate_man_ (),
  generate_html_ (),
  stdout__ (),
  suppress_undocumented_ (),
  suppress_usage_ (),
  long_usage_ (),
  short_usage_ (),
  option_length_ (0),
  ansi_color_ (),
  exclude_base_ (),
  class__ (),
  docvar_ (),
  hxx_prologue_ (),
  ixx_prologue_ (),
  cxx_prologue_ (),
  man_prologue_ (),
  html_prologue_ (),
  hxx_epilogue_ (),
  ixx_epilogue_ (),
  cxx_epilogue_ (),
  man_epilogue_ (),
  html_epilogue_ (),
  hxx_prologue_file_ (),
  ixx_prologue_file_ (),
  cxx_prologue_file_ (),
  man_prologue_file_ (),
  html_prologue_file_ (),
  hxx_epilogue_file_ (),
  ixx_epilogue_file_ (),
  cxx_epilogue_file_ (),
  man_epilogue_file_ (),
  html_epilogue_file_ (),
  hxx_suffix_ (".hxx"),
  ixx_suffix_ (".ixx"),
  cxx_suffix_ (".cxx"),
  man_suffix_ (".1"),
  html_suffix_ (".html"),
  option_prefix_ ("-"),
  option_separator_ ("--"),
  include_with_brackets_ (),
  include_prefix_ (),
  guard_prefix_ (),
  reserved_name_ (),
  options_file_ ()
{
  ::cli::argv_scanner s (argc, argv, erase);
  _parse (s, opt, arg);
  end = s.end ();
}

options::
options (int start,
         int& argc,
         char** argv,
         int& end,
         bool erase,
         ::cli::unknown_mode opt,
         ::cli::unknown_mode arg)
: help_ (),
  version_ (),
  include_path_ (),
  output_dir_ (),
  generate_modifier_ (),
  generate_specifier_ (),
  generate_parse_ (),
  generate_description_ (),
  generate_file_scanner_ (),
  suppress_inline_ (),
  cli_namespace_ ("::cli"),
  ostream_type_ ("::std::ostream"),
  generate_cxx_ (),
  generate_man_ (),
  generate_html_ (),
  stdout__ (),
  suppress_undocumented_ (),
  suppress_usage_ (),
  long_usage_ (),
  short_usage_ (),
  option_length_ (0),
  ansi_color_ (),
  exclude_base_ (),
  class__ (),
  docvar_ (),
  hxx_prologue_ (),
  ixx_prologue_ (),
  cxx_prologue_ (),
  man_prologue_ (),
  html_prologue_ (),
  hxx_epilogue_ (),
  ixx_epilogue_ (),
  cxx_epilogue_ (),
  man_epilogue_ (),
  html_epilogue_ (),
  hxx_prologue_file_ (),
  ixx_prologue_file_ (),
  cxx_prologue_file_ (),
  man_prologue_file_ (),
  html_prologue_file_ (),
  hxx_epilogue_file_ (),
  ixx_epilogue_file_ (),
  cxx_epilogue_file_ (),
  man_epilogue_file_ (),
  html_epilogue_file_ (),
  hxx_suffix_ (".hxx"),
  ixx_suffix_ (".ixx"),
  cxx_suffix_ (".cxx"),
  man_suffix_ (".1"),
  html_suffix_ (".html"),
  option_prefix_ ("-"),
  option_separator_ ("--"),
  include_with_brackets_ (),
  include_prefix_ (),
  guard_prefix_ (),
  reserved_name_ (),
  options_file_ ()
{
  ::cli::argv_scanner s (start, argc, argv, erase);
  _parse (s, opt, arg);
  end = s.end ();
}

options::
options (::cli::scanner& s,
         ::cli::unknown_mode opt,
         ::cli::unknown_mode arg)
: help_ (),
  version_ (),
  include_path_ (),
  output_dir_ (),
  generate_modifier_ (),
  generate_specifier_ (),
  generate_parse_ (),
  generate_description_ (),
  generate_file_scanner_ (),
  suppress_inline_ (),
  cli_namespace_ ("::cli"),
  ostream_type_ ("::std::ostream"),
  generate_cxx_ (),
  generate_man_ (),
  generate_html_ (),
  stdout__ (),
  suppress_undocumented_ (),
  suppress_usage_ (),
  long_usage_ (),
  short_usage_ (),
  option_length_ (0),
  ansi_color_ (),
  exclude_base_ (),
  class__ (),
  docvar_ (),
  hxx_prologue_ (),
  ixx_prologue_ (),
  cxx_prologue_ (),
  man_prologue_ (),
  html_prologue_ (),
  hxx_epilogue_ (),
  ixx_epilogue_ (),
  cxx_epilogue_ (),
  man_epilogue_ (),
  html_epilogue_ (),
  hxx_prologue_file_ (),
  ixx_prologue_file_ (),
  cxx_prologue_file_ (),
  man_prologue_file_ (),
  html_prologue_file_ (),
  hxx_epilogue_file_ (),
  ixx_epilogue_file_ (),
  cxx_epilogue_file_ (),
  man_epilogue_file_ (),
  html_epilogue_file_ (),
  hxx_suffix_ (".hxx"),
  ixx_suffix_ (".ixx"),
  cxx_suffix_ (".cxx"),
  man_suffix_ (".1"),
  html_suffix_ (".html"),
  option_prefix_ ("-"),
  option_separator_ ("--"),
  include_with_brackets_ (),
  include_prefix_ (),
  guard_prefix_ (),
  reserved_name_ (),
  options_file_ ()
{
  _parse (s, opt, arg);
}

void options::
print_usage (::std::ostream& os)
{
  os << "--help                       Print usage information and exit." << ::std::endl;

  os << "--version                    Print version and exit." << ::std::endl;

  os << "--include-path|-I <dir>      Search <dir> for bracket-included ('<>') options" << ::std::endl
     << "                             files." << ::std::endl;

  os << "--output-dir|-o <dir>        Write the generated files to <dir> instead of the" << ::std::endl
     << "                             current directory." << ::std::endl;

  os << "--generate-modifier          Generate option value modifiers in addition to" << ::std::endl
     << "                             accessors." << ::std::endl;

  os << "--generate-specifier         Generate functions for determining whether the" << ::std::endl
     << "                             option was specified on the command line." << ::std::endl;

  os << "--generate-parse             Generate 'parse()' functions instead of parsing" << ::std::endl
     << "                             constructors." << ::std::endl;

  os << "--generate-description       Generate the option description list that can be" << ::std::endl
     << "                             examined at runtime." << ::std::endl;

  os << "--generate-file-scanner      Generate the 'argv_file_scanner' implementation." << ::std::endl;

  os << "--suppress-inline            Generate all functions non-inline." << ::std::endl;

  os << "--cli-namespace <ns>         Generate the CLI support types in the <ns>" << ::std::endl
     << "                             namespace ('cli' by default)." << ::std::endl;

  os << "--ostream-type <type>        Output stream type instead of the default" << ::std::endl
     << "                             'std::ostream' that should be used to print usage" << ::std::endl
     << "                             and exception information." << ::std::endl;

  os << "--generate-cxx               Generate C++ code." << ::std::endl;

  os << "--generate-man               Generate documentation in the man page format." << ::std::endl;

  os << "--generate-html              Generate documentation in the HTML format." << ::std::endl;

  os << "--stdout                     Write output to STDOUT instead of a file." << ::std::endl;

  os << "--suppress-undocumented      Suppress the generation of documentation entries" << ::std::endl
     << "                             for undocumented options." << ::std::endl;

  os << "--suppress-usage             Suppress the generation of the usage printing" << ::std::endl
     << "                             code." << ::std::endl;

  os << "--long-usage                 If no short documentation string is provided, use" << ::std::endl
     << "                             the complete long documentation string in usage." << ::std::endl;

  os << "--short-usage                If specified together with '--long-usage'," << ::std::endl
     << "                             generate both short and long usage versions." << ::std::endl;

  os << "--option-length <len>        Indent option descriptions <len> characters when" << ::std::endl
     << "                             printing usage." << ::std::endl;

  os << "--ansi-color                 Use ANSI color escape sequences when printing" << ::std::endl
     << "                             usage." << ::std::endl;

  os << "--exclude-base               Exclude base class information from usage and" << ::std::endl
     << "                             documentation." << ::std::endl;

  os << "--class <fq-name>            Generate the man page or HTML documentation only" << ::std::endl
     << "                             for the <fq-name> options class." << ::std::endl;

  os << "--docvar|-v <name>=<val>     Set documentation variable <name> to the value" << ::std::endl
     << "                             <val>." << ::std::endl;

  os << "--hxx-prologue <text>        Insert <text> at the beginning of the generated" << ::std::endl
     << "                             C++ header file." << ::std::endl;

  os << "--ixx-prologue <text>        Insert <text> at the beginning of the generated" << ::std::endl
     << "                             C++ inline file." << ::std::endl;

  os << "--cxx-prologue <text>        Insert <text> at the beginning of the generated" << ::std::endl
     << "                             C++ source file." << ::std::endl;

  os << "--man-prologue <text>        Insert <text> at the beginning of the generated" << ::std::endl
     << "                             man page file." << ::std::endl;

  os << "--html-prologue <text>       Insert <text> at the beginning of the generated" << ::std::endl
     << "                             HTML file." << ::std::endl;

  os << "--hxx-epilogue <text>        Insert <text> at the end of the generated C++" << ::std::endl
     << "                             header file." << ::std::endl;

  os << "--ixx-epilogue <text>        Insert <text> at the end of the generated C++" << ::std::endl
     << "                             inline file." << ::std::endl;

  os << "--cxx-epilogue <text>        Insert <text> at the end of the generated C++" << ::std::endl
     << "                             source file." << ::std::endl;

  os << "--man-epilogue <text>        Insert <text> at the end of the generated man page" << ::std::endl
     << "                             text." << ::std::endl;

  os << "--html-epilogue <text>       Insert <text> at the end of the generated HTML" << ::std::endl
     << "                             text." << ::std::endl;

  os << "--hxx-prologue-file <file>   Insert the content of <file> at the beginning of" << ::std::endl
     << "                             the generated C++ header file." << ::std::endl;

  os << "--ixx-prologue-file <file>   Insert the content of <file> at the beginning of" << ::std::endl
     << "                             the generated C++ inline file." << ::std::endl;

  os << "--cxx-prologue-file <file>   Insert the content of <file> at the beginning of" << ::std::endl
     << "                             the generated C++ source file." << ::std::endl;

  os << "--man-prologue-file <file>   Insert the content of <file> at the beginning of" << ::std::endl
     << "                             the generated man page file." << ::std::endl;

  os << "--html-prologue-file <file>  Insert the content of <file> at the beginning of" << ::std::endl
     << "                             the generated HTML file." << ::std::endl;

  os << "--hxx-epilogue-file <file>   Insert the content of <file> at the end of the" << ::std::endl
     << "                             generated C++ header file." << ::std::endl;

  os << "--ixx-epilogue-file <file>   Insert the content of <file> at the end of the" << ::std::endl
     << "                             generated C++ inline file." << ::std::endl;

  os << "--cxx-epilogue-file <file>   Insert the content of <file> at the end of the" << ::std::endl
     << "                             generated C++ source file." << ::std::endl;

  os << "--man-epilogue-file <file>   Insert the content of <file> at the end of the" << ::std::endl
     << "                             generated man page file." << ::std::endl;

  os << "--html-epilogue-file <file>  Insert the content of <file> at the end of the" << ::std::endl
     << "                             generated HTML file." << ::std::endl;

  os << "--hxx-suffix <suffix>        Use <suffix> instead of the default '.hxx' to" << ::std::endl
     << "                             construct the name of the generated header file." << ::std::endl;

  os << "--ixx-suffix <suffix>        Use <suffix> instead of the default '.ixx' to" << ::std::endl
     << "                             construct the name of the generated inline file." << ::std::endl;

  os << "--cxx-suffix <suffix>        Use <suffix> instead of the default '.cxx' to" << ::std::endl
     << "                             construct the name of the generated source file." << ::std::endl;

  os << "--man-suffix <suffix>        Use <suffix> instead of the default '.1' to" << ::std::endl
     << "                             construct the name of the generated man page file." << ::std::endl;

  os << "--html-suffix <suffix>       Use <suffix> instead of the default '.html' to" << ::std::endl
     << "                             construct the name of the generated HTML file." << ::std::endl;

  os << "--option-prefix <prefix>     Use <prefix> instead of the default '-' as an" << ::std::endl
     << "                             option prefix." << ::std::endl;

  os << "--option-separator <sep>     Use <sep> instead of the default '--' as an" << ::std::endl
     << "                             optional separator between options and arguments." << ::std::endl;

  os << "--include-with-brackets      Use angle brackets (<>) instead of quotes (\"\") in" << ::std::endl
     << "                             the generated '#include' directives." << ::std::endl;

  os << "--include-prefix <prefix>    Add <prefix> to the generated '#include' directive" << ::std::endl
     << "                             paths." << ::std::endl;

  os << "--guard-prefix <prefix>      Add <prefix> to the generated header inclusion" << ::std::endl
     << "                             guards." << ::std::endl;

  os << "--reserved-name <name>=<rep> Add <name> with an optional <rep> replacement to" << ::std::endl
     << "                             the list of names that should not be used as" << ::std::endl
     << "                             identifiers." << ::std::endl;

  os << "--options-file <file>        Read additional options from <file> with each" << ::std::endl
     << "                             option appearing on a separate line optionally" << ::std::endl
     << "                             followed by space and an option value." << ::std::endl;
}

typedef
std::map<std::string, void (*) (options&, ::cli::scanner&)>
_cli_options_map;

static _cli_options_map _cli_options_map_;

struct _cli_options_map_init
{
  _cli_options_map_init ()
  {
    _cli_options_map_["--help"] = 
    &::cli::thunk< options, bool, &options::help_ >;
    _cli_options_map_["--version"] = 
    &::cli::thunk< options, bool, &options::version_ >;
    _cli_options_map_["--include-path"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::include_path_ >;
    _cli_options_map_["-I"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::include_path_ >;
    _cli_options_map_["--output-dir"] = 
    &::cli::thunk< options, std::string, &options::output_dir_ >;
    _cli_options_map_["-o"] = 
    &::cli::thunk< options, std::string, &options::output_dir_ >;
    _cli_options_map_["--generate-modifier"] = 
    &::cli::thunk< options, bool, &options::generate_modifier_ >;
    _cli_options_map_["--generate-specifier"] = 
    &::cli::thunk< options, bool, &options::generate_specifier_ >;
    _cli_options_map_["--generate-parse"] = 
    &::cli::thunk< options, bool, &options::generate_parse_ >;
    _cli_options_map_["--generate-description"] = 
    &::cli::thunk< options, bool, &options::generate_description_ >;
    _cli_options_map_["--generate-file-scanner"] = 
    &::cli::thunk< options, bool, &options::generate_file_scanner_ >;
    _cli_options_map_["--suppress-inline"] = 
    &::cli::thunk< options, bool, &options::suppress_inline_ >;
    _cli_options_map_["--cli-namespace"] = 
    &::cli::thunk< options, std::string, &options::cli_namespace_ >;
    _cli_options_map_["--ostream-type"] = 
    &::cli::thunk< options, std::string, &options::ostream_type_ >;
    _cli_options_map_["--generate-cxx"] = 
    &::cli::thunk< options, bool, &options::generate_cxx_ >;
    _cli_options_map_["--generate-man"] = 
    &::cli::thunk< options, bool, &options::generate_man_ >;
    _cli_options_map_["--generate-html"] = 
    &::cli::thunk< options, bool, &options::generate_html_ >;
    _cli_options_map_["--stdout"] = 
    &::cli::thunk< options, bool, &options::stdout__ >;
    _cli_options_map_["--suppress-undocumented"] = 
    &::cli::thunk< options, bool, &options::suppress_undocumented_ >;
    _cli_options_map_["--suppress-usage"] = 
    &::cli::thunk< options, bool, &options::suppress_usage_ >;
    _cli_options_map_["--long-usage"] = 
    &::cli::thunk< options, bool, &options::long_usage_ >;
    _cli_options_map_["--short-usage"] = 
    &::cli::thunk< options, bool, &options::short_usage_ >;
    _cli_options_map_["--option-length"] = 
    &::cli::thunk< options, std::size_t, &options::option_length_ >;
    _cli_options_map_["--ansi-color"] = 
    &::cli::thunk< options, bool, &options::ansi_color_ >;
    _cli_options_map_["--exclude-base"] = 
    &::cli::thunk< options, bool, &options::exclude_base_ >;
    _cli_options_map_["--class"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::class__ >;
    _cli_options_map_["--docvar"] = 
    &::cli::thunk< options, std::map<std::string, std::string>, &options::docvar_ >;
    _cli_options_map_["-v"] = 
    &::cli::thunk< options, std::map<std::string, std::string>, &options::docvar_ >;
    _cli_options_map_["--hxx-prologue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::hxx_prologue_ >;
    _cli_options_map_["--ixx-prologue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::ixx_prologue_ >;
    _cli_options_map_["--cxx-prologue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::cxx_prologue_ >;
    _cli_options_map_["--man-prologue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::man_prologue_ >;
    _cli_options_map_["--html-prologue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::html_prologue_ >;
    _cli_options_map_["--hxx-epilogue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::hxx_epilogue_ >;
    _cli_options_map_["--ixx-epilogue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::ixx_epilogue_ >;
    _cli_options_map_["--cxx-epilogue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::cxx_epilogue_ >;
    _cli_options_map_["--man-epilogue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::man_epilogue_ >;
    _cli_options_map_["--html-epilogue"] = 
    &::cli::thunk< options, std::vector<std::string>, &options::html_epilogue_ >;
    _cli_options_map_["--hxx-prologue-file"] = 
    &::cli::thunk< options, std::string, &options::hxx_prologue_file_ >;
    _cli_options_map_["--ixx-prologue-file"] = 
    &::cli::thunk< options, std::string, &options::ixx_prologue_file_ >;
    _cli_options_map_["--cxx-prologue-file"] = 
    &::cli::thunk< options, std::string, &options::cxx_prologue_file_ >;
    _cli_options_map_["--man-prologue-file"] = 
    &::cli::thunk< options, std::string, &options::man_prologue_file_ >;
    _cli_options_map_["--html-prologue-file"] = 
    &::cli::thunk< options, std::string, &options::html_prologue_file_ >;
    _cli_options_map_["--hxx-epilogue-file"] = 
    &::cli::thunk< options, std::string, &options::hxx_epilogue_file_ >;
    _cli_options_map_["--ixx-epilogue-file"] = 
    &::cli::thunk< options, std::string, &options::ixx_epilogue_file_ >;
    _cli_options_map_["--cxx-epilogue-file"] = 
    &::cli::thunk< options, std::string, &options::cxx_epilogue_file_ >;
    _cli_options_map_["--man-epilogue-file"] = 
    &::cli::thunk< options, std::string, &options::man_epilogue_file_ >;
    _cli_options_map_["--html-epilogue-file"] = 
    &::cli::thunk< options, std::string, &options::html_epilogue_file_ >;
    _cli_options_map_["--hxx-suffix"] = 
    &::cli::thunk< options, std::string, &options::hxx_suffix_ >;
    _cli_options_map_["--ixx-suffix"] = 
    &::cli::thunk< options, std::string, &options::ixx_suffix_ >;
    _cli_options_map_["--cxx-suffix"] = 
    &::cli::thunk< options, std::string, &options::cxx_suffix_ >;
    _cli_options_map_["--man-suffix"] = 
    &::cli::thunk< options, std::string, &options::man_suffix_ >;
    _cli_options_map_["--html-suffix"] = 
    &::cli::thunk< options, std::string, &options::html_suffix_ >;
    _cli_options_map_["--option-prefix"] = 
    &::cli::thunk< options, std::string, &options::option_prefix_ >;
    _cli_options_map_["--option-separator"] = 
    &::cli::thunk< options, std::string, &options::option_separator_ >;
    _cli_options_map_["--include-with-brackets"] = 
    &::cli::thunk< options, bool, &options::include_with_brackets_ >;
    _cli_options_map_["--include-prefix"] = 
    &::cli::thunk< options, std::string, &options::include_prefix_ >;
    _cli_options_map_["--guard-prefix"] = 
    &::cli::thunk< options, std::string, &options::guard_prefix_ >;
    _cli_options_map_["--reserved-name"] = 
    &::cli::thunk< options, std::map<std::string, std::string>, &options::reserved_name_ >;
    _cli_options_map_["--options-file"] = 
    &::cli::thunk< options, std::string, &options::options_file_ >;
  }
};

static _cli_options_map_init _cli_options_map_init_;

bool options::
_parse (const char* o, ::cli::scanner& s)
{
  _cli_options_map::const_iterator i (_cli_options_map_.find (o));

  if (i != _cli_options_map_.end ())
  {
    (*(i->second)) (*this, s);
    return true;
  }

  return false;
}

void options::
_parse (::cli::scanner& s,
        ::cli::unknown_mode opt_mode,
        ::cli::unknown_mode arg_mode)
{
  bool opt = true;

  while (s.more ())
  {
    const char* o = s.peek ();

    if (std::strcmp (o, "--") == 0)
    {
      s.skip ();
      opt = false;
      continue;
    }

    if (opt && _parse (o, s));
    else if (opt && std::strncmp (o, "-", 1) == 0 && o[1] != '\0')
    {
      switch (opt_mode)
      {
        case ::cli::unknown_mode::skip:
        {
          s.skip ();
          continue;
        }
        case ::cli::unknown_mode::stop:
        {
          break;
        }
        case ::cli::unknown_mode::fail:
        {
          throw ::cli::unknown_option (o);
        }
      }

      break;
    }
    else
    {
      switch (arg_mode)
      {
        case ::cli::unknown_mode::skip:
        {
          s.skip ();
          continue;
        }
        case ::cli::unknown_mode::stop:
        {
          break;
        }
        case ::cli::unknown_mode::fail:
        {
          throw ::cli::unknown_argument (o);
        }
      }

      break;
    }
  }
}

// Begin epilogue.
//
//
// End epilogue.

