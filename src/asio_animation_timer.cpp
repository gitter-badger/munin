#include "munin/asio_animation_timer.hpp"
#include <boost/asio/steady_timer.hpp>
#include <boost/make_unique.hpp>
#include <mutex>
#include <queue>

namespace munin {

namespace {

struct delayed_function
{
    std::function<void ()>  fn;
    std::chrono::steady_clock::time_point time;
};

struct compare_delayed_function
{
    bool operator()(delayed_function const &lhs, delayed_function const &rhs)
    {
        // In the priority queue, we want the next item (highest priority)
        // to be the one with the lowest time.
        return lhs.time > rhs.time;
    }
};

}

// ==========================================================================
// ASIO_ANIMATION_TIMER::IMPLEMENTATION_STRUCTURE
// ==========================================================================
struct asio_animation_timer::impl
{
    impl(boost::asio::io_context &ctx)
      : steady_timer_(ctx)
    {
    }

    std::recursive_mutex queue_mutex_;
    std::priority_queue<
        delayed_function, 
        std::vector<delayed_function>,
        compare_delayed_function
    > queue_;

    boost::asio::steady_timer steady_timer_;

    // ======================================================================
    // CALL_FUNCTION_IN
    // ======================================================================
    void call_function_in(
        std::function<void ()> const &fn,
        std::chrono::milliseconds delay)
    {
        std::lock_guard<std::recursive_mutex> guard{queue_mutex_};
        queue_.push({fn, now() + delay});
        schedule_next_execution();
    }

    // ======================================================================
    // SCHEDULE_NEXT_EXECUTION
    // ======================================================================
    void schedule_next_execution()
    {
        // Assumes a lock is held for queue_mutex_.

        // Set the timer to expire at the time the next delayed function
        // is set, and then set up a callback to execute any waiting functions
        // at that time.  The call to async_wait may be executed immediately
        // if the wait time has already passed, and this is why the mutex
        // must be recursive.
        steady_timer_.expires_at(queue_.top().time);
        steady_timer_.async_wait(
            [this](boost::system::error_code const &error)
            {
                if (!error)
                {
                    std::lock_guard<std::recursive_mutex> guard(queue_mutex_);
                    execute_waiting_functions();
                }
            });
    }

    // ======================================================================
    // EXECUTE_WAITING_FUNCTIONS
    // ======================================================================
    void execute_waiting_functions()
    {
        // Assumes a lock is held for queue_mutex_.

        // Execute any functions whose expiry time has elapsed.
        while (!queue_.empty() && queue_.top().time <= now())
        {
            queue_.top().fn();
            queue_.pop();
        }

        // If there are functions that remain to be executed, set up the timer
        // again.
        if (!queue_.empty())
        {
            schedule_next_execution();
        }
    }

    // ======================================================================
    // NOW
    // ======================================================================
    std::chrono::steady_clock::time_point now() const
    {
        return std::chrono::steady_clock::now();
    }
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
asio_animation_timer::asio_animation_timer(boost::asio::io_context &ctx)
  : pimpl_(boost::make_unique<impl>(ctx))
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
asio_animation_timer::~asio_animation_timer() = default;

// ==========================================================================
// DO_CALL_FUNCTION_IN
// ==========================================================================
void asio_animation_timer::do_call_function_in(
    std::function<void ()> const &fn,
    std::chrono::milliseconds delay)
{
    pimpl_->call_function_in(fn, delay);
}

// ==========================================================================
// DO_NOW
// ==========================================================================
std::chrono::steady_clock::time_point asio_animation_timer::do_now() const
{
    return pimpl_->now();
}

}