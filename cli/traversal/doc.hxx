// file      : cli/traversal/doc.hxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2017 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifndef CLI_TRAVERSAL_DOC_HXX
#define CLI_TRAVERSAL_DOC_HXX

#include <cli/traversal/elements.hxx>
#include <cli/semantics/doc.hxx>

namespace traversal
{
  struct doc: node<semantics::doc> {};
}

#endif // CLI_TRAVERSAL_DOC_HXX
