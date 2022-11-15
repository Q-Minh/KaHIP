/******************************************************************************
 * timer.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef TIMER_9KPDEP
#define TIMER_9KPDEP

#include <chrono>

class timer {
        public:
                using clock_type = std::chrono::steady_clock;

                timer() {
                        m_start = clock_type::now();
                } 

                void restart() { 
                        m_start = clock_type::now(); 
                } 

                double elapsed() { 
                        auto now = clock_type::now();
                        auto const us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_start).count();
                        return static_cast<double>(us) / 1'000'000.;
                }

                std::chrono::time_point<std::chrono::steady_clock> m_start;
}; 

#endif /* end of include guard: TIMER_9KPDEP */
