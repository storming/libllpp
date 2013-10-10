#include <iostream>
#include "libll++/member.h"

struct A {
    int a;
    int b;
    int foo() {
        return 0;
    }
};

struct B : A {
};

MEMBER_CHECKER_DECL(check_foo, foo);

int main() 
{
    std::cout << ll::offsetof_member(&A::a) << " "
              << ll::offsetof_member(&A::b) << " "
              //<< ll::offsetof_member(&A::foo) << " "
              << std::endl;

    std::cout << check_foo<A>::has::value << std::endl;
    std::cout << check_foo<A>::has_variable::value << std::endl;
    std::cout << check_foo<A>::has_class::value << std::endl;
    std::cout << check_foo<A>::has_function::value << std::endl;
    std::cout << check_foo<A>::has_signature<int()>::value << std::endl;
    std::cout << check_foo<A>::has_signature<int(int)>::value << std::endl;
    std::cout<<std::is_same<int, ll::member_of<decltype(&A::a)>::type>::value << std::endl;
    std::cout<<std::is_same<A, ll::member_of<decltype(&B::a)>::class_type>::value << std::endl;
	return 0;
}
