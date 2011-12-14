// file      : cli/source.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2011 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <iostream>

#include "source.hxx"

using std::cerr;

namespace
{
  //
  //
  struct option_init: traversal::option, context
  {
    option_init (context& c) : context (c), comma_ (false) {}

    virtual void
    traverse (type& o)
    {
      if (comma_)
        os << "," << endl
           << "  ";
      else
      {
        os << endl
           << ": ";
        comma_ = true;
      }

      os << emember (o);

      if (o.initialized_p ())
      {
        using semantics::expression;
        expression const& i (o.initializer ());

        switch (i.type ())
        {
        case expression::string_lit:
        case expression::char_lit:
        case expression::bool_lit:
        case expression::int_lit:
        case expression::float_lit:
        case expression::identifier:
            {
            os << " (" << i.value () << ")";
            break;
          }
        case expression::call_expr:
          {
            os << " " << i.value ();
            break;
          }
        }
      }
      else
        os << " ()";

      if (specifier && o.type ().name () != "bool")
        os << "," << endl
           << "  " << especifier_member (o) << " (false)";
    }

  private:
    bool comma_;
  };

  //
  //
  struct option_map: traversal::option, context
  {
    option_map (context& c) : context (c) {}

    virtual void
    traverse (type& o)
    {
      using semantics::names;

      string member (emember (o));
      string type (o.type ().name ());
      string scope (escape (o.scope ().name ()));
      string map ("_cli_" + scope + "_map_");

      names& n (o.named ());

      for (names::name_iterator i (n.name_begin ()); i != n.name_end (); ++i)
      {
        os << "_cli_" << scope << "_map_[\"" << *i << "\"] = " << endl
           << "&" << cli << "::thunk< " << scope << ", " << type << ", " <<
          "&" << scope << "::" << member;

        if (specifier && type != "bool")
          os << "," << endl
             << "  &" << scope << "::" << especifier_member (o);

        os << " >;";
      }
    }
  };

  //
  //
  struct option_desc: traversal::option, context
  {
    option_desc (context& c) : context (c) {}

    virtual void
    traverse (type& o)
    {
      using semantics::names;

      names& n (o.named ());

      os << "// " << o.name () << endl
         << "//" << endl
         << "{"
         << cli << "::option_names a;";

      for (names::name_iterator b (n.name_begin ()), i (b);
           i != n.name_end (); ++i)
      {
        if (i == b) // Skip the primary name.
          continue;

        os << "a.push_back (\"" << *i << "\");";
      }

      if (o.initialized_p ())
      {
        using semantics::expression;
        expression const& i (o.initializer ());

        switch (i.type ())
        {
        case expression::string_lit:
          {
            os << "std::string dv (" << i.value () << ");";
            break;
          }
        case expression::char_lit:
          {
            os << "std::string dv (1, " << i.value () << ");";
            break;
          }
        case expression::bool_lit:
        case expression::int_lit:
        case expression::float_lit:
          {
            os << "std::string dv (\"" << i.value () << "\");";
            break;
          }
        case expression::identifier:
        case expression::call_expr:
          {
            os << "std::string dv;";
            break;
          }
        }
      }
      else
        os << "std::string dv;";


      os << cli << "::option o (\"" << o.name () << "\", a, " <<
        (o.type ().name () == "bool" ? "true" : "false") << ", dv);"
         << "os.push_back (o);"
         << "}";
    }
  };

  //
  //
  struct option_length: traversal::option, context
  {
    option_length (context& c, size_t& l, type*& o)
        : context (c), length_ (l), option_ (o)
    {
    }

    virtual void
    traverse (type& o)
    {
      using semantics::names;

      type::doc_list const& doc (o.doc ());

      if (options.suppress_undocumented () && doc.empty ())
        return;

      size_t l (0);
      names& n (o.named ());

      for (names::name_iterator i (n.name_begin ()); i != n.name_end (); ++i)
      {
        if (l != 0)
          l++; // '|' seperator.

        l += i->size ();
      }

      string type (o.type ().name ());

      if (type != "bool")
      {
        l++; // ' ' seperator

        if (doc.size () > 0)
          l += doc[0].size ();
        else
          l += 5; // <arg>
      }

      if (l > length_)
      {
        length_ = l;
        option_ = &o;
      }
    }

