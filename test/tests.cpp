// Copyright 2021 GHA Test Team
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
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

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(5);
  }

  void TearDown() override {
    delete door;
  }

  TimedDoor* door;
};

TEST_F(TimedDoorTest, ConstructorSetsTimeout) {
  EXPECT_EQ(door->getTimeOut(), 5);
}

TEST_F(TimedDoorTest, InitiallyDoorIsClosed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
  TimedDoor door2(10);
  EXPECT_EQ(door2.getTimeOut(), 10);
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

TEST(DoorTimerAdapterTest, AdapterCallsThrowStateWhenDoorOpened) {
  TimedDoor door(5);
  DoorTimerAdapter adapter(door);

  door.unlock();
  EXPECT_TRUE(door.isDoorOpened());

  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(DoorTimerAdapterTest, AdapterDoesNotThrowWhenDoorClosed) {
  TimedDoor door(5);
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

