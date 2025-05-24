#include <cassert>
#include <memory>
#include <mutex>
#include <optional>
#include <iostream>
#include <windows.h>

class ArgsManager {
public:
protected:
    struct Arg {
        std::string m_help_param;
        std::string m_help_text;
        unsigned int m_flags;
    };
};

// 前向声明 node 命名空间和 NodeContext
namespace node {
    struct NodeContext;
}

// 定义 interfaces 命名空间
namespace interfaces {
    class Node {
    public:
        virtual ~Node() = default;
        virtual node::NodeContext* context() { return nullptr; }
        virtual void setContext(node::NodeContext* context) {}
    };

    class Chain {
    public:
        virtual ~Chain() = default;
        virtual node::NodeContext* context() { return nullptr; }
    };

    class Mining {
    public:
        virtual ~Mining() = default;
    };

    class Init {
    public:
        virtual ~Init() = default;
        virtual std::unique_ptr<Node> makeNode() { return nullptr; }
        virtual std::unique_ptr<Chain> makeChain() { return nullptr; }
        virtual std::unique_ptr<Mining> makeMining() { return nullptr; }
    };
}

// 定义 node::NodeContext
namespace node {
    struct NodeContext {
        interfaces::Init* init{ nullptr };
        ArgsManager* args{ nullptr };
    };
}

namespace util {
    class SignalInterrupt {
    public:
        SignalInterrupt() : m_flag(false) {
            std::cout << "SignalInterrupt 构造完成" << std::endl;
        }
        explicit operator bool() const {
            std::cout << "SignalInterrupt::operator bool 被调用" << std::endl;
            return m_flag.load();
        }
        bool operator()() {
            std::cout << "SignalInterrupt::operator() 被调用" << std::endl;
            return m_flag.exchange(true);
        }
        bool reset() {
            std::cout << "SignalInterrupt::reset 被调用" << std::endl;
            return m_flag.exchange(false);
        }
        bool wait() {
            std::cout << "SignalInterrupt::wait 被调用" << std::endl;
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return m_flag.load(); });
            return true;
        }
    private:
        std::atomic<bool> m_flag;
        std::mutex m_mutex;
        std::condition_variable m_cv;
    };
}

using node::NodeContext;

static std::optional<util::SignalInterrupt> g_shutdown;
ArgsManager gArgs;

void InitContext(NodeContext& node) {
    assert(!g_shutdown);
    g_shutdown.emplace();
    std::cout << "InitContext 执行，初始化 g_shutdown 和 node.args" << std::endl;
    node.args = &gArgs;
}

namespace node {
    namespace {
        class NodeImpl : public interfaces::Node {
        public:
            explicit NodeImpl(NodeContext& context) : m_context(nullptr) {
                std::cout << "NodeImpl 构造完成" << std::endl;
                setContext(&context);
            }
            node::NodeContext* context() override {
                std::cout << "NodeImpl::context 被调用" << std::endl;
                return m_context;
            }
            void setContext(node::NodeContext* context) override {
                std::cout << "NodeImpl::setContext 被调用" << std::endl;
                m_context = context;
            }
            NodeContext* m_context{ nullptr };
        };

        class ChainImpl : public interfaces::Chain {
        public:
            explicit ChainImpl(NodeContext& node) : m_node(node) {
                std::cout << "ChainImpl 构造完成" << std::endl;
            }
            node::NodeContext* context() override {
                std::cout << "ChainImpl::context 被调用" << std::endl;
                return &m_node;
            }
            NodeContext& m_node;
        };

        class MinerImpl : public interfaces::Mining {
        public:
            explicit MinerImpl(NodeContext& node) : m_node(node) {
                std::cout << "MinerImpl 构造完成" << std::endl;
            }
            NodeContext& m_node;
        };
    }
}

namespace interfaces {
    std::unique_ptr<Node> MakeNode(node::NodeContext& context) {
        std::cout << "MakeNode 创建 NodeImpl" << std::endl;
        return std::make_unique<node::NodeImpl>(context);
    }
    std::unique_ptr<Chain> MakeChain(node::NodeContext& context) {
        std::cout << "MakeChain 创建 ChainImpl" << std::endl;
        return std::make_unique<node::ChainImpl>(context);
    }
    std::unique_ptr<Mining> MakeMining(node::NodeContext& context) {
        std::cout << "MakeMining 创建 MinerImpl" << std::endl;
        return std::make_unique<node::MinerImpl>(context);
    }
}

namespace init {
    namespace {
        class BitcoindInit : public interfaces::Init {
        public:
            BitcoindInit(NodeContext& node) : m_node(node) {
                std::cout << "BitcoindInit 构造完成" << std::endl;
                InitContext(m_node);
                m_node.init = this;
            }
            std::unique_ptr<interfaces::Node> makeNode() override {
                std::cout << "BitcoindInit::makeNode 被调用" << std::endl;
                return interfaces::MakeNode(m_node);
            }
            std::unique_ptr<interfaces::Chain> makeChain() override {
                std::cout << "BitcoindInit::makeChain 被调用" << std::endl;
                return interfaces::MakeChain(m_node);
            }
            std::unique_ptr<interfaces::Mining> makeMining() override {
                std::cout << "BitcoindInit::makeMining 被调用" << std::endl;
                return interfaces::MakeMining(m_node);
            }
            NodeContext& m_node;
        };
    }
}

namespace interfaces {
    std::unique_ptr<Init> MakeNodeInit(NodeContext& node, int argc, char* argv[], int& exit_status) {
        std::cout << "MakeNodeInit 创建 BitcoindInit" << std::endl;
        return std::make_unique<init::BitcoindInit>(node);
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "开始执行 main 函数" << std::endl;
    node::NodeContext node;
    std::cout << "创建 NodeContext" << std::endl;
    auto init = std::make_unique<init::BitcoindInit>(node);
    std::cout << "创建 BitcoindInit" << std::endl;
    auto node_interface = init->makeNode();
    std::cout << "创建 Node 接口" << std::endl;
    auto chain_interface = init->makeChain();
    std::cout << "创建 Chain 接口" << std::endl;
    auto mining_interface = init->makeMining();
    std::cout << "创建 Mining 接口" << std::endl;
    std::cout << "Node 上下文地址: " << node_interface->context() << std::endl;
    std::cout << "Chain 上下文地址: " << chain_interface->context() << std::endl;
    std::cout << "main 函数执行完成" << std::endl;
    return 0;
}
