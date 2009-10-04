// file      : cli/generator.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <cctype>  // std::toupper, std::is{alpha,upper,lower}
#include <string>
#include <fstream>
#include <iostream>

#include <cutl/fs/auto-remove.hxx>

#include <cutl/compiler/code-stream.hxx>
#include <cutl/compiler/cxx-indenter.hxx>

#include "header.hxx"
#include "source.hxx"
#include "inline.hxx"

#include "runtime-header.hxx"
#include "runtime-source.hxx"

#include "context.hxx"
#include "generator.hxx"
#include "name-processor.hxx"

using namespace std;
using namespace cutl;
using namespace cutl;

using semantics::path;

namespace
{
  static char const header[] =
  "// This code was generated by CLI, a command line interface\n"
  "// compiler for C++.\n"
  "//\n\n";

  string
  make_guard (string const& file, context& ctx)
  {
    string g (file);

    // Split words, e.g., "FooBar" to "Foo_Bar" and convert everything
    // to upper case.
    //
    string r;
    for (string::size_type i (0), n (g.size ()); i < n - 1; ++i)
    {
      char c1 (g[i]);
      char c2 (g[i + 1]);

      r += toupper (c1);

      if (isalpha (c1) && isalpha (c2) && islower (c1) && isupper (c2))
        r += "_";
    }
    r += toupper (g[g.size () - 1]);

    return ctx.escape (r);
  }
}

generator::
generator ()
{
}

void generator::
generate (options const& ops, semantics::cli_unit& unit, path const& p)
{
  try
  {
    bool inl (!ops.suppress_inline ());

    path file (p.leaf ());
    string base (file.base ().string ());

    string hxx_name (base + ops.hxx_suffix ());
    string ixx_name (base + ops.ixx_suffix ());
    string cxx_name (base + ops.cxx_suffix ());

    path hxx_path (hxx_name);
    path ixx_path (ixx_name);
    path cxx_path (cxx_name);

    if (!ops.output_dir ().empty ())
    {
      path dir (ops.output_dir ());

      hxx_path = dir / hxx_path;
      ixx_path = dir / ixx_path;
      cxx_path = dir / cxx_path;
    }

    // Process names.
    //
    {
      context ctx (cerr, unit, ops);
      process_names (ctx);
    }

    fs::auto_removes auto_rm;

    //
    //
    ofstream hxx (hxx_path.string ().c_str ());

    if (!hxx.is_open ())
    {
      cerr << "error: unable to open '" << hxx_path << "' in write mode"
           << endl;
      throw failed ();
    }

    auto_rm.add (hxx_path);

    //
    //
    ofstream ixx;

    if (inl)
    {
      ixx.open (ixx_path.string ().c_str (), ios_base::out);

      if (!ixx.is_open ())
      {
        cerr << "error: unable to open '" << ixx_path << "' in write mode"
             << endl;
        throw failed ();
      }

      auto_rm.add (ixx_path);
    }

    //
    //
    ofstream cxx (cxx_path.string ().c_str ());

    if (!cxx.is_open ())
    {
      cerr << "error: unable to open '" << cxx_path << "' in write mode"
           << endl;
      throw failed ();
    }

    auto_rm.add (cxx_path);

    // Print headers.
    //
    hxx << header;
    if (inl)
      ixx << header;
    cxx << header;

    typedef compiler::ostream_filter<compiler::cxx_indenter, char> cxx_filter;

    // Include settings.
    //
    bool br (ops.include_with_brackets ());
    string ip (ops.include_prefix ());
    string gp (ops.guard_prefix ());

    if (!ip.empty () && ip[ip.size () - 1] != '/')
      ip.append ("/");

    if (!gp.empty () && gp[gp.size () - 1] != '_')
      gp.append ("_");

    // HXX
    //
    {
      cxx_filter filt (hxx);
      context ctx (hxx, unit, ops);

      string guard (make_guard (gp + hxx_name, ctx));

      hxx << "#ifndef " << guard << endl
          << "#define " << guard << endl
          << endl;

      generate_runtime_header_decl (ctx);
      generate_header (ctx);
      generate_runtime_header_impl (ctx);

      if (inl)
      {
        hxx << "#include " << (br ? '<' : '"') << ip << ixx_name <<
          (br ? '>' : '"') << endl
            << endl;
      }

      hxx << "#endif // " << guard << endl;
    }

    // IXX
    //
    if (inl)
    {
      cxx_filter filt (ixx);
      context ctx (ixx, unit, ops);
      generate_inline (ctx);
    }

    // CXX
    //
    {
      cxx_filter filt (cxx);
      context ctx (cxx, unit, ops);

      cxx << "#include " << (br ? '<' : '"') << ip << hxx_name <<
        (br ? '>' : '"') << endl
          << endl;

      if (!inl)
        generate_inline (ctx);

      generate_runtime_source (ctx);
      generate_source (ctx);
    }

    auto_rm.cancel ();
  }
  catch (semantics::invalid_path const& e)
  {
    cerr << "error: '" << e.path () << "' is not a valid filesystem path"
         << endl;
    throw failed ();
  }
  catch (fs::error const&)
  {
    // Auto-removal of generated files failed. Ignore it.
    //
    throw failed ();
  }
}
