// RUN: not %clang_cc1 -std=gnu++98 -verify -triple x86_64-apple-darwin -emit-llvm -o - %s | FileCheck %s

// CHECK: @a = global i32 10
int a = 10;
// CHECK: @ar = constant i32* @a
int &ar = a;

void f();
// CHECK: @fr = constant void ()* @_Z1fv
void (&fr)() = f;

struct S { int& a; };
// CHECK: @s = global %struct._Z1S { i32* @a }
S s = { a };

// PR5581
namespace PR5581 {
class C {
public:
  enum { e0, e1 };
  unsigned f;
};

// CHECK: @_ZN6PR55812g0E = global %class._ZN6PR55811CE { i32 1 }
C g0 = { C::e1 };
}

namespace test2 {
  struct A {
    static const double d = 1.0;
    static const float f = d / 2;
    static int g();
  } a;

  // CHECK: @_ZN5test22t0E = global double {{1\.0+e\+0+}}, align 8
  // CHECK: @_ZN5test22t1E = global [2 x double] [double {{1\.0+e\+0+}}, double {{5\.0+e-0*}}1], align 16
  // CHECK: @_ZN5test22t2E = global double* @_ZN5test21A1d
  // CHECK: @_ZN5test22t3E = global {{.*}} @_ZN5test21A1g
  double t0 = A::d;
  double t1[] = { A::d, A::f };
  const double *t2 = &a.d;
  int (*t3)() = &a.g;
}

// We don't expect to fold this in the frontend, but make sure it doesn't crash.
// CHECK: @PR9558 = global float 0.000000e+0
float PR9558 = reinterpret_cast<const float&>("asd");

// An initialized const automatic variable cannot be promoted to a constant
// global if it has a mutable member.
struct MutableMember {
  mutable int n;
};
int writeToMutable() {
  // CHECK-NOT: {{.*}}MM{{.*}} = {{.*}}constant
  const MutableMember MM = { 0 };
  return ++MM.n;
}

// Make sure we don't try to fold this in the frontend; the backend can't
// handle it.
// CHECK: @PR11705 = global i128 0
__int128_t PR11705 = (__int128_t)&PR11705;

// Make sure we don't try to fold this either.
// CHECK: @_ZZ23UnfoldableAddrLabelDiffvE1x = internal global i128 0
void UnfoldableAddrLabelDiff() { static __int128_t x = (long)&&a-(long)&&b; a:b:return;}

// But make sure we do fold this.
// CHECK: @_ZZ21FoldableAddrLabelDiffvE1x = internal global i64 sub (i64 ptrtoint (i8* blockaddress(@_Z21FoldableAddrLabelDiffv
void FoldableAddrLabelDiff() { static long x = (long)&&a-(long)&&b; a:b:return;}

// CHECK: @i = constant i32* bitcast (float* @PR9558 to i32*)
int &i = reinterpret_cast<int&>(PR9558);

int arr[2];
// CHECK: @pastEnd = constant i32* getelementptr ([2 x i32]* @arr, i32 0, i32 2)
int &pastEnd = arr[2];
