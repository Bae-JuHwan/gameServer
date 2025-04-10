#include <thread>
#include <iostream>
#include <vector>

void worker() {
	std::cout << std::this_thread::get_id() << std::endl;
}

int main() {
	std::vector <std::thread> workers;
	for (int i = 0; i < 10; ++i) {
		std::thread t;
		workers.emplace_back(worker);
	}
	for (auto& t : workers) {
		t.join();
	}
}