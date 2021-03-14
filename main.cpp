#include "ForwardList.hpp"

template <class T>
void test(const std::string& name, auto const& list, std::initializer_list<T>&& init_list) {
    auto a = list.begin(), ea = list.end();
    auto b = init_list.begin(), eb = init_list.end();
    bool result = true;
    while(a != ea and b != eb) {
        (result &= (*a == *b));
        std::cout << "{" << *a++ << "-" << *b++ << "} ";
        
    }
    result &= (a == ea and b == eb);
    std::cout << "Test " + name + (result ? " ok" : " PANIC") + "\n";
}

void tests() {
    
    ForwardList<int> l;
    l.push_front(1);
    l.push_front(2);
    l.push_front(3);
    test("Push_back", l, {3, 2, 1});
    
    ForwardList<int> l2(3, 1);
    test("N fill 1", l2, {1, 1, 1});
    
    ForwardList<int> l0(3);
    test("N fill default", l0, {0, 0, 0});
    
    ForwardList<int> l3(l.cbegin(), l.cend());
    test("Iter construct", l3, {3, 2, 1});
    
    ForwardList<int> l4(l2);
    test("Copy construct", l4, {1, 1, 1});
    
    ForwardList<int> l5({{1, 2, 3, 4, 5}});
    test("Init-list construct", l5, {1, 2, 3, 4, 5});
    
    ForwardList<X> l6({1, 2, 3});
    test("Init list X construct", l6, {1, 2, 3});
    l6.pop_front();
    test("Remove", l6, {2, 3});
    
    ForwardList<X> l7(std::move(l6));
    test("Move - empty", l6, std::initializer_list<X>{});
    test("Move - new", l7, {2, 3});
    
    ForwardList<int> l8({1, 2, 3, 4, 4, 4, 5, 6, 7, 4, 4, 4, 8, 9, 10});
    l8.remove(4);
    test("Remove 4", l8, {1, 2, 3, 5, 6, 7, 8, 9, 10});
    constexpr auto predicate = [] (int x) { return x % 2 == 0; };
    l8.remove_if(predicate);
    test("Remove if % 2", l8, {1, 3, 5, 7, 9});
    std::cout << "Size test: " << (l8.size() == 5 ? " ok" : " PANIC") << "\n";
    
    l8 = std::move(l);
    test("Move assign - l - empty", l, std::initializer_list<int>{});
    test("Move assign - l8 - assigned", l8, {3, 2, 1});
    
    l8 = l5;
    test("Copy assign - l - stay", l5, {1, 2, 3, 4, 5});
    test("Copy assign - l8 - assigned", l8, {1, 2, 3, 4, 5});
    
    ForwardList<int> l9({1, 2, 3, 4, 5, 6});
    l9.insert_after(++l9.begin(), 12);
    test("Insert copy", l9, {1, 2, 12, 3, 4, 5, 6});
    
    ForwardList<X> l10({1, 2, 3});
    l10.insert_after(++l10.begin(), 12);
    test("Insert move", l10, {1, 2, 12, 3});
    
    ForwardList<X> l11({1, 2, 3});
    l11.insert_after(++l11.begin(), 3, 12);
    test("Insert move", l11, {1, 2, 12, 12, 12, 3});
    
    l11.clear();
    test("Clear", l11, std::initializer_list<X>{});
    
    ForwardList<int> l12({1, 2});
    l12.insert_after(++l12.begin(), l9.begin(), l9.end());
    test("Iter insert after", l12, { 1, 2, 1, 2, 12, 3, 4, 5, 6});
    
    ForwardList<int> l13({1, 2});
    l13.insert_after(l13.begin(), {1, 2, 3});
    test("Initializer list insert after", l13, { 1, 1, 2, 3, 2 });
    
    ForwardList<X> l14{1, 2};
    l14.emplace_after(l14.begin(), 12);
    test("Emplace insert after", l14, { 1, 12, 2 });
    
    ForwardList<int> l15({1, 2, 3, 4, 5, 6, 7});
    l15.erase_after(++++l15.begin());
    test("Erase after element", l15, {1, 2, 3, 5, 6, 7});
    
    ForwardList<int> l16({1, 2, 3, 4, 5, 6, 7});
    l16.erase_after(std::next(l16.begin(), 2), l16.end());
    test("Erase range after element", l16, {1, 2, 3});
    
    ForwardList<int> l17({ 1, 2, 3, 4, 5, 6, 7});
    l17.resize(5);
    test("Resize shrink", l17, {3, 4, 5, 6, 7});
    l17.resize(8);
    test("Resize add", l17, {0, 0, 0, 3, 4, 5, 6,7});
    
    ForwardList<int> l18({ 1, 2, 3, 4, 5, 6, 7});
    l18.assign(5, 666);
    test("Assign shrink", l18, {666, 666, 666, 666, 666});
    l18.assign(8, 777);
    test("Assign add", l18, {777, 777, 777, 777, 777, 777, 777});
    l18.assign(l17.begin(), std::next(l17.begin(), 4));
    test("Assign range shrink", l18, {0, 0, 0, 3});
    l18.assign({ 1, 2, 3, 4, 5 });
    test("Assign initializer add", l18, {1, 2, 3, 4, 5});
    
    ForwardList<int> l19({1, 2, 3});
    ForwardList<int> l20({4, 5, 6, 7});
    l19.swap(l20);
    test("Swap 1", l19, {4, 5, 6, 7});
    test("Swap 2", l20, {1, 2, 3});
    l20.reverse();
    test("Reverse", l20, {3, 2, 1});
    
    ForwardList<int> l21({1, 2, 3, 4, 5});
    ForwardList<int> l22 = l21.split_when([] (int x) { return x == 3; });
    test("Split head", l21, {1, 2 });
    test("Split tail", l22, {3, 4, 5});
}

int main() {
    tests();
    return 0;
}
