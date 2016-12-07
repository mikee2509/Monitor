#ifndef MAIN_H
#define MAIN_H

void* produce(void *ptr);
void* consume(void *ptr);
void createA(unsigned sleep_time, char &producer_id);
void createB(unsigned sleep_time, char &producer_id);
void printBuffer();



#endif // MAIN_H