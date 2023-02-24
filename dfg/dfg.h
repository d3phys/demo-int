#pragma once

#include <cstdio>
#include <string>
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <assert.h>

namespace dfg {

class Graph;

class Node
{
  public:
    enum Value_t
    {
        Temporary = 0,
        LValue,
        LVRef,
        RVRef,
        Count
    };

    Node( const std::string& identifier, Value_t value_type)
      : value_type{ value_type}
      , identifier{ identifier}
    {}

  public:
    Value_t value_type;
    std::string identifier;
};

class Edge
{
  public:
    enum Pass_t
    {
        Invalid = 0,
        LVRef,
        ConstLVRef,
        RVRef,
        ConstRVRef,
        Copy,
        Move,
        Count
    };

    Edge( std::size_t from, std::size_t to, Pass_t pass_type)
      : neighbours{ from, to}
      , pass_type{ pass_type}
    {}

  public:
    std::pair<std::size_t, std::size_t> neighbours;
    Pass_t pass_type;
};

struct Subgraph
{
  public:
    std::vector<std::size_t> nodes;
    std::vector<std::size_t> child_subgraphs;
    std::string identifier;

    Subgraph( const std::string& identifier = "")
      : identifier{ identifier} {}
};

class Graph
{
  private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    std::vector<Subgraph> subgraphs_;
    std::stack<std::size_t> call_stack_;

  public:
    Graph();
    ~Graph();

          Node& getNode()                         { return nodes_.back(); }
          Node& getNode( std::size_t index)       { return nodes_.at( index); }
    const Node& getNode( std::size_t index) const { return nodes_.at( index); }

    Edge& getEdge()                   { return edges_.back(); }
    Edge& getEdge( std::size_t index) { return edges_.at( index); }

    void enterCall( const std::string& identifier);
    void leaveCall();

    std::size_t createSubgraph( const std::string& identifier);

    std::size_t createNode( Node::Value_t value_type, const std::string& identifier = "");
    std::size_t createEdge( std::size_t from, std::size_t to, Edge::Pass_t pass_type);
    void dump( FILE* stream) const;

  private:
    void dumpNode( FILE* stream, std::size_t index) const;
    void dumpEdge( FILE* stream, const Edge& edge) const;
    void dumpContent( FILE* stream, const Subgraph& subgraph) const;
    void dumpSubgraph( FILE* stream, std::size_t subgraph_index) const;
    void dumpGraph( FILE* stream, std::size_t graph_index) const;
};

class Scope
{
  public:
    Scope( Graph& graph, const std::string& identifier)
      : graph_{ graph}
    {
        graph_.enterCall( identifier);
    }

    ~Scope()
    {
        graph_.leaveCall();
    }

  private:
    Graph& graph_;
};

template <typename T>
class Proxy
{
  public:
    Proxy(const T& value);

    Proxy( const Proxy& other);
    Proxy& operator=( const Proxy& other);

    Proxy( Proxy&& other);
    Proxy& operator=( Proxy&& other);

    ~Proxy() = default;

    std::size_t getNodeIndex() const { return node_index_; };
    const T& getValue() const { return value_; }

    static Graph gGraph;

  private:
    T value_;
    std::size_t node_index_;
};

template <typename T>
Proxy<T>::Proxy( const T& value)
  : value_{ value},
    node_index_{ gGraph.createNode( Node::Value_t::Temporary)}
{}

template <typename T>
Proxy<T>::Proxy( const Proxy& other) : value_{ other.value_}, node_index_{ gGraph.createNode( Node::Value_t::Temporary)}
{
    gGraph.createEdge( other.node_index_, node_index_, Edge::Pass_t::Copy);
}

template <typename T>
Proxy<T>&
Proxy<T>::operator=( const Proxy& other)
{
    gGraph.createEdge( other.node_index_, node_index_, Edge::Pass_t::Copy);
    value_ = other.value_;
    return (*this);
}

template <typename T>
Proxy<T>::Proxy( Proxy&& other)
  : value_{ other.value_},
    node_index_{ gGraph.createNode( Node::Value_t::Temporary)}
{
    gGraph.createEdge( other.node_index_, node_index_, Edge::Pass_t::Move);
}

template <typename T>
Proxy<T>&
Proxy<T>::operator=( Proxy&& other)
{
    gGraph.createEdge( other.node_index_, node_index_, Edge::Pass_t::Move);
    value_ = std::move( other.value_);
    return (*this);
};

} /* namespace dfg */


