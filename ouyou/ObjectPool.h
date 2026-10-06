#pragma once

#include <vector>
#include <memory>
#include <stack>

// シンプルなオブジェクトプール（テンプレート）
template <typename T>
class ObjectPool
{
public:
    ObjectPool() = default;

    // プールから取得。存在しなければ new して返す
    std::shared_ptr<T> Acquire()
    {
        if (!mFree.empty())
        {
            auto obj = mFree.top();
            mFree.pop();
            return obj;
        }

        // カスタム deleter を使って、Release を呼ぶようにする
        std::shared_ptr<T> ptr(new T(), [this](T* p) {
            // デフォルト delete する代わりにプールに戻す
            std::shared_ptr<T> sp(p, [](T*){}); // 一時 shared_ptr（所有権は維持しない）
            mFree.push(sp);
        });
        return ptr;
    }

private:
    std::stack<std::shared_ptr<T>> mFree;
};
