import json
from graphviz import Digraph
import argparse

red_attributes = {"color": "red", "fillcolor": "red", "style": "filled", "dir": "back"}
green_edge = {"color": "green", "penwidth": "2", "fontsize" : "14", "dir": "back"}
red_edge = {"color": "red", "penwidth": "2", "fontsize" : "14", "dir": "back"}
blue_edge = {"color": "blue", "penwidth": "2", "fontsize" : "14", "dir": "back"}
black_edge = {"penwidth": "2", "fontsize" : "14", "dir": "back"}

def find_outliers(data_dict):
    # Convert dictionary values to a list
    values = list(data_dict.values())
    
    # Calculate Q1 and Q3
    Q1 = sorted(values)[int(len(values) * 0.25)]
    Q3 = sorted(values)[int(len(values) * 0.25)]
    
    # Compute IQR
    IQR = Q3 - Q1
    
    # Define upper bound for large outliers
    upper_bound = Q1 + 1.5 * IQR

    # Find large outliers
    outliers = {k: v for k, v in data_dict.items() if v > upper_bound}
    
    return outliers
    
def get_edge(percent):
    if percent > 0.30:
        return red_edge
    elif percent > 0.20:
        return blue_edge
    elif percent > 0.10:
        return green_edge
    else:
        return black_edge
    

def get_attributes(json_tree):
    node_attributes = []
    for key, value in json_tree.items():
        if key in ["time", "type"]:
            node_attributes.append(key + ":" + str(value))
    attr = {}
    attr["tooltip"] = "\n".join(node_attributes)
    return attr

def get_time(json_tree, parent_name, node_times=None):
    
    if node_times is None:
        node_times = {}
    
    if isinstance(json_tree, dict):
        node_name = json_tree.get("id")
        node_time = json_tree.get("time")
        attr = get_attributes(json_tree)                
        if parent_name == "":  # If it's the root node
            parent_name = node_name

        if "child" not in json_tree.keys() and "children" not in json_tree.keys():
            node_times[parent_name] = node_time

                
        for key, value in json_tree.items():
            if key in ["child", "children"]:  # If key is "child" or "children", treat as nodes
                if isinstance(value, list):  # If the value is a list, iterate through each item
                    child_times = [item["time"] for item in value]
                    # Sum the times
                    total_child_time = max(child_times)
                    node_times[parent_name] = node_time - total_child_time             
                    for item in value:
                        child_node_name = item.get("id")
                        get_time(item, child_node_name, node_times)
                else:  # If the value is not a list, just create a single node
                    child_node_name = value.get("id")       
                    total_child_time = value["time"]
                    node_times[parent_name] = node_time - total_child_time
                    get_time(value, child_node_name, node_times)

    return node_times

def json_to_dot(json_tree, parent_name=None, graph=None, top_10=[]):
    
    if graph is None:
        graph = Digraph('G')
    
    if isinstance(json_tree, dict):
        node_name = json_tree.get("id")
        node_time = json_tree.get("time")
        attr = get_attributes(json_tree)                
        if not parent_name:  # If it's the root node
            parent_name = node_name
        graph.node(node_name, label=node_name, **attr)
            
                
        for key, value in json_tree.items():
            if key in ["child", "children"]:  # If key is "child" or "children", treat as nodes
                if isinstance(value, list):  # If the value is a list, iterate through each item
                    
                    child_times = [item["time"] for item in value]
                    # Sum the times
                    total_child_time = sum(child_times)
                    # Calculate the percentage of the total time
                    # Round the percentage to 2 decimal places
                    id_time = {item["id"]: round(item["time"]/total_child_time,2) for item in value}
                    for item in value:
                        child_node_name = item.get("id")
                        attr = get_attributes(json_tree)
                        print(top_10)
                        print(child_node_name) 
                        if child_node_name in top_10:
                            attr.update(red_attributes)
                        graph.node(child_node_name, label=child_node_name, **(attr))
                        edge_attr = get_edge(id_time[child_node_name])
                        graph.edge(parent_name, child_node_name, label = str(id_time[child_node_name]), **(edge_attr))
                        json_to_dot(item, child_node_name, graph,top_10)
                else:  # If the value is not a list, just create a single node
                    child_node_name = value.get("id")
                    attr = get_attributes(json_tree)
                    # Check if node is in list
                    if  child_node_name in top_10:
                        attr.update(red_attributes)
                    graph.node(child_node_name, label=child_node_name, **attr)
                    graph.edge(parent_name, child_node_name, label = str(round(value["time"]/node_time,2)), **black_edge)
                    # graph.edge(parent_name, child_node_name)
                    json_to_dot(value, child_node_name, graph,top_10)
    return graph


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert JSON to DOT')
    parser.add_argument('--json', type=str, help='JSON file')
    args = parser.parse_args()
    
    with open(args.json, 'r') as f:
        json_tree = json.load(f)
        node_times = get_time(json_tree,"")
        # Find outlier items in dictionary based on value
        outlier_nodes = find_outliers(node_times)
        print(outlier_nodes)
        dot_graph = json_to_dot(json_tree,None,None,outlier_nodes.keys())
        # Change extension to svg
        svg_filename = args.json.replace(".json", ".svg")
        dot_graph.render(cleanup=True, format="svg", filename=args.json.replace(".json", ""))
    
    
