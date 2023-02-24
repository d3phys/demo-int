#include <cstdio>
#include <string>
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <assert.h>
#include "dfg.h"

namespace dfg {

Graph::Graph()
{
    subgraphs_.emplace_back( "G");
    call_stack_.push( 0);
}

Graph::~Graph()
{
    call_stack_.pop();
    assert( call_stack_.empty());
}

void
Graph::enterCall( const std::string& identifier)
{
    call_stack_.push( createSubgraph( identifier));
}

void
Graph::leaveCall()
{
    call_stack_.pop();
}

std::size_t
Graph::createSubgraph( const std::string& identifier)
{
    subgraphs_.emplace_back( identifier);
    subgraphs_.at( call_stack_.top()).child_subgraphs.emplace_back( subgraphs_.size() - 1);
    return subgraphs_.size() - 1;
}

std::size_t
Graph::createNode( Node::Value_t value_type,
                   const std::string& identifier)
{
    std::size_t node_index = nodes_.size();
    if ( identifier == "" )
    {
        nodes_.emplace_back( "tmp#" + std::to_string( nodes_.size()), value_type);
    } else
    {
        nodes_.emplace_back( identifier, value_type);
    }

    subgraphs_.at(call_stack_.top()).nodes.push_back( node_index);
    return node_index;
}

std::size_t
Graph::createEdge( std::size_t from,
                   std::size_t to,
                   Edge::Pass_t pass_type)
{
    edges_.emplace_back( from, to, pass_type);
    return edges_.size() - 1;
}

} /* namespace dfg */
