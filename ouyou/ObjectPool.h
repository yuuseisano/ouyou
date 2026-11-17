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

        // 新規生成分は独自の削除処理で回収する。プールは取得した参照より長く生存させる。
        std::shared_ptr<T> ptr(new T(), [this](T* p) {
            // オブジェクトを削除せず、空き一覧へ戻す。
            std::shared_ptr<T> sp(p, [](T*){}); // 削除処理を持たない参照として保持する。
            mFree.push(sp);
        });
        return ptr;
    }

private:
    std::stack<std::shared_ptr<T>> mFree;
};
