// file      : cli/context.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// license   : MIT; see accompanying LICENSE file

#include <stack>
#include <vector>
#include <cstring>  // strncmp()
#include <fstream>
#include <sstream>
#include <utility>  // move()
#include <iostream>

#include <cli/context.hxx>

using namespace std;

namespace
{
  char const* keywords[] =
  {
    "NULL",
    "and",
    "asm",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "compl",
    "const",
    "const_cast",
    "continue",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "end_eq",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "not",
    "not_eq",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq"
  };
}

context::
context (ostream& os_,
         output_type ot_,
         semantics::cli_unit& unit_,
         options_type const& ops)
    : data_ (new (shared) data),
      os (os_),
      unit (unit_),
      options (ops),
      ot (ot_),
      gen_modifier (options.generate_modifier ()),
      gen_specifier (options.generate_specifier () ||
                     options.generate_merge ()),
      gen_parse (options.generate_parse ()),
      gen_merge (options.generate_merge ()),
      inl (data_->inl_),
      opt_prefix (options.option_prefix ()),
      opt_sep (options.option_separator ()),
      cli (data_->cli_),
      exp (data_->exp_),
      exp_inl (data_->exp_inl_),
      reserved_name_map (options.reserved_name ()),
      keyword_set (data_->keyword_set_),
      link_regex (data_->link_regex_),
      id_set (data_->id_set_),
      ref_set (data_->ref_set_),
      heading_map (data_->heading_map_),
      toc (data_->toc_),
      tocs (data_->tocs_)
{
  if (options.suppress_usage ())
    gen_usage = ut_none;
  else
  {
    if (options.long_usage ())
      gen_usage = options.short_usage () ? ut_both : ut_long;
    else
      gen_usage = ut_short;
  }

  if (!options.suppress_inline ())
    data_->inl_ = "inline ";

  data_->cli_ = options.cli_namespace ();

  if (!cli.empty () && cli[0] != ':')
    data_->cli_ = "::" + data_->cli_;

  data_->exp_ = options.export_symbol ();

  if (!exp.empty ())
  {
    data_->exp_ += ' ';

    if (options.suppress_inline ())
      data_->exp_inl_ = data_->exp_;
  }

  for (size_t i (0); i < sizeof (keywords) / sizeof (char*); ++i)
    data_->keyword_set_.insert (keywords[i]);

  // Link regex.
  //
  for (vector<string>::const_iterator i (ops.link_regex ().begin ());
       i != ops.link_regex ().end (); ++i)
  {
    try
    {
      data_->link_regex_.push_back (regexsub (*i));
    }
    catch (const regex_format& e)
    {
      cerr << "error: invalid regex '" << *i << "': " << e.what () << endl;
      throw generation_failed ();
    }
  }

  toc = 0;
}

context::
context (context& c)
    : data_ (c.data_),
      os (c.os),
      unit (c.unit),
      options (c.options),
      ot (c.ot),
      gen_modifier (c.gen_modifier),
      gen_specifier (c.gen_specifier),
      gen_parse (c.gen_parse),
      gen_merge (c.gen_merge),
      gen_usage (c.gen_usage),
      inl (c.inl),
      opt_prefix (c.opt_prefix),
      opt_sep (c.opt_sep),
      cli (c.cli),
      exp (c.exp),
      exp_inl (c.exp_inl),
      reserved_name_map (c.reserved_name_map),
      keyword_set (c.keyword_set),
      link_regex (c.link_regex),
      id_set (c.id_set),
      ref_set (c.ref_set),
      heading_map (c.heading_map),
      toc (c.toc),
      tocs (c.tocs)
{
}

