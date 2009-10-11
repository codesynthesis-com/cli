// file      : cli/cli.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <memory>   // std::auto_ptr
#include <fstream>
#include <iostream>

#include <cutl/compiler/code-stream.hxx>

#include "usage.hxx"
#include "options.hxx"
#include "parser.hxx"
#include "generator.hxx"

using namespace std;
using namespace cutl;

int main (int argc, char* argv[])
{
  ostream& e (cerr);

  try
  {
    int end;
    options ops (argc, argv, end);

    // Handle --version
    //
    if (ops.version ())
    {
      e << "CodeSynthesis CLI command line interface compiler 0.0.2" << endl
        << "Copyright (C) 2009 Code Synthesis Tools CC" << endl;

      e << "This is free software; see the source for copying conditions. "
        << "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS "
        << "FOR A PARTICULAR PURPOSE." << endl;

      return 0;
    }

    // Handle --help
    //
    if (ops.help ())
    {
      e << "Usage: " << argv[0] << " [options] file"
        << endl
        << "Options:" << endl;

      compiler::ostream_filter<usage_indenter, char> filt (e);

      e << "--help" << endl
        << " Print usage information and exit."
        << endl;

      e << "--version" << endl
        << " Print version and exit."
        << endl;

      generator::usage ();

      return 0;
    }

    if (end == argc)
    {
      e << "error: no input file specified" << endl
        << "info: try '" << argv[0] << " --help' for more information" << endl;

      return 1;
    }

    semantics::path path (argv[end]);

    ifstream ifs (path.string ().c_str ());
    if (!ifs.is_open ())
    {
      e << path << ": error: unable to open in read mode" << endl;
      return 1;
    }

    ifs.exceptions (ifstream::failbit | ifstream::badbit);

    parser p;
    auto_ptr<semantics::cli_unit> unit (p.parse (ifs, path));

    generator g;
    g.generate (ops, *unit, path);
  }
  catch (cli::exception const& ex)
  {
    e << ex << endl;
    return 1;
  }
  catch (semantics::invalid_path const& ex)
  {
    e << "error: '" << ex.path () << "' is not a valid filesystem path"
      << endl;
    return 1;
  }
  catch (std::ios_base::failure const&)
  {
    e << argv[1] << ": error: read failure" << endl;
    return 1;
  }
  catch (parser::invalid_input const&)
  {
    // Diagnostics has already been issued by the parser.
    //
    return 1;
  }
  catch (generator::failed const&)
  {
    // Diagnostics has already been issued by the generator.
    //
    return 1;
  }
}
