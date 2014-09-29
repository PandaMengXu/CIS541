#include "mbed.h"
#include "rtos.h"

/**
 * Two Reader Two Writer Problem
 * Reader is prefered.
 *
 * Expected behavior
 * LED1 and LED2 indicate reader1 and reader2 are reading.
 * LED3 and LED4 indicates writer1 and write2 are writing.
 * When LED3 or LED4 is on, on other LEDs should be on
 * LED1 and LED2 can be on at the same time.
 */
Serial pc(USBTX, USBRX); // tx, rx
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

Mutex mutex;
Semaphore wrt(1);
int read_count = 0;

long shared_data = 0;

const int wait_max = 20000; //20000ms
const int wait_min = 1000; //1000ms

typedef struct thread_type {
    uint32_t type; //0 is read, 1 is write
    uint32_t id; //id of the type of threads
} thread_type_t;


/*
 * get random value in [0,1] 
 */
double get_rand(){
    return rand() / RAND_MAX;
}

/**
 * busy wait to hold LED on
 */
void busy_wait(){
    int i = 0;
    double sum = 0;
    for(i=0; i<10000000; i++){
        if ( i % 2 == 0)
            sum += i+i;
        else 
            sum -= i+i;
    }
}

/**
 * when reader is reading data, LED_i is on
*/
long read_data(uint32_t type, uint32_t id) {
    long data = 0;
    //turn on LED
    if (id == 1){
        led1 = 1;
    } else if (id == 2){
        led2 = 1;
    }
    //read shared data
    data = shared_data;
    pc.printf("Reader %d read %ld\r\n", id, data);
    busy_wait();
    //turn off LED
    if (id == 1){
        led1 = 0;
    } else if (id == 2){
        led2 = 0;
    }
    
    return data;
}

void write_data(uint32_t type, uint32_t id){
    //turn on LED
    if (id == 1){
        led3 = 1;
    } else if (id == 2){
        led4 = 1;
    }
    //write shared data
    shared_data = type * 10 + id;  
    pc.printf("Writer %d write %ld\r\n", id, shared_data);
    busy_wait();
    //turn off LED
    if (id == 1){
        led3 = 0;
    } else if (id == 2){
        led4 = 0;
    }
}  

void read_thread(void const* thread_param){
    thread_type_t * param = (thread_type_t*)thread_param;
    pc.printf("reader %d is called\r\n",param->id );
    
    while(true) {
        mutex.lock();
        read_count = read_count + 1;
        if (read_count == 1) {
            wrt.wait();
        }
        mutex.unlock();
        //read the data
        read_data(param->type, param->id);
        
        mutex.lock();
        read_count = read_count - 1;
        if (read_count == 0) {
            wrt.release();
        } 
        mutex.unlock();
        
        Thread::wait(wait_min + get_rand() * wait_max);
    }
}

void write_thread(void const* thread_param){
    thread_type_t * param = (thread_type_t*)thread_param;
    pc.printf("reader %d is called\r\n", param->id);
    
    while(true){
        wrt.wait();
        //write data
        write_data(param->type, param->id);
        
        wrt.release();
        Thread::wait(wait_min + get_rand() * wait_max);
    }
}

int main() {
    srand(time(NULL));
    
    thread_type_t reader1_param = {0, 1};
    thread_type_t reader2_param = {0, 2};
    thread_type_t writer1_param = {1, 1};
    thread_type_t writer2_param = {1, 2};
    Thread t1(read_thread, (void *)&reader1_param);
    Thread t2(read_thread, (void *)&reader2_param);
    Thread t3(write_thread, (void *)&writer1_param);
    Thread t4(write_thread, (void *)&writer2_param);

    while(1);
}
