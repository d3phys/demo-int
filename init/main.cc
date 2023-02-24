#include <cstdio>
#include <string>
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <assert.h>

#include "../dfg/dfg.h"

using Int = dfg::Proxy<int>;

template <>
dfg::Graph Int::gGraph{};

#define dfg_val( __v, __type)                                     \
    do { Int::gGraph.getNode().identifier = #__v;                 \
         Int::gGraph.getNode().value_type = dfg::Node::Value_t::__type; } while ( 0 )

#define dfg_arg( __v, __val_type, __type)                                                       \
    do { Int::gGraph.createEdge( __v.getNodeIndex(),                                            \
                                 Int::gGraph.createNode( dfg::Node::Value_t::__val_type, #__v), \
                                 dfg::Edge::Pass_t::__type); } while ( 0 )

#define dfg_e( __v) Int::gGraph.getNode().identifier = #__v;
#define dfg_scope() dfg::Scope super_unique_scope{ Int::gGraph, __PRETTY_FUNCTION__}

Int Func( Int& param)
{
    dfg_scope();
    dfg_arg( param, LVRef, ConstLVRef);

    return param;
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
    {
        dfg_scope();
        Int a{ 0}; dfg_val( a, LValue);
        a = Func( a);
        Int b = Add( a, 3); dfg_val( b, LValue);
    }

    Int::gGraph.dump( stdout);
    return 0;
}
