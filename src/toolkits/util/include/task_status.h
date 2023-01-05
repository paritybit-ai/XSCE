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


// typedef std::function<void()> non_para_func;
namespace xsce_ose_taskstatus
{
class TaskStatus {
    
    using NonParaFunc = std::function<void()>;
    using SetCurFuncCallBack = std::function<void (const std::string& cur_func)>;
    using SetProgressPerBucketCallBack = std::function<void (uint32_t progress_percentage, int bucket_id)>;
    using SetBucketNumCallBack = std::function<void (int bucket_pool_num1)>;
    using IsStopCallBack = std::function<bool ()>;
public:
    
    void SetCurFuncMeta(){
        if(SetCurFuncMeta_){
            SetCurFuncMeta_();
        }
    };
    void SetStop(){
        if(SetStop_){
            SetStop_();
        }
    };
    void IsStop(){
        if(IsStop_){
            IsStop_();
        }
    };   
    void SetProgressPerBucket(uint32_t progress_percentage, int bucket_id){
        if(SetProgressPerBucket_){
            SetProgressPerBucket_(progress_percentage, bucket_id);
        }
    };

    void SetBucketNum(int bucket_pool_num1){
        if(SetBucketNum_){
            SetBucketNum_(bucket_pool_num1);
        }
    }; 

    void RgeisterSetCurFuncMeta(NonParaFunc fun){
        SetCurFuncMeta_ = fun;
    }
    void RgeisterSetStop(NonParaFunc fun){
        SetStop_ = fun;
    }
    void RgeisterIsStop(IsStopCallBack fun){
        IsStop_ = fun;
    }
    void RgeisterSetProgressPerBucket(SetProgressPerBucketCallBack fun){
        SetProgressPerBucket_ = fun;
    }
    void RgeisterSetBucketNum(SetBucketNumCallBack fun){
        SetBucketNum_ = fun;
    }
    void RgeisterSetCurFunc(SetCurFuncCallBack fun){
        SetCurFunc_ = fun;
    }
public:
    NonParaFunc SetCurFuncMeta_;
    NonParaFunc SetStop_;
    IsStopCallBack IsStop_;
    SetCurFuncCallBack SetCurFunc_;
    SetProgressPerBucketCallBack SetProgressPerBucket_;
    SetBucketNumCallBack SetBucketNum_;

};

}
