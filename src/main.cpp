//    Copyright 2020 Mayank Mathur (Mynk-9 at Github)

//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at

//        http://www.apache.org/licenses/LICENSE-2.0

//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

/*
THIS FILE IS FOR TESTING PURPOSES ONLY,
AND DOES NOT CONTRIBUTE TO THE LIBRARY.
THE CODE HERE IS NOT DOCUMENTED.
*/

#include "DOMnode.hpp"
#include "DOMparser.hpp"
#include "DOMtree.hpp"
#include "taskflow/taskflow.hpp"
#include <CLI11/CLI11.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
std::atomic<size_t> counter{0};
/**
 * Use the DOM_PARSER_DEBUG_MODE to get debug output in console.
 * It would reduce output speed depending on the size of input file.
 * */
// #define DOM_PARSER_DEBUG_MODE
// #include "./test/test.hpp"
// #undef DOM_PARSER_DEBUG_MODE

void postTask(dom_parser::DOMnode &node) {
  std::cout << node.getTagName() << std::endl;
  std::cout << "post task\n";
  // Post task based on child updates
}

inline void debug_print(string s) {
  std::cout << "\n\tloadTest: " << s << "\n";
}

void printNode(dom_parser::DOMnode &node, dom_parser::DOMtree &tree,
               tf::Taskflow &taskflow, std::vector<tf::Task> &tasks,
               tf::Task &parent, tf::Task *pre_parent) {
  std::cout << node.getTagName() << std::endl;

  tf::Task pretsk = taskflow
                        .emplace([&]() {
                          // task = task
                          postTask(tree.getNode(node.getUID()));
                        })
                        .name("Pre|"+tree.getNode(node.getUID()).getTagName()+"|"+std::to_string(node.getUID()));

  if (pre_parent != nullptr) {
    pre_parent->precede(pretsk);
  }

  if (node.getChildrenUID().empty()) {
    pretsk.precede(parent);
  }

  for (auto child : node.getChildrenUID()) {
    tf::Task tsk = taskflow
                       .emplace([&]() {
                         // task = task
                         postTask(tree.getNode(child));
                       })
                       .name("Pst|"+tree.getNode(child).getTagName()+"|"+std::to_string(tree.getNode(child).getUID()));
    tasks.push_back(tsk);
    tsk.precede(parent);
    printNode(tree.getNode(child), tree, taskflow, tasks, tsk, &pretsk);
  }
}

int main(int argc, char **argv) {

  CLI::App app{"BinaryTree"};
  std::string model = "ebay.xml";
  app.add_option("-m,--dom-file-path", model, "./test/ebay.xml");
  CLI11_PARSE(app, argc, argv);
  std::cout << model;
  ofstream fout;
  dom_parser::DOMparser parser;
  string output_file = "./output.xml";
  filesystem::path file = filesystem::path(model);
  auto timer_start = chrono::steady_clock::now();
  int e;
  e = parser.loadTree(file);
  auto timer_stop = chrono::steady_clock::now();

  debug_print("PARSER RETURN VALUE: " + e);
  if (e != 0) {
    debug_print("Error: At line number: " + e);
    return -1;
  }

  debug_print("Loading done.");
  long long total_time =
      chrono::duration_cast<chrono::milliseconds>(timer_stop - timer_start)
          .count();

  // print the output
  //   debug_print("Writing output...");
  //   fout.open(output_file);
  //   fout << parser.getOutput();
  //   fout.close();
  //   debug_print("Output is present in: " + output_file);
  //std::cout << parser.getOutput();
  tf::Executor executor(2);
  std::vector<tf::Task> tasks;
  tf::Taskflow taskflow;

  dom_parser::DOMtree domtree = parser.getTree();
  dom_parser::DOMnodeUID uid = 0;

  dom_parser::DOMnode node = domtree.getNode(uid);

  tf::Task root =
      taskflow
          .emplace([&]() { counter.fetch_add(1, std::memory_order_relaxed); })
          .name(node.getTagName());
  printNode(node, domtree, taskflow, tasks, root, nullptr);

  taskflow.dump(std::cout);
  // auto beg = std::chrono::high_resolution_clock::now();
  // executor.run(taskflow).get();
  // auto end = std::chrono::high_resolution_clock::now();
  // auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - beg);

  // std::cout << "Counter " << counter << std::endl;
  // assert(counter + 1 == tasks.size());

  //   loadTest.set_file(model);
  //   //   int select_file = 2;

  //   //   loadTest.set_file("./test/" + files[select_file]);
  //   loadTest.set_verbose(true);
  //   long long ms = loadTest.run(output_file);
  // cout << "Completed Load Test on " << model << " in " << total_time
  //  << " milliseconds.\n";

  // cout << "Completed DOM processing in " << time.count() << " microseconds.\n";

  return 0;
}

// this file is for testing purposes