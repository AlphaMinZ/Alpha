#ifndef __ALPHA_RPC_ZKCLIENT_H__
#define __ALPHA_RPC_ZKCLIENT_H__

#ifndef THREADED
#define THREADED
#endif

#include <string>
#include <memory>
#include <zookeeper/zookeeper.h>

namespace alphaMin {

namespace rpc {

/**
 * @brief ZooKeeper客户端类
 * 
 */
class ZkClientRPC {
public:
    typedef std::shared_ptr<ZkClientRPC> ptr;

    ZkClientRPC();
    ZkClientRPC(const ZkClientRPC&) = delete;
    ZkClientRPC& operator=(const ZkClientRPC&) = delete;
    ~ZkClientRPC();

    /**
     * @brief 启动客户端
     * 
     */
    void start();

    /**
     * @brief 创建Znode节点
     * 
     * @param path 路径
     * @param data 节点数据
     * @param flags 节点标志，可以是如下参数
     * 0: 表示永久性节点，默认情况
     * ZOO_EPHEMERAL: 临时性节点
     * ZOO_SEQUENCE: 路径名后添加唯一的、单调递增的序号
     */
    void create(const std::string& path, const std::string& data, int flags = 0);

    /**
     * @brief 获取节点数据
     * 
     * @param path 节点路径
     * @return std::string 
     */
    std::string getData(const std::string& path);

private:
    zhandle_t* m_zkHandler; // zookeeper句柄
};

}

} 

#endif 