//
// Created by Aeomanate on 14.03.2021.
//

#ifndef MAIN_CPP_FORWARDLIST_HPP
#define MAIN_CPP_FORWARDLIST_HPP

#include <iostream>
#include <memory>
#include <iterator>
#include <functional>
#include <utility>

template <class T, class U>
concept comparable_with = requires (U u) {
    { T() == u } -> std::convertible_to<bool>;
};

template <class T, class... Params>
concept predicate = requires (T t, Params... params) {
    { t(params...) } -> std::convertible_to<bool>;
};

template <class T>
class ForwardList {
    struct Node {
        explicit Node(Node* next, T const& value)
            : value(value)
            , next(next)
        { }
        
        explicit Node(Node* next, T&& value)
            : value(std::move(value))
            , next(next)
        { }
        
        template <class... Params>
        explicit Node(Node* next, Params&&... params)
            : value(std::forward<Params>(params)...)
            , next(next)
        { }
        
        explicit Node(std::unique_ptr<Node>&& next, T const& value)
            : value(value)
            , next(std::move(next))
        { }
        
        explicit Node(std::unique_ptr<Node>&& next, T&& value)
            : value(std::move(value))
            , next(std::move(next))
        { }
        
        template <class... Params>
        explicit Node(std::unique_ptr<Node>&& next, Params&&... params)
            : value(std::forward<params>(params)...)
            , next(std::move(next))
        { }
        
        Node(Node&& other) noexcept
            : value(std::move(other.value))
            , next(std::move(other.next))
        { }
        
        T value;
        std::unique_ptr<Node> next;
    };
    
    template <bool is_const = false>
    struct ForwardIterator {
      public:
        template <class U>
        using add_const_if      = std::conditional_t<is_const, std::add_const_t<U>, U>;
        using Self              = ForwardIterator;
        using SelfR             = Self&;
        using SelfCR            = Self const&;
        using SelfRR            = Self&&;
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = ptrdiff_t;
        using pointer           = add_const_if<value_type>*;
        using reference         = add_const_if<value_type>&;
      public:
        ForwardIterator(): node(nullptr) { }
        explicit ForwardIterator(Node* node): node(node) { }
        ForwardIterator(SelfCR other): node(other.node) { }
        ForwardIterator(SelfRR other) noexcept: node(other.node) { other.node = nullptr; }
        operator ForwardIterator<true>() { return ForwardIterator<true>(node); }
        
        Self operator++(int) {Self t = *this; ++*this; return t; }
        SelfR operator++() { node = node->next.get(); return *this; }
        friend bool operator==(SelfCR a, SelfCR b) { return a.node == b.node; }
        reference operator*() const { return node->value; }
        reference operator*() { return node->value; }
        
        SelfR operator=(SelfCR other) { node = other.node; return *this; }
      
      public:
        Node* node = nullptr;
    };
  
  public:
    using iterator = ForwardIterator<>;
    using const_iterator = ForwardIterator<true>;
  
  public:
    ForwardList() = default;
    
    ForwardList(size_t n, T const& value) { while(n--) push_front(value); }
    
    explicit ForwardList(size_t n) { while(n--) push_front(T()); }
    
    ForwardList(ForwardList const& other): ForwardList(other.begin(), other.end()) { }
    
    ForwardList(ForwardList&& other) noexcept
        : head(std::move(other.head))
    { other.head.reset(); }
    
    ForwardList(std::input_iterator auto begin, std::input_iterator auto end) { recursive_insert(begin, end); }
    
    template <std::convertible_to<T> U>
    ForwardList(std::initializer_list<U>&& l) { recursive_insert(l.begin(), l.end()); }
    
    
    void assign(size_t n, T const& value = T()) {
        iterator cur = begin();
        if(n < count_nodes) {
            size_t counter = 0;
            for(size_t i = 0; i != n; ++i) {
                *cur = value;
                counter += 1;
                if(i+1 != n) ++cur;
            }
            cur.node->next.reset();
            count_nodes = counter;
        } else {
            for(size_t i = 0; i != count_nodes; ++i, --n) {
                *cur = value;
                if(i+1 != count_nodes) ++cur;
            }
            while(--n) {
                insert_after(cur, value);
                ++cur;
            }
        }
    }
    void assign(std::input_iterator auto a, std::input_iterator auto b) {
        iterator cur = begin(), stop = end(), prev;
        size_t counter = 0;
        while(a != b and cur != stop) {
            prev = cur;
            *cur++ = *a++;
            counter += 1;
        }
        if(a != b) {
            insert_after(prev, a, b);
        } else {
            prev.node->next.reset();
            count_nodes = counter;
        }
    }
    template <std::convertible_to<T> U>
    void assign(std::initializer_list<U>&& ilist) {
        assign(ilist.begin(), ilist.end());
    }
    
