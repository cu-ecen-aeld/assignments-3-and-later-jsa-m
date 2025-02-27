#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter


    struct thread_data* data = (struct thread_data *) thread_param;

    //This line does the following: Declares a pointer data of type struct thread_data*, Casts the thread_param pointer to struct thread_data*
    //Assigns the result to data

    usleep(data->wait_to_obtain_ms * 1000); // Wait before attempting to obtain the mutex

    // Try to lock the mutex
    if (pthread_mutex_lock(data->mutex) != 0) {
        data->thread_complete_success = false;
        return thread_param;
    }

    // Hold the mutex for the specified time
    usleep(data->wait_to_release_ms * 1000);

     // Unlock the mutex
    pthread_mutex_unlock(data->mutex);
    data->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    bool success = false;

    // Allocate memory for thread_data
    struct thread_data *data = (struct thread_data*) malloc(sizeof(struct thread_data));

    if (!data) {
        return success;
    }

     // Initialize thread_data
    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = false;

    int result = pthread_create(thread, NULL, threadfunc, (void*)data);

    /* Explanation:
    pthread_t *thread
        A pointer to a pthread_t variable, where the function will store the new thread's ID.
        In our case, thread is passed, so the newly created thread ID is stored in *thread.

    const pthread_attr_t *attr
        Specifies attributes for the new thread (such as stack size or scheduling policy).
        NULL means the default attributes are used.

    void *(*start_routine)(void *)
        A function pointer to the function that will be executed by the new thread.
        In this case, threadfunc is passed, meaning the new thread will start executing threadfunc(void *arg).

    void *arg
        A pointer to data that will be passed to the thread function.
        In our case, (void*)data is passed, which means data (a pointer to struct thread_data) will be available in threadfunc.
    */

    if (result != 0) {
        ERROR_LOG("pthread_create failed with error %d", result);
        free(data); // Free allocated memory on failure
        return success;
    }

    success = true;

    // Store thread ID
    data->thread_id = *thread;


    return success;
}

