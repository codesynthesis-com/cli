// file      : cli/generator.hxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2011 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifndef CLI_GENERATOR_HXX
#define CLI_GENERATOR_HXX

#include "options.hxx"
#include "semantics/unit.hxx"

class generator
{
public:
  generator ();

  class failed {};

  void
  generate (options const&, semantics::cli_unit&, semantics::path const&);

private:
  generator (generator const&);

  generator&
  operator= (generator const&);
};

#endif // CLI_GENERATOR_HXX
