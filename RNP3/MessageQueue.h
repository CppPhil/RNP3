#pragma once
#include <queue>
#include <mutex>

namespace utils {
    template <class ValueType>
    class ThreadSafeQueue final {
    public:
        using this_type = ThreadSafeQueue;
        using value_type = ValueType;
        using container_type = std::queue<value_type>;
        using mutex_type = std::mutex;
        using lock_type = std::unique_lock<mutex_type>;

        value_type pop(); // returns the 'first' item; blocks if there are no elements until there is at least one
        void push(value_type const &data); // add an element to the back
        void push(value_type &&data);
        bool isEmpty() const; // query the queue as to whether it is empty or not

    private:
        container_type cont_; // the underlying std::queue container
        mutable mutex_type mu_; // mutext to synchronize access
        std::condition_variable cvHasMsgs_; // condvar to wake up threads sleeping on it
    }; // END of class ThreadSafeQueue

    template <class ValueType>
    typename ThreadSafeQueue<ValueType>::value_type ThreadSafeQueue<ValueType>::pop() { // remove and retrieve
        lock_type lock{ mu_ };
        cvHasMsgs_.wait(lock, [this] { // wait until the container is no longer empty
            return !cont_.empty(); // will happen instantly if it has elements
        });
        auto retMe = cont_.front(); // get the first element
        cont_.pop(); // remove it from the queue
        return retMe;
    }

    template <class ValueType>
    void ThreadSafeQueue<ValueType>::push(value_type const &data) {
        lock_type lock{ mu_ };
        cont_.push(data); // add the element to the back
        cvHasMsgs_.notify_all(); // wake up the threads waiting for an item to show up in pop()
    }

    template <class ValueType>
    void ThreadSafeQueue<ValueType>::push(value_type &&data) {
        lock_type lock{ mu_ };
        cont_.push(std::move(data));
        cvHasMsgs_.notify_all();
    }

    template <class ValueType>
    bool ThreadSafeQueue<ValueType>::isEmpty() const {
        lock_type lock{ mu_ };
        return cont_.empty();
    }
} // END of namespace utils
