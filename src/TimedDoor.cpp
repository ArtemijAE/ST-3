// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <stdexcept>
#include <thread>
#include <chrono>

void Timer::sleep(int timeout) {
  std::this_thread::sleep_for(std::chrono::seconds(timeout));
}

void Timer::tregister(int timeout, TimerClient* client) {
  this->client = client;
  std::thread([this, timeout]() {
    sleep(timeout);
    if (this->client != nullptr) {
      this->client->Timeout();
    }
  }).detach();
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

