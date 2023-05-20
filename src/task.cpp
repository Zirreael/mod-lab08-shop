#include "task.h"

Statistic::Statistic():
    workTime{ fsec(0) },
    acceptedClients{ 0 },
    declinedClients{ 0 },
    cashWorkTime{ std::vector<fsec>() },
    clientVisitTime{ fsec(0) },
    queueLength{ 0 },
    queueTacts{ 0 }
{
}

void Shop::CashControl() {
    while (working) {
        Guard lock(mtx);
        stats.queueTacts++;
        stats.queueLength += clientsQueue.size();
        if (clientsQueue.empty() || cashPool.empty()) {
            std::this_thread::yield();
            continue;
        }
        auto cash = cashPool.front();
        cashPool.pop();
        auto client = clientsQueue.front();
        clientsQueue.pop();
        auto end = Clock::now();
        fsec time = end - client.visitTime;
        stats.clientVisitTime += time;
        cash->Service(client.items);
    }
}

Shop::Shop(int count, float intensity, int maxLen):
    maxLength{ maxLen },
    cashCount{ count },
    intens{ intensity },
    working{ true },
    cashPool{ std::queue<CashThread*>() },
    stats{ Statistic() },
    clientsQueue{ std::queue<Client>() },
    cashThreads{ std::vector<CashThread>() },
    startTime{ Clock::now() }
{
    for (int i = 0; i < count; i++) {
        CashThread cashT(i, intensity, *this, mtx);
        cashThreads.push_back(cashT);
        Guard lock(mtx);
        stats.cashWorkTime.push_back(fsec(0));
    }
    for (int i = 0; i < count; i++) {
        cashPool.push(&cashThreads[i]);
    }
    pool = std::thread(&Shop::CashControl, this);
}

Shop::Shop(const Shop& other) :
    Shop(other.cashCount, other.intens, other.maxLength)
{
}

void Shop::Service(Client client) {
    Guard lock(mtx);
    if (clientsQueue.size() > maxLength) {
        stats.declinedClients++;
    } else {
        stats.acceptedClients++;
        clientsQueue.push(client);
    }
}

Statistic Shop::getStats() {
    Guard lock(mtx);
    return stats;
}

Shop::CashThread& Shop::getCash(int ID) {
    return cashThreads[ID];
}

bool Shop::IsWork() {
    return working;
}

void Shop::Off() {
    if (!working) return;
    working = false;
    pool.join();
    for (auto& cashT : cashThreads) {
        cashT.Off();
    }
    auto end = Clock::now();
    stats.workTime = end - startTime;
}

Shop::~Shop() {
    Off();
}

Shop::CashThread::CashThread(int ID, float intense, Shop& shop_, std::mutex& shopMutex_) :
    id{ ID },
    intens{ intense },
    shop{ shop_ },
    shopMutex{ shopMutex_ },
    working{ true },
    itemsCount{ 0 },
    thrd{ std::thread(&CashThread::Work, this) }
{
}

Shop::CashThread::CashThread(const CashThread& other) :
    CashThread(other.id, other.intens, other.shop, other.shopMutex)
{
    Guard lock(locMutex);
    itemsCount = other.itemsCount;
}

Shop shop_ = Shop(0, 0, 0);
std::mutex sm;
Shop::CashThread::CashThread() :
    id{ 0 },
    intens{ 0 },
    shop{ shop_ },
    shopMutex{ sm },
    working{ true },
    itemsCount{ 0 },
    thrd{ std::thread(&CashThread::Work, this) }
{
}

void Shop::CashThread::Work() {
    auto start = Clock::now();
    while (working) {
        auto count = 0;
        {
            Guard lock(locMutex);
            count = itemsCount;
        }
        if (count <= 0) {
            start = Clock::now();
            std::this_thread::yield();
            continue;
        }
        auto time = fsec(1 / intens);
        std::this_thread::sleep_for(time);
        Guard lock(locMutex);
        itemsCount--;
        if (itemsCount == 0) {
            auto end = Clock::now();
            fsec time = end - start;
            {
                Guard lock(shopMutex);
                shop.stats.cashWorkTime[id] += time;
                shop.stats.clientVisitTime += time;
            }
            Release();
        }
    }
}

void Shop::CashThread::Release() {
    Guard lock(shop.mtx);
    shop.cashPool.push(this);
}

void Shop::CashThread::Service(int items) {
    Guard lock(locMutex);
    itemsCount += items;
}

void Shop::CashThread::Off() {
    if (!working) return;
    working = false;
    thrd.join();
}

Shop::CashThread::~CashThread() {
    Off();
}

Client::Client(int itemsCount) :
    items { itemsCount },
    visitTime{ Clock::now() }
{
}

void SpawnClients(Shop& shop, int count, float intens, int items)
{
    int temp = 0;
    for (int i = 0; i < count; i++) {
        int count = std::rand() % (items - 1 + 6) + 1;
        temp += count;
        Client client(count);
        shop.Service(client);
        std::this_thread::sleep_for(fsec(1 / intens));
    }
    std::cout << "\nСреднее количество товаров: " << temp / count;
}
