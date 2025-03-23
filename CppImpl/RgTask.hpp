#ifndef RG_TASK_HPP
#define RG_TASK_HPP

#include <functional>
#include <cstdint>
#include "task.h"
#include "inc/RgAssert.h"
#include "inc/Math.hpp"

namespace RgEmbeddedOs
{
	namespace NativeConsts
	{
		// these macros may use native type casts, so need some redirection
		constexpr UBaseType_t MAX_TASK_PRIORITY = configMAX_PRIORITIES - 1;
		constexpr uint32_t MIN_TASK_STACK_SIZE = configMINIMAL_STACK_SIZE;
	}

	class RgTask
	{

	private:
		// Константа для уникальной проверки экземпляра
		static constexpr uint32_t UNIQUE_WORD_VALUE = 0xDEBEADEF;

		const char* TaskName;
		const uint16_t TaskStackSize;
		const UBaseType_t  TaskPriority;
		const uint32_t TaskTimeout;

		TaskHandle_t _taskHandle = nullptr;
		bool _isActive = false;

#if INCLUDE_uxTaskGetStackHighWaterMark == 1
		uint8_t _stackUsage = 0;
#endif

		// Статическая функция, необходимая для FreeRTOS
		static void TaskRunner(void* instancePtr) {
			RG_ASSERT_MSG(instancePtr != nullptr, "TaskRunner received a null pointer");

			RgTask* task = static_cast<RgTask*>(instancePtr);
			if(task->UNIQUE_WORD == UNIQUE_WORD_VALUE)
				task->onRun();
			else
				RG_ASSERT_MSG(false, "Invalid object passed to TaskRunner or object corrupted");
		}

	public:
		// Уникальный идентификатор для проверки типа
		const uint32_t UNIQUE_WORD = UNIQUE_WORD_VALUE;
		std::function<void()> threadBody;

		// Возвращает процентное соотношение использованного стека
		uint8_t getStackUsage() const {
			return _stackUsage;
		}

		void onRun() {
			while (true) {
				if (threadBody && _isActive) {
					threadBody();
				}
				else{
					vTaskDelay(TaskTimeout);
				}

#if INCLUDE_uxTaskGetStackHighWaterMark == 1
				// Обновление информации об использовании стека
				_stackUsage = RgBase::Math::percentageFromValue(static_cast<uint16_t>(uxTaskGetStackHighWaterMark(nullptr)), TaskStackSize);
#endif
			}
		}

		// Метод для создания и запуска задачи
		RgTask& initTask()
		{
			BaseType_t result = xTaskCreate(
				TaskRunner,          			// Функция задачи
				TaskName,             			// Имя задачи
				TaskStackSize,            		// Размер стека задачи
				reinterpret_cast<void*>(this), 	// Указатель на объект
				TaskPriority,             		// Приоритет задачи
				&_taskHandle          			// Handle задачи
			);

			RG_ASSERT_MSG(result == pdPASS, "Task creation failed");

			stop();

			return *this;
		}

		// Метод для удаления задачи
		RgTask& deleteTask()
		{
			if (_taskHandle != nullptr)
			{
				stop();						// Приостновка задачи
				vTaskDelete(_taskHandle);  	// Удаление задачи
				_taskHandle = nullptr;      // Обнуление указателя
			}

			return *this;
		}

		// Метод для запуска задачи
		RgTask& start() {
			RG_ASSERT_MSG(_taskHandle != nullptr, "Task handle is null");
			vTaskResume(_taskHandle);  // Возобновление задачи
			_isActive = true;

			return *this;
		}

		// Метод для запуска задачи
		RgTask& stop() {
			RG_ASSERT_MSG(_taskHandle != nullptr, "Task handle is null");
			vTaskSuspend(_taskHandle); // Приостновка задачи
			_isActive = false;

			return *this;
		}

		// Конструктор по умолчанию
		RgTask() = delete;

		// Удаляем конструктор копирования
		RgTask(const RgTask&) = delete;

		// Удаляем оператор присваивания
		RgTask& operator=(const RgTask&) = delete;

		// Конструктор
		RgTask(const char* const taskName, uint16_t stackSize, UBaseType_t priority, uint32_t timeout = 500) :
			TaskName(taskName), TaskStackSize(stackSize), TaskPriority(priority), TaskTimeout(timeout)
		{
			RG_ASSERT_MSG(priority <= NativeConsts::MAX_TASK_PRIORITY, "Invalid task priority value.");
			RG_ASSERT_MSG(stackSize >= NativeConsts::MIN_TASK_STACK_SIZE, "Invalid task stack size.");
		}
	};
}

#endif

