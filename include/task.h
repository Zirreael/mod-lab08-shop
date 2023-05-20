#pragma once
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <iostream>

typedef std::chrono::system_clock Clock;
typedef std::lock_guard<std::mutex> Guard;
using fsec = std::chrono::duration<float>;

struct Statistic {
public:
  fsec workTime;
  int acceptedClients;
  int declinedClients;
  std::vector<fsec> cashWorkTime;
  fsec clientVisitTime;
  int queueLength;
  int queueTacts;

public:
  Statistic();
};

class Client {
public:
  int items;
  Clock::time_point visitTime;

public:
  explicit Client(int itemsCount);
};

class Shop {
private:
  class CashThread {
  private:
    std::thread thrd;
    bool working;
    int itemsCount;
    std::mutex locMutex;
    std::mutex& shopMutex;
    Shop& shop;

  public:
    const int id;
    const float intens;

  private:
    void Work();
    void Release();

  public:
    explicit CashThread(int ID, float intense, Shop& shop_, std::mutex& shopMutex_);
    CashThread(const CashThread& other);
    CashThread();
    void Service(int items);
    void Off();

    ~CashThread();
  };
  CashThread cash;
  bool working;
  std::thread pool;
  std::queue<Client> clientsQueue;
  std::vector<CashThread> cashThreads;

public:
  float intens;
  int cashCount;
  int maxLength;
  std::mutex mtx;
  std::queue<CashThread*> cashPool;
  Statistic stats;
  Clock::time_point startTime;

private:
  void CashControl();

public:
  Shop(int count, float intensity, int maxLen);
  Shop(const Shop& other);
  void Service(Client client);
  Statistic getStats();
  CashThread& getCash(int ID);
  bool IsWork();
  void Off();

  ~Shop();
};

void SpawnClients(Shop& shop, int count, float intens, int items);
