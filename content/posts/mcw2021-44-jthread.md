---
title: "[MCW2021-44] std::jthread 简要说明"
date: 2021-11-07T22:53:10+08:00
draft: false
---

- 需要标准: c++20
- 测试环境: Fedora34, gcc 11.2.1

## 说明
`std::jthread` 提供了和 `std::thread` 几乎一致的行为。除了：
- 提供了析构的时候自动join的功能，可能因此命名为 `jthread`
- 在特定情况下更方便地取消或者结束线程

## 析构自动join

```c++
std::thread t([](){
    sleep_for(seconds(1));
});
t.join();
```

参考这个代码在使用`std::thread`的时候，我需要对线程对象，在释放前进行显式的`join`或者`detach`操作，否则线程对象在释放的时候会被销毁，但是线程还在运行，导致程序崩溃。

在使用`std::jthread`的时候，那么就不需要了，对象默认在析构的时候会自动`join`运行的线程函数。

这一点对于编写一些包含线程函数的类的时候，特别有意义。
设想一下如果一个类包含一个线程函数，并且希望在对象生命结束的时候，去等待线程结束，那么势必需要在析构函数里面增加线程对象`join`的操作。
而如果你是一个代码洁癖控的话，那么 [Rule Of Five](https://en.cppreference.com/w/cpp/language/rule_of_three) 可能会让你除了实现析构，还去显示地实现拷贝和移动的构造函数和赋值操作。
当然目前在现代C++里面你可以方便地使用类似下面的代码来简要申明即可。
```c++
Foo(const Foo&) = delete;
Foo& operator=(const Foo&) = delete;
Foo(Foo&&) = default;
Foo& operator=(Foo&&) = default;
```

而是用`jthread`去实现的话，那么就可遵循 *Rule of Zero* 就会使代码更加简洁。

## 取消/结束线程

下面是一个非常简单的循环线程函数，当我们从外部给它一个取消的信号时（`running` -> `false`），它就会结束运行。
```c++
void thread_loop(bool &running)
{
    int i = 0;
    while(running) {
        sleep_for(seconds(1));
        cout << "running: " << ++i << endl;
    }
}
```

当然这需要额外引入一个变量 running，而在使用`jthread`的时候，事情就变得简单一些了。
参考下面的代码：
```c++
void jthread_loop(const std::stop_token& stop)
{
    int i = 0;
    while(not stop.stop_requested()) {
        sleep_for(seconds(1));
        cout << "running: " << ++i << endl;
    }
}
```
当线程对象结束时，内部的`stop_token`对象会收到stop的请求，因此线程函数就可以使用这个标识来作为判断是否需要退出循环的依据。

完整的代码示例，可以参考：[main.cpp](https://github.com/matrixji/modern-cpp-weekly/blob/main/source/2021/w44/main.cpp)


## 备注
考虑到文章会通过微信转发，我尽量使用了`using`来减少代码中的宽度。如果觉得阅读不便利，还望指正。

