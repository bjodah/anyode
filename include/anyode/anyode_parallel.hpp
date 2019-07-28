#pragma once
#include <exception>
#include <mutex>

namespace anyode_parallel {
    class ThreadException {
        std::mutex m_lock;
    public:
        std::vector<std::pair<int, std::exception_ptr> > m_exc;
        //ThreadException(): m_exc(nullptr) {}
        void rethrow() {
            for (const auto& idx_exc : m_exc) {
                const auto& exc = idx_exc.second;
                if (exc) {
                    std::rethrow_exception(exc);
                }
            }
        }
        void capture_exception(int idx){
            std::unique_lock<std::mutex> guard(m_lock);
            m_exc.push_back({idx, std::current_exception()});
        }
        template <typename F, typename... P>
        void run(int idx, F f, P... p){
            try {
                f(p...);
            } catch (...) {
                capture_exception(idx);
            }
        }
        bool holds_exception() { return m_exc.size() > 0; }
    };
}