    ForwardList& operator=(ForwardList<T> const& other) {
        head.reset();
        recursive_insert(other.begin(), other.end());
        return *this;
    }
    ForwardList& operator=(ForwardList&& other) noexcept {
        head = std::move(other.head);
        return *this;
    }
    
    
    [[nodiscard]] bool empty() const { return size() == 0; }
    [[nodiscard]] size_t size() const { return count_nodes; }
    void swap(ForwardList& other) { std::swap(head, other.head); }
    void resize(size_t new_size, T const& value = T()) {
        if(count_nodes == new_size) return;
        if(count_nodes < new_size) {
            while(count_nodes != new_size) push_front(value);
        } else {
            while(count_nodes != new_size) pop_front();
        }
    }
    void reverse() {
        ForwardList temp;
        for(auto&& item : *this) {
            temp.push_front(std::move(item));
        }
        swap(temp);
    }
    ForwardList split_when(predicate<T> auto cond) {
        if(cond(front())) return ForwardList(std::move(*this));
        const_iterator cur = begin();
        while(cur.node->next and !cond(cur.node->next->value)) ++cur;
        ForwardList result;
        result.head = std::move(cur.node->next);
        return result;
    }
    
    T& front() { return head->value; }
    void push_front(T&& value) {
        head = std::make_unique<Node>(head.release(), std::move(value));
        count_nodes += 1;
    }
    void push_front(T const& value) {
        head = std::make_unique<Node>(head.release(), value);
        count_nodes += 1;
    }
    
    const_iterator insert_after(const_iterator i, T const& value) {
        i.node->next = std::make_unique<Node>(i.node->next.release(), value);
        count_nodes += 1;
        return ++i;
    }
    const_iterator insert_after(const_iterator i, T&& value) {
        i.node->next = std::make_unique<Node>(i.node->next.release(), std::move(value));
        count_nodes += 1;
        return ++i;
    }
    const_iterator insert_after(const_iterator i, size_t n, T const& value) {
        while(n--) i = insert_after(i, value);
        return i;
    }
    const_iterator insert_after(const_iterator i, std::input_iterator auto begin, std::input_iterator auto end) {
        while(begin != end) i = insert_after(i, *begin++);
        return i;
    }
    const_iterator insert_after(const_iterator i, std::initializer_list<T> ilist) {
        return insert_after(i, ilist.begin(), ilist.end());
    }
    template <class... Args>
    const_iterator emplace_after(const_iterator i, Args&&... args) {
        i.node->next = std::make_unique<Node>(i.node->next.release(), std::forward<Args>(args)...);
        count_nodes += 1;
        return ++i;
    }
    
    
    void pop_front() {
        auto next = std::move(head->next);
        head.swap(next);
        count_nodes -= 1;
    }
    
    void clear() {
        head.reset();
        count_nodes = 0;
    }
    
    const_iterator erase_after(const_iterator i) {
        if(!i.node->next) return end();
        i.node->next = std::move(i.node->next->next);
        count_nodes -= 1;
        return ++i;
    }
    const_iterator erase_after(const_iterator begin, const_iterator end) {
        while(const_iterator(begin.node->next.get()) != end) erase_after(begin);
        return begin;
    }
    
    void remove(comparable_with<T> auto to_remove) {
        auto predicate = [&to_remove] (T const& t) { return t == to_remove; };
        remove_if(predicate);
    }
    template <predicate<T> Predicate>
    void remove_if(Predicate predicate) {
        if(empty()) return;
        while(predicate(*begin())) pop_front();
        if(empty()) return;
        
        iterator cur = begin(), stop = end(), next = cur;
        while(next != stop) {
            next = cur;
            ++next;
            if(next == stop) break;
            if(!predicate(*next)) {
                ++next;
                ++cur;
                continue;
            }
            auto temp_after_next = std::move(next.node->next);
            cur.node->next.swap(temp_after_next);
            count_nodes -= 1;
            
        }
    }
    
    
    [[nodiscard]] iterator begin()              { return iterator(head.get());       }
    [[nodiscard]] iterator end()                { return iterator();                 }
    [[nodiscard]] const_iterator begin()  const { return cbegin();                   }
    [[nodiscard]] const_iterator end()    const { return cend();                     }
    [[nodiscard]] const_iterator cbegin() const { return const_iterator(head.get()); }
    [[nodiscard]] const_iterator cend()   const { return const_iterator();           }
  
  private:
    template <std::input_iterator ForwartIt>
    void recursive_insert(ForwartIt begin, ForwartIt end) {
        auto t = begin;
        if(begin != end) recursive_insert(++begin, end);
        else return;
        push_front(*t);
    }
  
  private:
    std::unique_ptr<Node> head;
    size_t count_nodes = 0;
};

struct X {
    X(int x): x(x) { std::cout << "Construct " << x << "\n"; }
    X(X&& other) noexcept : x(other.x) { std::cout << "Move " << x << "\n"; }
    X(X const& other): x(other.x) { std::cout << "Copy " << x << "\n"; }
    int x;
    friend std::ostream& operator<<(std::ostream& out, X const& some_x) {
        out << some_x.x;
        return out;
    }
    bool operator== (X const& other) const { return x == other.x; }
};

#endif //MAIN_CPP_FORWARDLIST_HPP
