#include <damoe.h>
#include <iostream>  

using std::cout;
using std::endl;

using node::NodeContext;

//static int g_shutdown;
static std::optional<util::SignalInterrupt> g_shutdown;
ArgsManager gArgs;

void InitContext(NodeContext& node) {
    assert(!g_shutdown);
    g_shutdown.emplace();

    node.args = &gArgs;
    //    node.shutdown_signal = &*g_shutdown;
    //    node.shutdown_request = [&node] {
    //        assert(node.shutdown_signal);
    //        if (!(*node.shutdown_signal)()) return false;
    //        // Wake any threads that may be waiting for the tip to change.
    //        //if (node.notifications) WITH_LOCK(node.notifications->m_tip_block_mutex, node.notifications->m_tip_block_cv.notify_all());
    //        return true;
    //        };
}

namespace interfaces {
    std::unique_ptr<Node> MakeNode(node::NodeContext& context) {
        return std::make_unique<node::NodeImpl>(context);
    }
    std::unique_ptr<Chain> MakeChain(node::NodeContext& context) {
        return std::make_unique<node::ChainImpl>(context);
    }
    std::unique_ptr<Mining> MakeMining(node::NodeContext& context) {
        return std::make_unique<node::MinerImpl>(context);
    }
} // namespace interfaces

namespace init {
    namespace {
        class BitcoindInit : public interfaces::Init {
        public:
            BitcoindInit(NodeContext& node) : m_node(node) {
                InitContext(m_node);
                m_node.init = this;
            }
            std::unique_ptr<interfaces::Node> makeNode() override {
                return interfaces::MakeNode(m_node);
            }
            std::unique_ptr<interfaces::Chain> makeChain() override {
                return interfaces::MakeChain(m_node);
            }
            std::unique_ptr<interfaces::Mining> makeMining() override {
                return interfaces::MakeMining(m_node);
            }
            // std::unique_ptr<interfaces::Echo> makeEcho() override {
            //     return interfaces::MakeEcho();
            // }
            NodeContext& m_node;
        };
    } // namespace
} // namespace init

namespace interfaces {
    std::unique_ptr<Init> MakeNodeInit(NodeContext& node, int argc, char* argv[], int& exit_status)
    {
        return std::make_unique<init::BitcoindInit>(node);
    }
}

int main() {
    cout << "开始执行 main 函数" << endl;
    node::NodeContext node;
    cout << "创建 NodeContext" << endl;
    auto init = std::make_unique<init::BitcoindInit>(node);
    cout << "创建 BitcoindInit" << endl;
    auto node_interface = init->makeNode();
    cout << "创建 Node 接口" << endl;
    auto chain_interface = init->makeChain();
    cout << "创建 Chain 接口" << endl;
    auto mining_interface = init->makeMining();
    cout << "创建 Mining 接口" << endl;
    // 调用方法以验证上下文  
    cout << "Node 上下文地址: " << node_interface->context() << endl;
    cout << "Chain 上下文地址: " << chain_interface->context() << endl;
    cout << "main 函数执行完成" << endl;
    return 0;
}
