#ifndef KERNEL_CORE_HPP
#define KERNEL_CORE_HPP

#include "FreeRTOS.h"
#include "task.h"

namespace RgEmbeddedOs
{
	class KernelCore
	{
	public:
		// Частота тиков системы (количество тиков в секунду), определяется конфигурацией FreeRTOS
		static constexpr TickType_t TickRateHz = configTICK_RATE_HZ;

		// Перечисление состояний планировщика задач
		enum class SchedulerState
		{
			Suspended = taskSCHEDULER_SUSPENDED, // Планировщик приостановлен
			NotStarted = taskSCHEDULER_NOT_STARTED, // Планировщик ещё не был запущен
			Running = taskSCHEDULER_RUNNING, // Планировщик запущен и выполняет задачи
		};

	#if (INCLUDE_xTaskGetSchedulerState == 1)
		// Возвращает текущее состояние планировщика задач
		static inline SchedulerState getSchedulerState() { return static_cast<SchedulerState>(xTaskGetSchedulerState()); }
		// Проверяет, запущен ли планировщик
		static inline bool isRunning() { return getSchedulerState() == SchedulerState::Running; }
	#endif

		// Запускает планировщик задач (начинает выполнение задач)
		static inline void start() { vTaskStartScheduler(); }
		// Останавливает планировщик задач (при завершении выполнения задач)
		static inline void stop() { vTaskEndScheduler(); }

		// Приостанавливает планировщик задач (временно блокирует переключение задач)
		static inline void lock() { vTaskSuspendAll(); }
		// Возобновляет работу планировщика задач, возвращает true, если были разблокированы задачи
		static inline bool unlock() { return (xTaskResumeAll() == pdTRUE); }

		// Возвращает текущее количество тиков системы с учётом того, выполняется ли вызов в прерывании
		static inline TickType_t getTickCount()
		{
			if(xPortIsInsideInterrupt() == pdTRUE)
				return xTaskGetTickCountFromISR(); // Получение количества тиков из обработчика прерывания

			return xTaskGetTickCount(); // Получение количества тиков в обычном режиме
		}
		// Возвращает время работы системы в миллисекундах
		static uint32_t getUptimeMs() { return getTickCount() * portTICK_PERIOD_MS; }

		// Возвращает текущий объём свободной кучи (heap)
		static size_t getFreeHeap() { return xPortGetFreeHeapSize(); }
		// Возвращает минимальный когда-либо зафиксированный объём свободной кучи
		static size_t getMinimumEverFreeHeap() { return xPortGetMinimumEverFreeHeapSize(); }

		// Входит в критическую секцию (запрещает прерывания)
		static void enterCritical() { vPortEnterCritical(); }
		// Выходит из критической секции (разрешает прерывания)
		static void exitCritical() { vPortExitCritical(); }

		// Задержка выполнения задачи на указанное количество тиков (зависит от TickRateHz)
		static void delay(TickType_t ticks) { vTaskDelay(ticks); }
		// Задержка выполнения задачи до определённого момента времени (зависит от TickRateHz)
		static void delayUntil(TickType_t* previousWakeTime, TickType_t ticks) { vTaskDelayUntil(previousWakeTime, ticks); }
	};
}

#endif
