#include "RenderObjects.h"
#include "taskflow/taskflow.hpp"
#include <iomanip>
#include <iostream>

int main() {

  //  Nodes being created.
  std::unique_ptr<ColumnBox> row154a4 = std::make_unique<ColumnBox>();
  row154a4.get()->isroot = false;
  row154a4.get()->children.reserve(20);
  for (int i = 0; i < 20; i++) {
    row154a4->children.push_back(
        new ContainerBox(nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0xac347 + i));
    auto child = row154a4->children[i];
    child->flex = 1;
    child->isroot = true;
    // child->setTaskID();
  }
  std::unique_ptr<ContainerBox> root = std::make_unique<ContainerBox>(
      row154a4.get(), 0, 0, 0, 0, 0, 0, 0, 0, 0x0);
  root->setConstraints(1262, 1262, 684, 684); // Viewport size
  root.get()->isroot =
      true; // Set root to true. This is used to expand root to occupy viewport.

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

  // Provide task name for those that do not have it in constructor
  root->setTaskID(0);
  row154a4.get()->setTaskID(0x154a4);

  // Creates a single task for everything nested under this node.
  row154a4->flatten = true;
  // root->flatten = true;

  // Create taskflow
  root->getTasks(taskmap, taskflows);

  // Visualize task graph
  std::ofstream fout;
  fout.open("test_tasks.dot");
  taskflows.dump(fout);
  fout.close();

  tf::Executor executor_1(4);
  beg = std::chrono::high_resolution_clock::now();
  executor_1.run_n(taskflows, 1000).wait();
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  std::cout << std::dec
            << "8 thread DOM processing 1x work: " << time.count() / 1000
            << " nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates.
  std::cout << std::setw(4) << std::hex << root->toJson();

  // std::cout << root->setTaskID();
  return 0;
}
