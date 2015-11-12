// file      : cli/context.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2009-2011 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <stack>
#include <cstring>  // strncmp()
#include <iostream>

#include "context.hxx"

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
         semantics::cli_unit& unit_,
         options_type const& ops)
    : data_ (new (shared) data),
      os (os_),
      unit (unit_),
      options (ops),
      modifier (options.generate_modifier ()),
      specifier (options.generate_specifier ()),
      inl (data_->inl_),
      opt_prefix (options.option_prefix ()),
      opt_sep (options.option_separator ()),
      cli (data_->cli_),
      reserved_name_map (options.reserved_name ()),
      keyword_set (data_->keyword_set_)
{
  if (options.suppress_usage ())
    usage = ut_none;
  else
  {
    if (options.long_usage ())
      usage = options.short_usage () ? ut_both : ut_long;
    else
      usage = ut_short;
  }

  if (!options.suppress_inline ())
    data_->inl_ = "inline ";

  data_->cli_ = options.cli_namespace ();

  if (!cli.empty () && cli[0] != ':')
    data_->cli_ = "::" + data_->cli_;

  for (size_t i (0); i < sizeof (keywords) / sizeof (char*); ++i)
    data_->keyword_set_.insert (keywords[i]);
}

context::
context (context& c)
    : data_ (c.data_),
      os (c.os),
      unit (c.unit),
      options (c.options),
      modifier (c.modifier),
      specifier (c.specifier),
      usage (c.usage),
      inl (c.inl),
      opt_prefix (c.opt_prefix),
      opt_sep (c.opt_sep),
      cli (c.cli),
      reserved_name_map (c.reserved_name_map),
      keyword_set (c.keyword_set)
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
      r += "\\i{";
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

  size_t p (string::npos);

  for (size_t i (0), n (s.size ()); i < n; ++i)
  {
    if (p == string::npos && s[i] == '<')
    {
      p = i;
      continue;
    }

    if (p != string::npos)
    {
      if (s[i] == '>')
      {
        string a (s, p + 1, i - p - 1);

        if (set.find (a) != set.end ())
        {
          r += "\\i{";

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

    r += s[i];
  }

  // If we found the opening '<' but no closing '>', add the rest.
  //
  if (p != string::npos)
    r.append (s, p, string::npos);

  return r;
}

void context::
format_line (output_type ot, string& r, const char* s, size_t n)
{
  bool escape (false);
  stack<unsigned char> blocks; // Bit 0: code; 1: italic; 2: bold.

  for (size_t i (0); i < n; ++i)
  {
    char c (s[i]);

    if (escape)
    {
      bool block (false);

      switch (c)
      {
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
      case 'c':
        {
          unsigned char b (1);
          size_t j (i + 1);

          if (j < n)
          {
            if (s[j] == 'i')
            {
              b |= 2;
              j++;

              if (j < n && s[j] == 'b')
              {
                b |= 4;
                j++;
              }
            }
            else if (s[j] == 'b')
            {
              b |= 4;
              j++;

              if (j < n && s[j] == 'i')
              {
                b |= 2;
                j++;
              }
            }
          }

          if (j < n && s[j] == '{')
          {
            i = j;
            blocks.push (b);
            block = true;
            break;
          }

          r += 'c';
          break;
        }
      case 'i':
        {
          unsigned char b (2);
          size_t j (i + 1);

          if (j < n)
          {
            if (s[j] == 'c')
            {
              b |= 1;
              j++;

              if (j < n && s[j] == 'b')
              {
                b |= 4;
                j++;
              }
            }
            else if (s[j] == 'b')
            {
              b |= 4;
              j++;

              if (j < n && s[j] == 'c')
              {
                b |= 1;
                j++;
              }
            }
          }

          if (j < n && s[j] == '{')
          {
            i = j;
            blocks.push (b);
            block = true;
            break;
          }

          r += 'i';
          break;
        }
      case 'b':
        {
          unsigned char b (4);
          size_t j (i + 1);

          if (j < n)
          {
            if (s[j] == 'c')
            {
              b |= 1;
              j++;

              if (j < n && s[j] == 'i')
              {
                b |= 2;
                j++;
              }
            }
            else if (s[j] == 'i')
            {
              b |= 2;
              j++;

              if (j < n && s[j] == 'c')
              {
                b |= 1;
                j++;
              }
            }
          }

          if (j < n && s[j] == '{')
          {
            i = j;
            blocks.push (b);
            block = true;
            break;
          }

          r += 'b';
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
               << "documentation paragraph '" << string (s, 0, n) << "'"
               << endl;
          throw generation_failed ();
        }
      }

      // If we just added a new block, add corresponding output markup.
      //
      if (block)
      {
        unsigned char b (blocks.top ());

        switch (ot)
        {
        case ot_plain:
          {
            if (b & 1)
              r += "'";
            break;
          }
        case ot_html:
          {
            if (b & 1)
              r += "<code>";

            if (b & 2)
              r += "<i>";

            if (b & 4)
              r += "<b>";

            break;
          }
        case ot_man:
          {
            if ((b & 6) == 6)
              r += "\\f(BI";
            else if (b & 2)
              r += "\\fI";
            else if (b & 4)
              r += "\\fB";

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
          if (!blocks.empty ())
          {
            unsigned char b (blocks.top ());

            switch (ot)
            {
            case ot_plain:
              {
                if (b & 1)
                  r += "'";
                break;
              }
            case ot_html:
              {
                if (b & 4)
                  r += "</b>";

                if (b & 2)
                  r += "</i>";

                if (b & 1)
                  r += "</code>";

                break;
              }
            case ot_man:
              {
                if (b & 6)
                  r += "\\fP";
                break;
              }
            }

            blocks.pop ();
            break;
          }

          // Fall through.
        }
      default:
        r += c;
        break;
      }
    }
  }

  if (escape)
  {
    cerr << "error: unterminated escape sequence in documentation "
         << "paragraph '" << string (s, 0, n) << "'" << endl;
    throw generation_failed ();
  }

  if (!blocks.empty ())
  {
    unsigned char b (blocks.top ());
    string bs;

    if (b & 1) bs += 'c';
    if (b & 2) bs += 'i';
    if (b & 4) bs += 'b';

    cerr << "error: unterminated formatting block '\\" << bs << "' "
         << "in documentation paragraph '" << string (s, 0, n) << "'" << endl;
    throw generation_failed ();
  }
}

struct block
{
  enum kind_type {h, ul, ol, dl, li, text, pre};

  kind_type kind;
  bool para;     // True if first text fragment should be in own paragraph.
  string header; // Term in dl's li.
  string value;

  block (kind_type k, bool p, const string& h = "")
      : kind (k), para (p), header (h) {}
};

static const char* block_kind_str[] = {
  "\\h", "\\ul", "\\ol", "\\dl", "\\li", "text", "preformatted text"};

inline ostream&
operator<< (ostream& os, block::kind_type k)
{
  return os << block_kind_str[k];
}

string context::
format (output_type ot, string const& s, bool para)
{
  stack<block> blocks;
  blocks.push (block (block::text, para)); // Top-level.

  bool last (false);
  for (size_t b (0), e; !last; b = e + 1)
  {
    bool pre (s[b] == 0x02);

    const char* l;
    size_t n;

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
    }

    const char* ol (l); // Original, full line for diagnostics.
    size_t on (n);

    // First determine what kind of paragraph block this is.
    //
    block::kind_type k;
    size_t pop (0); // Pop count.

    if (pre)
    {
      k = block::pre;
    }
    else
    {
      if (n >= 3 && strncmp (l, "\\h|", 3) == 0)
      {
        k = block::h;
        l += 3;
        n -= 3;
      }
      else if (n >= 4 &&
               (strncmp (l, "\\ul|", 4) == 0 ||
                strncmp (l, "\\ol|", 4) == 0 ||
                strncmp (l, "\\dl|", 4) == 0))
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
      else if (n >= 4 && strncmp (l, "\\li|", 4) == 0)
      {
        k = block::li;
        l += 4;
        n -= 4;
      }
      else
        k = block::text;

      // Skip leading spaces after opening '|'.
      //
      if (k != block::text)
        while (n != 0 && (*l == 0x20 || *l == 0x0D || *l == 0x09)) {l++; n--;}

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
      case block::li: good = (k == block::text || k == block::pre); break;
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
    case block::text:
    case block::pre:
      break;
    }

    // Push the block into the stack.
    //
    switch (k)
    {
    case block::h: blocks.push (block (k, false)); break;
    case block::ul:
    case block::ol:
    case block::dl: blocks.push (block (k, true)); break;
    case block::li:
      {
        string h;
        if (blocks.top ().kind == block::dl)
        {
          format_line (ot, h, l, n);
          n = 0;
        }

        blocks.push (block (k, false, h));
        break;
      }
    case block::text: break; // No push.
    case block::pre: break;  // No push.
    }

    // Output paragraph text.
    //
    if (n != 0)
    {
      block& b (blocks.top ());
      string& v (b.value);
      bool first (v.empty ());

      // Separate paragraphs with a blank line.
      //
      if (!first)
        v += "\n\n";

      if (k == block::pre)
      {
        if (ot == ot_html)
          v += "<pre>";

        v.append (l, n);

        if (ot == ot_html)
          v += "</pre>";
      }
      else
      {
        if (!first || b.para)
        {
          if (ot == ot_html)
            v += "<p>";
        }

        format_line (ot, v, l, n);

        if (!first || b.para)
        {
          if (ot == ot_html)
            v += "</p>";
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
      string& pv (pb.value);
      string& ph (pb.header);

      blocks.pop ();

      block& b (blocks.top ());
      string& v (b.value);

      if (ot == ot_html)
      {
        // Separate paragraphs with a blank line.
        //
        if (!v.empty ())
          v += "\n\n";

        switch (pb.kind)
        {
        case block::h:  v += "<h1>" + pv + "</h1>"; break;
        case block::ul: v += "<ul>\n" + pv + "\n</ul>"; break;
        case block::ol: v += "<ol>\n" + pv + "\n</ol>"; break;
        case block::dl: v += "<dl>\n" + pv + "\n</dl>"; break;
        case block::li:
          {
            if (b.kind == block::dl)
            {
              v += "<dt>" + ph + "</dt>\n";
              v += "<dd>" + pv + "</dd>";
            }
            else
              v += "<li>" + pv + "</li>";

            break;
          }
        case block::text:
        case block::pre: assert (false);
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

  return blocks.top ().value;
}

/*

struct block
{
  enum value {h, ul, ol, dl, li, text} v_;
  block (value v = text): v_ (v) {}
  operator value () const {return v_;}
};

const char* block_str[] = {"\\h", "\\ul", "\\ol", "\\dl", "\\li", "text"};

inline ostream&
operator<< (ostream& os, block b)
{
  return os << block_str[b];
}

string context::
format (output_type ot, string const& s, bool first_para)
{
  string r;
  stack<block> blocks;

  // Flag that indicates whether this is the first paragraph.
  //
  bool para (first_para);

  // Iterate over lines (paragraphs) or pre-formatted sections.
  //
  for (size_t b (0), e; ; b = e + 1)
  {
    bool pre (s[b] == 0x02);
    bool last;

    if (pre)
    {
      e = s.find (0x03, ++b);
      assert (e != string::npos);
      last = (e + 1 == s.size ());
    }
    else
    {
      e = s.find ('\n', b);
      last = (e == string::npos);
    }

    const char* l (s.c_str () + b);
    size_t n ((e != string::npos ? e : s.size ()) - b);

    if (pre)
    {
      ++e; // Skip newline that follows 0x03.

      if (ot == ot_html)
        r += "<pre>";

      r.append (l, n);

      if (ot == ot_html)
        r += "</pre>";

      para = true;
    }
    else
    {
      const char* ol (l); // Original, full line, for diagnostics.
      size_t on (n);

      // First determine what kind of paragraph block this is (i.e.,
      // handle the "prefix").
      //
      block b (block::text);

      if (n >= 3 && strncmp (l, "\\h|", 3) == 0)
      {
        b = block::h;
        l += 3;
        n -= 3;
      }
      else if (n >= 4 &&
               (strncmp (l, "\\ul|", 4) == 0 ||
                strncmp (l, "\\ol|", 4) == 0 ||
                strncmp (l, "\\dl|", 4) == 0))
      {
        switch (l[1])
        {
        case 'u': b = block::ul; break;
        case 'o': b = block::ol; break;
        case 'd': b = block::dl; break;
        }

        l += 4;
        n -= 4;
      }
      else if (n >= 4 && strncmp (l, "\\li|", 4) == 0)
      {
        b = block::li;
        l += 4;
        n -= 4;
      }

      // Skip leading spaces after opening '|'.
      //
      while (n != 0 && (*l == 0x20 || *l == 0x0D || *l == 0x09)) {l++; n--;}

      // Next figure out how many blocks we need to pop at the end of this
      // paragraph (i.e., handle the "suffix"). Things get a bit complicated
      // since '|' could be escaped.
      //
      size_t pc (0); // Pop count.
      for (; n - pc > 0 && l[n - pc - 1] == '|'; ++pc) ;
      if (pc != 0)
      {
        // To determine whether the first '|' is part of an escape sequence
        // we have to find the first non-backslash character and then figure
        // out who escapes whom.
        //
        size_t ec (0); // Escape count.
        for (; n - pc - ec > 0 && l[n - pc - ec - 1] == '\\'; ++ec) ;

        // If we have an odd number of backslashes, then the last '|' is
        // escaped.
        //
        if (ec % 2 != 0)
          --pc;

        n -= pc; // Number of special '|' at the end.

        // Skip trailing spaces before closing '|'.
        //
        while (n != 0 && (l[n - 1] == 0x20 ||
                          l[n - 1] == 0x0D ||
                          l[n - 1] == 0x09))
          n--;
      }

      if (pc > blocks.size () + (b != block::text ? 1 : 0))
      {
        cerr << "error: extraneous '|' at the end of paragraph '"
             << string (ol, 0, on) << "'" << endl;
        throw generation_failed ();
      }

      // Outer block or 'text' if top level.
      //
      block ob (blocks.empty () ? block (block::text) : blocks.top ());

      // Verify that this block type is valid in this context. Skip
      // empty text blocks (can happen if we just have '|').
      //
      if (b != block::text || n != 0)
      {
        bool good (true);

        switch (ob)
        {
        case block::h: good = false; break;
        case block::ul:
        case block::ol:
        case block::dl: good = (b == block::li); break;
        case block::li: good = (b == block::text); break;
        case block::text: good = (b != block::li); break;
        }

        if (!good)
        {
          cerr << "error: " << b << " inside " << ob << " "
               << "in documentation string '" << s << "'" << endl;
          throw generation_failed ();
        }
      }

      // Verify the block itself.
      //
      switch (b)
      {
      case block::h:

        // \h blocks are only valid if we are required to start a new
        // paragraph (first_para is true).
        //
        if (!first_para)
        {
          cerr << "error: paragraph '" << string (ol, 0, on) << "' "
               << "not allowed in '" << s << "'" << endl;
          throw generation_failed ();
        }

        // \h must be single-paragraph.
        //
        if (pc == 0)
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
      case block::ul:
      case block::ol:
      case block::dl:

        if (pc != 0)
        {
          cerr << "error: empty list '" << string (ol, 0, on) << "' "
               << "in documentation string '" << s << "'" << endl;
          throw generation_failed ();
        }

        if (n != 0)
        {
          cerr << "error: unexpected text after " << b << "| "
               << "in paragraph '" << string (ol, 0, on) << "'" << endl;
          throw generation_failed ();
        }

        break;
      case block::li:

        if (ob == block::dl)
        {
          if (n == 0)
          {
            cerr << "error: term text missing in paragraph '"
                 << string (ol, 0, on) << "'" << endl;
            throw generation_failed ();
          }
        }

        break;
      case block::text:
        break;
      }

      // Push the paragraph block.
      //
      if (b != block::text)
        blocks.push (b);

      // Output opening markup.
      //
      switch (ot)
      {
      case ot_plain:
        switch (b)
        {
        case block::li:
          switch (ob)
          {
          case block::ul: r += "* "; break;
          case block::ol:
          case block::dl:
          default: break;
          }
        case block::h:
        case block::ul:
        case block::ol:
        case block::dl:
        case block::text:
          break;
        }
        break;
      case ot_html:
        switch (b)
        {
        case block::h:  r += "<h1>"; break;
        case block::ul: r += "<ul>"; break;
        case block::ol: r += "<ol>"; break;
        case block::dl: r += "<dl>"; break;
        case block::li: r += (ob == block::dl ? "<dt>" : "<li>"); break;
        case block::text: if (n != 0 && para) r += "<p>"; break;
        }
        break;
      case ot_man:
        break; // @@ TODO
      }

      // Output paragraph text.
      //
      if (n != 0)
        format_line (ot, r, l, n);

      // Set the para flag and output intermediate markup, if any.
      //
      switch (ot)
      {
      case ot_plain:
        break;
      case ot_html:
        switch (b)
        {
        case block::li:
          if (ob == block::dl) r += "</dt>\n<dd>";
          para = (ob != block::dl && n != 0);
          break;
        case block::text:
          if (n != 0 && para) r += "</p>";
          para = para || (n != 0);
          break;
        default:
          para = true;
          break;
        }
        break;
      case ot_man:
        break; // @@ TODO
      }

      // Pop paragraph blocks.
      //
      for (; pc != 0; --pc)
      {
        b = blocks.top ();
        blocks.pop ();

        ob = blocks.empty () ? block (block::text) : blocks.top ();

        if (ot == ot_html)
        {
          switch (b)
          {
          case block::h:  r += "</h1>"; break;
          case block::ul: r += "</ul>"; break;
          case block::ol: r += "</ol>"; break;
          case block::dl: r += "</dl>"; break;
          case block::li: r += (ob == block::dl ? "</dd>" : "</li>"); break;
          case block::text: break;
          }

          if (pc != 1) // Add empty line unless this is the last separator.
            r += "\n\n";

          para = true; // End of a block always means new paragraph.
        }
      }
    }

    if (last)
      break;

    if (para)
      r += "\n\n";
  }

  if (!blocks.empty ())
  {
    cerr << "error: unterminated paragraph " << blocks.top () << " "
         << "in documentation string '" << s << "'" << endl;
    throw generation_failed ();
  }

  return r;
}
*/

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

void context::
cli_open ()
{
  string::size_type b (0), e;

  do
  {
    e = cli.find ("::", b);
    string n (cli, b, e == string::npos ? e : e - b);

    if (!n.empty ())
      os << "namespace " << n << "{";

    b = e;

    if (b == string::npos)
      break;

    b += 2;

  } while (true);
}

void context::
cli_close ()
{
  string::size_type b (0), e;

  do
  {
    e = cli.find ("::", b);
    string n (cli, b, e == string::npos ? e : e - b);

    if (!n.empty ())
      os << "}";

    b = e;

    if (b == string::npos)
      break;

    b += 2;

  } while (true);
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
