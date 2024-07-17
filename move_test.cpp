#include <iostream>
#include <vector>
#include <string>

class MoveTest {
public:

    MoveTest() {
    }

    MoveTest(const MoveTest& a) {
        std::cout << "    copy struct is using\n";
        p = new char(*(a.p));
    }

    MoveTest(MoveTest&& a) {
        std::cout << "    move struct is using\n";
        p = a.p;
        a.p = nullptr;
        delete a.p;
        a.a = 100;
        this->a = 99;
    }  

    ~MoveTest() {
        if (p){
            delete p;
        }
    }

    void add_char(char text) {
        p = new char(text);

    }
    void change_vec(int a) {
        vec[0] = a;
    }
    int a;
    char* p = nullptr;
    std::vector<int> vec {4};

};


void test_f(MoveTest test) {
    std::cout << " ===after move, test.a is " <<test.a <<" test.vec is " << test.vec.front() << std::endl; 
    test.a = 1;
}

void test_f(MoveTest& test) {
    std::cout << " ===++after move, test.a is " << test.a <<" test.vec is " << test.vec.front() << std::endl; 
    test.a = 1;
}
int main() {
    MoveTest test1;
    test1.add_char('a');
    MoveTest test2 = test1;
    MoveTest test3 = std::move(test2);
    test3.change_vec(9);
    test_f(std::move(test3)); // after add std::move to test3, the move is using, which made the test3 undefined;
    std::cout << " after move, test3.a is " << test3.a << " test3.vec is "<< test3.vec.front() << std::endl; 


    return 0;
}
