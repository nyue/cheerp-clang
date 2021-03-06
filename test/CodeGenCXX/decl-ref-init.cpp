// RUN: %clang_cc1 -triple x86_64-apple-darwin -emit-llvm %s -o - | \
// RUN: FileCheck %s
// RUN: %clang_cc1 -triple i386-apple-darwin -emit-llvm %s -o - | \
// RUN: FileCheck %s

struct A {};

struct B 
{ 
  operator A&();
}; 


struct D : public B {
  operator A();
};

extern B f(); 
extern D d(); 

int main() {
	const A& rca = f();
	const A& rca2 = d();
}

// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z1A* @_ZN1BcvR1AEv
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z1A* @_ZN1BcvR1AEv
