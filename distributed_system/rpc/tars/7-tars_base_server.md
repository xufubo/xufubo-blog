# 概述
tars是一个高性能RPC框架，除了实现了RPC核心部分外，还为运营、运维、发布、监控、扩容等提供了支持，因此，除了RPC核心的实现之外，还实现了一系列的基础服务用于支撑这些功能的运行，本节主要介绍这些基础服务的功能。  

# 基础服务用到的tars协议
在 [tars install](./tars_install.md) 中已经知道：  
 - 手工部署的核心基础服务：tarsAdminRegistry, tarsregistry, tarsnode, tarsconfig, tarspatch  
 - 通过管理平台部署的普通基础服务：tarsstat, tarsproperty,tarsnotify, tarslog，tarsquerystat，tarsqueryproperty   

1. tarsAdminRegistry
tars不仅解决了普通的rpc框架需要解决的问题，也解决了RPC核心问题之外的问题，服务治理就是其中一项。tarsAdminRegistry就是与web管理端进行交互的接入管理服务，对服务的管理都有通过这个服务进行。该服务的接口定义如下： 

**AdminReg.tars**:  
-----------------------------------------
```tars
#include "NodeDescriptor.tars"

module tars
{
    struct ServerStateDesc
    {
        0 require string settingStateInReg;
        1 require string presentStateInReg;
        2 require string presentStateInNode;
        3 require int processId;
        4 require string patchVersion;
        5 require string patchTime;
        6 require string patchUser;
    };
    
    enum tarsErrCode
    {
        //请求的node节点没有注册或者节点心跳超时
        EM_TARS_NODE_NOT_REGISTRY_ERR = 1001,
        //主控请求node节点超时
        EM_TARS_CALL_NODE_TIMEOUT_ERR = 1002,
        //从主控数据库获取服务的描述信息错误
        EM_TARS_LOAD_SERVICE_DESC_ERR = 1003,
        //服务可能处在停止或者发布过程中
        EM_TARS_SERVICE_STATE_ERR = 1004,
        //同样的请求已经在处理了
        EM_TARS_REQ_ALREADY_ERR = 1005,
        //请求来自非法的ip地址,如并非合法的主控地址
        EM_TARS_INVALID_IP_ERR = 1006,
        //参数错误
        EM_TARS_PARAMETER_ERR = 1007,
        //非tars方面的问题，用于node判断业务机器问题等
        EM_TARS_OTHER_ERR = 1008,
        //获取patch file异常
        EM_TARS_GET_PATCH_FILE_ERR = 1009,
        //调用patch server准备发布包失败
        EM_TARS_PREPARE_ERR = 1010,
        //未知错误
        EM_TARS_UNKNOWN_ERR = -1,
        //成功
        EM_TARS_SUCCESS = 0
    };

    /**
    * 管理平台过来的命令
    */
    enum EMTaskCommand
    {
        EM_CMD_START,
        EM_CMD_STOP,
        EM_CMD_PATCH,
        EM_CMD_UNINSTALL
    };

    /**
    * 任务的状态
    */
    enum EMTaskStatus {
        EM_T_NOT_START,
        EM_T_RUNNING,
        EM_T_SUCCESS,
        EM_T_FAILED,
        EM_T_CANCEL,
        EM_T_PARIAL,
    };

    /**
    * 任务ITEM状态
    */
    enum EMTaskItemStatus
    {
        EM_I_NOT_START,
        EM_I_RUNNING,
        EM_I_SUCCESS,
        EM_I_FAILED,
        EM_I_CANCEL,
    };

    /**
    * 异步任务
    */
    struct TaskItemReq
    {
        0 optional string taskNo;
        1 optional string itemNo;
        2 optional string application;
        3 optional string serverName;
        4 optional string nodeName;
        5 optional string setName;
        6 optional string command;
        7 optional string userName;
        8 optional map<string, string> parameters;
    };

    /**
    * 任务当前的信息
    */
    struct TaskItemRsp
    {
        0 optional TaskItemReq req;
        1 optional string  startTime;
        2 optional string  endTime;
        3 optional EMTaskItemStatus     status;
        4 optional string  statusInfo;
        5 optional string  executeLog;
    };

    /**
    * Task任务
    */
    struct TaskReq
    {
        0 optional vector<TaskItemReq> taskItemReq;
        1 optional string taskNo;
        2 optional bool   serial;
        3 optional string userName;
    };

    struct TaskRsp
    {
        0 optional vector<TaskItemRsp> taskItemRsp;
        1 optional string taskNo;
        2 optional bool   serial;
        3 optional string userName;
        4 optional EMTaskStatus status; 
    };

	enum ServerGridStatus
    {
    	//正常发布服务
		NORMAL	= 0,
		//自动扩容时，灰度服务，即部分流量分到灰度节点上
		GRID	= 1,
		//发布服务，但是发布后，没用流量过来
		NO_FLOW = 2	
    };
	
    struct ServerGridDesc
    {
    	0 require string			application;
		1 require string			servername;
		2 require string			nodename;
		3 require ServerGridStatus		status;
    };
	
    interface AdminReg
    {

        /**
         * 请求异步任务
         *
         * @param taskList : 任务列表
         * @param serial: 串行
         *
         * @return 0: 成功, <0:失败
         */
        int addTaskReq(TaskReq taskReq);

        /**
         * 获取任务状态
         *
         * @param taskIdList : 任务列表id
         *
         * @return 任务状态
         */
        int getTaskRsp(string taskNo, out TaskRsp taskRsp);

        /**
         * 获取根据应用, 服务, 命令字获取历史任务状态
         *
         * @param application : 应用名
         * @param serverName : 服务名
         * @param command : 命令字
         *
         * @return TaskRsp
         */
        int getTaskHistory(string application, string serverName, string command, out vector<TaskRsp> taskRsps);

        /**
        * 设置task item的信息(内部使用)
        * @param itemNo, itemno
        * @param startTime, 开始时间
        * @param endTime, 结束时间
        * @param status, 状态
        * @param log, 异常日志
        * @return int, 0: 成功, <0: 失败
        */
        int setTaskItemInfo(string itemNo, map<string, string> info);

        //////////////////////////////////////////////////////////////////////////////////
        /**
         * 获取application列表
         *
         * @param out result : 结果描述
         *
         * @return application列表
         */
        vector<string> getAllApplicationNames(out string result);

        /**
         * 获取node列表
         *
         * @param out result : 结果描述
         *
         * @return node 列表
         */
        vector<string> getAllNodeNames(out string result);

        /**
         * ping node
         *
         * @param name: node id
         * @param out result : 结果描述
         *
         * @return : true-ping通；false-不通
         */
        bool pingNode(string name, out string result);

        /**
         * get node version
         *
         * @param name: node id
         * @param out version ：版本
         * @param out result : 结果描述
         *
         * @return : 0-成功 others-失败
         */
        int getNodeVesion(string name, out string version, out string result);

        /**
         * 停止 node
         *
         * @param name: node id
         * @param out result : 结果描述
         *
         * @return : 0-成功 others-失败
         */
        int shutdownNode(string name, out string result);

        /***********server****************/

        /**
         * 获取server列表
         *
         * @param name: null
         * @param out result : 结果描述
         *
         * @return: server列表及相关信息
         */
        vector<vector<string> > getAllServerIds(out string result);

        /**
         * 获取特定server状态
         *
         * @param application: 应用
         * @param serverName : server名
         * @param nodeNmae   : node id
         * @param out state  : 状态
         * @param out result : 结果描述
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int getServerState(string application,string serverName, string nodeName, out ServerStateDesc state, out string result);

        /**
         * 获取某ip所属group 用于机房部署  例如某ip属于那个机房
         *
         * @param nodeNmae   : ip
         * @param out int    : group id
         * @param out result : 结果描述
         *
         * @return : 处理结果
         */
        int getGroupId(string ip, out int groupId, out string result);

        /**
         * 启动特定server
         *
         * @param application: 应用
         * @param serverName : server名
         * @param nodeName   : node id
         * @param out result : 结果描述
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int startServer(string application,string serverName, string nodeName, out string result);

        /**
         * 停止特定server
         *
         * @param application: 应用
         * @param serverName : server名
         * @param nodeName   : node id
         * @param out result : 结果描述
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int stopServer(string application,string serverName, string nodeName, out string result);

        /**
         * 重启特定server
         *
         * @param application: 应用
         * @param serverName : server名
         * @param nodeName   : node id
         * @param out result : 结果描述
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int restartServer(string application,string serverName, string nodeName, out string result);

        /**
         * 通知server
         *
         * @param application: 应用
         * @param serverName : server名
         * @param nodeName   : node id
         * @param command    : 命令
         * @param out result : 结果描述
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int notifyServer(string application,string serverName, string nodeName, string command, out string result);

        /**
        * 卸载服务
        * @param application
        * @param serverName
        * @param nodeName
        * @return 0: 成功, <0: 失败
        */
        int undeploy(string application, string serverName, string nodeName, string user,  out string log);
       
        /**
         * 批量发布服务
         *
         * @param vector<PatchRequest>: 批量发布的请求
         * @param out result : 结果描述
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int batchPatch(PatchRequest req, out string result);

        /**
         * 更新发布状态
         *
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int updatePatchLog(string application, string serverName, string nodeName, string patchId, string user, string patchType, bool succ);
        
        /**
        * 获取服务发布进度
        * @param application  : 服务所属应用名
        * @param serverName   : 服务名
        * @param nodeName     : node id
        * @out tPatchInfo     ：发布信息
        *
        * @return : 返回值详见tarsErrCode枚举值
        */
        int getPatchPercent(string application,string serverName,string nodeName,out PatchInfo tPatchInfo);

        /**
         * 加载特定server
         *
         * @param application: 应用
         * @param serverName : server名
         * @param nodeName   : node id
         * @param out result : 结果描述
         *
         * @return : 返回值详见tarsErrCode枚举值
         */
        int loadServer(string application, string serverName, string nodeName, out string result);

        /**
         * 获取相应模板
         *
         * @param profileName: 模板名称
         * @param out profileTemplate: 模板内容
         * @param out resultDesc: 结果描述
         *
         * @return : 0-成功 others-失败
         */
        int getProfileTemplate(string profileName , out string profileTemplate, out string resultDesc);

        /**
         * 获取服务相应模板
         *
         * @param application: 应用
         * @param serverName : server名
         * @param nodeName   : node id
         * @param out profileTemplate: 模板内容
         * @param out resultDesc: 结果描述
         *
         * @return : 0-成功 others-失败
         */
        int getServerProfileTemplate(string application, string serverName, string nodeName, out string profileTemplate, out string resultDesc);

        /**
         * node通过接口获取连接上主控的node ip
         * @param sNodeIp:  node 的ip
         *
         * @return 0-成功 others-失败
         */
        int getClientIp(out string sClientIp);
     
		/**
         * 自动伸缩时调用的灰度发布接口
         *
         * @param gridDescList: 灰度发布的信息
         * @param gridFailDescList: 灰度发布节点失败的信息
         * @param out resultDesc: 结果描述
         *
         * @return : 0-成功 others-失败
         */
        int gridPatchServer(vector<ServerGridDesc> gridDescList, out vector<ServerGridDesc> gridFailDescList, out string resultDesc);
	 
    };
};
```
-----------------------------------------

