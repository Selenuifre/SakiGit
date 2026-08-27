#ifndef OPERATORS_H
#define OPERATORS_H

// CRTP 混入类：为只定义了 operator== 的类自动生成 operator!=。
//
// 用法：
//   class MyClass : public EqualityOperators<MyClass> {
//   public:
//       bool operator==(const MyClass& other) const { ... }
//       // operator!= 由 EqualityOperators 自动提供
//   };
//
// 当前应用于：Branch, Commit, FileChange, Repository, Stash

template <typename Derived>
class EqualityOperators
{
public:
    friend bool operator!=(const Derived& lhs, const Derived& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif // OPERATORS_H
