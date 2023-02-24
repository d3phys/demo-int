#include <cstdio>
#include <string>
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <assert.h>
#include "dfg.h"

namespace dfg {

void
Graph::dump( FILE* stream) const
{
    dumpGraph( stream, 0);
}

void
Graph::dumpNode( FILE* stream,
                 std::size_t index) const
{
    const Node& node = nodes_.at( index);
    std::fprintf( stream, "\"%%%ld\"[label=\"%s\", ", index, node.identifier.c_str());
    switch ( node.value_type )
    {
      case Node::Value_t::Temporary:
        std::fprintf( stream, "color = \"red\"]\n");
        break;
      case Node::Value_t::LValue:
        std::fprintf( stream, "color = \"grey\"]\n");
        break;
      case Node::Value_t::LVRef:
        std::fprintf( stream, "color = \"grey\", style = \"dashed\"]\n");
        break;
      case Node::Value_t::RVRef:
        std::fprintf( stream, "color = \"grey\", style = \"dashed\"]\n");
        break;
      default:
        break;
    }
}

void
Graph::dumpEdge( FILE* stream,
                 const Edge& edge) const
{
    std::fprintf( stream, "\"%%%ld\"->\"%%%ld\"", edge.neighbours.first, edge.neighbours.second);

    switch ( edge.pass_type )
    {
      case Edge::Pass_t::LVRef:
        std::fprintf( stream, "[style=\"dashed\", label=\" &\"]\n");
        break;
      case Edge::Pass_t::ConstLVRef:
        std::fprintf( stream, "[style=\"dashed\", label=\" const &\"]\n");
        break;
      case Edge::Pass_t::RVRef:
        std::fprintf( stream, "[style=\"dashed\", label=\" &&\"]\n");
        break;
      case Edge::Pass_t::ConstRVRef:
        std::fprintf( stream, "[style=\"dashed\", label=\" const &&\"]\n");
        break;
      case Edge::Pass_t::Copy:
        std::fprintf( stream, "[label=\" copy\", color=\"red\"]\n");
        break;
      case Edge::Pass_t::Move:
        std::fprintf( stream, "[label=\" move\", color=\"green\"]\n");
        break;
      default:
        break;
    }
}

void
Graph::dumpContent( FILE* stream,
                    const Subgraph& subgraph) const
{
    for ( auto it = subgraph.nodes.begin(); it != subgraph.nodes.end(); it++ )
    {
        dumpNode( stream, *it);
    }

    for ( auto it = subgraph.child_subgraphs.begin(); it != subgraph.child_subgraphs.end(); it++ )
    {
        dumpSubgraph( stream, *it);
    }
}

void
Graph::dumpSubgraph( FILE* stream,
                     std::size_t subgraph_index) const
{
    const Subgraph& subgraph = subgraphs_.at( subgraph_index);

    std::fprintf( stream, "subgraph cluster_%ld {\n", subgraph_index);
    std::fprintf( stream, "style = \"rounded\"\nlabel=\"%s\"\n", subgraph.identifier.c_str());
    dumpContent( stream, subgraph);
    std::fprintf( stream, "}\n");
}

void
Graph::dumpGraph( FILE* stream,
                  std::size_t graph_index) const
{
    const Subgraph& subgraph = subgraphs_.at( graph_index);
    std::fprintf( stream, "digraph \"%s\" {\n", subgraph.identifier.c_str());

    std::fprintf( stream, "fontname=\"Courier New\"\n"
                          "edge [fontname=\"Courier New\"]\n");
    std::fprintf( stream, R"(
node [penwidth = 2, shape = box, fillcolor = white,
  style = "rounded, filled", fontname = "Courier"]
    )" "\n");

    dumpContent( stream, subgraph);
    for ( auto it = edges_.begin(); it != edges_.end(); it++ )
    {
        dumpEdge( stream, *it);
    }

    std::fprintf( stream, "}\n");
}

} /* namespace dfg */
