import json
from graphviz import Digraph
import argparse

red_attributes = {"color": "red", "fillcolor": "red", "style": "filled"}

def get_attributes(json_tree):
    node_attributes = []
    for key, value in json_tree.items():
        if key not in ["id", "child", "children", "error"]:
            node_attributes.append(key + ":" + str(value))
    attr = {}
    attr["tooltip"] = "\n".join(node_attributes)
    return attr

def json_to_dot(json_tree, parent_name=None, graph=None):
    
    if graph is None:
        graph = Digraph('G')
    
    if isinstance(json_tree, dict):
        node_name = json_tree.get("id")
        attr = get_attributes(json_tree)                
        if not parent_name:  # If it's the root node
            parent_name = node_name
            if json_tree.get("error", 0) != 0:
                attr.update(red_attributes)
            graph.node(node_name, label=node_name, **attr)
            
                
        for key, value in json_tree.items():
            if key in ["child", "children"]:  # If key is "child" or "children", treat as nodes
                if isinstance(value, list):  # If the value is a list, iterate through each item
                    for item in value:
                        child_node_name = item.get("id")
                        attr = get_attributes(json_tree) 
                        if item.get("error",0) != 0:
                            attr.update(red_attributes)
                        graph.node(child_node_name, label=child_node_name, **(attr))
                        graph.edge(parent_name, child_node_name)
                        json_to_dot(item, child_node_name, graph)
                else:  # If the value is not a list, just create a single node
                    child_node_name = value.get("id")
                    attr = get_attributes(json_tree)
                    if value.get("error",0) != 0:
                        attr.update(red_attributes)
                    graph.node(child_node_name, label=child_node_name, **attr)
                    graph.edge(parent_name, child_node_name)
                    json_to_dot(value, child_node_name, graph)
    return graph

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert JSON to DOT')
    parser.add_argument('--json', type=str, help='JSON file')
    args = parser.parse_args()
    
    with open(args.json, 'r') as f:
        json_tree = json.load(f)
        dot_graph = json_to_dot(json_tree)
        # Change extension to svg
        # svg_filename = args.json.replace(".json", ".svg")
        dot_graph.render(cleanup=True, format="svg", filename=args.json.replace(".json", ""))
    
    
