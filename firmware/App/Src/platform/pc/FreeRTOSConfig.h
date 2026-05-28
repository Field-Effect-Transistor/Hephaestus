#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ( 1000000 ) 
#define configTICK_RATE_HZ                      ( 1000 )    
#define configMAX_PRIORITIES                    ( 7 )

#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 16384 ) 
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1

#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 2 * 1024 * 1024 ) ) /* 2 Мегабайти */
#define configSUPPORT_DYNAMIC_ALLOCATION        1

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE )

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1

/* Заглушки для POSIX */
#define xPortIsInsideInterrupt()                ( 0 )
#define traceISR_EXIT_TO_SCHEDULER()            
#define traceISR_EXIT()                         

#endif /* FREERTOS_CONFIG_H */
