// RUN: %clang_cc1 -fno-rtti -emit-llvm -triple=i386-pc-win32 %s -o - | FileCheck %s --check-prefix=CHECK32
// RUN: %clang_cc1 -fno-rtti -emit-llvm -triple=x86_64-pc-win32 %s -o - | FileCheck %s --check-prefix=CHECK64

struct S {
  int x, y, z;
};

// U is not trivially copyable, and requires inalloca to pass by value.
struct U {
  int u;
  U();
  ~U();
  U(const U &);
};

struct C {
  virtual void foo();
  virtual int bar(int, double);
  virtual S baz(int);
  virtual S qux(U);
};

namespace {
struct D {
  virtual void foo();
};
}

void f() {
  void (C::*ptr)();
  ptr = &C::foo;
  ptr = &C::foo; // Don't crash trying to define the thunk twice :)

  int (C::*ptr2)(int, double);
  ptr2 = &C::bar;

  S (C::*ptr3)(int);
  ptr3 = &C::baz;

  void (D::*ptr4)();
  ptr4 = &D::foo;

  S (C::*ptr5)(U);
  ptr5 = &C::qux;


// CHECK32-LABEL: define void @"\01?f@@YAXXZ"()
// CHECK32: store i8* bitcast (void (%"struct.\01?C@@"*, ...)* @"\01??_9C@@$BA@AE" to i8*), i8** %ptr
// CHECK32: store i8* bitcast (void (%"struct.\01?C@@"*, ...)* @"\01??_9C@@$B3AE" to i8*), i8** %ptr2
// CHECK32: store i8* bitcast (void (%"struct.\01?C@@"*, ...)* @"\01??_9C@@$B7AE" to i8*), i8** %ptr3
// CHECK32: store i8* bitcast (void (%"struct.\01?D@?A@@"*, ...)* @"\01??_9D@?A@@$BA@AE" to i8*), i8** %ptr4
// CHECK32: }
//
// CHECK64-LABEL: define void @"\01?f@@YAXXZ"()
// CHECK64: store i8* bitcast (void (%"struct.\01?C@@"*, ...)* @"\01??_9C@@$BA@AA" to i8*), i8** %ptr
// CHECK64: store i8* bitcast (void (%"struct.\01?C@@"*, ...)* @"\01??_9C@@$B7AA" to i8*), i8** %ptr2
// CHECK64: store i8* bitcast (void (%"struct.\01?C@@"*, ...)* @"\01??_9C@@$BBA@AA" to i8*), i8** %ptr3
// CHECK64: store i8* bitcast (void (%"struct.\01?D@?A@@"*, ...)* @"\01??_9D@?A@@$BA@AA" to i8*), i8** %ptr
// CHECK64: }
}


// Thunk for calling the 1st virtual function in C with no parameters.
// CHECK32-LABEL: define linkonce_odr x86_thiscallcc void @"\01??_9C@@$BA@AE"(%"struct.\01?C@@"* %this, ...)
// CHECK32-NOT:             unnamed_addr
// CHECK32: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 0
// CHECK32: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK32: musttail call x86_thiscallcc void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK32-NEXT: ret void
// CHECK32: }
//
// CHECK64-LABEL: define linkonce_odr void @"\01??_9C@@$BA@AA"(%"struct.\01?C@@"* %this, ...)
// CHECK64-NOT:             unnamed_addr
// CHECK64: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 0
// CHECK64: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK64: musttail call void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK64-NEXT: ret void
// CHECK64: }

// Thunk for calling the 2nd virtual function in C, taking int and double as parameters, returning int.
// CHECK32-LABEL: define linkonce_odr x86_thiscallcc void @"\01??_9C@@$B3AE"(%"struct.\01?C@@"* %this, ...)
// CHECK32: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 1
// CHECK32: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK32: musttail call x86_thiscallcc void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK32-NEXT: ret void
// CHECK32: }
//
// CHECK64-LABEL: define linkonce_odr void @"\01??_9C@@$B7AA"(%"struct.\01?C@@"* %this, ...)
// CHECK64: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 1
// CHECK64: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK64: musttail call void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK64-NEXT: ret void
// CHECK64: }

// Thunk for calling the 3rd virtual function in C, taking an int parameter, returning a struct.
// CHECK32-LABEL: define linkonce_odr x86_thiscallcc void @"\01??_9C@@$B7AE"(%"struct.\01?C@@"* %this, ...)
// CHECK32: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 2
// CHECK32: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK32: musttail call x86_thiscallcc void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK32-NEXT: ret void
// CHECK32: }
//
// CHECK64-LABEL: define linkonce_odr void @"\01??_9C@@$BBA@AA"(%"struct.\01?C@@"* %this, ...)
// CHECK64: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 2
// CHECK64: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK64: musttail call void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK64-NEXT: ret void
// CHECK64: }

// Thunk for calling the virtual function in internal class D.
// CHECK32-LABEL: define internal x86_thiscallcc void @"\01??_9D@?A@@$BA@AE"(%"struct.\01?D@?A@@"* %this, ...)
// CHECK32: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?D@?A@@"*, ...)** %{{.*}}, i64 0
// CHECK32: [[CALLEE:%.*]] = load void (%"struct.\01?D@?A@@"*, ...)** [[VPTR]]
// CHECK32: musttail call x86_thiscallcc void (%"struct.\01?D@?A@@"*, ...)* [[CALLEE]](%"struct.\01?D@?A@@"* %{{.*}}, ...)
// CHECK32-NEXT: ret void
// CHECK32: }
//
// CHECK64-LABEL: define internal void @"\01??_9D@?A@@$BA@AA"(%"struct.\01?D@?A@@"* %this, ...)
// CHECK64: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?D@?A@@"*, ...)** %{{.*}}, i64 0
// CHECK64: [[CALLEE:%.*]] = load void (%"struct.\01?D@?A@@"*, ...)** [[VPTR]]
// CHECK64: musttail call void (%"struct.\01?D@?A@@"*, ...)* [[CALLEE]](%"struct.\01?D@?A@@"* %{{.*}}, ...)
// CHECK64-NEXT: ret void
// CHECK64: }

// Thunk for calling the fourth virtual function in C, taking a struct parameter
// and returning a struct.
// CHECK32-LABEL: define linkonce_odr x86_thiscallcc void @"\01??_9C@@$BM@AE"(%"struct.\01?C@@"* %this, ...)
// CHECK32: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 3
// CHECK32: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK32: musttail call x86_thiscallcc void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK32-NEXT: ret void
// CHECK32: }
//
// CHECK64-LABEL: define linkonce_odr void @"\01??_9C@@$BBI@AA"(%"struct.\01?C@@"* %this, ...)
// CHECK64: [[VPTR:%.*]] = getelementptr inbounds void (%"struct.\01?C@@"*, ...)** %{{.*}}, i64 3
// CHECK64: [[CALLEE:%.*]] = load void (%"struct.\01?C@@"*, ...)** [[VPTR]]
// CHECK64: musttail call void (%"struct.\01?C@@"*, ...)* [[CALLEE]](%"struct.\01?C@@"* %{{.*}}, ...)
// CHECK64: ret void
// CHECK64: }
