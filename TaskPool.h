#ifndef TASKPOOL_H
#define TASKPOOL_H

#include <memory>
#include <queue>
#include <mutex>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>
#include <vector>
#include <functional>
#include <stdexcept>

 template<typename thread_t>
 class TaskPool{
     using task_t = std::function<void()>;
     struct pri_task{
         int pri;
         task_t task;
         pri_task(int p, task_t e)
             : pri(p), task(e){}
     };

     struct greater_pri : public std::binary_function<pri_task, pri_task, bool> {
         bool operator()(const pri_task& left, const pri_task& right) const{
             return left.pri > right.pri;
         }
     };

 public:
     TaskPool(size_t size = 0)
         : m_kill_flag(false),
         m_commit_flag(true){
         size = size < 1 ? std::thread::hardware_concurrency() : size;
         for (size_t i = 0; i < size; i++){
             m_pool.emplace_back(&TaskPool<thread_t>::thread_schedual, this);
             //printf("emplace_back one thread schedual\n");
         }
     }

     virtual ~TaskPool(){
         wait_and_kill_all();
     }

     void close_commit(){
         this->m_commit_flag.store(false);
     }

     void resume_commit(){
         this->m_commit_flag.store(true);
     }

     void kill_all_threads(){
         //printf("join and kill all threads\n");
         this->m_kill_flag.store(true);
         m_cv_cond.notify_all();
         for (thread_t& thrd : m_pool){
             thrd.join();
         }
     }

     int tasks_count(){
         std::lock_guard<std::mutex> lock_block(m_mutex);
         return m_tasks.size();
     }

     void wait_and_kill_all(){
         while (tasks_count() > 0)
         {}
         kill_all_threads();
     }

     //提交一个优先级为0的任务
     template<typename func_t, typename... args_t>
     auto commit(func_t&& func, args_t&&... args)->std::future<decltype(func(args...))>{
         return commit(0, func, args...);
     }

     //提交一个带优先级的任务
     template<typename func_t, typename... args_t>
     auto commit(int pri, func_t&& func, args_t&&... args)->std::future<decltype(func(args...))>{
         if (!m_commit_flag.load()){
             throw std::runtime_error("thread pool had closed for task commit");
         }

         using ret_t = decltype(func(args...));
         auto new_task = std::make_shared<std::packaged_task<ret_t()> >(
             std::bind(std::forward<func_t>(func), std::forward<args_t>(args)...));
         //printf("create a new task with std::bind()\n");

         {
             std::lock_guard<std::mutex> lock_block(m_mutex);
             m_tasks.push(pri_task(pri, [new_task](){
                 (*new_task)();
             }));
             //printf("wrapped this task into a lambda function and enqued\n");
         }

         //printf("notify all on condition variable\n");
         m_cv_cond.notify_all();

         //printf("return a future\n");
         std::future<ret_t> future = new_task->get_future();
         return future;
     }

 protected:
     bool fetch_task(task_t& curr_task){
         //printf("thread[%d] waits for a task\n", std::this_thread::get_id());
         std::unique_lock<std::mutex> lock(m_mutex);
         m_cv_cond.wait(lock, [this](){return !m_tasks.empty() || m_kill_flag.load(); });//wait直到有task可以提取

         if (!m_kill_flag.load()){
             curr_task = std::move(m_tasks.top().task);
             m_tasks.pop();
             return true;
         }
         //printf("m_kill_flag has been marked\n", std::this_thread::get_id());
         return false;
     }

     void thread_schedual(){
         //printf("[%d]step into schedual routine\n", std::this_thread::get_id());
         while (!m_kill_flag.load()){
             task_t curr_task;
             if (fetch_task(curr_task)){
                 curr_task();
             }
             else{
                 // printf("fetch_task returned an invalid task\n");
                 break;
             }
         }//while (true)
     }

 protected:
     std::vector<thread_t> m_pool;
     std::priority_queue<pri_task,
         std::vector<pri_task>,
         greater_pri> m_tasks;
     std::mutex m_mutex;
     std::condition_variable m_cv_cond;
     std::atomic<bool> m_kill_flag;
     std::atomic<bool> m_commit_flag;
 };
#endif // TASKPOOL_H
