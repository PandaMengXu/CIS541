#include "mbed.h"
#include "rtos.h"

/**
 * Dining Philosopher Problem
 * Use a "waiter" to handle all requests of chopsticks. 
 * A philosopher can get its chopsticks only when
 * he can get both chopsticks at one shot.
 */
Serial pc(USBTX, USBRX); // tx, rx
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

const int num_philosophers = 5;
Mutex mutex;
int chopsticks[num_philosophers];

/**
 * get random value in [0,1] * 5000ms 
 * used as the time for thinking/eating
 */
double get_rand(){
    return (rand() / RAND_MAX) * 5000;
}

/**
 * return succeed = 1 only when both chopsticks are free.
 */
int request_chopsticks(int p_id){
    int success = 0;
    int left_chp = p_id % num_philosophers;
    int right_chp = ((p_id + num_philosophers) - 1) % num_philosophers;
    mutex.lock(); 
    if ( chopsticks[left_chp] == 1 && chopsticks[right_chp] == 1) {
        //have both chopsticks
        chopsticks[left_chp] = 0;
        chopsticks[right_chp] = 0;
        success = 1;
        pc.printf("philosopher %d request chopsticks succeed\r\n", p_id);
    } else {
        pc.printf("philosopher %d request chopsticks fails\r\n", p_id);
    }
    mutex.unlock();
    return success;    
}

/**
 * release chopsticks always succeed 
 */
void release_chopsticks(int p_id){
    int left_chp = p_id % num_philosophers;
    int right_chp = ((p_id + num_philosophers) - 1) % num_philosophers;
    mutex.lock();
    chopsticks[left_chp] = 1;
    chopsticks[right_chp] = 1;
    mutex.unlock();
}

void think(int p_id){
     pc.printf("Philosopher %d is thinking\r\n", p_id);
     Thread::wait(get_rand());
}

void pick_up_chopsticks(int p_id){
    pc.printf("Philosopher %d requests chopsticks...\r\n", p_id);
    while( request_chopsticks(p_id) != 1);
    pc.printf("Philosopher %d requests chopsticks succeed\r\n", p_id);
}

void eat(int p_id){
    pc.printf("Philosopher %d is eating\r\n", p_id);
    Thread::wait(get_rand());
}

void put_down_chopsticks(int p_id){
    release_chopsticks(p_id);
    pc.printf("Philosopher %d put down both chopsticks\r\n", p_id);
}

void philosopher(void const * p_id_p) {
    int p_id = *((int *)p_id_p);
    while(true){
        //think
        think(p_id);
        //pick up chopsticks
        pick_up_chopsticks(p_id);
        //eat
        eat(p_id);
        //put down chopsticks
        put_down_chopsticks(p_id);
    }
}

void int_philosophers(int* p_id, int num_p_id){
    int i = 0;
    for(i=0; i<num_p_id; i++){
        p_id[i] = i;
    }
    
    for(i=0; i<num_philosophers; i++){
        chopsticks[i] = 1;
    }
}

int main() {
    int p_id[num_philosophers];
    Thread* p[num_philosophers];
    int i = 0;
    
    srand(time(NULL));
    
    int_philosophers(p_id, num_philosophers);
    
    for(i=0; i<num_philosophers; i++){
        p[i] = new Thread(philosopher, p_id + i);
    }
    
    while(1);
}
