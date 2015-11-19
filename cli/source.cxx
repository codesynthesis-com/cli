// file      : cli/source.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2011 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <iostream>

#include "source.hxx"

using namespace std;

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


  // Return the number of "text characters", ignoring any escape sequences
  // (e.g., ANSI color).
  //
  static size_t
  text_size (const string& s, size_t p = 0, size_t n = string::npos)
  {
    size_t r (0);

    n = n == string::npos ? s.size () : n + p;

    // The start position (p) might be pointing half-way into the
    // escape sequence. So we always have to scan from the start.
    //
    for (size_t i (0), m (s.size ()); i < n; ++i)
    {
      if (s[i] == '\033') // ANSI escape: "\033[Nm"
      {
        i += 3;
        assert (i < m && s[i] == 'm');
        continue;
      }

      if (i >= p)
        ++r;
    }

    return r;
  }

  static string
  escape_str (string const& s)
  {
    string r;

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
      case '\033':
        {
          r += "\\033";
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

  // This function assumes that the string opening part has already been
  // written. The 'first' argument is the number of characters already
  // written in the first line (e.g., an option name).
  //
  static void
  wrap_lines (ostream& os,
              const string& d,
              size_t indent = 0,
              size_t first = 0)
  {
    assert (!d.empty ());

    string ind (indent, ' ');
    os << string (indent - first, ' ');

    size_t b (0), e (0), i (0);
    for (size_t n (d.size ()); i < n; ++i)
    {
      if (d[i] == ' ' || d[i] == '\n')
        e = i;

      if (d[i] == '\n' || text_size (d, b, i - b) == 79 - indent)
      {
        if (b != 0) // Not a first line.
          os << endl
             << "   << \"" << ind;

        string s (d, b, (e != b ? e : i) - b);
        os << escape_str (s) << "\" << ::std::endl";

        // Handle consecutive newlines (e.g., pre, paragraph separator).
        //
        if (d[i] == '\n')
        {
          for (; i + 1 < n && d[i + 1] == '\n'; e = ++i)
            os << endl
               << "   << ::std::endl";
        }

        b = e = (e != b ? e : i) + 1;
      }
    }

    // Flush the last line.
    //
    if (b != i)
    {
      if (b != 0)
        os << endl
           << "   << \"" << ind;

      string s (d, b, i - b);
      os << escape_str (s) << "\" << ::std::endl";
    }
  }

  struct option_length: traversal::option, context
  {
    option_length (context& c, size_t& l)
        : context (c), length_ (l), option_ (0) {}

    option_length (context& c, size_t& l, type*& o)
        : context (c), length_ (l), option_ (&o) {}

    virtual void
    traverse (type& o)
    {
      using semantics::names;

      semantics::doc_strings const& doc (o.doc ());

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

      if (type != "bool" || doc.size () >= 3)
      {
        l++; // ' ' seperator

        string s (doc.size () > 0 ? doc[0] : string ("<arg>"));

        if (options.ansi_color ())
        {
          std::set<string> arg_set;
          s = translate_arg (s, arg_set);
        }

        l += text_size (format (ot_plain, s, false));
      }

      if (l > length_)
      {
        length_ = l;
        if (option_ != 0)
          *option_ = &o;
      }
    }

  private:
    size_t& length_;
    type** option_;
  };

  //
  //
  struct option_usage: traversal::option, context
  {
    option_usage (context& c, size_t l, usage_type u)
        : context (c), length_ (l), usage_ (u) {}

    virtual void
    traverse (type& o)
    {
      using semantics::names;

      semantics::doc_strings const& doc (o.doc ());

      if (options.suppress_undocumented () && doc.empty ())
        return;

      bool color (options.ansi_color ());

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

        if (color)
          os << "\\033[1m"; // Bold.

        os << escape_str (*i);

        if (color)
          os << "\\033[0m";

        l += i->size ();
      }

      string type (o.type ().name ());

      std::set<string> arg_set;
      if (type != "bool" || doc.size () >= 3)
      {
        os << ' ';
        l++;

        string s (doc.size () > 0 ? doc[0] : string ("<arg>"));

        if (color)
          s = translate_arg (s, arg_set);

        s = format (ot_plain, s, false);

        os << escape_str (s);
        l += text_size (s);
      }

      // Figure out which documentation string we should use.
      //
      string d;
      {
        size_t i (type == "bool" && doc.size () < 3 ? 0 : 1);

        if (doc.size () > i) // Have at least one.
        {
          if (usage == ut_both && usage_ == ut_long)
          {
            d = doc.size () > i + 1 // Have both short and long?
              ? doc[i + 1]          // Then use long.
              : doc[i];
          }
          else // Short or long.
          {
            d = doc.size () > i + 1 // Have both short and long?
              ? doc[i]              // Then use short,
              : (usage == ut_long   // Otherwise, if asked for long,
                 ? doc[i]           // Then use long,
                 : first_sentence (doc[i])); // Else first sentence of long.
          }
        }
      }

      // Format the documentation string.
      //
      if (color)
        d = translate (d, arg_set);

      d = format (ot_plain, d, false);

      if (!d.empty ())
        wrap_lines (os, d, length_ + 1, l); // +1 for extra space after arg.
      else
        os << "\" << std::endl";

      os << ";"
         << endl;
    }

  private:
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

  private:
    size_t length_;
    usage_type usage_;
  };

  //
  //
  struct base_parse: traversal::class_, context
  {
    base_parse (context& c): context (c) {}

    virtual void
    traverse (type& c)
    {
      os << "// " << escape (c.name ()) << " base" << endl
         << "//" << endl
         << "if (" << fq_name (c) << "::_parse (o, s))" << endl
         << "return true;"
         << endl;
    }
  };

  //
  //
  struct base_desc: traversal::class_, context
  {
    base_desc (context& c): context (c) {}

    virtual void
    traverse (type& c)
    {
      os << "// " << escape (c.name ()) << " base" << endl
         << "//" << endl
         << fq_name (c) << "::fill (os);"
         << endl;
    }
  };

  struct base_usage: traversal::class_, context
  {
    base_usage (context& c, usage_type u): context (c), usage_ (u) {}

    virtual void
    traverse (type& c)
    {
      const char* t (
        usage != ut_both
        ? ""
        : usage_ == ut_short ? "short_" : "long_");

      os << "// " << escape (c.name ()) << " base" << endl
         << "//" << endl
         << fq_name (c) << "::print_" << t << "usage (os);"
         << endl;
    }

  private:
    usage_type usage_;
  };

  //
  //
  struct class_: traversal::class_, context
  {
    class_ (context& c)
        : context (c),
          base_parse_ (c),
          base_desc_ (c),
          option_map_ (c),
          option_desc_ (c)
    {
      inherits_base_parse_ >> base_parse_;
      inherits_base_desc_ >> base_desc_;
      names_option_map_ >> option_map_;
      names_option_desc_ >> option_desc_;
    }

    virtual void
    traverse (type& c)
    {
      string name (escape (c.name ()));

      bool abst (c.abstract ());
      bool ho (has<semantics::option> (c));
      bool hb (c.inherits_begin () != c.inherits_end ());

      os << "// " << name << endl
         << "//" << endl
         << endl;

      // c-tors
      //
      string um (cli + "::unknown_mode");

      os << name << "::" << endl
         << name << " ()";
      {
        option_init init (*this);
        traversal::names names_init (init);
        names (c, names_init);
      }
      os << "{"
         << "}";

      if (!abst)
      {
        bool p (options.generate_parse ());
        string n (
          p
          ? "void " + name + "::\n" + (name != "parse" ? "parse" : "parse_")
          : name + "::\n" + name);

        os << n << " (int& argc," << endl
           << "char** argv," << endl
           << "bool erase," << endl
           << um << " opt," << endl
           << um << " arg)";
        if (!p)
        {
          option_init init (*this);
          traversal::names names_init (init);
          names (c, names_init);
        }
        os << "{"
           << cli << "::argv_scanner s (argc, argv, erase);"
           << "_parse (s, opt, arg);"
           << "}";

        os << n << " (int start," << endl
           << "int& argc," << endl
           << "char** argv," << endl
           << "bool erase," << endl
           << um << " opt," << endl
           << um << " arg)";
        if (!p)
        {
          option_init init (*this);
          traversal::names names_init (init);
          names (c, names_init);
        }
        os << "{"
           << cli << "::argv_scanner s (start, argc, argv, erase);"
           << "_parse (s, opt, arg);"
           << "}";

        os << n << " (int& argc," << endl
           << "char** argv," << endl
           << "int& end," << endl
           << "bool erase," << endl
           << um << " opt," << endl
           << um << " arg)";
        if (!p)
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

        os << n << " (int start," << endl
           << "int& argc," << endl
           << "char** argv," << endl
           << "int& end," << endl
           << "bool erase," << endl
           << um << " opt," << endl
           << um << " arg)";
        if (!p)
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

        os << n << " (" << cli << "::scanner& s," << endl
           << um << " opt," << endl
           << um << " arg)";
        if (!p)
        {
          option_init init (*this);
          traversal::names names_init (init);
          names (c, names_init);
        }
        os << "{"
           << "_parse (s, opt, arg);"
           << "}";
      }

      // Usage.
      //
      if (usage != ut_none)
      {
        bool b (hb && !options.exclude_base ());

        // Calculate option length.
        //
        size_t len (0);
        {
          semantics::option* o (0);
          size_t max (options.option_length ());

          // We need to go into our bases unless --exclude-base was
          // specified.
          //
          {
            traversal::class_ ct;
            option_length olt (*this, len, o);
            traversal::inherits i;
            traversal::names n;

            if (b)
              ct >> i >> ct;

            ct >> n >> olt;
            ct.traverse (c);

            // Now do the same for each base and issue a warning if any
            // base has shorter option length than derived.
            //
            if (b && max == 0)
            {
              size_t d_len (len);
              semantics::option* d_o (o);

              for (type::inherits_iterator i (c.inherits_begin ());
                   i != c.inherits_end (); ++i)
              {
                type& b (i->base ());

                len = 0;
                ct.traverse (b);

                if (len == d_len)
                  continue;

                cerr << c.file () << ":" << c.line () << ":" << c.column ()
                     << " warning: derived class option length is greater "
                     << "than that of a base class '" << b.name () << "'"
                     << endl;

                cerr << b.file () << ":" << b.line () << ":" << b.column ()
                     << " note: class '" << b.name () << "' is defined here"
                     << endl;

                cerr << c.file () << ":" << c.line () << ":" << c.column ()
                     << " note: use --option-length to specify uniform length"
                     << endl;
              }

              len = d_len;
              o = d_o;
            }
          }

          if (len != 0 && max != 0)
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

        // If len is 0 then it means we have no options to print.
        //
        os << "void " << name << "::" << endl
           << "print_" << (usage == ut_both ? "short_" : "") << "usage (" <<
          options.ostream_type () << "&" << (len != 0 || b ? " os)" : ")")
           << "{";

        // Call our bases.
        //
        if (b)
        {
          base_usage t (*this, usage == ut_both ? ut_short : usage);
          traversal::inherits i (t);
          inherits (c, i);
        }

        // Print option usage.
        //
        {
          option_usage t (*this, len, usage == ut_both ? ut_short : usage);
          traversal::names n (t);
          names (c, n);
        }

        os << "}";

        // Long version.
        //
        if (usage == ut_both)
        {
          os << "void " << name << "::" << endl
             << "print_long_usage (" <<
            options.ostream_type () << "&" << (len != 0 || b ? " os)" : ")")
             << "{";

          if (b)
          {
            base_usage t (*this, ut_long);
            traversal::inherits i (t);
            inherits (c, i);
          }

          {
            option_usage t (*this, len, ut_long);
            traversal::names n (t);
            names (c, n);
          }

          os << "}";
        }
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
           << "{"
           << name << "::fill (os);"
           << "}"
           << "};";

        os << "static " << desc << "_init " << desc << "_init_ (" <<
          desc << "_);"
           << endl;

        os << "void " << name << "::" << endl
           << "fill (" << cli << "::options& " << (ho || hb ? " os)" : ")")
           << "{";

        // Add the entries from our bases first so that our entires
        // override any conflicts.
        //
        inherits (c, inherits_base_desc_);
        names (c, names_option_desc_);

        os << "}";

        os << "const " << cli << "::options& " << name << "::" << endl
           << "description ()"
           << "{"
           << "return " << desc << "_;"
           << "};";
      }

      // _parse ()
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

      os << "bool " << name << "::" << endl
         << "_parse (const char* o, " << cli << "::scanner& s)"
         << "{"
         << map << "::const_iterator i (" << map << "_.find (o));"
         << endl
         << "if (i != " << map << "_.end ())"
         << "{"
         << "(*(i->second)) (*this, s);"
         << "return true;"
         << "}";

      // Try our bases, from left-to-right.
      //
      inherits (c, inherits_base_parse_);

      os << "return false;"
         << "}";

      if (!abst)
      {
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
             << "}";

        os << "if (" << (sep ? "opt && " : "") << "_parse (o, s));";

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
    }

  private:
    base_parse base_parse_;
    traversal::inherits inherits_base_parse_;

    base_desc base_desc_;
    traversal::inherits inherits_base_desc_;

    option_map option_map_;
    traversal::names names_option_map_;

    option_desc option_desc_;
    traversal::names names_option_desc_;
  };

  // Page usage.
  //
  enum paragraph {para_none, para_text, para_option};

  struct doc: traversal::doc, context
  {
    doc (context& c, usage_type u, paragraph& p)
        : context (c), usage_ (u), para_ (p) {}

    virtual void
    traverse (type& ds)
    {
      if (ds.name ().compare (0, 3, "doc") != 0) // Ignore doc variables.
        return;

      // Figure out which documentation string we should use.
      //
      // n = 1 - common doc string
      // n = 2 - arg string, common doc string
      // n > 2 - arg string, short string, long string
      //
      size_t n (ds.size ());
      string d;

      if (usage == ut_both && usage_ == ut_long)
      {
        d = n > 2                     // Have both short and long?
          ? ds[2]                     // Then use long.
          : (n == 1 ? ds[0] : ds[1]); // Else, use common.
      }
      else // Short or long.
      {
        d = n > 2                     // Have both short and long?
          ? ds[1]                     // Then use short,
          : (n == 1 ? ds[0] : ds[1]); // Else, use common (no first sentence).
      }

      std::set<string> arg_set;
      if (n > 1 && options.ansi_color ())
        translate_arg (ds[0], arg_set);

      d = format (ot_plain, translate (d, arg_set), true);

      if (d.empty ())
        return;

      if (para_ == para_none) // First.
        os << "os << \"";
      else
        os << "os << ::std::endl" << endl
           << "   << \"";

      wrap_lines (os, d);
      os << ";"
         << endl;

      para_ = para_text;
    }

  private:
    usage_type usage_;
    paragraph& para_;
  };

  struct class_usage: traversal::class_, context
  {
    class_usage (context& c, usage_type u, paragraph& p)
        : context (c), usage_ (u), para_ (p) {}

    virtual void
    traverse (type& c)
    {
      // Figure out if this class has any documentation by calculating
      // the option length. If it is 0, then we don't have any.
      //
      size_t len (0);
      {
        option_length olt (*this, len);
        traversal::class_ ct;
        traversal::inherits i;
        traversal::names n;

        if (!options.exclude_base ()) // Go into bases unless --exclude-base.
          ct >> i >> ct;

        ct >> n >> olt;
        ct.traverse (c);
      }

      if (len == 0)
        return;

      if (para_ == para_text)
        os << "os << ::std::endl;";

      const char* t (
        usage != ut_both
        ? ""
        : usage_ == ut_short ? "short_" : "long_");

      os << fq_name (c) << "::print_" << t << "usage (os);"
         << endl;

      para_ = para_option;
    }

  private:
    usage_type usage_;
    paragraph& para_;
  };
}

void
generate_source (context& ctx)
{
  ostream& os (ctx.os);

  os << "#include <map>" << endl
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

  // Entire page usage.
  //
  if (ctx.usage != ut_none && ctx.options.page_usage_specified ())
  {
    const string& qn (ctx.options.page_usage ());
    string n (ctx.escape (ctx.substitute (ctx.ns_open (qn, false))));

    usage u (ctx.usage);
    string const& ost (ctx.options.ostream_type ());

    {
      os << "void" << endl
         << n << (u == ut_both ? "short_" : "") << "usage (" << ost << "& os)"
         << "{"
         << "CLI_POTENTIALLY_UNUSED (os);"
         << endl;

      paragraph para (para_none);

      traversal::cli_unit unit;
      traversal::names unit_names;
      traversal::namespace_ ns;
      doc dc (ctx, u == ut_both ? ut_short : u, para);
      class_usage cl (ctx, u == ut_both ? ut_short : u, para);

      unit >> unit_names;
      unit_names >> dc;
      unit_names >> ns;
      unit_names >> cl;

      traversal::names ns_names;

      ns >> ns_names;
      ns_names >> dc;
      ns_names >> ns;
      ns_names >> cl;

      unit.dispatch (ctx.unit);

      os << "}";
    }

    // Long version.
    //
    if (u == ut_both)
    {
      os << "void" << endl
         << n << "long_usage (" << ost << "& os)"
         << "{"
         << "CLI_POTENTIALLY_UNUSED (os);"
         << endl;

      paragraph para (para_none);

      traversal::cli_unit unit;
      traversal::names unit_names;
      traversal::namespace_ ns;
      doc dc (ctx, ut_long, para);
      class_usage cl (ctx, ut_long, para);

      unit >> unit_names;
      unit_names >> dc;
      unit_names >> ns;
      unit_names >> cl;

      traversal::names ns_names;

      ns >> ns_names;
      ns_names >> dc;
      ns_names >> ns;
      ns_names >> cl;

      unit.dispatch (ctx.unit);

      os << "}";
    }

    ctx.ns_close (qn, false);
  }
}
