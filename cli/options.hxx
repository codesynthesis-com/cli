// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

#ifndef CLI_OPTIONS_HXX
#define CLI_OPTIONS_HXX

// Begin prologue.
//
//
// End prologue.

#include <deque>
#include <iosfwd>
#include <string>
#include <cstddef>
#include <exception>

#ifndef CLI_POTENTIALLY_UNUSED
#  if defined(_MSC_VER) || defined(__xlC__)
#    define CLI_POTENTIALLY_UNUSED(x) (void*)&x
#  else
#    define CLI_POTENTIALLY_UNUSED(x) (void)x
#  endif
#endif

namespace cli
{
  class usage_para
  {
    public:
    enum value
    {
      none,
      text,
      option
    };

    usage_para (value);

    operator value () const 
    {
      return v_;
    }

    private:
    value v_;
  };

  class unknown_mode
  {
    public:
    enum value
    {
      skip,
      stop,
      fail
    };

    unknown_mode (value);

    operator value () const 
    {
      return v_;
    }

    private:
    value v_;
  };

  // Exceptions.
  //

  class exception: public std::exception
  {
    public:
    virtual void
    print (::std::ostream&) const = 0;
  };

  ::std::ostream&
  operator<< (::std::ostream&, const exception&);

  class unknown_option: public exception
  {
    public:
    virtual
    ~unknown_option () throw ();

    unknown_option (const std::string& option);

    const std::string&
    option () const;

    virtual void
    print (::std::ostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string option_;
  };

  class unknown_argument: public exception
  {
    public:
    virtual
    ~unknown_argument () throw ();

    unknown_argument (const std::string& argument);

    const std::string&
    argument () const;

    virtual void
    print (::std::ostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string argument_;
  };

  class missing_value: public exception
  {
    public:
    virtual
    ~missing_value () throw ();

    missing_value (const std::string& option);

    const std::string&
    option () const;

    virtual void
    print (::std::ostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string option_;
  };

  class invalid_value: public exception
  {
    public:
    virtual
    ~invalid_value () throw ();

    invalid_value (const std::string& option,
                   const std::string& value);

    const std::string&
    option () const;

    const std::string&
    value () const;

    virtual void
    print (::std::ostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string option_;
    std::string value_;
  };

  class eos_reached: public exception
  {
    public:
    virtual void
    print (::std::ostream&) const;

    virtual const char*
    what () const throw ();
  };

  class file_io_failure: public exception
  {
    public:
    virtual
    ~file_io_failure () throw ();

    file_io_failure (const std::string& file);

    const std::string&
    file () const;

    virtual void
    print (::std::ostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string file_;
  };

  class unmatched_quote: public exception
  {
    public:
    virtual
    ~unmatched_quote () throw ();

    unmatched_quote (const std::string& argument);

    const std::string&
    argument () const;

    virtual void
    print (::std::ostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string argument_;
  };

  class scanner
  {
    public:
    virtual
    ~scanner ();

    virtual bool
    more () = 0;

    virtual const char*
    peek () = 0;

    virtual const char*
    next () = 0;

    virtual void
    skip () = 0;
  };

  class argv_scanner: public scanner
  {
    public:
    argv_scanner (int& argc, char** argv, bool erase = false);
    argv_scanner (int start, int& argc, char** argv, bool erase = false);

    int
    end () const;

    virtual bool
    more ();

    virtual const char*
    peek ();

    virtual const char*
    next ();

    virtual void
    skip ();

    private:
    int i_;
    int& argc_;
    char** argv_;
    bool erase_;
  };

  class argv_file_scanner: public argv_scanner
  {
    public:
    argv_file_scanner (int& argc,
                       char** argv,
                       const std::string& option,
                       bool erase = false);

    argv_file_scanner (int start,
                       int& argc,
                       char** argv,
                       const std::string& option,
                       bool erase = false);

    struct option_info
    {
      // If search_func is not NULL, it is called, with the arg
      // value as the second argument, to locate the options file.
      // If it returns an empty string, then the file is ignored.
      //
      const char* option;
      std::string (*search_func) (const char*, void* arg);
      void* arg;
    };

