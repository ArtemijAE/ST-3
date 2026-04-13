// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <stdexcept>

class TimerImpl {
 private:
  TimerClient* client;
  int timeout;
  bool active;

 public:
  TimerImpl() : client(nullptr), timeout(0), active(false) {}

  void tregister(int t, TimerClient* c) {
    timeout = t;
    client = c;
    active = true;
  }

  void tick() {
    if (active && timeout > 0) {
      timeout--;
      if (timeout == 0 && client != nullptr) {
        client->Timeout();
        active = false;
      }
    }
  }
};

static TimerImpl globalTimer;

void Timer::sleep(int timeout) {
  // Не используем реальный sleep в тестах
}

void Timer::tregister(int timeout, TimerClient* client) {
  globalTimer.tregister(timeout, client);
}

void Timer::tick() {
  globalTimer.tick();
}

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& door) : door(door) {}

void DoorTimerAdapter::Timeout() {
  if (door.isDoorOpened()) {
    door.throwState();
  }
}

TimedDoor::TimedDoor(int timeout) : iTimeout(timeout), isOpened(false) {
  adapter = new DoorTimerAdapter(*this);
}

bool TimedDoor::isDoorOpened() {
  return isOpened;
}

void TimedDoor::unlock() {
  isOpened = true;
  Timer timer;
  timer.tregister(iTimeout, adapter);
}

void TimedDoor::lock() {
  isOpened = false;
}

int TimedDoor::getTimeOut() const {
  return iTimeout;
}

void TimedDoor::throwState() {
  throw std::runtime_error("Door was left open too long!");
}