  private:
    size_t& length_;
    type*& option_;
  };

  //
  //
  struct option_usage: traversal::option, context
  {
    option_usage (context& c, size_t l) : context (c), length_ (l) {}

    virtual void
    traverse (type& o)
    {
      using semantics::names;

      type::doc_list const& doc (o.doc ());

      if (options.suppress_undocumented () && doc.empty ())
        return;

      size_t l (0);
      names& n (o.named ());

      os << "os << \"";

      for (names::name_iterator i (n.name_begin ()); i != n.name_end (); ++i)
      {
        if (l != 0)
        {
          os << '|';
          l++;
        }

        os << escape_str (*i);
        l += i->size ();
      }

      string type (o.type ().name ());

      if (type != "bool")
      {
        os << ' ';
        l++;

        if (doc.size () > 0)
        {
          os << escape_str (format (doc[0], ot_plain));
          l += doc[0].size ();
        }
        else
        {
          os << "<arg>";
          l += 5;
        }
      }

      // If we have both the long and the short descriptions, use
      // the short one. Otherwise, use the first sentence from the
      // long one ubless --long-usage was specified.
      //
      string d;

      if (type == "bool")
      {
        if (doc.size () > 1)
          d = doc[0];
        else if (doc.size () > 0)
          d = options.long_usage () ? doc[0] : first_sentence (doc[0]);
      }
      else
      {
        if (doc.size () > 2)
          d = doc[1];
        else if (doc.size () > 1)
          d = options.long_usage () ? doc[1] : first_sentence (doc[1]);
      }

      // Format the documentation string.
      //
      d = format (d, ot_plain);

      if (!d.empty ())
      {
        pad (length_ - l);

        size_t b (0), e (0), i (0);

        for (size_t n (d.size ()); i < n; ++i)
        {
          if (d[i] == ' ' || d[i] == '\n')
            e = i;

          // Assume we have 78 characters instead of 79 per line to make
          // sure we get the same output on Windows (which has two character
          // for a newline).
          //
          if (d[i] == '\n' || i - b == 78 - length_)
          {
            if (b != 0)
            {
              os << endl
                 << "   << \"";
              pad ();
            }

            string s (d, b, (e != b ? e : i) - b);
            os << escape_str (s) << "\" << ::std::endl";

            if (d[i] == '\n')
              os << endl
                 << "   << ::std::endl";

            b = e = (e != b ? e : i) + 1;
          }
        }

        // Flush the last line.
        //
        if (b != i)
        {
          if (b != 0)
          {
            os << endl
               << "   << \"";
            pad ();
          }

          string s (d, b, i - b);
          os << escape_str (s) << "\" << ::std::endl";
        }
      }
      else
        os << "\" << std::endl";

      os << ";"
         << endl;
    }

  private:
    void
    pad (size_t n)
    {
      for (; n > 0; --n)
        os << ' ';

      os << ' '; // Space between arg and description.
    }

    void
    pad ()
    {
      pad (length_);
    }

    string
    first_sentence (string const& s)
    {
      size_t p (s.find ('.'));

      // Add some heuristics here: check that there is a space
      // (or end of text) after the period.
      //
      while (p != string::npos &&
             p + 1 <= s.size () &&
             s[p + 1] != ' ' &&
             s[p + 1] != '\n')
        p = s.find ('.', p + 1);

      return p == string::npos ? s : string (s, 0, p + 1);
    }

    string
    escape_str (string const& s)
    {
      string r;
      r.reserve (s.size ());

      for (size_t i (0), n (s.size ()); i < n; ++i)
      {
        switch (s[i])
        {
        case '\\':
          {
            r += "\\\\";
            break;
          }
        case '"':
          {
            r += "\\\"";
            break;
          }
        default:
          {
            r += s[i];
            break;
          }
        }
      }

      return r;
    }

