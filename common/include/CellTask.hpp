#pragma once

//任务类型-基类
#include<thread>
#include<mutex>
#include<list>
#include<functional>
#include"CellThread.hpp"

class CellTaskServer
{
public:
	int _serverId = -1;
private:
	typedef std::function<void()> CellTask;
	//任务数据
	std::list<CellTask> _tasks;
	//任务缓冲
	std::list<CellTask> _tasksBuff;
	std::mutex _mutex;
	//线程
	CellThread _thread;
public:
	//添加任务
	void addTask(CellTask task) {
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuff.push_back(task);
	}
	//启动服务
	void Start() {
		//线程
		_thread.Start(nullptr, [this](CellThread* pThread) {
			OnRun(pThread);
		});
	}
	//关闭服务
	void Close() {
		//CellLog::Info("CellTaskServer<%d> Close begin\n",_serverId);
		_thread.Close();
		//CellLog::Info("CellTaskServer<%d> Close end\n", _serverId);
	}
protected:
	//工作函数
	void OnRun(CellThread *pThread) {
		while (pThread->isRun())
		{
			if (!_tasksBuff.empty()) {
				//任务缓冲-新任务加入
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuff) {
					_tasks.push_back(pTask);
				}
				_tasksBuff.clear();
			}
			if (_tasks.empty()) {
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//处理任务
			for (auto pTask : _tasks) {
				pTask();
			}
			_tasks.clear();
		}
		//处理缓冲队列中的任务
		for (auto pTask : _tasksBuff)
		{
			pTask();
		}
		_tasksBuff.clear();
		//CellLog::Info("CellTaskServer<%d> OnRun exit\n", _serverId);
	}
};
