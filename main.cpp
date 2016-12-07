#include <iostream>
#include <pthread.h>
#include <queue>
#include <deque>
#include <unistd.h>
#include "monitor.h"
#include "main.h"
#include "iterable_queue.h"

//Buffer capacity
#define MAX_BUFFER  9

//Number of products created by producers
#define NUM_PRODUCED_A 20
#define NUM_PRODUCED_B 20

//Number of times a consumers attempt to access a product
#define NUM_ATTEMPTS_C 20
#define NUM_ATTEMPTS_D 20

//Consumers' and producer's sleep times in miliseconds
#define TIME_PRODUCER_A  100
#define TIME_PRODUCER_B  200
#define TIME_CONSUMER_C  200
#define TIME_CONSUMER_D  300

using namespace std;

iterable_queue<int> buffer;  //Buffer is a FIFO queue
int areProducersWorking = 0;  //Used only for better printing
int itemNumber = 1;
const char indent[] = "                                   ";

Semaphore print(1); //Used only for better printing

class ProducerConsumer : public Monitor
{
    Condition full, sum_check, size_check;
    int count;
    int sum;

public:
    ProducerConsumer() : count(0), sum(0) 
    {}

    void insertA(int item)
    {
        enter();
        if(sum<=20)
            wait(sum_check);
        if(count==MAX_BUFFER)
            wait(full);
        buffer.push(item);
        sum += item;
        ++count;

        cout << "A PRODUCED ELEMENT:  " << item << endl;
        printBuffer();

        if(count==4)
            signal(size_check);
        leave();
    }

    void insertB(int item)
    {
        enter();
        if(count==MAX_BUFFER)
            wait(full);
        buffer.push(item);
        sum += item;
        ++count;

        cout << "B PRODUCED ELEMENT:  " << item << endl;
        printBuffer();

        if(sum>20)
            signal(sum_check);
        if(count>3)
            signal(size_check);
        leave();
    }

    void remove(char consumer_id)
    {
        int item;
        enter();
        if(count<=3)
            wait(size_check);
        item = buffer.front();
        buffer.pop();
        sum -= item;
        --count;

        cout << indent << consumer_id << " READ & DELETED PRODUCT: " << item << endl;
        if(areProducersWorking == 0)
            printBuffer();

        if(count==MAX_BUFFER-1)
            signal(full);
        leave();
    }
};
   
ProducerConsumer producerConsumer;       



void printBuffer()
{
    cout << "BUFFER: ";
    for(auto it=buffer.begin(); it!=buffer.end(); ++it)
        cout << *it << " ";
    cout << endl;
}


void* produce(void *ptr) 
{
    char producer_id = *((char*) ptr); 
    switch (producer_id) {
        case 'A': 
            createA(TIME_PRODUCER_A, producer_id);
            break;
        case 'B':
            createB(TIME_PRODUCER_B, producer_id);
            break;
        default:
            throw runtime_error("Wrong argument in produce()");
    }
    cout << "\n*********************PRODUCER " << producer_id << " THREAD ENDS**********************\n\n";
}


void createA(unsigned sleep_time, char &producer_id) 
{
    print.p();
    ++areProducersWorking;
    print.v();

    int new_product;

    for (int i = 1; i <= NUM_PRODUCED_A; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        new_product = itemNumber++;
        cout << "   " << producer_id << " wants to create: " << new_product << endl;
        producerConsumer.insertA(new_product);
    }

    print.p();
    --areProducersWorking;
    print.v();
}


void createB(unsigned sleep_time, char &producer_id) 
{
    print.p();
    ++areProducersWorking;
    print.v();

    int new_product;

    for (int i = 1; i <= NUM_PRODUCED_B; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        new_product = itemNumber++;
        cout << "   " << producer_id << " wants to create: " << new_product << endl;
        producerConsumer.insertB(new_product);
    }

    print.p();
    --areProducersWorking;
    print.v();
}


void* consume(void *ptr)
{
    char consumer_id = *((char*) ptr);
    if(consumer_id != 'C' && consumer_id != 'D')
        runtime_error("Wrong argument in consume()");

    unsigned sleep_time = consumer_id == 'C' ? TIME_CONSUMER_C : TIME_CONSUMER_D;
    int numAttempts = consumer_id == 'C' ? NUM_ATTEMPTS_C : NUM_ATTEMPTS_D;
    for(int i = 1; i <= numAttempts; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        cout << indent << "   " << consumer_id << " wants to read" << endl;
        producerConsumer.remove(consumer_id);
    }

    cout << "\n*********************COMSUMER " << consumer_id << " THREAD ENDS**********************\n\n";
}

int main() 
{
    pthread_t producerA, producerB, consumerC, consumerD;

    char *symbolA = new char('A'),
         *symbolB = new char('B'), 
         *symbolC = new char('C'), 
         *symbolD = new char('D');

    cout << "! ======================START OF SIMULATION===================== !" << endl;

    pthread_create(&producerA, NULL, produce, (void*) symbolA);
    pthread_create(&producerB, NULL, produce, (void*) symbolB);
    pthread_create(&consumerC, NULL, consume, (void*) symbolC);
    pthread_create(&consumerD, NULL, consume, (void*) symbolD);

    //pthread_join(producerA, NULL);
    pthread_join(producerB, NULL);
    pthread_join(consumerC, NULL);
    pthread_join(consumerD, NULL);

    cout << "! ======================END OF SIMULATION====================== !" << endl;
    cout << "Elements left in buffer: " << buffer.size() << endl;

    delete symbolA;
    delete symbolB;
    delete symbolC;
    delete symbolD;

    return 0;
}
