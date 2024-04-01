#ifndef task_manager_h
#define task_manager_h

#include <Arduino.h>
#include "priority_queue.h"
#include <queue>
//#include <deque>


#include "static_queue.h" //https://github.com/BenjiHansell/static_queue/blob/master/static_queue.h

#include <CircularBuffer.hpp> // https://github.com/rlogiacco/CircularBuffer

// компоратор для приоритетной очереди. Осуществляет сортировку в соответсвии с первым элементом (временем).
// Учитывает переполнение счетчика времени uint32_t, переполненные элементы заносятся в конец. 
auto cmp = [](std::pair<uint32_t, std::function<void()> *> left, std::pair<uint32_t, std::function<void()> *> right) {
//bool cmp(const std::pair<uint32_t, std::function<void()> *> &left, const std::pair<uint32_t, std::function<void()> *> &right) {

  // Возможно 4 случая (1 - переполнен, 0 - нет):
  // (0, 0), (0, 1), (1, 0), (1 , 1)
  const uint32_t now_time = millis();
  bool overflow_1 = false, overflow_2 = false;

  if (now_time - left.first  < left.first  - now_time) overflow_1 = true;
  if (now_time - right.first < right.first - now_time) overflow_2 = true;
  
  if (overflow_1 ^ overflow_2) {
    // случай (1, 0), (0, 1)
    return overflow_1 < overflow_2;
  } else {
    // случай (0, 0), (1, 1)
    return left.first < right.first;
  }

};


typedef std::pair<uint32_t, std::function<void()> *> my_pair;
/*############################################################################################################*/
/*############################################################################################################*/
/*############################################################################################################*/
class task_manager {
  HardwareTimer timer;
  // костыльная реализация написанная "с мира по нитке" выскакивает с ошибкой линковщика undefined reference to `task_manager::tasks'
  static StaticPriorityQueue <my_pair, 100, decltype(cmp)> tasks; //{cmp};

  // некомпилируется даже с отсутствием static из-за
  //error: no type named 'value_type' in 'class static_queue<std::pair<long unsigned int, std::function<void()>*>, 10>'
  //error: no type named 'reference' in 'class static_queue<std::pair<long unsigned int, std::function<void()>*>, 10>'
  //error: no type named 'size_type' in 'class static_queue<std::pair<long unsigned int, std::function<void()>*>, 10>'
  // и т.д.
  std::priority_queue <my_pair, static_queue <my_pair, 10>, decltype(cmp)> tasks{cmp};

  // один в один аналогичные ошибки.
  std::priority_queue <std::pair<uint32_t, std::function<void()> *>,  CircularBuffer <std::pair<uint32_t, std::function<void()> *>, 100>, decltype(cmp)> tasks{cmp};
public:
  /*############################################################################################################*/
  // внутри конструктора создаем таймер на 1000 Гц и в качестве вызываемой функции используем static void test_function()
  task_manager(TIM_TypeDef* timer_instance = TIM9, uint32_t timer_frequency=1000) {
    timer = HardwareTimer(timer_instance);
    timer.attachInterrupt(this->test_function);
  }

  static void test_function(){
    // внутри функции модифицируем очередь (исполняем и удаляем "просроченные" элементы)
    // код абстрактный, для демонстрации неработоспособности
    tasks.pop();
    //tasks.top().second();
  }
};

// https://habr.com/ru/articles/527044/
// на habr писалость что необходимо статические члены классов инициализировать так, но выскакивает ошибка повторной инициализации
//StaticPriorityQueue  <std::pair<uint32_t, std::function<void()> *>, 100, decltype(cmp)> task_manager::tasks{cmp};


#endif
