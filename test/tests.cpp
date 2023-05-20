// Copyright 2021 GHA Test Team
#include <gtest/gtest.h>
#include "task.h"
TEST(TestCaseName, Test1) {
  float shopIntens = 10;
	float clientIntens = 30;
	int cash = 5;
	int items = 7;
	int len = 10;
  Shop shop(cash, shopIntens, len);
	SpawnClients(shop, 100, clientIntens, items);
	shop.Off();
  auto stats = shop.getStats();
  int res = stats.acceptedClients + stats.declinedClients;
  EXPECT_EQ(res, 100);
}

TEST(TestCaseName, Test2) {
  float shopIntens = 10;
	float clientIntens = 30;
	int cash = 5;
	int items = 7;
	int len = 10;
  Shop shop(cash, shopIntens, len);
	SpawnClients(shop, 100, clientIntens, items);
	shop.Off();
  auto stats = shop.getStats();
  float workTime = stats.workTime.count();
  int id = 1;
	float sumTime = 0;
	for (auto& time : stats.cashWorkTime) {
		sumTime += time.count();
		id++;
	}
	float avgTime = sumTime / cash;
  float res = workTime - avgTime;
  EXPECT_TRUE(res < avgTime);
}

TEST(TestCaseName, Test3) {
  float shopIntens = 10;
	float clientIntens = 30;
	int cash = 5;
  int cash2 = 10;
	int items = 7;
	int len = 10;
  Shop shop(cash, shopIntens, len);
  Shop shop2(cash2, shopIntens, len);
	SpawnClients(shop, 100, clientIntens, items);
	shop.Off();
  SpawnClients(shop2, 100, clientIntens, items);
	shop2.Off();
  auto stats = shop.getStats();
  auto stats2 = shop2.getStats();
  int res = stats.acceptedClients;
  int res2 = stats2.acceptedClients;
  EXPECT_TRUE(res < res2);
}

TEST(TestCaseName, Test4) {
  float shopIntens = 10;
  float shopIntens2 = 5;
	float clientIntens = 30;
	int cash = 5;
	int items = 7;
	int len = 10;
  Shop shop(cash, shopIntens, len);
  Shop shop2(cash, shopIntens2, len);
	SpawnClients(shop, 100, clientIntens, items);
	shop.Off();
  SpawnClients(shop2, 100, clientIntens, items);
	shop2.Off();
  auto stats = shop.getStats();
  auto stats2 = shop2.getStats();
  int res = stats.acceptedClients;
  int res2 = stats2.acceptedClients;
  EXPECT_TRUE(res2 < res);
}
