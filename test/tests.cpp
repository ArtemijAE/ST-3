// Copyright 2021 GHA Test Team
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Mock;

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class MockTimer {
 public:
  MockTimerClient* client;
  int registeredTimeout;

  void tregister(int timeout, TimerClient* client) {
    this->registeredTimeout = timeout;
    this->client = static_cast<MockTimerClient*>(client);
  }

  void triggerTimeout() {
    if (client != nullptr) {
      client->Timeout();
    }
  }
};

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(1);
  }

  void TearDown() override {
    delete door;
  }

  TimedDoor* door;
};

TEST_F(TimedDoorTest, ConstructorSetsTimeout) {
  EXPECT_EQ(door->getTimeOut(), 1);
}

TEST_F(TimedDoorTest, InitiallyDoorIsClosed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
}

TEST_F(TimedDoorTest, LockClosesDoor) {
  door->unlock();
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
  TimedDoor door2(10);
  EXPECT_EQ(door2.getTimeOut(), 10);
}

TEST_F(TimedDoorTest, NoExceptionWhenDoorClosedBeforeTimeout) {
  door->unlock();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  door->lock();
  std::this_thread::sleep_for(std::chrono::milliseconds(600));
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, ExceptionThrownWhenDoorRemainsOpen) {
  door->unlock();
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, MultipleUnlockLockSequences) {
  for (int i = 0; i < 3; i++) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
  }
}

TEST_F(TimedDoorTest, ThrowStateThrowsRuntimeError) {
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, DifferentTimeouts) {
  TimedDoor door1(3);
  TimedDoor door2(7);
  TimedDoor door3(15);

  EXPECT_EQ(door1.getTimeOut(), 3);
  EXPECT_EQ(door2.getTimeOut(), 7);
  EXPECT_EQ(door3.getTimeOut(), 15);
}

TEST(TimerTest, TregisterStoresClient) {
  MockTimerClient client;
  MockTimer timer;

  timer.tregister(5, &client);
  EXPECT_EQ(timer.registeredTimeout, 5);
  EXPECT_EQ(timer.client, &client);
}

TEST(TimerTest, TimeoutCallsClientTimeout) {
  MockTimerClient client;
  MockTimer timer;

  EXPECT_CALL(client, Timeout()).Times(1);

  timer.tregister(0, &client);
  timer.triggerTimeout();
}

TEST(DoorTimerAdapterTest, AdapterCallsThrowStateWhenDoorOpened) {
  TimedDoor door(1);
  DoorTimerAdapter adapter(door);

  door.unlock();
  EXPECT_TRUE(door.isDoorOpened());

  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(DoorTimerAdapterTest, AdapterDoesNotThrowWhenDoorClosed) {
  TimedDoor door(1);
  DoorTimerAdapter adapter(door);

  door.lock();
  EXPECT_FALSE(door.isDoorOpened());

  EXPECT_NO_THROW(adapter.Timeout());
}

TEST(MockDoorTest, MockDoorCanBeUsed) {
  MockDoor mockDoor;

  EXPECT_CALL(mockDoor, lock()).Times(1);
  EXPECT_CALL(mockDoor, unlock()).Times(1);
  EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(testing::Return(false));

  mockDoor.lock();
  mockDoor.unlock();
  EXPECT_FALSE(mockDoor.isDoorOpened());
}

