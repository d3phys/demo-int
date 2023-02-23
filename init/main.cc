#include <cstdio>
#include <string>
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <assert.h>

namespace trace {

class Trace
{
  private:
    std::string func_name_;
  public:
    Trace(const char *func)
      : func_name_ {func}
    {
        std::fprintf(stderr, "%s entered\n", func_name_.c_str());
    }

    ~Trace()
    {
        std::fprintf(stderr, "%s leaved\n", func_name_.c_str());
    }
};

} /* namespace */


namespace dfg {

#define TRACE_CALL trace::Trace dfg_super_duper_trace{ __PRETTY_FUNCTION__};

class Graph;

class Node
{
  public:
    using Printer_t = void (*)( FILE*, const Node&);

    enum Value_t
    {
        Temporary = 0,
        LValue,
        LVRef,
        RVRef,
        Count
    };

    Node( std::size_t index, const std::string& identifier, Value_t value_type)
      : value_type{ value_type}
      , identifier{ identifier}
      , index{ index}
    {}

  public:
    Value_t value_type;
    std::string identifier;
    std::size_t index;
};

class Edge
{
  public:
    using Printer_t = void (*)( FILE*, const Graph&, const Edge&);

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
    using Printer_t = void (*)( FILE*, const Subgraph&);

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
    Graph()
    {
        subgraphs_.emplace_back( "G");
        call_stack_.push( 0);
    }

    ~Graph()
    {
        call_stack_.pop();
        assert( call_stack_.empty());
    }

          Node& getNode()                         { return nodes_.back(); }
          Node& getNode( std::size_t index)       { return nodes_.at( index); }
    const Node& getNode( std::size_t index) const { return nodes_.at( index); }

    Edge& getEdge()                   { return edges_.back(); }
    Edge& getEdge( std::size_t index) { return edges_.at( index); }

    void enterCall( const std::string& identifier)
    {
        call_stack_.push( createSubgraph( identifier));
    }

    void leaveCall()
    {
        call_stack_.pop();
    }

    std::size_t createSubgraph( const std::string& identifier)
    {
        subgraphs_.emplace_back( identifier);
        subgraphs_.at( call_stack_.top()).child_subgraphs.emplace_back( subgraphs_.size() - 1);
        return subgraphs_.size() - 1;
    }

    std::size_t createNode( Node::Value_t value_type, const std::string& identifier = "")
    {
        std::size_t node_index = nodes_.size();
        if ( identifier == "" )
        {
            nodes_.emplace_back( node_index, "tmp#" + std::to_string( nodes_.size()), value_type);
        } else
        {
            nodes_.emplace_back( node_index, identifier, value_type);
        }

        subgraphs_.at(call_stack_.top()).nodes.push_back( node_index);
        return node_index;
    }

    std::size_t createEdge( std::size_t from, std::size_t to, Edge::Pass_t pass_type)
    {
        edges_.emplace_back( from, to, pass_type);
        return edges_.size() - 1;
    }

    void dump( FILE* stream) const
    {
        dumpGraph( stream, 0);
    }

  private:

    void dumpNode( FILE* stream, std::size_t index) const
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

    void dumpEdge( FILE* stream, const Edge& edge) const
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

    void dumpContent( FILE* stream, const Subgraph& subgraph) const
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

    void dumpSubgraph( FILE* stream, std::size_t subgraph_index) const
    {
        const Subgraph& subgraph = subgraphs_.at( subgraph_index);

        std::fprintf( stream, "subgraph cluster_%ld {\n", subgraph_index);
        std::fprintf( stream, "style = \"rounded\"\nlabel=\"%s\"\n", subgraph.identifier.c_str());
        dumpContent( stream, subgraph);
        std::fprintf( stream, "}\n");
    }

    void dumpGraph( FILE* stream, std::size_t graph_index) const
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

    Proxy& operator+( const Proxy& other);

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
}

} /* namespace dfg */


using Int = dfg::Proxy<int>;

template <>
dfg::Graph Int::gGraph{};

Int
func()
{
    TRACE_CALL
    return Int{ 32};
}

void
Movement( Int&& var)
{
    TRACE_CALL
    //dfg_arg(var, dfg_Pass::Copy);

    Int c = var;
}

#define dfg_val( __v, __type)                                     \
    do { Int::gGraph.getNode().identifier = #__v;                 \
         Int::gGraph.getNode().value_type = dfg::Node::Value_t::__type; } while ( 0 )

#define dfg_arg( __v, __val_type, __type)                                                       \
    do { Int::gGraph.createEdge( __v.getNodeIndex(),                                            \
                                 Int::gGraph.createNode( dfg::Node::Value_t::__val_type, #__v), \
                                 dfg::Edge::Pass_t::__type); } while ( 0 )

#define dfg_e( __v) Int::gGraph.getNode().identifier = #__v;
#define dfg_scope() dfg::Scope super_unique_scope{ Int::gGraph, __func__}


Int Func( Int& param)
{
    dfg_scope();
    dfg_arg( param, LVRef, ConstLVRef);

    return Int{ 3};
}

Int Add( const Int& rhs, const Int& lhs)
{
    dfg_scope();
    dfg_arg( rhs, LVRef, ConstLVRef);
    dfg_arg( lhs, LVRef, ConstLVRef);

    return rhs.getValue() + lhs.getValue();
}

int
main()
{
    TRACE_CALL

//    Int b = 23; dfg_var( b);
 //   Int c = 23; dfg_var( c);

  //  Int result = b + c; dfg_var( result);

   // Int copy_elision = func(); dfg_var( copy_elision);

    {
        dfg_scope();
        Int a{ 0}; dfg_val( a, LValue);
        a = Func( a);
        Int b = Add( a, 3); dfg_val( b, LValue);
    }

    Int::gGraph.dump( stdout);
    return 0;
}

#if 0
static void
gv_PrintBasicNode( FILE* stream,
                   const dfg::Node& node,
                   const char* style = "")
{
    std::fprintf( stream, "\"%s\" %s\n", node.identifier.c_str(), style);
}

static void
gv_PrintBasicEdge( FILE* stream,
                   const dfg::Graph& graph,
                   const dfg::Edge& edge,
                   const char* style = "")
{
    std::fprintf( stream, "\"%s\"->\"%s\" %s\n",
                  graph.getNode( edge.neighbours.first).identifier.c_str(),
                  graph.getNode( edge.neighbours.second).identifier.c_str(),
                  style);
}

void InitPrintTable( dfg::PrintTable& print_table)
{
#define PRINT( __type, __fmt)                                                                   \
print_table.node_printers[dfg::Node::Value_t::__type] = [](FILE* stream, const dfg::Node& node) \
{                                                                                               \
    gv_PrintBasicNode( stream, node, __fmt);                                                    \
};

    PRINT( Temporary, "")
    PRINT( LValue,    "")
    PRINT( LVRef,     "")
    PRINT( RVRef,     "")

#undef PRINT

#define PRINT( __type, __fmt)                                                                                            \
print_table.edge_printers[dfg::Edge::Pass_t::__type] = [](FILE* stream, const dfg::Graph& graph, const dfg::Edge& edge)  \
{                                                                                                                        \
    gv_PrintBasicEdge( stream, graph, edge, __fmt);                                                                      \
};

    PRINT( LVRef,      "")
    PRINT( ConstLVRef, "")
    PRINT( RVRef,      "")
    PRINT( ConstRVRef, "")
    PRINT( Move,       "")
    PRINT( Copy,       "")

#undef PRINT
};
#endif

