
#include "mbed.h"
#include "rtos.h"
 
DigitalOut led1(LED1);
DigitalOut led2(LED2);
osThreadId mainThreadID;
Thread *thread2;

Serial pc(USBTX, USBRX); // tx, rx
 
void signal_thread(void const *pThread2ID) {
    while (true) {
        Thread::wait(1000);
        osSignalSet(mainThreadID, 0x2);
        thread2->signal_set(0x1);
        pc.printf("signal thread set signal to threadid %d\r\n", *(osThreadId*)pThread2ID);
    }
}

void action_thread(void const *argument){
  while (true) {
        pc.printf("atction thread start new loop\r\n");
        thread2->signal_wait(0x1);
        pc.printf("atction thread get signal\r\n");
        led1 = !led1;
    }
 }
 
int main (void) {
    mainThreadID = osThreadGetId();
    
    osThreadId thread2ID;
    
    thread2 = new Thread(action_thread);
    Thread thread1(signal_thread);
    
    while (true) {
        pc.printf("main thread start new loop\r\n");
        osSignalWait(0x2, osWaitForever);
        pc.printf("main thread get signal\r\n");
        led2 = !led2;
    }

   
}