    argv_file_scanner (int& argc,
                        char** argv,
                        const option_info* options,
                        std::size_t options_count,
                        bool erase = false);

    argv_file_scanner (int start,
                       int& argc,
                       char** argv,
                       const option_info* options,
                       std::size_t options_count,
                       bool erase = false);

    virtual bool
    more ();

    virtual const char*
    peek ();

    virtual const char*
    next ();

    virtual void
    skip ();

    private:
    const option_info*
    find (const char*) const;

    void
    load (const std::string& file);

    typedef argv_scanner base;

    const std::string option_;
    option_info option_info_;
    const option_info* options_;
    std::size_t options_count_;

    std::string hold_;
    std::deque<std::string> args_;
    bool skip_;
  };

  template <typename X>
  struct parser;
}

#include <map>

#include <string>

#include <vector>

#include <cstddef>

#include <cli/option-types.hxx>

class options
{
  public:
  options ();

  options (int& argc,
           char** argv,
           bool erase = false,
           ::cli::unknown_mode option = ::cli::unknown_mode::fail,
           ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  options (int start,
           int& argc,
           char** argv,
           bool erase = false,
           ::cli::unknown_mode option = ::cli::unknown_mode::fail,
           ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  options (int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::cli::unknown_mode option = ::cli::unknown_mode::fail,
           ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  options (int start,
           int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::cli::unknown_mode option = ::cli::unknown_mode::fail,
           ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  options (::cli::scanner&,
           ::cli::unknown_mode option = ::cli::unknown_mode::fail,
           ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  // Option accessors.
  //
  const bool&
  help () const;

  const bool&
  version () const;

  const std::vector<std::string>&
  include_path () const;

  bool
  include_path_specified () const;

  const std::string&
  output_dir () const;

  bool
  output_dir_specified () const;

  const cxx_version&
  std () const;

  bool
  std_specified () const;

  const bool&
  generate_modifier () const;

  const bool&
  generate_specifier () const;

  const bool&
  generate_parse () const;

  const bool&
  generate_description () const;

  const bool&
  generate_file_scanner () const;

  const bool&
  generate_vector_scanner () const;

  const bool&
  suppress_inline () const;

  const bool&
  suppress_cli () const;

  const std::string&
  cli_namespace () const;

  bool
  cli_namespace_specified () const;

  const std::string&
  ostream_type () const;

  bool
  ostream_type_specified () const;

  const bool&
  generate_cxx () const;

  const bool&
  generate_man () const;

  const bool&
  generate_html () const;

  const bool&
  generate_txt () const;

  const bool&
  stdout_ () const;

  const bool&
  suppress_undocumented () const;

  const bool&
  suppress_usage () const;

  const bool&
  long_usage () const;

  const bool&
  short_usage () const;

  const std::string&
  page_usage () const;

  bool
  page_usage_specified () const;

  const std::size_t&
  option_length () const;

  bool
  option_length_specified () const;

  const bool&
  ansi_color () const;

  const bool&
  exclude_base () const;

  const bool&
  include_base_last () const;

  const std::map<std::string, std::string>&
  class_doc () const;

  bool
  class_doc_specified () const;

  const std::vector<std::string>&
  class_ () const;

  bool
  class__specified () const;

  const std::map<std::string, std::string>&
  docvar () const;

  bool
  docvar_specified () const;

  const std::vector<std::string>&
  link_regex () const;

  bool
  link_regex_specified () const;

  const bool&
  link_regex_trace () const;

  const std::map<char, std::string>&
  html_heading_map () const;

  bool
  html_heading_map_specified () const;

  const bool&
  omit_link_check () const;

  const std::vector<std::string>&
  hxx_prologue () const;

  bool
  hxx_prologue_specified () const;

  const std::vector<std::string>&
  ixx_prologue () const;

  bool
  ixx_prologue_specified () const;

  const std::vector<std::string>&
  cxx_prologue () const;

  bool
  cxx_prologue_specified () const;

  const std::vector<std::string>&
  man_prologue () const;

  bool
  man_prologue_specified () const;

  const std::vector<std::string>&
  html_prologue () const;

  bool
  html_prologue_specified () const;

  const std::vector<std::string>&
  txt_prologue () const;

  bool
  txt_prologue_specified () const;

  const std::vector<std::string>&
  hxx_epilogue () const;

  bool
  hxx_epilogue_specified () const;

  const std::vector<std::string>&
  ixx_epilogue () const;

  bool
  ixx_epilogue_specified () const;

  const std::vector<std::string>&
  cxx_epilogue () const;

  bool
  cxx_epilogue_specified () const;

  const std::vector<std::string>&
  man_epilogue () const;

  bool
  man_epilogue_specified () const;

  const std::vector<std::string>&
  html_epilogue () const;

  bool
  html_epilogue_specified () const;

  const std::vector<std::string>&
  txt_epilogue () const;

  bool
  txt_epilogue_specified () const;

  const std::string&
  hxx_prologue_file () const;

  bool
  hxx_prologue_file_specified () const;

  const std::string&
  ixx_prologue_file () const;

  bool
  ixx_prologue_file_specified () const;

  const std::string&
  cxx_prologue_file () const;

  bool
  cxx_prologue_file_specified () const;

  const std::string&
  man_prologue_file () const;

  bool
  man_prologue_file_specified () const;

  const std::string&
  html_prologue_file () const;

  bool
  html_prologue_file_specified () const;

  const std::string&
  txt_prologue_file () const;

  bool
  txt_prologue_file_specified () const;

  const std::string&
  hxx_epilogue_file () const;

  bool
  hxx_epilogue_file_specified () const;

  const std::string&
  ixx_epilogue_file () const;

  bool
  ixx_epilogue_file_specified () const;

  const std::string&
  cxx_epilogue_file () const;

  bool
  cxx_epilogue_file_specified () const;

  const std::string&
  man_epilogue_file () const;

  bool
  man_epilogue_file_specified () const;

  const std::string&
  html_epilogue_file () const;

  bool
  html_epilogue_file_specified () const;

  const std::string&
  txt_epilogue_file () const;

  bool
  txt_epilogue_file_specified () const;

  const std::string&
  output_prefix () const;

  bool
  output_prefix_specified () const;

  const std::string&
  output_suffix () const;

  bool
  output_suffix_specified () const;

  const std::string&
  hxx_suffix () const;

  bool
  hxx_suffix_specified () const;

  const std::string&
  ixx_suffix () const;

  bool
  ixx_suffix_specified () const;

  const std::string&
  cxx_suffix () const;

  bool
  cxx_suffix_specified () const;

  const std::string&
  man_suffix () const;

  bool
  man_suffix_specified () const;

  const std::string&
  html_suffix () const;

  bool
  html_suffix_specified () const;

  const std::string&
  txt_suffix () const;

  bool
  txt_suffix_specified () const;

  const std::string&
  option_prefix () const;

  bool
  option_prefix_specified () const;

  const std::string&
  option_separator () const;

  bool
  option_separator_specified () const;

  const bool&
  include_with_brackets () const;

  const std::string&
  include_prefix () const;

  bool
  include_prefix_specified () const;

  const std::string&
  guard_prefix () const;

  bool
  guard_prefix_specified () const;

  const std::map<std::string, std::string>&
  reserved_name () const;

  bool
  reserved_name_specified () const;

  const std::string&
  options_file () const;

  bool
  options_file_specified () const;

  // Print usage information.
  //
  static ::cli::usage_para
  print_usage (::std::ostream&,
               ::cli::usage_para = ::cli::usage_para::none);

  // Implementation details.
  //
  protected:
  bool
  _parse (const char*, ::cli::scanner&);

  private:
  void
  _parse (::cli::scanner&,
          ::cli::unknown_mode option,
          ::cli::unknown_mode argument);

  public:
  bool help_;
  bool version_;
  std::vector<std::string> include_path_;
  bool include_path_specified_;
  std::string output_dir_;
  bool output_dir_specified_;
  cxx_version std_;
  bool std_specified_;
  bool generate_modifier_;
  bool generate_specifier_;
  bool generate_parse_;
  bool generate_description_;
  bool generate_file_scanner_;
  bool generate_vector_scanner_;
  bool suppress_inline_;
  bool suppress_cli_;
  std::string cli_namespace_;
  bool cli_namespace_specified_;
  std::string ostream_type_;
  bool ostream_type_specified_;
  bool generate_cxx_;
  bool generate_man_;
  bool generate_html_;
  bool generate_txt_;
  bool stdout__;
  bool suppress_undocumented_;
  bool suppress_usage_;
  bool long_usage_;
  bool short_usage_;
  std::string page_usage_;
  bool page_usage_specified_;
  std::size_t option_length_;
  bool option_length_specified_;
  bool ansi_color_;
  bool exclude_base_;
  bool include_base_last_;
  std::map<std::string, std::string> class_doc_;
  bool class_doc_specified_;
  std::vector<std::string> class__;
  bool class__specified_;
  std::map<std::string, std::string> docvar_;
  bool docvar_specified_;
  std::vector<std::string> link_regex_;
  bool link_regex_specified_;
  bool link_regex_trace_;
  std::map<char, std::string> html_heading_map_;
  bool html_heading_map_specified_;
  bool omit_link_check_;
  std::vector<std::string> hxx_prologue_;
  bool hxx_prologue_specified_;
  std::vector<std::string> ixx_prologue_;
  bool ixx_prologue_specified_;
  std::vector<std::string> cxx_prologue_;
  bool cxx_prologue_specified_;
  std::vector<std::string> man_prologue_;
  bool man_prologue_specified_;
  std::vector<std::string> html_prologue_;
  bool html_prologue_specified_;
  std::vector<std::string> txt_prologue_;
  bool txt_prologue_specified_;
  std::vector<std::string> hxx_epilogue_;
  bool hxx_epilogue_specified_;
  std::vector<std::string> ixx_epilogue_;
  bool ixx_epilogue_specified_;
  std::vector<std::string> cxx_epilogue_;
  bool cxx_epilogue_specified_;
  std::vector<std::string> man_epilogue_;
  bool man_epilogue_specified_;
  std::vector<std::string> html_epilogue_;
  bool html_epilogue_specified_;
  std::vector<std::string> txt_epilogue_;
  bool txt_epilogue_specified_;
  std::string hxx_prologue_file_;
  bool hxx_prologue_file_specified_;
  std::string ixx_prologue_file_;
  bool ixx_prologue_file_specified_;
  std::string cxx_prologue_file_;
  bool cxx_prologue_file_specified_;
  std::string man_prologue_file_;
  bool man_prologue_file_specified_;
  std::string html_prologue_file_;
  bool html_prologue_file_specified_;
  std::string txt_prologue_file_;
  bool txt_prologue_file_specified_;
  std::string hxx_epilogue_file_;
  bool hxx_epilogue_file_specified_;
  std::string ixx_epilogue_file_;
  bool ixx_epilogue_file_specified_;
  std::string cxx_epilogue_file_;
  bool cxx_epilogue_file_specified_;
  std::string man_epilogue_file_;
  bool man_epilogue_file_specified_;
  std::string html_epilogue_file_;
  bool html_epilogue_file_specified_;
  std::string txt_epilogue_file_;
  bool txt_epilogue_file_specified_;
  std::string output_prefix_;
  bool output_prefix_specified_;
  std::string output_suffix_;
  bool output_suffix_specified_;
  std::string hxx_suffix_;
  bool hxx_suffix_specified_;
  std::string ixx_suffix_;
  bool ixx_suffix_specified_;
  std::string cxx_suffix_;
  bool cxx_suffix_specified_;
  std::string man_suffix_;
  bool man_suffix_specified_;
  std::string html_suffix_;
  bool html_suffix_specified_;
  std::string txt_suffix_;
  bool txt_suffix_specified_;
  std::string option_prefix_;
  bool option_prefix_specified_;
  std::string option_separator_;
  bool option_separator_specified_;
  bool include_with_brackets_;
  std::string include_prefix_;
  bool include_prefix_specified_;
  std::string guard_prefix_;
  bool guard_prefix_specified_;
  std::map<std::string, std::string> reserved_name_;
  bool reserved_name_specified_;
  std::string options_file_;
  bool options_file_specified_;
};

#include <cli/options.ixx>

// Begin epilogue.
//
//
// End epilogue.

#endif // CLI_OPTIONS_HXX