  private:
    size_t length_;
  };

  //
  //
  struct class_: traversal::class_, context
  {
    class_ (context& c)
        : context (c), option_map_ (c), option_desc_ (c)
    {
      names_option_map_ >> option_map_;
      names_option_desc_ >> option_desc_;
    }

    virtual void
    traverse (type& c)
    {
      string name (escape (c.name ()));
      bool ho (has<semantics::option> (c));

      os << "// " << name << endl
         << "//" << endl
         << endl;

      // c-tors
      //
      string um (cli + "::unknown_mode");

      os << name << "::" << endl
         << name << " (int& argc," << endl
         << "char** argv," << endl
         << "bool erase," << endl
         << um << " opt," << endl
         << um << " arg)";
      {
        option_init init (*this);
        traversal::names names_init (init);
        names (c, names_init);
      }
      os << "{"
         << cli << "::argv_scanner s (argc, argv, erase);"
         << "_parse (s, opt, arg);"
         << "}";

      os << name << "::" << endl
         << name << " (int start," << endl
         << "int& argc," << endl
         << "char** argv," << endl
         << "bool erase," << endl
         << um << " opt," << endl
         << um << " arg)";
      {
        option_init init (*this);
        traversal::names names_init (init);
        names (c, names_init);
      }
      os << "{"
         << cli << "::argv_scanner s (start, argc, argv, erase);"
         << "_parse (s, opt, arg);"
         << "}";

      os << name << "::" << endl
         << name << " (int& argc," << endl
         << "char** argv," << endl
         << "int& end," << endl
         << "bool erase," << endl
         << um << " opt," << endl
         << um << " arg)";
      {
        option_init init (*this);
        traversal::names names_init (init);
        names (c, names_init);
      }
      os << "{"
         << cli << "::argv_scanner s (argc, argv, erase);"
         << "_parse (s, opt, arg);"
         << "end = s.end ();"
         << "}";

      os << name << "::" << endl
         << name << " (int start," << endl
         << "int& argc," << endl
         << "char** argv," << endl
         << "int& end," << endl
         << "bool erase," << endl
         << um << " opt," << endl
         << um << " arg)";
      {
        option_init init (*this);
        traversal::names names_init (init);
        names (c, names_init);
      }
      os << "{"
         << cli << "::argv_scanner s (start, argc, argv, erase);"
         << "_parse (s, opt, arg);"
         << "end = s.end ();"
         << "}";

      os << name << "::" << endl
         << name << " (" << cli << "::scanner& s," << endl
         << um << " opt," << endl
         << um << " arg)";
      {
        option_init init (*this);
        traversal::names names_init (init);
        names (c, names_init);
      }
      os << "{"
         << "_parse (s, opt, arg);"
         << "}";

      // Usage.
      //
      if (usage)
      {
        os << "void " << name << "::" << endl
           << "print_usage (::std::ostream&" << (ho ? " os" : "") << ")"
           << "{";

        // Calculate option length.
        //
        size_t len (0);
        {
          semantics::option* o (0);
          option_length t (*this, len, o);
          traversal::names n (t);
          names (c, n);

          size_t max (options.option_length ());

          if (max != 0)
          {
            if (len > max)
            {
              cerr << o->file () << ":" << o->line () << ":" << o->column ()
                   << " error: option length " << len << " is greater than "
                   << max << " specified with --option-length" << endl;
              throw generation_failed ();
            }

            len = max;
          }
        }

        // Print option usage.
        //
        {
          option_usage t (*this, len);
          traversal::names n (t);
          names (c, n);
        }

        os << "}";
      }

      // Description.
      //
      if (options.generate_description ())
      {
        string desc ("_cli_" + name + "_desc");

        os << "static " << cli << "::options " << desc << "_;"
           << endl;

        os << "struct " << desc << "_init"
           << "{"
           << desc << "_init (" << cli << "::options& os)"
           << "{";

        names (c, names_option_desc_);

        os << "}"
           << "};"
           << "static " << desc << "_init " << desc << "_init_ (" <<
          desc << "_);"
           << endl;

        os << "const " << cli << "::options& " << name << "::" << endl
           << "description ()"
           << "{"
           << "return " << desc << "_;"
           << "};";
      }

      // _parse()
      //
      string map ("_cli_" + name + "_map");

      os << "typedef" << endl
         << "std::map<std::string, void (*) (" <<
        name << "&, " << cli << "::scanner&)>" << endl
         << map << ";"
         << endl
         << "static " << map << " " << map << "_;"
         << endl;

      os << "struct " << map << "_init"
         << "{"
         << map << "_init ()"
         << "{";

      names (c, names_option_map_);

      os << "}"
         << "};"
         << "static " << map << "_init " << map << "_init_;"
         << endl;

      bool pfx (!opt_prefix.empty ());
      bool sep (!opt_sep.empty ());

      os << "void " << name << "::" << endl
         << "_parse (" << cli << "::scanner& s," << endl
         << um << " opt_mode," << endl
         << um << " arg_mode)"
         << "{";

      if (sep)
        os << "bool opt = true;" // Still recognizing options.
           << endl;

      os << "while (s.more ())"
         << "{"
         << "const char* o = s.peek ();";

      if (sep)
        os << endl
           << "if (std::strcmp (o, \"" << opt_sep << "\") == 0)"
           << "{"
           << "s.skip ();" // We don't want to remove the separator.
           << "opt = false;"
           << "continue;"
           << "}"
           << map << "::const_iterator i (" << endl
           << "opt ? " << map << "_.find (o) : " << map << "_.end ());";
      else
        os << map << "::const_iterator i (" << map << "_.find (o));";

      os << endl
         << "if (i != " << map << "_.end ())"
         << "{"
         << "(*(i->second)) (*this, s);"
         << "}";

      // Unknown option.
      //
      if (pfx)
      {
        size_t n (opt_prefix.size ());

        os << "else if (";

        if (sep)
          os << "opt && ";

        os << "std::strncmp (o, \"" << opt_prefix << "\", " <<
          n << ") == 0 && o[" << n << "] != '\\0')"
           << "{"
           << "switch (opt_mode)"
           << "{"
           << "case " << cli << "::unknown_mode::skip:" << endl
           << "{"
           << "s.skip ();"
           << "continue;"
           << "}"
           << "case " << cli << "::unknown_mode::stop:" << endl
           << "{"
           << "break;"
           << "}"
           << "case " << cli << "::unknown_mode::fail:" << endl
           << "{"
           << "throw " << cli << "::unknown_option (o);"
           << "}"
           << "}" // switch
           << "break;" // The stop case.
           << "}";
      }

      // Unknown argument.
      //
      os << "else"
         << "{"
         << "switch (arg_mode)"
         << "{"
         << "case " << cli << "::unknown_mode::skip:" << endl
         << "{"
         << "s.skip ();"
         << "continue;"
         << "}"
         << "case " << cli << "::unknown_mode::stop:" << endl
         << "{"
         << "break;"
         << "}"
         << "case " << cli << "::unknown_mode::fail:" << endl
         << "{"
         << "throw " << cli << "::unknown_argument (o);"
         << "}"
         << "}" // switch
         << "break;" // The stop case.
         << "}"

         << "}" // for
         << "}";
    }

  private:
    option_map option_map_;
    traversal::names names_option_map_;

    option_desc option_desc_;
    traversal::names names_option_desc_;
  };
}

void
generate_source (context& ctx)
{
  ctx.os << "#include <map>" << endl
         << "#include <cstring>" << endl
         << endl;

  traversal::cli_unit unit;
  traversal::names unit_names;
  namespace_ ns (ctx);
  class_ cl (ctx);

  unit >> unit_names >> ns;
  unit_names >> cl;

  traversal::names ns_names;

  ns >> ns_names >> ns;
  ns_names >> cl;

  unit.dispatch (ctx.unit);
}
