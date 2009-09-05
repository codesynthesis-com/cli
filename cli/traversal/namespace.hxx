// file      : cli/traversal/namespace.hxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifndef CLI_TRAVERSAL_NAMESPACE_HXX
#define CLI_TRAVERSAL_NAMESPACE_HXX

#include <traversal/elements.hxx>
#include <semantics/namespace.hxx>

namespace traversal
{
  struct namespace_: scope_template<semantics::namespace_>
  {
    virtual void
    traverse (type&);

    virtual void
    pre (type&);

    virtual void
    post (type&);
  };
}

#endif // CLI_TRAVERSAL_NAMESPACE_HXX
