#pragma once 
//2019.3.3
#include<chrono>
#include<thread>
#include<condition_variable>

class CellSemaphore
{
public:
	CellSemaphore() {
		_wait = 0;
		_wakeup = 0;
	}
	~CellSemaphore() {

	}
	void wait() {
		//等待
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0) {
			_cv.wait(lock, [this]() ->bool{
				return _wakeup > 0;
			});
			--_wakeup;
		}
		//while (_isWaitExit)
		//{//信号量
		//	std::chrono::milliseconds t(1);		//定时
		//	std::this_thread::sleep_for(t);		//等待
		//}
	}
	void wakeUp() {
		//唤醒
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait <= 0) {
			++_wakeup;
			_cv.notify_one();
		}
	}
private:
	//等待计数
	int _wait;
	//唤醒计数
	int _wakeup;
	std::condition_variable _cv;
	std::mutex _mutex;
};

