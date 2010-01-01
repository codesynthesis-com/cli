// file      : cli/semantics/namespace.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2010 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <semantics/namespace.hxx>

namespace semantics
{
  // type info
  //
  namespace
  {
    struct init
    {
      init ()
      {
        using compiler::type_info;

        type_info ti (typeid (namespace_));
        ti.add_base (typeid (scope));
        insert (ti);
      }
    } init_;
  }
}
