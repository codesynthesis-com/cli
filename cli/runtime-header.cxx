// file      : cli/runtime-header.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2010 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include "runtime-header.hxx"

using namespace std;

void
generate_runtime_header (context& ctx)
{
  ostream& os (ctx.os);

  if (ctx.options.generate_file_scanner ())
    os << "#include <deque>" << endl;

  if (ctx.options.generate_description ())
    os << "#include <map>" << endl
       << "#include <vector>" << endl;

  os << "#include <iosfwd>" << endl
     << "#include <string>" << endl
     << "#include <exception>" << endl
     << endl;

  ctx.cli_open ();

  // unknown_mode
  //
  os << "class unknown_mode"
     << "{"
     << "public:" << endl
     << "enum value"
     << "{"
     << "skip," << endl
     << "stop," << endl
     << "fail" << endl
     << "};"
     << "unknown_mode (value v);"
     << endl
     << "operator value () const {return v_;}" // Can't generate outside.
     << "private:" << endl
     << "value v_;"
     << "};";

  // Exceptions.
  //

  os << "// Exceptions." << endl
     << "//" << endl
     << endl;

  os << "class exception: public std::exception"
     << "{"
     << "public:" << endl
     << "virtual void" << endl
     << "print (std::ostream&) const = 0;"
     << "};";

  os << "std::ostream&" << endl
     << "operator<< (std::ostream&, const exception&);"
     << endl;

  os << "class unknown_option: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~unknown_option () throw ();"
     << endl
     << "unknown_option (const std::string& option);"
     << endl
     << "const std::string&" << endl
     << "option () const;"
     << endl
     << "virtual void" << endl
     << "print (std::ostream&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const throw ();"
     << endl
     << "private:" << endl
     << "std::string option_;"
     << "};";

  os << "class unknown_argument: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~unknown_argument () throw ();"
     << endl
     << "unknown_argument (const std::string& argument);"
     << endl
     << "const std::string&" << endl
     << "argument () const;"
     << endl
     << "virtual void" << endl
     << "print (std::ostream&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const throw ();"
     << endl
     << "private:" << endl
     << "std::string argument_;"
     << "};";

  os << "class missing_value: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~missing_value () throw ();"
     << endl
     << "missing_value (const std::string& option);"
     << endl
     << "const std::string&" << endl
     << "option () const;"
     << endl
     << "virtual void" << endl
     << "print (std::ostream&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const throw ();"
     << endl
     << "private:" << endl
     << "std::string option_;"
     << "};";

  os << "class invalid_value: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~invalid_value () throw ();"
     << endl
     << "invalid_value (const std::string& option," << endl
     << "const std::string& value);"
     << endl
     << "const std::string&" << endl
     << "option () const;"
     << endl
     << "const std::string&" << endl
     << "value () const;"
     << endl
     << "virtual void" << endl
     << "print (std::ostream&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const throw ();"
     << endl
     << "private:" << endl
     << "std::string option_;"
     << "std::string value_;"
     << "};";

  os << "class eos_reached: public exception"
     << "{"
     << "public:" << endl
     << "virtual void" << endl
     << "print (std::ostream&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const throw ();"
     << "};";

  if (ctx.options.generate_file_scanner ())
  {
    os << "class file_io_failure: public exception"
       << "{"
       << "public:" << endl
       << "virtual" << endl
       << "~file_io_failure () throw ();"
       << endl
       << "file_io_failure (const std::string& file);"
       << endl
       << "const std::string&" << endl
       << "file () const;"
       << endl
       << "virtual void" << endl
       << "print (std::ostream&) const;"
       << endl
       << "virtual const char*" << endl
       << "what () const throw ();"
       << endl
       << "private:" << endl
       << "std::string file_;"
       << "};";

    os << "class unmatched_quote: public exception"
       << "{"
       << "public:" << endl
       << "virtual" << endl
       << "~unmatched_quote () throw ();"
       << endl
       << "unmatched_quote (const std::string& argument);"
       << endl
       << "const std::string&" << endl
       << "argument () const;"
       << endl
       << "virtual void" << endl
       << "print (std::ostream&) const;"
       << endl
       << "virtual const char*" << endl
       << "what () const throw ();"
       << endl
       << "private:" << endl
       << "std::string argument_;"
       << "};";
  }

  // scanner
  //
  os << "class scanner"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~scanner ();"
     << endl
     << "virtual bool" << endl
     << "more () = 0;"
     << endl
     << "virtual const char*" << endl
     << "peek () = 0;"
     << endl
     << "virtual const char*" << endl
     << "next () = 0;"
     << endl
     << "virtual void" << endl
     << "skip () = 0;"
     << "};";

  // argv_scanner
  //
  os << "class argv_scanner: public scanner"
     << "{"
     << "public:" << endl
     << "argv_scanner (int& argc, char** argv, bool erase = false);"
     << "argv_scanner (int start, int& argc, char** argv, bool erase = false);"
     << endl
     << "int" << endl
     << "end () const;"
     << endl
     << "virtual bool" << endl
     << "more ();"
     << endl
     << "virtual const char*" << endl
     << "peek ();"
     << endl
     << "virtual const char*" << endl
     << "next ();"
     << endl
     << "virtual void" << endl
     << "skip ();"
     << endl
     << "private:" << endl
     << "int i_;"
     << "int& argc_;"
     << "char** argv_;"
     << "bool erase_;"
     << "};";

  // argv_file_scanner
  //
  if (ctx.options.generate_file_scanner ())
  {
    os << "class argv_file_scanner: public argv_scanner"
       << "{"
       << "public:" << endl
       << "argv_file_scanner (int& argc," << endl
       << "char** argv," << endl
       << "const std::string& file_option," << endl
       << "bool erase = false);"
       << endl
       << "argv_file_scanner (int start," << endl
       << "int& argc," << endl
       << "char** argv," << endl
       << "const std::string& file_option," << endl
       << "bool erase = false);"
       << endl
       << "virtual bool" << endl
       << "more ();"
       << endl
       << "virtual const char*" << endl
       << "peek ();"
       << endl
       << "virtual const char*" << endl
       << "next ();"
       << endl
       << "virtual void" << endl
       << "skip ();"
       << endl
       << "private:" << endl
       << "void" << endl
       << "load (const char* file);"
       << endl
       << "typedef argv_scanner base;"
       << endl
       << "const std::string option_;"
       << "std::string hold_;"
       << "std::deque<std::string> args_;";

    if (!ctx.opt_sep.empty ())
      os << "bool skip_;";

    os << "};";
  }

  // Option description.
  //
  if (ctx.options.generate_description ())
  {
    os << "typedef std::vector<std::string> option_names;"
       << endl;

    os << "class option"
       << "{"
       << "public:" << endl
       << endl
       << "const std::string&" << endl
       << "name () const;"
       << endl
       << "const option_names&" << endl
       << "aliases () const;"
       << endl
       << "bool" << endl
       << "flag () const;"
       << endl
       << "const std::string&" << endl
       << "default_value () const;"
       << endl
       << "public:"
       << "option ();"
       << "option (const std::string& name," << endl
       << "const option_names& aliases," << endl
       << "bool flag," << endl
       << "const std::string& default_value);"
       << endl
       << "private:"
       << "std::string name_;"
       << "option_names aliases_;"
       << "bool flag_;"
       << "std::string default_value_;"
       << "};";

    os << "class options: public std::vector<option>"
       << "{"
       << "public:" << endl
       << "typedef std::vector<option> container_type;"
       << endl
       << "container_type::const_iterator" << endl
       << "find (const std::string& name) const;"
       << endl
       << "void" << endl
       << "push_back (const option&);"
       << "private:"
       << "typedef std::map<std::string, container_type::size_type> map_type;"
       << "map_type map_;"
       << "};";
  }

  ctx.cli_close ();
}
