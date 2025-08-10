#include <BS_thread_pool.hpp>

int main() {
	BS::thread_pool<BS::tp::priority | BS::tp::pause> pool(4);

	for (int i = 0; i < 10; ++i) {
        pool.detach_task([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Task " << i << " done\n";
        }, BS::pr::high);
	}

	pool.pause();
	pool.wait();

	std::cout << "All tasks completed, exiting.\n";
	return 0;
}

