#include <cstdio>
#include <string>
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>

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

class PrintTable
{
  public:
    Subgraph::Printer_t subgraph_printer{};
    Node::Printer_t node_printers[Node::Value_t::Count] {0};
    Edge::Printer_t edge_printers[Edge::Pass_t::Count]  {0};
};

class Graph
{
  private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    std::vector<Subgraph> subgraphs_;

    std::stack<std::size_t> call_stack_;
    Subgraph subgraph_;

  public:
    Graph()
    {
        subgraphs_.emplace_back( "");
    }

          Node& getNode()                         { return nodes_.back(); }
          Node& getNode( std::size_t index)       { return nodes_.at( index); }
    const Node& getNode( std::size_t index) const { return nodes_.at( index); }

    Edge& getEdge()                   { return edges_.back(); }
    Edge& getEdge( std::size_t index) { return edges_.at( index); }

    std::size_t createSubgraph( const std::string& identifier, std::size_t parent = 0)
    {
        subgraphs_.emplace_back( identifier);
        subgraphs_.at( parent).child_subgraphs.emplace_back( subgraphs_.size() - 1);
        return subgraphs_.size() - 1;
    }

    std::size_t createNode( Node::Value_t value_type, const std::string& identifier = "")
    {
        if ( identifier == "" )
        {
            nodes_.emplace_back( "tmp#" + std::to_string( nodes_.size()), value_type);
        } else
        {
            nodes_.emplace_back( identifier, value_type);
        }

        std::size_t node_index = nodes_.size() - 1;
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
        dumpSubgraph( stream, subgraph_);

        for ( auto it = edges_.begin(); it != edges_.end(); it++ )
        {
            dumpEdge( stream, *it);
        }
    }

  private:

    void dumpNode( FILE* stream, const Node& node) const
    {
        std::fprintf( stream, "\"%s\"", node.identifier.c_str());
        switch ( node.value_type )
        {
          case Node::Value_t::Temporary:
            std::fprintf( stream, "\n");
            break;
          case Node::Value_t::LValue:
            std::fprintf( stream, "\n");
            break;
          case Node::Value_t::LVRef:
            std::fprintf( stream, "\n");
            break;
          case Node::Value_t::RVRef:
            std::fprintf( stream, "\n");
            break;
          default:
            break;
        }
    }

    void dumpEdge( FILE* stream, const Edge& edge) const
    {
        std::fprintf( stream, "\"%s\"->\"%s\"",
                      getNode( edge.neighbours.first).identifier.c_str(),
                      getNode( edge.neighbours.second).identifier.c_str());

        switch ( edge.pass_type )
        {
          case Edge::Pass_t::LVRef:
            std::fprintf( stream, "");
            break;
          case Edge::Pass_t::ConstLVRef:
            std::fprintf( stream, "");
            break;
          case Edge::Pass_t::RVRef:
            std::fprintf( stream, "");
            break;
          case Edge::Pass_t::ConstRVRef:
            std::fprintf( stream, "");
            break;
          case Edge::Pass_t::Copy:
            std::fprintf( stream, "");
            break;
          case Edge::Pass_t::Move:
            std::fprintf( stream, "");
            break;
          default:
            break;
        }
    }

    void dumpSubgraph( FILE* stream, const Subgraph& subgraph) const
    {
        std::fprintf( stream, "subgraph \"%s\" {\n", subgraph.identifier.c_str());
        for ( auto it = subgraph.nodes.begin(); it != subgraph.nodes.end(); it++ )
        {
            dumpNode( stream, nodes_.at(*it));
        }

        for ( auto it = subgraph.child_subgraphs.begin(); it != subgraph.child_subgraphs.end(); it++ )
        {
            dumpSubgraph( stream, subgraphs_.at(*it));
        }

        std::fprintf( stream, "}\n");
    }
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
Proxy<T>::Proxy( const Proxy& other)
  : value_{ other.value_},
    node_index_{ gGraph.createNode( Node::Value_t::Temporary)}
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


#define dfg_v( __v) Int::gGraph.getNode().identifier = #__v;
#define dfg_p( __v, __type) Int::gGraph.createNode( __type, #__v);
#define dfg_e( __v) Int::gGraph.getNode().identifier = #__v;

int
main()
{
    TRACE_CALL

//    Int b = 23; dfg_var( b);
 //   Int c = 23; dfg_var( c);

  //  Int result = b + c; dfg_var( result);

   // Int copy_elision = func(); dfg_var( copy_elision);

    Int a{ 0}; dfg_v( a);
    Int b = a; dfg_v( b);
    Int c{ 0}; dfg_v( c);
    Int d{ 0}; dfg_v( d);

    dfg_p( arr, dfg::Node::Value_t::LValue);

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

