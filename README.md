# DOM-Parser
 A small DOM(Document Object Model) Parsing library. Works on C++14 and above.
  
Current capabilities:
 1) Scan markup code.
 2) Store the markup data in a tree structure.
 3) Storage of the data is optimized in terms of space and time complexity for fast manipulations like moving whole subtrees and multiple deletions-additions of nodes/attributes. 
 4) Provide minfied or pretty-printed output.
 
 How it works:
 1) Input file is feeded to lexer which reads ahead of parser and creates and stores tokens in a buffer.
 2) Parser successively takes tokens from lexer and parses it.
 3) The parsed stuff is saved to a tree data structure.
 4) After whatever changes necessary being done to the DOM, user can have two options of printing out the document:  
     1) Minified
     2) Pretty-printed

Needed work:
  1) Currently it is not resilient to syntax errors.

# Parallelized DOM parser

[Taskflow](https://github.com/taskflow/taskflow)

## Dependencies
  1) C++14
  2) CMake
  3) Boost
  4) C++ taskflow library 



```bash
cmake -B Build .
cd Build; make
./src/dom_parser -m ../include/test/ebay.xml
TF_ENABLE_PROFILER=simple.json ./src/dom_parser -m ../include/test/ebay.xml
```

```bash
# Google Benchmark. Runs for threading levels 1-10.
cmake -B Build .
cd Build; make
./src/dom_bench

```

```
./src/dom_parser -m ../include/test/app.xml
```

![alt text](./example/layout.svg)
- Pre task (produce layout constraints for child)
- Post (based on child’s size, position child)
- Size (fused with post)
