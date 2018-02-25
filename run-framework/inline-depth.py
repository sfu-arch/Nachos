import sys, pprint
import networkx as nx


def getNodeByLabel(CG, name):
    for n in CG:
        if 'label' not in CG.node[n].keys():
            continue
        if CG.node[n]['label'].find(name)  != -1:
            return n 
    assert False


def main(filename, fnname):
    """
    1. Remove recursion edges
    2. Do a DFS from the function of interest to 
    find the reachable nodes 
    3. Create a new dag with the function as root,
    and retain all edges which exist between pairs
    of nodes which are reachable.
    4. Run the longest path on the new DAG
    """
    CG = nx.DiGraph(nx.drawing.nx_agraph.read_dot(filename))
    CG.remove_edges_from([e for e in CG.edges_iter() if e[0] == e[1]])
    assert nx.algorithms.is_directed_acyclic_graph(CG) 

    n = getNodeByLabel(CG, fnname) 
    r = nx.dfs_preorder_nodes(CG, n)
    r = set(r)
    G = nx.DiGraph([ e for e in CG.edges_iter() if e[0] in r and e[1] in r ])
    print nx.dag_longest_path_length(G)




if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
