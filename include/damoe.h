#ifndef DAMOE_H
#define DAMOE_H

#include <cassert>
#include <memory>
#include <mutex>
#include <optional>
// #include <atomic>
// #include <condition_variable>

class ArgsManager {
public:
    // Placeholder for ArgsManager class
    // This class would typically handle command-line arguments and configuration options.
protected:
    struct Arg {
        std::string m_help_param;
        std::string m_help_text;
        unsigned int m_flags;
    };
};

namespace node {
    class NodeContext;
}

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
        // virtual void setContext(node::NodeContext* context) { }
        //! Get internal node context. Useful for testing, but not
        //! accessible across processes.
        virtual node::NodeContext* context() { return nullptr; }
    };

    class Mining {
    public:
        virtual ~Mining() = default;
        //! Return implementation of Mining interface.
        std::unique_ptr<Mining> MakeMining(node::NodeContext& node);
    };

    class Init {
    public:
        virtual ~Init() = default;
        virtual std::unique_ptr<Node> makeNode() { return nullptr; }
        virtual std::unique_ptr<Chain> makeChain() { return nullptr; }
        virtual std::unique_ptr<Mining> makeMining() { return nullptr; }
        // virtual std::unique_ptr<WalletLoader> makeWalletLoader(Chain& chain) { return nullptr; }
        // virtual std::unique_ptr<Echo> makeEcho() { return nullptr; }
        // virtual Ipc* ipc() { return nullptr; }
        // virtual bool canListenIpc() { return false; }
    };

} // namespace interfaces

namespace node {
    struct NodeContext {
        //! Init interface for initializing current process and connecting to other processes.
        interfaces::Init* init{ nullptr };
        ArgsManager* args{ nullptr }; // Currently a raw pointer because the memory is not managed by this struct
    };

    namespace {
        class NodeImpl : public interfaces::Node {
        public:
            explicit NodeImpl(NodeContext& context) {
                setContext(&context);
            }

            NodeContext* m_context{ nullptr };
        };

        class ChainImpl : public interfaces::Chain {
        public:
            explicit ChainImpl(NodeContext& node) : m_node(node) {

            }

            NodeContext& m_node;
        };

        class MinerImpl : public interfaces::Mining {
        public:
            explicit MinerImpl(NodeContext& node) : m_node(node) {

            }
            NodeContext& m_node;
        };
    } // anonymous namespace of node

} // namespace node

namespace util {
    class SignalInterrupt {
    public:
        SignalInterrupt();
        explicit operator bool() const;
        [[nodiscard]] bool operator()();
        [[nodiscard]] bool reset();
        [[nodiscard]] bool wait();

    private:
        std::atomic<bool> m_flag;

        // On windows use a condition variable, since we don't have any signals there
        std::mutex m_mutex;
        std::condition_variable m_cv;
    };
}   // namespace util

#endif  // DAMOE_H
