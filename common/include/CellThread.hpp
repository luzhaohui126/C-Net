#pragma once
//2019.3.3
#include"CellSemaphore.hpp"

class CellThread
{
public:
	static void Sleep(time_t dt) {
		std::chrono::milliseconds t(dt);
		std::this_thread::sleep_for(t);
	}
private:
	typedef std::function<void(CellThread*)> EventCall;
public:
	CellThread() {
		_isRun = false;
	}
	~CellThread() {

	}
	//启动线程
	void Start(
		EventCall onCreate=nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun) {
			//初始函数
			_isRun = true;
			if (onCreate)
				_OnCreate = onCreate;
			if (onRun)
				_OnRun = onRun;
			if (onDestory)
				_OnDestory = onDestory;
			//线程
			std::thread t(std::mem_fn(&CellThread::OnWork), this);
			t.detach();
		}
	}
	//关闭
	void Close() {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun) {
			_isRun = false;
			_sem.wait();
		}
	}
	//工作中退出-不需要信号
	void Exit() {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun) {
			_isRun = false;
		}
	}
	bool isRun() {
		return _isRun;
	}
private:
	//事件回调函数 
	EventCall _OnCreate;
	EventCall _OnRun;
	EventCall _OnDestory;
	//工作
	void OnWork() {
		if (_OnCreate)
			_OnCreate(this);
		if (_OnRun)
			_OnRun(this);
		if (_OnDestory)
			_OnDestory(this);
		_sem.wakeUp();
	}
private:
	//是否运行中
	bool _isRun = false;
	//不同线程间加锁
	std::mutex _mutex;
	//信号控制
	CellSemaphore _sem;
};
