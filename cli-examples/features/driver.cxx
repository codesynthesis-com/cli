// file      : features/driver.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// license   : MIT; see accompanying LICENSE file

#include <utility>   // pair
#include <iostream>
#include <iterator>
#include <algorithm>

#include "options.hxx"

using namespace std;

int
main (int argc, char* argv[])
{
  try
  {
    features::options o (argc, argv);

    // --out-dir | -o
    //
    if (!o.out_dir ().empty ())
      cerr << "output dir: " << o.out_dir () << endl;

    // --first-name & --last-name
    //
    cerr << "first name: " << o.first_name () << endl
         << "last name : " << o.last_name () << endl;

    // --vector | -v & --set | -s
    //
    if (!o.vector ().empty ())
    {
      copy (o.vector ().begin (), o.vector ().end (),
            ostream_iterator<int> (cerr, " "));
      cerr << endl;
    }

    if (!o.set ().empty ())
    {
      copy (o.set ().begin (), o.set ().end (),
            ostream_iterator<int> (cerr, " "));
      cerr << endl;
    }

    // --map | -m
    //
    {
      typedef map<string, bool> str_map;
      const str_map& m = o.map ();
      str_map::const_iterator i (m.find ("a"));

      if (i != m.end ())
        cerr << "value for the 'a' map key: " << i->second << endl;
    }

    // --multimap
    //
    {
      typedef multimap<string, int> str_multimap;
      const str_multimap& m = o.multimap ();

      pair<str_multimap::const_iterator, str_multimap::const_iterator> r (
        m.equal_range ("a"));

      for (str_multimap::const_iterator i (r.first); i != r.second; ++i)
        cerr << "value for the 'a' multimap key: " << i->second << endl;
    }
  }
  catch (const cli::exception& e)
  {
    cerr << e << endl;
    return 1;
  }
}
