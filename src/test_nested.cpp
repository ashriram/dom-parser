#include "RenderObjects.h"
#include "taskflow/taskflow.hpp"
#include <iomanip>
#include <iostream>

int main() {
  std::unique_ptr<ColumnBox> col154a4 = std::make_unique<ColumnBox>();
  int num_children = 500;
  col154a4.get()->isroot = false;
  col154a4.get()->children.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    col154a4->children.push_back(
        new ContainerBox(nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0xac347 + i));
    auto child = col154a4->children[i];
    child->flex = 1;
    child->isroot = false;
  }

  std::unique_ptr<ColumnBox> col154b4 = std::make_unique<ColumnBox>();
  col154b4.get()->isroot = false;
  col154b4.get()->children.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    col154b4->children.push_back(
        new ContainerBox(nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0xbc347 + i));
    auto child = col154b4->children[i];
    child->flex = 1;
    child->isroot = false;
  }

  std::unique_ptr<ColumnBox> col154c4 = std::make_unique<ColumnBox>();
  col154b4.get()->isroot = false;
  col154b4.get()->children.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    col154b4->children.push_back(
        new ContainerBox(nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0xbc347 + i));
    auto child = col154b4->children[i];
    child->flex = 1;
    child->isroot = false;
  }

  std::unique_ptr<ColumnBox> col154d4 = std::make_unique<ColumnBox>();
  col154b4.get()->isroot = false;
  col154b4.get()->children.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    col154b4->children.push_back(
        new ContainerBox(nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0xbc347 + i));
    auto child = col154b4->children[i];
    child->flex = 1;
    child->isroot = false;
  }

  std::unique_ptr<RowBox> row154e4 = std::make_unique<RowBox>();
  row154e4.get()->isroot = false;
  row154e4.get()->children.reserve(4);
  row154e4->children.push_back(col154a4.get());
  row154e4->children.push_back(col154b4.get());
  row154e4->children.push_back(col154c4.get());
  row154e4->children.push_back(col154d4.get());

  row154e4->children[0]->flex = 1;
  row154e4->children[1]->flex = 1;
  row154e4->children[2]->flex = 1;
  row154e4->children[3]->flex = 1;

  std::unique_ptr<ContainerBox> root = std::make_unique<ContainerBox>(
      row154e4.get(), 0, 0, 0, 0, 0, 0, 0, 0, 0x0);
  root->setConstraints(1262, 1262, 684, 684); // Viewport size
  root.get()->isroot =
      true; // Set root to true. This is used to expand root to occupy viewport.
  root->setTaskID(0);
  col154a4.get()->setTaskID(0x154a4);
  col154b4.get()->setTaskID(0x154b4);
  col154c4.get()->setTaskID(0x154c4);
  col154d4.get()->setTaskID(0x154d4);
  row154e4.get()->setTaskID(0x154e4);

  auto beg = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; i++) {
    root->preLayout(1); // Perform layout algorithm
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg) / 1000;
  std::cout << "Completed DOM processing in " << time.count()
            << "nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates. Currently sets global coordinates
  std::cout << std::setw(4) << std::hex << root->toJson();

  //  Test Parallel Version
  // Global task map
  std::unordered_map<std::string, tf::Task> taskmap;
  // Global taskflow for execution
  tf::Taskflow taskflows;

  // root->setTaskID(0);
 

  // Creates a single task for everything nested under this node.
  col154a4->flatten = true;
  col154b4->flatten = true;
  col154c4->flatten = true;
  col154d4->flatten = true;
  //row154e4->flatten = true;
  // root->flatten = true;

  // Create taskflow
  root->getTasks(taskmap, taskflows);

// col154a4->getTasks(taskmap,taskflows);
  // Visualize task graph
  std::ofstream fout;
  fout.open("test_nested.dot");
  taskflows.dump(fout);
  fout.close();

  // Get input for number of threads
  int num_threads = 4;
  std::cout << "Enter number of threads: ";
  std::cin >> num_threads;

  tf::Executor executor_1(num_threads);
  beg = std::chrono::high_resolution_clock::now();
  executor_1.run_n(taskflows, 1000).wait();
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  std::cout << std::dec <<
            num_threads << " thread DOM processing 1x work: " << time.count() / 1000
            << " nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates.
  std::cout << std::setw(4) << std::hex << root->toJson();

  // std::cout << root->setTaskID();
  return 0;

  // return 0;
}
