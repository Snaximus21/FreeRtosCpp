#ifndef RG_TIMER_HPP_
#define RG_TIMER_HPP_

#include <functional>
#include "timers.h"

namespace RgEmbeddedOs
{
	class RgTimer {
	public:
		using TimerCallback_t = std::function<void()>;

		RgTimer(const char* name, TickType_t period, bool autoReload, TimerCallback_t callback)
			: callback(std::move(callback))
		{
			handle = xTimerCreate(name, period, autoReload ? pdTRUE : pdFALSE, this, &RgTimer::TimerCallback);
		}

		~RgTimer() {
			if (handle) {
				xTimerDelete(handle, 0);
			}
		}

		void start(TickType_t delay = 0) {
			xTimerStart(handle, delay);
		}

		void stop() {
			xTimerStop(handle, 0);
		}

		bool isActive() const {
			return xTimerIsTimerActive(handle) != pdFALSE;
		}

	private:
		static void TimerCallback(TimerHandle_t xTimer)
		{
			auto* timer = static_cast<RgTimer*>(pvTimerGetTimerID(xTimer));
			if (timer && timer->callback) {
				timer->callback();
			}
		}

		TimerHandle_t handle;
		TimerCallback_t callback;
	};
}

#endif
