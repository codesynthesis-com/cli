// This code was generated by CLI, a command line interface
// compiler for C++.
//

namespace cli
{
  // unknown_mode
  //
  inline unknown_mode::
  unknown_mode (value v)
  : v_ (v)
  {
  }

  // exception
  //
  inline std::ostream&
  operator<< (std::ostream& os, const exception& e)
  {
    e.print (os);
    return os;
  }

  // unknown_option
  //
  inline unknown_option::
  unknown_option (const std::string& option)
  : option_ (option)
  {
  }

  inline const std::string& unknown_option::
  option () const
  {
    return option_;
  }

  // unknown_argument
  //
  inline unknown_argument::
  unknown_argument (const std::string& argument)
  : argument_ (argument)
  {
  }

  inline const std::string& unknown_argument::
  argument () const
  {
    return argument_;
  }

  // missing_value
  //
  inline missing_value::
  missing_value (const std::string& option)
  : option_ (option)
  {
  }

  inline const std::string& missing_value::
  option () const
  {
    return option_;
  }

  // invalid_value
  //
  inline invalid_value::
  invalid_value (const std::string& option,
                 const std::string& value)
  : option_ (option),  value_ (value)
  {
  }

  inline const std::string& invalid_value::
  option () const
  {
    return option_;
  }

  inline const std::string& invalid_value::
  value () const
  {
    return value_;
  }

  // file_io_failure
  //
  inline file_io_failure::
  file_io_failure (const std::string& file)
  : file_ (file)
  {
  }

  inline const std::string& file_io_failure::
  file () const
  {
    return file_;
  }

  // argv_scanner
  //
  inline argv_scanner::
  argv_scanner (int& argc, char** argv, bool erase)
  : i_ (1), argc_ (argc), argv_ (argv), erase_ (erase)
  {
  }

  inline argv_scanner::
  argv_scanner (int start, int& argc, char** argv, bool erase)
  : i_ (start), argc_ (argc), argv_ (argv), erase_ (erase)
  {
  }

  inline int argv_scanner::
  end () const
  {
    return i_;
  }

  // argv_file_scanner
  //
  inline argv_file_scanner::
  argv_file_scanner (int& argc,
                     char** argv,
                     const std::string& option,
                     bool erase)
  : argv_scanner (argc, argv, erase),
    option_ (option),
    skip_ (false)
  {
  }

  inline argv_file_scanner::
  argv_file_scanner (int start,
                     int& argc,
                     char** argv,
                     const std::string& option,
                     bool erase)
  : argv_scanner (start, argc, argv, erase),
    option_ (option),
    skip_ (false)
  {
  }
}

// options
//

inline const bool& options::
help () const
{
  return this->help_;
}

inline const bool& options::
version () const
{
  return this->version_;
}

inline const std::string& options::
output_dir () const
{
  return this->output_dir_;
}

inline const bool& options::
generate_modifier () const
{
  return this->generate_modifier_;
}

inline const bool& options::
generate_file_scanner () const
{
  return this->generate_file_scanner_;
}

inline const bool& options::
suppress_inline () const
{
  return this->suppress_inline_;
}

inline const bool& options::
suppress_usage () const
{
  return this->suppress_usage_;
}

inline const bool& options::
long_usage () const
{
  return this->long_usage_;
}

inline const std::size_t& options::
option_length () const
{
  return this->option_length_;
}

inline const bool& options::
generate_cxx () const
{
  return this->generate_cxx_;
}

inline const bool& options::
generate_man () const
{
  return this->generate_man_;
}

inline const bool& options::
generate_html () const
{
  return this->generate_html_;
}

inline const std::string& options::
man_prologue () const
{
  return this->man_prologue_;
}

inline const std::string& options::
man_epilogue () const
{
  return this->man_epilogue_;
}

inline const std::string& options::
html_prologue () const
{
  return this->html_prologue_;
}

inline const std::string& options::
html_epilogue () const
{
  return this->html_epilogue_;
}

inline const std::string& options::
class_ () const
{
  return this->class__;
}

inline const bool& options::
stdout_ () const
{
  return this->stdout__;
}

inline const std::string& options::
hxx_suffix () const
{
  return this->hxx_suffix_;
}

inline const std::string& options::
ixx_suffix () const
{
  return this->ixx_suffix_;
}

inline const std::string& options::
cxx_suffix () const
{
  return this->cxx_suffix_;
}

inline const std::string& options::
man_suffix () const
{
  return this->man_suffix_;
}

inline const std::string& options::
html_suffix () const
{
  return this->html_suffix_;
}

inline const std::string& options::
option_prefix () const
{
  return this->option_prefix_;
}

inline const std::string& options::
option_separator () const
{
  return this->option_separator_;
}

inline const bool& options::
include_with_brackets () const
{
  return this->include_with_brackets_;
}

inline const std::string& options::
include_prefix () const
{
  return this->include_prefix_;
}

inline const std::string& options::
guard_prefix () const
{
  return this->guard_prefix_;
}

inline const std::map<std::string, std::string>& options::
reserved_name () const
{
  return this->reserved_name_;
}

inline const std::string& options::
options_file () const
{
  return this->options_file_;
}

