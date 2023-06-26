// file      : cli/runtime-header.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// license   : MIT; see accompanying LICENSE file

#include <cli/runtime-header.hxx>

using namespace std;

void
generate_runtime_header (context& ctx)
{
  ostream& os (ctx.os);

  if (ctx.options.generate_file_scanner ())
    os << "#include <list>" << endl
       << "#include <deque>" << endl;

  if (ctx.options.generate_description ())
    os << "#include <map>" << endl;

  if (ctx.options.generate_description ()  ||
      ctx.options.generate_vector_scanner ())
    os << "#include <vector>" << endl;

  os << "#include <iosfwd>" << endl
     << "#include <string>" << endl
     << "#include <cstddef>" << endl
     << "#include <exception>" << endl
     << endl;

  // VC++ and xlC don't like the (void)x expression if x is a reference
  // to an incomplete type. On the other hand, GCC warns that (void*)&x
  // doesn't have any effect.
  //
  os << "#ifndef CLI_POTENTIALLY_UNUSED" << endl
     << "#  if defined(_MSC_VER) || defined(__xlC__)" << endl
     << "#    define CLI_POTENTIALLY_UNUSED(x) (void*)&x" << endl
     << "#  else" << endl
     << "#    define CLI_POTENTIALLY_UNUSED(x) (void)x" << endl
     << "#  endif" << endl
     << "#endif" << endl
     << endl;

  ctx.ns_open (ctx.cli);

  string const& exp (ctx.exp);
  string const& exp_inl (ctx.exp_inl);

  // usage_para
  //
  if (!ctx.options.suppress_usage ())
    os << "class " << exp_inl << "usage_para"
       << "{"
       << "public:" << endl
       << "enum value"
       << "{"
       << "none," << endl
       << "text," << endl
       << "option" << endl
       << "};"
       << "usage_para (value);"
       << endl
       << "operator value () const {return v_;}" // Can't generate outside.
       << "private:" << endl
       << "value v_;"
       << "};";

  // unknown_mode
  //
  os << "class " << exp_inl << "unknown_mode"
     << "{"
     << "public:" << endl
     << "enum value"
     << "{"
     << "skip," << endl
     << "stop," << endl
     << "fail" << endl
     << "};"
     << "unknown_mode (value);"
     << endl
     << "operator value () const {return v_;}" // Can't generate outside.
     << "private:" << endl
     << "value v_;"
     << "};";

  // Exceptions.
  //

  string const& os_type (ctx.options.ostream_type ());

  const char* nothrow (ctx.options.std () < cxx_version::cxx11
                       ? "throw ()"
                       : "noexcept");

  os << "// Exceptions." << endl
     << "//" << endl
     << endl;

  os << "class " << exp << "exception: public std::exception"
     << "{"
     << "public:" << endl
     << "virtual void" << endl
     << "print (" << os_type << "&) const = 0;"
     << "};";

  os << exp_inl << os_type << "&" << endl
     << "operator<< (" << os_type << "&, const exception&);"
     << endl;

  os << "class " << exp << "unknown_option: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~unknown_option () " << nothrow << ';'
     << endl
     << "unknown_option (const std::string& option);"
     << endl
     << "const std::string&" << endl
     << "option () const;"
     << endl
     << "virtual void" << endl
     << "print (" << os_type << "&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const " << nothrow << ';'
     << endl
     << "private:" << endl
     << "std::string option_;"
     << "};";

  os << "class " << exp << "unknown_argument: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~unknown_argument () " << nothrow << ';'
     << endl
     << "unknown_argument (const std::string& argument);"
     << endl
     << "const std::string&" << endl
     << "argument () const;"
     << endl
     << "virtual void" << endl
     << "print (" << os_type << "&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const " << nothrow << ';'
     << endl
     << "private:" << endl
     << "std::string argument_;"
     << "};";

  os << "class " << exp << "missing_value: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~missing_value () " << nothrow << ';'
     << endl
     << "missing_value (const std::string& option);"
     << endl
     << "const std::string&" << endl
     << "option () const;"
     << endl
     << "virtual void" << endl
     << "print (" << os_type << "&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const " << nothrow << ';'
     << endl
     << "private:" << endl
     << "std::string option_;"
     << "};";

  os << "class " << exp << "invalid_value: public exception"
     << "{"
     << "public:" << endl
     << "virtual" << endl
     << "~invalid_value () " << nothrow << ';'
     << endl
     << "invalid_value (const std::string& option," << endl
     << "const std::string& value," << endl
     << "const std::string& message = std::string ());"
     << endl
     << "const std::string&" << endl
     << "option () const;"
     << endl
     << "const std::string&" << endl
     << "value () const;"
     << endl
     << "const std::string&" << endl
     << "message () const;"
     << endl
     << "virtual void" << endl
     << "print (" << os_type << "&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const " << nothrow << ';'
     << endl
     << "private:" << endl
     << "std::string option_;"
     << "std::string value_;"
     << "std::string message_;"
     << "};";

  os << "class " << exp << "eos_reached: public exception"
     << "{"
     << "public:" << endl
     << "virtual void" << endl
     << "print (" << os_type << "&) const;"
     << endl
     << "virtual const char*" << endl
     << "what () const " << nothrow << ';'
     << "};";

  if (ctx.options.generate_file_scanner ())
  {
    os << "class " << exp << "file_io_failure: public exception"
       << "{"
       << "public:" << endl
       << "virtual" << endl
       << "~file_io_failure () " << nothrow << ';'
       << endl
       << "file_io_failure (const std::string& file);"
       << endl
       << "const std::string&" << endl
       << "file () const;"
       << endl
       << "virtual void" << endl
       << "print (" << os_type << "&) const;"
       << endl
       << "virtual const char*" << endl
       << "what () const " << nothrow << ';'
       << endl
       << "private:" << endl
       << "std::string file_;"
       << "};";

    os << "class " << exp << "unmatched_quote: public exception"
       << "{"
       << "public:" << endl
       << "virtual" << endl
       << "~unmatched_quote () " << nothrow << ';'
       << endl
       << "unmatched_quote (const std::string& argument);"
       << endl
       << "const std::string&" << endl
       << "argument () const;"
       << endl
       << "virtual void" << endl
       << "print (" << os_type << "&) const;"
       << endl
       << "virtual const char*" << endl
       << "what () const " << nothrow << ';'
       << endl
       << "private:" << endl
       << "std::string argument_;"
       << "};";
  }

  if (ctx.options.generate_group_scanner ())
  {
    os << "class " << exp << "unexpected_group: public exception"
       << "{"
       << "public:" << endl
       << "virtual" << endl
       << "~unexpected_group () " << nothrow << ';'
       << endl
       << "unexpected_group (const std::string& argument," << endl
       <<                   "const std::string& group);"
       << endl
       << "const std::string&" << endl
       << "argument () const;"
       << endl
       << "const std::string&" << endl
       << "group () const;"
       << endl
       << "virtual void" << endl
       << "print (std::ostream&) const;"
       << endl
       << "virtual const char*" << endl
       << "what () const " << nothrow << ';'
       << endl
       << "private:" << endl
       << "std::string argument_;"
       << "std::string group_;"
       << "};";

    os << "class " << exp << "group_separator: public exception" << endl
       << "{"
       << "public:" << endl
       << "virtual" << endl
       << "~group_separator () " << nothrow << ';'
       << endl
       << "// Note: either (but not both) can be empty." << endl
       << "//" << endl
       << "group_separator (const std::string& encountered," << endl
       <<                  "const std::string& expected);"
       << endl
       << "const std::string&" << endl
       << "encountered () const;"
       << endl
       << "const std::string&" << endl
       << "expected () const;"
       << endl
       << "virtual void" << endl
       << "print (std::ostream&) const;"
       << endl
       << "virtual const char*" << endl
       << "what () const " << nothrow << ';'
       << endl
       << "private:" << endl
       << "std::string encountered_;"
       << "std::string expected_;"
       << "};";
  }

  // scanner
  //
  os << "// Command line argument scanner interface."                   << endl
     << "//"                                                            << endl
     << "// The values returned by next() are guaranteed to be valid"   << endl
     << "// for the two previous arguments up until a call to a third"  << endl
     << "// peek() or next()."                                          << endl
     << "//"                                                            << endl
     << "// The position() function returns a monotonically-increasing" << endl
     << "// number which, if stored, can later be used to determine the"<< endl
     << "// relative position of the argument returned by the following"<< endl
     << "// call to next(). Note that if multiple scanners are used to" << endl
     << "// extract arguments from multiple sources, then the end"      << endl
     << "// position of the previous scanner should be used as the"     << endl
     << "// start position of the next."                                << endl
     << "//"                                                            << endl
     << "class " << exp << "scanner"
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
     << endl
     << "virtual std::size_t" << endl
     << "position () = 0;"
     << "};";

  // argv_scanner
  //
  os << "class " << exp << "argv_scanner: public scanner"
     << "{"
     << "public:" << endl
     << "argv_scanner (int& argc," << endl
     << "char** argv," << endl
     << "bool erase = false," << endl
     << "std::size_t start_position = 0);"
     << endl
     << "argv_scanner (int start," << endl
     << "int& argc," << endl
     << "char** argv," << endl
     << "bool erase = false," << endl
     << "std::size_t start_position = 0);"
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
     << "virtual std::size_t" << endl
     << "position ();"
     << endl
     << "protected:" << endl
     << "std::size_t start_position_;"
     << "int i_;"
     << "int& argc_;"
     << "char** argv_;"
     << "bool erase_;"
     << "};";

  // vector_scanner
  //
  if (ctx.options.generate_vector_scanner ())
  {
    os << "class " << exp << "vector_scanner: public scanner"
       << "{"
       << "public:" << endl
       << "vector_scanner (const std::vector<std::string>&," << endl
       << "std::size_t start = 0," << endl
       << "std::size_t start_position = 0);"
       << endl
       << "std::size_t" << endl
       << "end () const;"
       << endl
       << "void" << endl
       << "reset (std::size_t start = 0, std::size_t start_position = 0);"
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
       << "virtual std::size_t" << endl
       << "position ();"
       << endl
       << "private:" << endl
       << "std::size_t start_position_;"
       << "const std::vector<std::string>& v_;"
       << "std::size_t i_;"
       << "};";
  }

  // argv_file_scanner
  //
  if (ctx.options.generate_file_scanner ())
  {
    os << "class " << exp << "argv_file_scanner: public argv_scanner"
       << "{"
       << "public:" << endl
       << "argv_file_scanner (int& argc," << endl
       << "char** argv," << endl
       << "const std::string& option," << endl
       << "bool erase = false," << endl
       << "std::size_t start_position = 0);"
       << endl
       << "argv_file_scanner (int start," << endl
       << "int& argc," << endl
       << "char** argv," << endl
       << "const std::string& option," << endl
       << "bool erase = false," << endl
       << "std::size_t start_position = 0);"
       << endl
       << "argv_file_scanner (const std::string& file," << endl
       << "const std::string& option," << endl
       << "std::size_t start_position = 0);"
       << endl
       << "struct option_info"
       << "{"
       << "// If search_func is not NULL, it is called, with the arg" << endl
       << "// value as the second argument, to locate the options file." << endl
       << "// If it returns an empty string, then the file is ignored." << endl
       << "//" << endl
       << "const char* option;"
       << "std::string (*search_func) (const char*, void* arg);"
       << "void* arg;"
       << "};"
       << "argv_file_scanner (int& argc," << endl
       << "char** argv," << endl
       << "const option_info* options," << endl
       << "std::size_t options_count," << endl
       << "bool erase = false," << endl
       << "std::size_t start_position = 0);"
       << endl
       << "argv_file_scanner (int start," << endl
       << "int& argc," << endl
       << "char** argv," << endl
       << "const option_info* options," << endl
       << "std::size_t options_count," << endl
       << "bool erase = false," << endl
       << "std::size_t start_position = 0);"
       << endl
       << "argv_file_scanner (const std::string& file," << endl
       << "const option_info* options = 0," << endl
       << "std::size_t options_count = 0," << endl
       << "std::size_t start_position = 0);"
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
       << "virtual std::size_t" << endl
       << "position ();"
       << endl
       << "// Return the file path if the peeked at argument came from a file and" << endl
       << "// the empty string otherwise. The reference is guaranteed to be valid" << endl
       << "// till the end of the scanner lifetime." << endl
       << "//" << endl
       << "const std::string&" << endl
       << "peek_file ();"
       << endl
       << "// Return the 1-based line number if the peeked at argument came from" << endl
       << "// a file and zero otherwise." << endl
       << "//" << endl
       << "std::size_t" << endl
       << "peek_line ();"
       << endl
       << "private:" << endl
       << "const option_info*" << endl
       << "find (const char*) const;"
       << endl
       << "void" << endl
       << "load (const std::string& file);"
       << endl
       << "typedef argv_scanner base;"
       << endl
       << "const std::string option_;"
       << "option_info option_info_;"
       << "const option_info* options_;"
       << "std::size_t options_count_;"
       << endl
       << "struct arg"
       << "{"
       << "std::string value;"
       << "const std::string* file;"
       << "std::size_t line;"
       << "};"
       << "std::deque<arg> args_;"
       << "std::list<std::string> files_;"
       << endl
       << "// Circular buffer of two arguments." << endl
       << "//" << endl
       << "std::string hold_[2];"
       << "std::size_t i_;";

    if (!ctx.opt_sep.empty ())
      os << endl
         << "bool skip_;";

    os << endl
       << "static int zero_argc_;"
       << "static std::string empty_string_;"
       << "};";
  }

  // group_scanner
  //
  if (ctx.options.generate_group_scanner ())
  {
    os << "class " << exp << "group_scanner: public scanner"
       << "{"
       << "public:" << endl
       << "group_scanner (scanner&);"
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
       << "virtual std::size_t" << endl
       << "position ();"
       << endl
       << "// The group is only available after the call to next()"     << endl
       << "// (and skip() -- in case one needs to make sure the group"  << endl
       << "// was empty, or some such) and is only valid (and must be"  << endl
       << "// handled) until the next call to any of the scanner"       << endl
       << "// functions (including more())."                            << endl
       << "//"                                                          << endl
       << "// Note also that argument positions within each group start"<< endl
       << "// from 0."                                                  << endl
       << "//"                                                          << endl
       << "scanner&" << endl
       << "group ();"
       << endl
       << "// Escape an argument that is a group separator. Return the" << endl
       << "// passed string if no escaping is required."                << endl
       << "//" << endl
       << "static const char*" << endl
       << "escape (const char*);"
       << endl
       << "private:" << endl
       << "enum state"
       << "{"
       << "peeked,  // Argument peeked at with peek()."       << endl
       << "scanned, // Argument scanned with next()."         << endl
       << "skipped, // Argument skipped with skip()/initial." << endl
       << "};"
       << "enum separator"
       << "{"
       << "none,"            << endl
       << "open,      // {"  << endl
       << "close,     // }"  << endl
       << "open_plus, // +{" << endl
       << "close_plus // }+" << endl
       << "};"
       << "static separator" << endl
       << "sense (const char*);"
       << endl

       << "// Scan the leading groups, the next argument/argument pack,"<< endl
       << "// and the trailing groups."                                 << endl
       << "//"                                                          << endl
       << "void" << endl
       << "scan_group ();"
       << endl
       << "scanner& scan_;"
       << "state state_;"
       << endl
       << "// Circular buffer of two arguments." << endl
       << "//" << endl
       << "std::vector<std::string> arg_[2];"
       << "std::size_t i_, j_, pos_;"
       << endl
       << "std::vector<std::string> group_;"
       << "vector_scanner group_scan_;"
       << "};";
  }

  // Option description.
  //
  if (ctx.options.generate_description ())
  {
    os << "typedef std::vector<std::string> option_names;"
       << endl;

    os << "class " << exp_inl << "option"
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
       << "private:" << endl
       << "std::string name_;"
       << "option_names aliases_;"
       << "bool flag_;"
       << "std::string default_value_;"
       << "};";

    os << "class " << exp << "options: public std::vector<option>"
       << "{"
       << "public:" << endl
       << "typedef std::vector<option> container_type;"
       << endl
       << "container_type::const_iterator" << endl
       << "find (const std::string& name) const;"
       << endl
       << "void" << endl
       << "push_back (const option&);"
       << "private:" << endl
       << "typedef std::map<std::string, container_type::size_type> map_type;"
       << "map_type map_;"
       << "};";
  }

  // Parser class template. Provide a forward declaration to allow
  // custom specializations.
  //
  os << "template <typename X>" << endl
     << "struct parser;"
     << endl;

  ctx.ns_close (ctx.cli);
}