string context::
escape (string const& name) const
{
  typedef string::size_type size;

  string r;
  size n (name.size ());

  // In most common cases we will have that many characters.
  //
  r.reserve (n);

  for (size i (0); i < n; ++i)
  {
    char c (name[i]);

    if (i == 0)
    {
      if (!((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_'))
        r = (c >= '0' && c <= '9') ? "cxx_" : "cxx";
    }

    if (!((c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') ||
          c == '_'))
      r += '_';
    else
      r += c;
  }

  if (r.empty ())
    r = "cxx";

  // Custom reserved words.
  //
  reserved_name_map_type::const_iterator i (reserved_name_map.find (r));

  if (i != reserved_name_map.end ())
  {
    if (!i->second.empty ())
      return i->second;
    else
      r += L'_';
  }

  // Keywords
  //
  if (keyword_set.find (r) != keyword_set.end ())
  {
    r += '_';

    // Re-run custom words.
    //
    i = reserved_name_map.find (r);

    if (i != reserved_name_map.end ())
    {
      if (!i->second.empty ())
        return i->second;
      else
        r += L'_';
    }
  }

  return r;
}

string context::
process_link_target (const string& tg)
{
  bool t (options.link_regex_trace ());

  if (t)
    cerr << "link: '" << tg << "'" << endl;

  string r;
  bool found (false);

  for (regex_mapping::const_iterator i (link_regex.begin ());
       i != link_regex.end (); ++i)
  {
    if (t)
      cerr << "try: '" << i->regex () << "' : ";

    if (i->match (tg))
    {
      r = i->replace (tg);
      found = true;

      if (t)
        cerr << "'" << r << "' : ";
    }

    if (t)
      cerr << (found ? '+' : '-') << endl;

    if (found)
      break;
  }

  return found ? r : tg;
}

void context::
preprocess_ascii_tree (string& s)
{
  // tree --charset=UTF-8 uses the following box-drawing characters (see
  // color.c):
  //
  // CHAR       UTF-8    ASCII
  //----------------------------
  //
  // |          E29482   |
  //
  // --         E29480   -
  //
  // |-         E2949C   |
  //
  // |_         E29494   `
  //
  // <nbspace>    C2A0   <space>
  //
  // Note that here we rely on the fact that neither E2 nor C2 can appear as
  // continuation bytes.
  //
  for (size_t i (0); i != s.size (); ++i)
  {
    i = s.find_first_of ("\xE2\xC2", i);

    if (i == string::npos)
      break;

    if (s[i] == '\xE2')
    {
      if (s[i + 1] == '\x94')
      {
        const char* r;
        switch (s[i + 2])
        {
        case '\x80': r = "-"; break;
        case '\x82':
        case '\x9c': r = "|"; break;
        case '\x94': r = "`"; break;
        default:               continue;
        }

        s.replace (i, 3, r);
      }
    }
    else
    {
      if (s[i + 1] == '\xA0')
        s.replace (i, 2, " ");
    }
  }
}

string context::
translate_arg (string const& s, std::set<string>& set)
{
  string r;
  r.reserve (s.size ());
  set.clear ();

  size_t p (string::npos);

  for (size_t i (0), n (s.size ()); i < n; ++i)
  {
    if (p == string::npos && s[i] == '<')
    {
      p = i;
      r += "\\ci{";
      continue;
    }

    if (p != string::npos && s[i] == '>')
    {
      set.insert (string (s, p + 1, i - p - 1));
      r += '}';
      p = string::npos;
      continue;
    }

    if (p != string::npos && s[i] == '}' && s[i - 1] != '\\')
    {
      r += "\\}";
      continue;
    }

    r += s[i];
  }

  return r;
}

string context::
translate (string const& s, std::set<string> const& set)
{
  string r;
  r.reserve (s.size ());

  bool pre (false);
  size_t p (string::npos);

  for (size_t i (0), n (s.size ()); i < n; ++i)
  {
    char c (s[i]);

    // Skip pre-formatted fragments.
    //
    if (c == (pre ? 0x03 : 0x02))
    {
      pre = !pre;

      if (p != string::npos)
      {
        assert (pre);
        r.append (s, p, i - p);
        p = string::npos;
      }
    }
    else if (!pre)
    {
      if (p == string::npos && c == '<')
      {
        p = i;
        continue;
      }

      if (p != string::npos)
      {
        if (c == '>')
        {
          string a (s, p + 1, i - p - 1);

          if (set.find (a) != set.end ())
          {
            r += "\\ci{";

            for (size_t j (0), n (a.size ()); j < n; ++j)
            {
              if (a[j] == '}' && (j == 0 || a[j - 1] != '\\'))
                r += "\\}";
              else
                r += a[j];
            }

            r += '}';
          }
          else
          {
            r += '<';
            r += a;
            r += '>';
          }
          p = string::npos;
        }
        continue;
      }
    }

    r += c;
  }

  // If we found the opening '<' but no closing '>', add the rest.
  //
  if (p != string::npos)
    r.append (s, p, string::npos);

  return r;
}

void context::
format_line (output_type ot, string& r, const char* l, size_t n)
{
  bool color (options.ansi_color ());

  typedef unsigned char span; // Mask.

  const span code = 1;
  const span itlc = 2;
  const span bold = 4;
  const span link = 8;
  const span note = 16;

  vector<span> spans;

  string link_target;
  string link_section;     // If not empty, man section; target is man name.
  bool link_empty (false); // Link has no text.

  bool escape (false);
  for (size_t i (0); i < n; ++i)
  {
    char c (l[i]);

    if (escape)
    {
      bool new_span (false);

      switch (c)
      {
      case ' ':
        {
          // Non-ignorable space.
          //
          switch (ot)
          {
          case ot_plain:
            {
              r += ' ';
              break;
            }
          case ot_html:
            {
              r += "&#160;";
              break;
            }
          case ot_man:
            {
              r += "\\ ";
              break;
            }
          }

          break;
        }
      case '-':
        {
          // N-dash. If someone wants m-dash, can use \-- with \-{}- as
          // a way to "escape" an n-dash followed by hyphen.
          //
          switch (ot)
          {
          case ot_plain:
            {
              r += "--";
              break;
            }
          case ot_html:
            {
              r += "&#8211;";
              break;
            }
          case ot_man:
            {
              r += "\\(en";
              break;
            }
          }

          break;
        }
      case 'n':
        {
          switch (ot)
          {
          case ot_plain:
            {
              r += '\n';
              break;
            }
          case ot_html:
            {
              if (!r.empty () && r[r.size () - 1] != '\n')
                r += '\n';

              r += "<br/>";
              break;
            }
          case ot_man:
            {
              if (!r.empty () && r[r.size () - 1] != '\n')
                r += '\n';

              // Note that if we have several consecutive breaks, they
              // will be collapsed into a single one. No, .sp doesn't
              // work (or, more exactly, will only work for two breaks).
              //
              r += ".br";
              break;
            }
          }

          // Skip following spaces.
          //
          for (; i + 1 < n && l[i + 1] == ' '; ++i) ;

          switch (ot)
          {
          case ot_plain: break;
          case ot_html:
          case ot_man:
            {
              if (i + 1 < n) // More text in this paragraph?
                r += "\n";

              break;
            }
          }

          break;
        }
      case 'c':
        {
          span s (code);
          size_t j (i + 1);

          if (j < n)
          {
            if (l[j] == 'i')
            {
              s |= itlc;
              j++;

              if (j < n && l[j] == 'b')
              {
                s |= bold;
                j++;
              }
            }
            else if (l[j] == 'b')
            {
              s |= bold;
              j++;

              if (j < n && l[j] == 'i')
              {
                s |= itlc;
                j++;
              }
            }
          }

          if (j < n && l[j] == '{')
          {
            i = j;
            spans.push_back (s);
            new_span = true;
            break;
          }

          r += 'c';
          break;
        }
      case 'i':
        {
          span s (itlc);
          size_t j (i + 1);

          if (j < n)
          {
            if (l[j] == 'c')
            {
              s |= code;
              j++;

              if (j < n && l[j] == 'b')
              {
                s |= bold;
                j++;
              }
            }
            else if (l[j] == 'b')
            {
              s |= bold;
              j++;

              if (j < n && l[j] == 'c')
              {
                s |= code;
                j++;
              }
            }
          }

          if (j < n && l[j] == '{')
          {
            i = j;
            spans.push_back (s);
            new_span = true;
            break;
          }

          r += 'i';
          break;
        }
      case 'b':
        {
          span s (bold);
          size_t j (i + 1);

          if (j < n)
          {
            if (l[j] == 'c')
            {
              s |= code;
              j++;

              if (j < n && l[j] == 'i')
              {
                s |= itlc;
                j++;
              }
            }
            else if (l[j] == 'i')
            {
              s |= itlc;
              j++;

              if (j < n && l[j] == 'c')
              {
                s |= code;
                j++;
              }
            }
          }

          if (j < n && l[j] == '{')
          {
            i = j;
            spans.push_back (s);
            new_span = true;
            break;
          }

          r += 'b';
          break;
        }
      case 'l':
        {
          if (i + 1 < n && l[i + 1] == '{')
          {
            string& t (link_target);

            if (!t.empty ())
            {
              cerr << "error: nested links in documentation paragraph '"
                   << string (l, 0, n) << "'" << endl;
              throw generation_failed ();
            }

            // Find the end of the link target.
            //
            size_t b (++i + 1), e (b);
            for (; i + 1 < n; ++i)
            {
              char c (l[i + 1]);

              if (c == ' ' || c == '}')
              {
                e = i + 1;

                if (c == ' ') // Skip spaces.
                  for (++i; i + 1 < n && l[i + 1] == ' '; ++i) ;

                break;
              }
            }

            // Run the link target through format_line(ot_plain) to handle
            // escaping (e.g., \\$).
            //
            format_line (ot_plain, t, l + b, e - b);

            if (t.empty ())
            {
              cerr << "error: missing link target in documentation paragraph '"
                   << string (l, 0, n) << "'" << endl;
              throw generation_failed ();
            }

            // See if link_target is a man page name/section.
            //
            size_t o (t.find ('('));

            if (o != string::npos)
            {
              link_section.assign (t, o + 1, t.size () - o - 2);

              if (t[t.size () - 1] != ')' || link_section.empty ())
              {
                cerr << "error: missing man section in '" << t << "'" << endl;
                throw generation_failed ();
              }

              string n (t, 0, o);

              if (n.empty ())
              {
                cerr << "error: missing man page in '" << t << "'" << endl;
                throw generation_failed ();
              }

              t = n;
            }
            else
              link_section.clear ();

            // If this is a local fragment reference, add it to the set to be
            // verified at the end.
            //
            if (t[0] == '#')
            {
              assert (link_section.empty ()); // Not a man page.
              ref_set.insert (string (t, 1, string::npos));
            }

            link_empty = i + 1 < n && l[i + 1] == '}';

            spans.push_back (link);
            new_span = true;
            break;
          }

          r += 'l';
          break;
        }
        case 'N':
        {
          if (i + 1 < n && l[i + 1] == '{')
          {
            ++i;
            spans.push_back (note);
            new_span = true;
            break;
          }

          r += 'N';
          break;
        }
      case '\\':
        {
          switch (ot)
          {
          case ot_man:
            {
              r += "\\e";
              break;
            }
          default:
            {
              r += '\\';
              break;
            }
          }
          break;
        }
      case '"':
        {
          r += '"';
          break;
        }
      case '\'':
        {
          r += '\'';
          break;
        }
      case '}':
        {
          r += '}';
          break;
        }
      case '|':
        {
          r += '|';
          break;
        }
      default:
        {
          cerr << "error: unknown escape sequence '\\" << c << "' in "
               << "documentation paragraph '" << string (l, 0, n) << "'"
               << endl;
          throw generation_failed ();
        }
      }

      // If we just added a new span, add corresponding output markup.
      //
      if (new_span)
      {
        span s (spans.back ());

        span es (0); // Effective span.
        for (vector<span>::iterator i (spans.begin ());
             i != spans.end ();
             ++i)
          es |= *i & (code | itlc | bold);

        switch (ot)
        {
        case ot_plain:
          {
            if ((s & note) != 0)
            {
              r += "[Note: ";
            }
            else if ((s & link) == 0)
            {
              if (color)
              {
                if (s & bold)
                  r += "\033[1m";

                if (s & itlc)
                  r += "\033[4m";
              }
            }

            break;
          }
        case ot_html:
          {
            if (s & note)
            {
              r += "<span class=\"note\">";
            }
            else if (s & link)
            {
              r += "<a href=\"";

              // It might be useful to include the man section into the regex
              // somehow.
              //
              string t (link_section.empty ()
                        ? link_target
                        : link_target + options.html_suffix ());

              string pt (process_link_target (t));

              if (pt.empty ())
              {
                cerr << "error: link '" << t << "' became empty" << endl;
                throw generation_failed ();
              }

              r += pt;
              r += "\">";
            }
            else
            {
              if (s & code)
                r += "<code>";

              if (s & itlc)
                r += "<i>";

              if (s & bold)
                r += "<b>";
            }

            break;
          }
        case ot_man:
          {
            if ((s & note) != 0)
            {
              cerr << "error: \\N{} in man output not yet supported" << endl;
              throw generation_failed ();
            }

            if ((s & link) == 0)
            {
              if ((es & itlc) && (es & bold))
                r += "\\f(BI";
              else if (es & itlc)
                r += "\\fI";
              else if (es & bold)
                r += "\\fB";
            }

            break;
          }
        }
      }

      escape = false;
    }
    else // Not escape.
    {
      switch (c)
      {
      case '\\':
        {
          escape = true;
          break;
        }
      case '.':
        {
          if (ot == ot_man)
            r += "\\.";
          else
            r += '.';
          break;
        }
      case '}':
        {
          if (!spans.empty ())
          {
            span s (spans.back ());
            spans.pop_back ();

            span es (0); // New effective span.
            for (vector<span>::iterator i (spans.begin ());
                 i != spans.end ();
                 ++i)
              es |= *i & (code | itlc | bold);

            switch (ot)
            {
            case ot_plain:
              {
                if ((s & note) != 0)
                {
                  r += ']';
                }
                else if ((s & link) != 0)
                {
                  string t (link_section.empty ()
                            ? link_target
                            : link_target + "(" + link_section + ")");

                  string pt (process_link_target (t));

                  if (pt.empty ())
                  {
                    if (link_empty)
                    {
                      cerr << "error: link target '" << t << "' became empty "
                           << "and link text is also empty" << endl;
                      throw generation_failed ();
                    }
                  }
                  else
                  {
                    if (!link_empty)
                      r += " (";

                    if (link_section.empty ())
                      r += pt;
                    else
                    {
                      if (color)
                        r += "\033[1m";

                      r += pt;

                      if (color)
                        r += "\033[0m";
                    }

                    if (!link_empty)
                      r += ")";
                  }
                }
                else
                {
                  if (color)
                  {
                    // While there are codes to turn off bold (22) and
                    // underline (24), it is not clear how widely they
                    // are supported.
                    //
                    r += "\033[0m"; // Clear all.

                    if (es & bold)
                      r += "\033[1m";

                    if (es & itlc)
                      r += "\033[4m";
                  }
                }

                break;
              }
            case ot_html:
              {
                if ((s & note) != 0)
                {
                  r += "</span>";
                }
                else if ((s & link) != 0)
                {
                  if (link_empty)
                  {
                    if (link_section.empty ())
                      r += link_target;
                    else
                    {
                      r += "<code><b>";
                      r += link_target + "(" + link_section + ")";
                      r += "</b></code>";
                    }
                  }

                  r += "</a>";
                }
                else
                {
                  if (s & bold)
                    r += "</b>";

                  if (s & itlc)
                    r += "</i>";

                  if (s & code)
                    r += "</code>";
                }

                break;
              }
            case ot_man:
              {
                assert ((s & note) == 0);

                if ((s & link) != 0)
                {
                  string t (link_section.empty ()
                            ? link_target
                            : link_target + "(" + link_section + ")");

                  string pt (process_link_target (t));

                  if (pt.empty ())
                  {
                    if (link_empty)
                    {
                      cerr << "error: link target '" << t << "' became empty "
                           << "and link text is also empty" << endl;
                      throw generation_failed ();
                    }
                  }
                  else
                  {
                    if (!link_empty)
                      r += " (";

                    if (link_section.empty ())
                      r += pt;
                    else
                      r += "\\fB" + pt + "\\fP";

                    if (!link_empty)
                      r += ")";
                  }
                }
                else
                {
                  // At first sight, \fP (select previous font) looks like
                  // exactly what we need here. However, it doesn't quite
                  // have the stack semantics that we need.
                  //
                  if ((es & itlc) && (es & bold))
                    r += "\\f(BI";
                  else if (es & itlc)
                    r += "\\fI";
                  else if (es & bold)
                    r += "\\fB";
                  else
                    r += "\\fR";
                }

                break;
              }
            }

            if (s & link)
              link_target.clear ();

            break;
          }
        }
        // Fall through.
      default:
        r += c;
        break;
      }
    }
  }

  if (escape)
  {
    cerr << "error: unterminated escape sequence in documentation "
         << "paragraph '" << string (l, 0, n) << "'" << endl;
    throw generation_failed ();
  }

  if (!spans.empty ())
  {
    unsigned char b (spans.back ());
    string bs;

    if (b & code) bs += 'c';
    if (b & itlc) bs += 'i';
    if (b & bold) bs += 'b';
    if (b & link) bs  = 'l';
    if (b & note) bs  = 'N';


    cerr << "error: unterminated formatting span '\\" << bs << "' "
         << "in documentation paragraph '" << string (l, 0, n) << "'" << endl;
    throw generation_failed ();
  }
}

struct block
{
  // The semantic meaning of \hN and their mapping to HTML is as follows:
  //
  // \h0 - preface       <h1 class="preface">
  // \H  - part          <h1 class="part">
  // \h1 - chapter       <h1>
  // \h  - section       <h1> or <h2>
  // \h2 - sub-section   <h3>
  //
  // In HTML, if \h0 or \h1 was seen, then \h is automatically mappend to
  // <h2>. Otherwise it is <h1>.
  //
  // The specifier (0, H, 1, h, 2) is stored in header.
  //
  enum kind_type {h, ul, ol, dl, li, note, text, pre};

  kind_type kind;
  bool para;     // True if first text fragment should be in own paragraph.

  string id;
  string header;
  string value;
  string trailer;

  block (kind_type k, bool p, const string& i, const string& h = "")
      : kind (k), para (p), id (i), header (h) {}
};

static const char* block_kind_str[] = {
  "\\h", "\\ul", "\\ol", "\\dl", "\\li", "\\N", "text", "preformatted text"};

inline ostream&
operator<< (ostream& os, block::kind_type k)
{
  return os << block_kind_str[k];
}

// Given the contents of a block element, convert any leading/trailing <br/>
// elements to top/bottom margins. While conceptually it would seem better to
// translate them to padding, margins actually give better practical results
// since they overlap with container's margins.
//
static string
html_margin (string& v)
{
  size_t top (0), bot (0);

  const char* b (v.c_str ());
  const char* e (v.c_str () + v.size ());

  for (; e - b >= 5 && strncmp (b, "<br/>", 5) == 0; ++top)
  {
    b += 5;

    if (b != e && *b == '\n') // Remove following newline, if any.
      ++b;
  }

  for (; e - b >= 5 && strncmp (e - 5, "<br/>", 5) == 0; ++bot)
  {
    e -= 5;

    if (e != b && *(e - 1) == '\n') // Remove preceding newline, if any.
      --e;
  }

  if (top == 0 && bot == 0)
    return "";

  string t;
  t.swap (v);
  v.assign (b, e - b);

  ostringstream os;

  if (top != 0)
    os << "margin-top:" << top << "em";

  if (bot != 0)
    os << (top != 0 ? ";" : "") << "margin-bottom:" << bot << "em";

  return os.str ();
}

// The same idea except there are no margins. So we just strip .br.
//
static void
man_margin (string& v)
{
  size_t top (0), bot (0);

  const char* b (v.c_str ());
  const char* e (v.c_str () + v.size ());

  for (; e - b >= 3 && strncmp (b, ".br", 3) == 0; ++top)
  {
    b += 3;

    if (b != e && *b == '\n') // Remove following newline, if any.
      ++b;
  }

  for (; e - b >= 3 && strncmp (e - 3, ".br", 3) == 0; ++bot)
  {
    e -= 3;

    if (e != b && *(e - 1) == '\n') // Remove preceding newline, if any.
      --e;
  }

  if (top != 0 || bot != 0)
  {
    string t;
    t.swap (v);
    v.assign (b, e - b);
  }
}

string context::
format (semantics::scope& scope, string const& s, bool para)
{
  stack<block> blocks;
  blocks.push (block (block::text, para, "")); // Top-level.

  // Number of li in ol. Since we don't support nested lists (except in HTML
  // where this is unused), we don't need to push it into the stack.
  //
  size_t ol_count (0);

  // Mapping of \h to HTML tag. By default it is <h1> until we encounter
  // \h0 or \h1 at which point we change it to <h2>. It can also be mapped
  // with --html-heading-map.
  //
  char html_h ('\0');

  typedef map<char, string> html_hmap;
  if (ot == ot_html)
  {
    const html_hmap& hm (options.html_heading_map ());
    html_hmap::const_iterator mi (hm.find ('h'));

    if (mi != hm.end ())
    {
      // Note: this mapping back is necessary for TOC to function correctly
      // with multiple string fragments.
      //
      if      (mi->second == "h1") html_h = '1';
      else if (mi->second == "h2") html_h = '2';
    }
  }

  bool last (false);
  for (size_t b (0), e; !last; b = e + 1)
  {
    bool pre (s[b] == 0x02);

    const char* l;
    size_t n;
    string subst; // Variable-substituted.

    if (pre)
    {
      ++b; // Skip 0x02.

      e = s.find (0x03, b);
      assert (e != string::npos);

      l = s.c_str () + b;
      n = e - b;

      ++e; // Skip newline that follows 0x03.
      last = (e == s.size ());
    }
    else
    {
      e = s.find ('\n', b);
      last = (e == string::npos);

      l = s.c_str () + b;
      n = (last ? s.size () : e) - b;

      // Perform variable expansions (\$var$). Also detect the switch
      // to/from TOC mode.
      //
      unsigned short t (toc);
      if (substitute (scope, l, n, subst))
      {
        if (subst.empty ())
          continue;

        if (t != toc)
        {
          // This is a TOC prologue/epilogue (returned by start/end_toc()) and
          // we shouldn't be formatting it.
          //
          assert (blocks.size () == 1);
          string& v (blocks.top ().value);

          // Require that \$TOC$ appears in its own doc string. Failed this
          // it is tricky to get indentation right.
          //
          if (!v.empty () || !last)
          {
            cerr << "error: TOC variable should be in its own documentation "
                 << "string" << endl;
            throw generation_failed ();
          }

          switch (ot)
          {
          case ot_plain: break;
          case ot_html:
            {
              // Different "newline protocol" inside TOC.
              //
              v += subst;

              if (toc)
                v += '\n';
            }
          case ot_man: break;
          }

          continue;
        }

        l = subst.c_str ();
        n = subst.size ();
      }
    }

    // If this is TOC phase 2, then we simply ignore everything.
    //
    if (toc == 2)
      continue;

    const char* ol (l); // Original, full line for diagnostics.
    size_t on (n);

    // First determine what kind of paragraph block this is.
    //
    block::kind_type k (block::h);
    string id;
    string header;
    string trailer;

    size_t pop (0); // Pop count.

    if (pre)
    {
      k = block::pre;
    }
    else
    {
      // \h \H
      //
      if (n >= 3 &&
          l[0] == '\\' &&
          (l[1] == 'h' || l[1] == 'H') &&
          (l[2] == '|' || l[2] == '#'))
      {
        k = block::h;
        header = l[1];
        l += 3;
        n -= 3;
      }
      //
      // \h0 \h1 \h2
      //
      else if (n >= 4 &&
               l[0] == '\\' &&
               l[1] == 'h' &&
               (l[3] == '|' || l[3] == '#'))
      {
        if (l[2] != '0' && l[2] != '1' && l[2] != '2')
        {
          cerr << "error: '0', '1', or '2' expected in \\hN| in '"
               << string (ol, 0, on) << "'" << endl;
          throw generation_failed ();
        }

        k = block::h;
        header = l[2];
        l += 4;
        n -= 4;
      }
      //
      // \ul \ol \dl
      //
      else if (n >= 4 &&
               l[0] == '\\' &&
               (l[1] == 'u' || l[1] == 'o' || l[1] == 'd') &&
               l[2] == 'l' &&
               (l[3] == '|' || l[3] == '#'))
      {
        switch (l[1])
        {
        case 'u': k = block::ul; break;
        case 'o': k = block::ol; break;
        case 'd': k = block::dl; break;
        }

        l += 4;
        n -= 4;
      }
      //
      // \li
      //
      else if (n >= 4 &&
               l[0] == '\\' &&
               l[1] == 'l' &&
               l[2] == 'i' &&
               (l[3] == '|' || l[3] == '#'))
      {
        k = block::li;
        l += 4;
        n -= 4;
      }
      //
      // \N (note)
      //
      else if (n >= 3 &&
               l[0] == '\\' &&
               l[1] == 'N' &&
               (l[2] == '|' || l[2] == '#'))
      {
        k = block::note;
        l += 3;
        n -= 3;
      }
      else
        k = block::text;

      // Get the id, if present.
      //
      if (k != block::text && *(l - 1) == '#')
      {
        for (; n != 0 && *l != '|'; ++l, --n)
          id += *l;

        if (n == 0)
        {
          cerr << "error: paragraph begin '|' expected after id in '"
               << string (ol, 0, on) << "'" << endl;
          throw generation_failed ();
        }

        ++l; --n; // Skip '|'.
      }

      // Skip leading spaces after opening '|'.
      //
      if (k != block::text)
        while (n != 0 && (*l == 0x20 || *l == 0x0D || *l == 0x09)) {++l; --n;}

      // Next figure out how many blocks we need to pop at the end of this
      // paragraph. Things get a bit complicated since '|' could be escaped.
      //
      for (; n - pop > 0 && l[n - pop - 1] == '|'; ++pop) ;
      if (pop != 0)
      {
        // To determine whether the first '|' is part of an escape sequence
        // we have to find the first non-backslash character and then figure
        // out who escapes whom.
        //
        size_t ec (0); // Escape count.
        for (; n - pop - ec > 0 && l[n - pop - ec - 1] == '\\'; ++ec) ;

        // If we have an odd number of backslashes, then the last '|' is
        // escaped.
        //
        if (ec % 2 != 0)
          --pop;

        n -= pop; // Number of special '|' at the end.

        // Skip trailing spaces before closing '|'.
        //
        while (n != 0 && (l[n - 1] == 0x20 ||
                          l[n - 1] == 0x0D ||
                          l[n - 1] == 0x09)) n--;
      }
    }

    // Outer block kind.
    //
    block::kind_type ok (blocks.top ().kind);

    // Verify that this block type is valid in this context. Ignore
    // empty text blocks (can happen if we just have '|').
    //
    if (k != block::text || n != 0)
    {
      bool good (true);

      switch (ok)
      {
      case block::h: good = false; break;
      case block::ul:
      case block::ol:
      case block::dl: good = (k == block::li); break;
      case block::li: good = (k == block::note ||
                              k == block::text ||
                              k == block::pre  ||
                              (ot == ot_html && (k == block::ul ||
                                                 k == block::ol ||
                                                 k == block::dl))); break;
      case block::note: good = (k == block::text ||
                                k == block::pre  ||
                                (ot == ot_html && (k == block::ul ||
                                                   k == block::ol ||
                                                   k == block::dl))); break;
      case block::text: good = (k != block::li); break;
      case block::pre: assert (false);
      }

      if (!good)
      {
        cerr << "error: " << k << " inside " << ok << " "
             << "in documentation string '" << s << "'" << endl;
        throw generation_failed ();
      }
    }

    // Check id for duplicates. Do it only on the non-TOC pass.
    //
    if (!toc && !id.empty ())
    {
      if (!id_set.insert (id).second)
      {
        cerr << "error: duplicate id '" << id << "' in documentation "
             << "string '" << s << "'" << endl;
        throw generation_failed ();
      }
    }

    // Verify the block itself.
    //
    switch (k)
    {
    case block::h:
      {
        // \h blocks are only valid if we are required to start a new
        // paragraph (first_para is true).
        //
        if (!para)
        {
          cerr << "error: paragraph '" << string (ol, 0, on) << "' "
               << "not allowed in '" << s << "'" << endl;
          throw generation_failed ();
        }

        // \h must be single-paragraph.
        //
        if (pop == 0)
        {
          cerr << "error: '|' expected at the end of paragraph '"
               << string (ol, 0, on) << "'" << endl;
          throw generation_failed ();
        }

        // \h must not be empty.
        //
        if (n == 0)
        {
          cerr << "error: empty paragraph '" << string (ol, 0, on) << "' "
               << "in documentation string '" << s << "'" << endl;
          throw generation_failed ();
        }

        break;
      }
    case block::ul:
    case block::ol:
    case block::dl:
      {
        if (pop != 0)
        {
          cerr << "error: empty list '" << string (ol, 0, on) << "' "
               << "in documentation string '" << s << "'" << endl;
          throw generation_failed ();
        }

        if (n != 0)
        {
          cerr << "error: unexpected text after " << k << "| "
               << "in paragraph '" << string (ol, 0, on) << "'" << endl;
          throw generation_failed ();
        }

        break;
      }
    case block::li:
      {
        if (ok == block::dl)
        {
          if (n == 0)
          {
            cerr << "error: term text missing in paragraph '"
                 << string (ol, 0, on) << "'" << endl;
            throw generation_failed ();
          }
        }

        break;
      }
    case block::note:
    case block::text:
    case block::pre:
      break;
    }

    // Push the block into the stack.
    //
    switch (k)
    {
    case block::h: blocks.push (block (k, false, id, header)); break;
    case block::ul:
    case block::ol: if (ot != ot_html) ol_count = 0; // Fall through.
    case block::dl: blocks.push (block (k, true, id)); break;
    case block::li:
      {
        switch (blocks.top ().kind)
        {
        case block::ol:
          {
            if (ot != ot_html)
            {
              ostringstream os;
              os << ++ol_count;
              header = os.str ();
            }
            break;
          }
        case block::dl:
          {
            format_line (ot, header, l, n);
            n = 0;
            break;
          }
        default:
          break;
        }

        blocks.push (block (k, false, id, header));
        break;
      }
    case block::note:
      {
        blocks.push (block (k, true, id, header));
        break;
      }
    case block::text: break; // No push.
    case block::pre: break;  // No push.
    }

    // Output paragraph text. If we are in TOC mode and this is a top-level
    // block, then pretend we don't have anything to write. For non-top-level
    // blocks, we handle this below, when we pop them.
    //
    if (toc && blocks.size () == 1)
      n = 0;

    if (n != 0)
    {
      block& b (blocks.top ());
      string& v (b.value);
      bool first (v.empty ());

      switch (ot)
      {
      case ot_plain:
        {
          // Separate paragraphs with a blank line.
          //
          if (!first)
            v += "\n\n";

          if (k == block::pre)
            v.append (l, n);
          else
          {
            /*
            // Strip a single "\n" at the beginning of the paragraph since we
            // already wrote one newline as part of starting the paragraph. An
            // example where this might be useful:
            //
            // li|
            //
            //   \
            //   ...
            //   \
            //
            //   \n|
            //
            if (n >= 2 && l[0] == '\\' && l[1] == 'n')
            {
              l += 2;
              n -= 2;
            }

            if (n != 0)
            */

            format_line (ot, v, l, n);
          }

          break;
        }
      case ot_html:
        {
          // Separate paragraphs with a blank line.
          //
          if (!first)
            v += "\n\n";

          if (k == block::pre)
          {
            v += "<pre>";
            v.append (l, n);
            v += "</pre>";
          }
          else
          {
            if (!first || b.para)
            {
              string t;
              format_line (ot, t, l, n);
              string s (html_margin (t));

              // If the entire paragraph is wrapped in <code> then signal this
              // fact via the "code" class. This can be used, for example, to
              // change alignment from "justify" to "left" in order to support
              // custom alignments (in combination with non-ignorable space
              // escaping; think multi-line synopsis in man pages).
              //
              // While at it also cleanup nested <code></code> fragments (we
              // may end up with quite a few of them due to the <arg>
              // translation).
              //
              string c;
              {
                size_t n (t.size ());

                if (n >= 13                            &&
                    t.compare (0, 6, "<code>") == 0    &&
                    t.compare (n - 7, 7, "</code>") == 0)
                {
                  string tt;

                  // Make sure this is one contigous <code>.
                  //
                  size_t b (1); // <code>/</code> balance.
                  for (size_t i (6), j (0);
                       i != n && (i = t.find ("code>", i)) != string::npos;
                       j = i = i + 5)
                  {
                    if      (t[i - 1] == '<')                    b++;
                    else if (t[i - 1] == '/' && t[i - 2] == '<') b--;
                    else continue;

                    // Append previous chunk.
                    //
                    tt.append (t, j, i - j - (t[i - 1] == '<' ? 1 : 2));

                    if (b == 0)
                    {
                      if (i + 5 == n)
                      {
                        tt.append (t, i - 2, 7); // Closing </code>.
                        t = move (tt);
                        c = "code";
                      }

                      break;
                    }
                  }
                }
              }

              v += "<p";
              if (!c.empty ()) v += " class=\"" + c + "\"";
              if (!s.empty ()) v += " style=\"" + s + "\"";
              v += ">";
              v += t;
              v += "</p>";
            }
            else
              format_line (ot, v, l, n);
          }

          break;
        }
      case ot_man:
        {
          if (b.para)
          {
            if (!first)
              v += "\n";

            v += ".PP\n";
          }
          else
          {
            if (!first)
              v += "\n\n";
          }

          if (k == block::pre)
          {
            v += ".nf\n";

            // Note that if we have several consecutive blank lines, they
            // will be collapsed into a single one. No, .sp doesn't work.
            //
            char c, p ('\n'); // Current and previous characters.
            for (size_t i (0); i != n; p = c, ++i)
            {
              switch (c = l[i])
              {
              case '\\': v += '\\'; break;
              case '.':  v += p != '\n' ? "\\" : "\\&\\"; break;
              }

              v += c;
            }

            v += "\n.fi";
          }
          else
          {
            format_line (ot, v, l, n);
          }

          break;
        }
      }
    }

    // Pop paragraph blocks.
    //
    if (pop >= blocks.size ()) // >= to account for top-level.
    {
      cerr << "error: extraneous '|' at the end of paragraph '"
           << string (ol, 0, on) << "'" << endl;
      throw generation_failed ();
    }

    for (; pop != 0; --pop)
    {
      block pb (blocks.top ()); // move
      string& pi (pb.id);
      string& ph (pb.header);
      string& pv (pb.value);

      blocks.pop ();

      block& b (blocks.top ());
      string& v (b.value);

      // Handle poping into top-level block in TOC mode.
      //
      if (toc && blocks.size () == 1)
      {
        if (pb.kind == block::h)
        {
          switch (ot)
          {
          case ot_plain: break;
          case ot_html:
            {
              char t (ph[0]);

              if (pi.empty ())
              {
                cerr << "error: TOC heading '" << pv << "' has no id" << endl;
                throw generation_failed ();
              }

              pv = "<a href=\"#" + pi + "\">" + pv + "</a>";

              // Unwind heading levels that are deeper ("more sub") than us.
              //
              for (; tocs.size () != 1; tocs.pop_back ())
              {
                toc_entry const& e (tocs.back ());

                bool pop (true);

                switch (t)
                {
                case '0':
                case 'H':
                case '1': break; // Always unwind.
                case 'h':
                  {
                    pop = html_h != '2' || e.type == 'h' || e.type == '2';
                    break;
                  }
                case '2': pop = e.type == '2'; break;
                }

                if (!pop)
                  break;

                // If we have sub-headings, then we need to close the table.
                //
                if (e.count != 0)
                {
                  size_t l (tocs.size ());

                  v += string (l * 4 - 2, ' ') + "</table>\n";
                  v += string (l * 4 - 4, ' ') + "</td></tr>\n";
                }
                else
                  // Otherwise it is inline.
                  //
                  v += "</td></tr>\n";
              }

              size_t l (tocs.size ());
              toc_entry& e (tocs.back ());

              // If this is a first sub-entry, then we need to open the
              // sub-table.
              //
              string in ((l * 4) - 2, ' ');

              if (l > 1 && e.count == 0)
              {
                v += "\n";
                v += in + "<table class=\"toc\">\n";
              }

              in += "  ";

              switch (t)
              {
              case 'H':
                {
                  v += in + "<tr><th colspan=\"2\">" + pv + "</th></tr>\n";
                  break;
                }
              case '0':
              case '1':
              case 'h':
              case '2':
                {
                  // Calculate Chapter(X)/Section(X.Y)/Subsection(X.Y.Z) unless
                  // it is the preface (or a subsection thereof).
                  //
                  string n;
                  if (t != '0')
                  {
                    ++e.count;

                    for (toc_stack::const_iterator i (tocs.begin ());
                         i != tocs.end ();
                         ++i)
                    {
                      if (i->type == '0')
                      {
                        n.clear ();
                        break;
                      }

                      if (!n.empty ())
                        n += '.';

                      ostringstream os;
                      os << i->count;
                      n += os.str ();
                    }
                  }

                  v += in + "<tr>";

                  if (!n.empty ())
                  {
                    v += "<th>" + n + "</th><td>";
                    heading_map[pi] = n; // Save it for later.
                  }
                  else
                    v += "<td class=\"preface\" colspan=\"2\">";

                  v += pv; // No newline
                  tocs.push_back (toc_entry (t));
                  break;
                }
              }

              // Same as in non-TOC mode below.
              //
              // This only works automatically for a single string fragment.
              // For multiple string fragments use --html-heading-map.
              //
              if (html_h == '\0' && (t == '0' || t == '1'))
                html_h = '2';

              break;
            }
          case ot_man: break;
          }
        }

        continue;
      }

      switch (ot)
      {
      case ot_plain:
        {
          const char* sn (v.empty () ? "" : "\n");
          const char* dn (v.empty () ? "" : "\n\n");

          switch (pb.kind)
          {
          case block::h:
            {
              v += dn;

              if (options.ansi_color ())
                v += "\033[1m"; // Bold.

              v += pv;

              if (options.ansi_color ())
                v += "\033[0m";

              break;
            }
          case block::ul:
          case block::ol:
          case block::dl: v += dn + pv; break;
          case block::li:
            {
              v += sn;
              size_t ind (0), vn (pv.size ());

              switch (b.kind)
              {
              case block::ul:
                v += "* ";
                ind = 2;
                break;
              case block::ol:
                v += ph + ". ";
                ind = ph.size () + 2;
                break;
              case block::dl:
                v += ph + "\n" + (vn != 0 && pv[0] != '\n' ? "    " : "");
                ind = 4;
                break;
              default:
                break;
              }

              // Add value with indentation for subsequent paragraphs.
              //
              char c, p ('\0'); // Current and previous characters.
              for (size_t i (0); i != vn; p = c, ++i)
              {
                c = pv[i];

                if (p == '\n' && c != '\n') // Don't indent blanks.
                  v += string (ind, ' ');

                v += c;
              }

              break;
            }
          case block::note:
            {
              v += dn;

              // Add a special character (bell) plus space in front of every
              // line (including blanks). The bell is recognized and
              // translated by txt_wrap_lines() to '|'.
              //
              char p ('\n'); // Previous character.
              for (size_t i (0), n (pv.size ()); i != n; ++i)
              {
                char c (pv[i]);

                if (p == '\n')
                  v += (c != '\n' ? "\x07 " : "\x07");

                v += c;
                p = c;
              }

              break;
            }
          case block::text:
          case block::pre: assert (false);
          }

          break;
        }
      case ot_html:
        {
          if (!v.empty ())
            v += "\n\n";

          switch (pb.kind)
          {
          case block::h:
            {
              char t (ph[0]);

              string h;
              string c;

              const html_hmap& hm (options.html_heading_map ());
              html_hmap::const_iterator mi (hm.find (t));

              if (mi == hm.end ())
              {
                switch (t)
                {
                case '0': h = "h1"; c = "preface"; break;
                case 'H': h = "h1"; c = "part"; break;
                case '1': h = "h1"; break;
                case 'h': h = html_h != '2' ? "h1" : "h2"; break;
                case '2': h = "h3"; break;
                }
              }
              else
                h = mi->second;

              v += '<' + h;
              if (!pi.empty ()) v += " id=\"" + pi + '"';
              if (!c.empty ())  v += " class=\"" + c + '"';
              v += '>';

              // See if we have a heading number (assigned by TOC).
              //
              if (!pi.empty ())
              {
                heading_map_type::const_iterator i (heading_map.find (pi));

                if (i != heading_map.end ())
                  v += i->second + ' ';
              }

              v += pv;

              v += "</" + h + '>';

              // This only works automatically for a single string fragment.
              // For multiple string fragments use --html-heading-map.
              //
              if (html_h == '\0' && (t == '0' || t == '1'))
                html_h = '2';

              break;
            }
          case block::ul: v += "<ul>\n" + pv + "\n</ul>"; break;
          case block::ol: v += "<ol>\n" + pv + "\n</ol>"; break;
          case block::dl: v += "<dl>\n" + pv + "\n</dl>"; break;
          case block::li:
            {
              string pvs (html_margin (pv));

              if (b.kind == block::dl)
              {
                string phs (html_margin (ph));

                v += (phs.empty () ? "<dt>" : "<dt style=\"" + phs + "\">")
                  + ph + "</dt>\n";

                v += (pvs.empty () ? "<dd>" : "<dd style=\"" + pvs + "\">")
                  + pv + "</dd>";
              }
              else
                v += (pvs.empty () ? "<li>" : "<li style=\"" + pvs + "\">")
                  + pv + "</li>";

              break;
            }
          case block::note:
            {
              v += "<div class=\"note\">\n";
              v += pv;
              v += "\n</div>";

              break;
            }
          case block::text:
          case block::pre: assert (false);
          }

          break;
        }
      case ot_man:
        {
          // Seeing that we always write a macro, one newline is enough.
          //
          if (!v.empty ())
            v += "\n";

          switch (pb.kind)
          {
          case block::h:  v += ".SH \"" + pv + "\""; break;
          case block::ul:
          case block::ol:
          case block::dl:
            {
              if (!b.para) // First list inside .IP.
              {
                // .IP within .IP? Just shoot me in the head already! We
                // have to manually indent it with .RS/.RE *and* everything
                // that comes after it (since .PP resets the indent). Why
                // not just indent the whole list content? Because then the
                // first line will never start on the same line as the term.
                //
                v += ".RS\n";
                b.trailer = "\n.RE";
                b.para = true; // Start emitting .PP from now on.
              }

              v += pv;
              break;
            }
          case block::li:
            {
              man_margin (ph); // Strip leading/trailing .br.

              switch (b.kind)
              {
              case block::ul: v += ".IP \\(bu 2em\n" + pv; break;
              case block::ol: v += ".IP " + ph + ". 4em\n" + pv; break;
              //
              // Add .br to force the definition to start on the new line.
              //
              case block::dl: v += ".IP \"" + ph + "\"\n.br\n" + pv; break;
              default: break;
              }

              break;
            }
          case block::note:
            {
              cerr << "error: " << pb.kind << "| in man output not yet "
                   << "supported" << endl;
              throw generation_failed ();
            }
          case block::text:
          case block::pre: assert (false);
          }

          break;
        }
      }
    }
  }

  assert (!blocks.empty ()); // Should have top-level.

  if (blocks.size () > 1)
  {
    cerr << "error: unterminated paragraph " << blocks.top ().kind << " "
         << "in documentation string '" << s << "'" << endl;
    throw generation_failed ();
  }

  block& b (blocks.top ());

  switch (ot)
  {
  case ot_man: return b.value + b.trailer;
  default: return b.value;
  }
}

string context::
start_toc ()
{
  switch (ot)
  {
  case ot_plain: break;
  case ot_html:
    {
      tocs.push_back (toc_entry ('\0'));
      return "  <table class=\"toc\">";
    }
  case ot_man: break;
  }

  return string ();
}

string context::
end_toc ()
{
  switch (ot)
  {
  case ot_plain: break;
  case ot_html:
    {
      string v;

      // Unwind the TOC stack until we reach top level. Same code as in
      // format().
      //
      for (; tocs.size () != 1; tocs.pop_back ())
      {
        toc_entry const& e (tocs.back ());

        // If we have sub-headings, then we need to close the table.
        //
        if (e.count != 0)
        {
          size_t l (tocs.size ());

          v += string (l * 4 - 2, ' ') + "</table>\n";
          v += string (l * 4 - 4, ' ') + "</td></tr>\n";
        }
        else
          // Otherwise it is inline.
          //
          v += "</td></tr>\n";
      }

      v += "  </table>";
      return v;
    }
  case ot_man: break;
  }

  return string ();
}

void context::
verify_id_ref ()
{
  if (!options.omit_link_check ())
  {
    bool f (false);

    for (id_set_type::const_iterator i (ref_set.begin ());
         i != ref_set.end ();
         ++i)
    {
      if (id_set.find (*i) == id_set.end ())
      {
        cerr << "error: no id for fragment link '#" << *i << "'" << endl;
        f = true;
      }
    }

    if (f)
      throw generation_failed ();
  }

  id_set.clear ();
  ref_set.clear ();
  heading_map.clear ();
}

string context::
substitute (const string& s, const path* d)
{
  string r;

  // Scan the string looking for variables ($var$) or paths.
  //
  size_t b (0), e (b);
  for (size_t n (s.size ()); e != n; ++e)
  {
    if (s[e] == '$' && e + 1 != n)
    {
      if (s[e + 1] == '$') // Escape.
      {
        r.append (s, b, ++e - b); // Keep one, skip the other.
        b = e + 1;
        continue;
      }

      bool file (false);

      // Scan for as long as it is a C identifier or a file.
      //
      size_t p (e + 1); // Position of the second '$'.
      for (; p != n; ++p)
      {
        char c (s[p]);
        bool f (p == e + 1); // First.

        if (d != 0 && f && c == '.' && p + 1 != n &&
            (s[p + 1] == '/' ||                                   // "./"
             (s[p + 1] == '.' && p + 2 != n && s[p + 2] == '/'))) // "../"
          file = true;

        if (file)
        {
          if (c == '$')
            break;
        }
        else
        {
          if (!(c == '_' ||
                ('a' <= c && c <= 'z') ||
                ('A' <= c && c <= 'Z') ||
                (!f && '0' <= c && c <= '9')))
            break;
        }
      }

      // Note: check that the second '$' is not escaped.
      //
      if (p != n && s[p] == '$' && (p + 1 == n || s[p + 1] != '$'))
      {
        r.append (s, b, e - b); // Save what came before the expansion.

        // Var/file name.
        //
        ++e;
        string v (s, e, p - e);

        if (file)
        {
          path p (*d / path (v));
          p.normalize (); // Apply '.' and '..'.

          ifstream ifs (p.string ().c_str (), ifstream::in | ifstream::binary);
          if (!ifs.is_open ())
          {
            cerr << p << ": error: unable to open in read mode" << endl;
            throw generation_failed ();
          }

          // We don't expect our file to contain '\0' so use that as the
          // delimiter to read the entire file with getline().
          //
          string s;
          getline (ifs, s, '\0');
          r += s;
        }
        else
        {
          // Handle special variables.
          //
          if (v == "TOC")
          {
            if (!r.empty () || p + 1 != n)
            {
              cerr << "error: TOC variable should be on its own line" << endl;
              throw generation_failed ();
            }

            // Invert the TOC mode.
            //
            if ((toc = toc ? 0 : 1))
              r = start_toc ();
            else
              r = end_toc ();

            return r;
          }

          // Lookup and substiute the variable.
          //
          using semantics::doc;

          if (doc* d = unit.lookup<doc> ("", "var: " + v))
            r += d->front ();
          else
          {
            cerr << "error: undefined variable '" << v << "' in '" << s << "'"
                 << endl;
            throw generation_failed ();
          }
        }

        e = p;
        b = e + 1;
      }
    }
  }

  r.append (s, b, e - b); // Last chunk.
  return r;
}

bool context::
substitute (semantics::scope& scope, const char* s, size_t n, string& result)
{
  bool sub (false);
  result.clear ();

  // Scan the string looking for variables (\$var$).
  //
  size_t b (0), e (b);
  for (char p ('\0'); e != n; ++e)
  {
    char c (s[e]);

    if (p == '\\')
    {
      if (c == '\\') // Escape sequence.
      {
        p = '\0';
        continue;
      }

      if (c == '$')
      {
        // Variable expansion.
        //
        sub = true;

        // Find the closing '$'.
        //
        size_t p (e + 1); // Position of the second '$'.
        for (; p != n && s[p] != '$'; ++p) ;

        if (p == n)
        {
          cerr << "error: missing closing '$' in '" << string (s, n) << "'"
               << endl;
          throw generation_failed ();
        }

        result.append (s, b, e - b - 1); // Save what came before.

        // Var name.
        //
        ++e;
        string v (s, e, p - e);

        // Handle special variables.
        //
        if (v == "TOC")
        {
          if (!result.empty () || p + 1 != n)
          {
            cerr << "error: TOC variable should be its own paragraph" << endl;
            throw generation_failed ();
          }

          // Invert the TOC mode.
          //
          if ((toc = toc ? 0 : 1))
            result = start_toc ();
          else
            result = end_toc ();

          return true;
        }

        // Lookup and substiute.
        //
        using semantics::doc;

        if (doc* d = unit.lookup<doc> (scope.fq_name (), "var: " + v))
          result += d->front ();
        else
        {
          cerr << "error: undefined variable '" << v << "' in '"
               << string (s, n) << "'" << endl;
          throw generation_failed ();
        }

        e = p;
        b = e + 1;
      }
    }

    p = s[e];
  }

  if (sub)
    result.append (s, b, e - b); // Last chunk.

  return sub;
}

string context::
fq_name (semantics::nameable& n, bool cxx_name)
{
  using namespace semantics;

  string r;

  if (dynamic_cast<cli_unit*> (&n))
  {
    return ""; // Map to global namespace.
  }
  else
  {
    r = fq_name (n.scope ());
    r += "::";
    r += cxx_name ? escape (n.name ()) : n.name ();
  }

  return r;
}

string context::
ns_open (const string& qn, bool last)
{
  string::size_type b (0), e;

  do
  {
    e = qn.find ("::", b);
    string n (qn, b, e == string::npos ? e : e - b);

    if (!n.empty () && (last || e != string::npos))
      os << "namespace " << n << "{";

    b = e;

    if (b == string::npos)
      return n;

    b += 2;

  } while (true);
}

void context::
ns_close (const string& qn, bool last)
{
  string::size_type b (0), e;

  do
  {
    e = qn.find ("::", b);
    string n (qn, b, e == string::npos ? e : e - b);

    if (!n.empty () && (last || e != string::npos))
      os << "}";

    b = e;

    if (b == string::npos)
      return;

    b += 2;

  } while (true);
}

class_doc context::
class_doc (semantics::class_& c)
{
  typedef map<string, string> map;
  const map& m (options.class_doc ());

  string n (c.fq_name ());

  map::const_iterator i (m.find (n));

  if (i == m.end ())
  {
    n = string (n, 2, string::npos); // Try without leading '::'.
    i = m.find (n);
  }

  if (i == m.end ())
    return cd_default;

  const string& k (i->second);

  if (k == "exclude")
    return cd_exclude;
  if (k == "exclude-base")
    return cd_exclude_base;
  else if (k == "short")
    return cd_short;
  else if (k == "long")
    return cd_long;
  else
  {
    cerr << "error: unknown --class-doc kind value '" << k << "'" << endl;
    throw generation_failed ();
  }
}

string context::
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

// namespace
//

void namespace_::
pre (type& ns)
{
  string name (ns.name ());

  if (!name.empty ())
    os << "namespace " << escape (name)
       << "{";
}

void namespace_::
post (type& ns)
{
  if (!ns.name ().empty ())
    os << "}";
}
