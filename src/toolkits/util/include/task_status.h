#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>
// #include "task_meta_client.h"
// #include "third_party/xsce-toolkits/xlog/include/loginterface.h"



namespace xsce_ose
{
class TaskStatus {
public:
    TaskStatus();
    ~TaskStatus();
    void SetCurFuncMeta(){
        SetCurFuncMeta_();
    };
    std::function<void ()> SetCurFuncMeta_;


std::shared_ptr<xsce::TaskStatus> (*GetTaskStatus)(const std::string& task_id);
std::function<std::shared_ptr<xsce::TaskStatus>(const std::string&)> GetTaskStatus;


}
