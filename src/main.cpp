#include <iostream>
#include <random>
#include <cmath>
#include "task.h"

struct Result {
	float l;
	float mu;
	float r;
	float P0;
	float Pq;
	float Prej;
	float Q;
	float A;
	float Lq;
	float Lsrv;
	float t;
	float tq;
};

void PrintResult(Result res);
Result Calculate(float shop_intens, float clients_intens, int n, int m);
Result CalcActual(Statistic stats, int len);

int main() {
	setlocale(LC_ALL, "Russian");
	float shopIntens = 10;
	float clientIntens = 30;
	int cash = 5;
	int items = 7;
	int len = 10;
	Shop shop(cash, shopIntens, len);
	std::cout << "Ìàãàçèí ñîçäàí.";
	SpawnClients(shop, 100, clientIntens, items);
	std::cout << "\nÏîêóïàòåëè ñîçäàíû.\n";
	shop.Off();
	auto stats = shop.getStats();
	float workTime = stats.workTime.count();
	float visitTime = stats.clientVisitTime.count() / stats.acceptedClients;
	std::cout << "\nÎáñëóæåíî êëèåíòîâ: " << stats.acceptedClients;
	std::cout << "\nÎòêëîíåíî: " << stats.declinedClients;
	std::cout << "\nÎáùåå âðåìÿ ðàáîòû: " << workTime << "\n";
	int id = 1;
	float sumTime = 0;
	for (auto& time : stats.cashWorkTime) {
		std::cout << "\nÊàññà ¹" << id << " ðàáîòàëà " << time.count();
		sumTime += time.count();
		id++;
	}
	float avgTime = sumTime / cash;
	std::cout << "\n\nÑðåäíåå âðåìÿ ðàáîòû: " << avgTime;
	std::cout << "\nÑðåäíåå âðåìÿ ïðîñòîÿ: " << workTime - avgTime;
	Result theory = Calculate(shopIntens, clientIntens * items, cash, len);
	Result real = CalcActual(stats, len);
	std::cout << "\n\nÒåîðèòè÷åñêèå ðåçóëüòàòû:\n";
	PrintResult(theory);
	std::cout << "\n\nÐåàëüíûå ðåçóëüòàòû:\n";
	PrintResult(real);
}

void PrintResult(Result res) {
	std::cout << "\nÂåðîÿòíîñòü îòêàçà: " << res.Prej;
	std::cout << "\nÎòíîñèòåëüíàÿ ïðîïóñêíàÿ ñïîñîáíîñòü: " << res.Q;
	std::cout << "\nÀáñîëþòíàÿ ïðîïóñêíàÿ ñïîñîáíîñòü: " << res.A;
	std::cout << "\nÑðåäíÿÿ äëèíà î÷åðåäè: " << res.Lq;
	std::cout << "\nÑðåäíåå âðåìÿ îáñëóæèâàíèÿ: " << res.t;
}

long Fact(int n)
{
	long f = 1;
	if ((n == 0) || (n == 1))
		f = 1;
	else
		for (int i = 1; i <= n; i++)
			f *= i;
	return f;
}

Result Calculate(float shop_intens, float clients_intens, int n, int m) {
	Result res = Result();
	long f = Fact(n);
	res.l = clients_intens;
	res.mu = shop_intens;
	res.r = res.l / res.mu;
	res.P0 = 1;
	for (int i = 1; i <= n; i++) {
		res.P0 += powf(res.r, i) / Fact(i);
	}
	for (int i = n + 1; i <= n + m; i++) {
		res.P0 += powf(res.r, i) / (powf(n, i - n) * f);
	}
	res.P0 = 1 / res.P0;
	res.Pq = powf(res.r, n) / f * (1 - powf((res.r / n), m)) / 1 - (res.r / n);
	res.Prej = res.P0 * (powf(res.r, n + m) / (powf(n, m) * f));
	res.Q = 1 - res.Prej;
	res.A = res.l * res.Q;
	float rn = res.r / n;
	res.Lq = powf(res.r, n + 1) / (n * f) * res.P0;
	res.Lq *= (1 - powf(rn, m) * (1 + m * (1 - rn))) / ((1 - rn) * (1 - rn));
	res.Lsrv = res.A / res.mu;
	res.t = res.Lq / res.l + res.Q / res.mu;
	res.tq = res.Lq / res.l;
	return res;
}

Result CalcActual(Statistic stats, int len) {
	Result res = Result();
	int accepted = stats.acceptedClients;
	int declined = stats.declinedClients;
	int all = accepted + declined;
	int cashCount = stats.cashWorkTime.size();
	float workTime = stats.workTime.count();
	res.t = stats.clientVisitTime.count() / accepted;
	res.Lq = (float)stats.queueLength / stats.queueTacts;
	res.Prej = (float)declined / all;
	res.Q = 1 - res.Prej;
	res.A = accepted / workTime;
	res.l = all / workTime;
	res.tq = res.Lq / res.l;
	res.mu = res.Q / (res.t - res.tq);
	res.Lsrv = res.A / res.mu;
	float totalTime = 0;
	for (auto& time : stats.cashWorkTime) {
		totalTime += time.count();
	}
	float avgWork = totalTime / cashCount;
	float avgFree = workTime - avgWork;
	float t = avgFree / workTime;
	res.P0 = powf(t, cashCount);
	res.Pq = res.Lq / len;
	res.r = res.l / res.mu;
	return res;
}
