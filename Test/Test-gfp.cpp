#include <fstream>
using namespace std;

#include "Math/gfp.h"

void test_add(bigint a, bigint b, bigint c)
{
  gfp ga(a), gb(b), gc(c);
  gfp sum = ga + gb;
  if (sum != gc)
    {
      cout << ga << " + " << gb << " = " << sum << " != " << gc << " mod " << gfp::pr() << endl;
    }
}

void test_mul(bigint a, bigint b, bigint c)
{
  gfp ga(a), gb(b), gc(c);
  gfp sum = ga * gb;
  if (sum != gc)
    {
      cout << ga << " * " << gb << " = " << sum << " != " << gc << " mod " << gfp::pr() << endl;
    }
}

void test(int num)
{
  bigint p;
  stringstream ss;
  ss << "Auto-Test-Data/" << num << "/SharingData.txt";
  cout << "Using prime for file " << ss.str() << endl;
  ifstream inpf(ss.str());
  inpf >> p;
  inpf.close();
  cout << "p = " << p << endl;

  gfp::init_field(p);

  test_add(1, 1, 2);
  test_add(0, 1, 1);
  test_add(1, -1, 0);
  test_add(-1, -1, -2);

  test_mul(1, 1, 1);
  test_mul(0, 1, 0);
  test_mul(1, -1, -1);
  test_mul(-1, -1, 1);

  cout << endl;
}

int main()
{
  test(1);
  test(14);
}


