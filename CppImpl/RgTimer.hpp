#ifndef RG_TIMER_HPP_
#define RG_TIMER_HPP_

#include <functional>
#include "timers.h"

namespace RgEmbeddedOs
{
	class RgTimer {
	public:
		using TimerCallback_t = std::function<void()>;

		RgTimer(const char* name, TickType_t period, bool autoReload, TimerCallback_t callback = nullptr)
			: callback_(std::move(callback))
		{
			handle_ = xTimerCreate(name, period, autoReload ? pdTRUE : pdFALSE, this, &RgTimer::TimerCallback);
		}

		~RgTimer() {
			if (handle_) {
				stop();
				xTimerDelete(handle_, 0);
			}
		}

		void start(TickType_t delay = 0) {
			xTimerStart(handle_, delay);
		}

		void stop() {
			xTimerStop(handle_, 0);
		}

		void setPeriod(TickType_t period) {
		    xTimerChangePeriod(handle_, period, 0);
		}

		void setCallback(TimerCallback_t callback) {
			callback_ = std::move(callback);
		}

		bool isActive() const {
			return xTimerIsTimerActive(handle_) != pdFALSE;
		}

	private:
		static void TimerCallback(TimerHandle_t xTimer)
		{
			auto* timer = static_cast<RgTimer*>(pvTimerGetTimerID(xTimer));
			if (timer && timer->callback_) {
				timer->callback_();
			}
		}

		TimerHandle_t handle_;
		TimerCallback_t callback_;
	};
}

#endif
