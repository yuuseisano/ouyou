#pragma once

#include <memory>

// 状態インターフェイス（テンプレート）
template <typename T>
class State
{
public:
    virtual ~State() = default;
    virtual void Enter(T& owner) = 0;
    virtual void Execute(T& owner) = 0;
    virtual void Exit(T& owner) = 0;
};

// 簡易状態機械
template <typename T>
class StateMachine
{
public:
    StateMachine(T& owner) : mOwner(owner), mCurrent(nullptr) {}

    void ChangeState(std::shared_ptr<State<T>> newState)
    {
        if (mCurrent) mCurrent->Exit(mOwner);
        mCurrent = newState;
        if (mCurrent) mCurrent->Enter(mOwner);
    }

    void Update()
    {
        if (mCurrent) mCurrent->Execute(mOwner);
    }

private:
    T& mOwner;
    std::shared_ptr<State<T>> mCurrent;
};
