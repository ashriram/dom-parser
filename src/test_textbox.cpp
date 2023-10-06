#include "RenderObjects.h"
#include "taskflow/taskflow.hpp"
#include <iomanip>
#include <iostream>

int main() {
  FT_Library library;
  FT_Init_FreeType(&library);
  int num_children = 8;
   std::unique_ptr<ColumnBox> row154a4 =
      std::make_unique<ColumnBox>();
  row154a4.get()->isroot = false;
  row154a4.get()->setTaskID(0x154a4);
  row154a4.get()->children.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    std::string text = 
    "* Maintained";
        row154a4->children.push_back(new TextBox(
            text, library, "Helvetica-Bold.ttf", 16, 1.2, 0xac347 + i));
    // row154a4->children.push_back(new ContainerBox(nullptr,
    // 0,0,0,0,0,0,0,0, 0xac347 + i));
    auto child = row154a4->children[i];
    child->flex = 1;
    child->isroot = false;
  }

  std::unique_ptr<ContainerBox> root = std::make_unique<ContainerBox>(
      row154a4.get(), 0, 0, 0, 0, 0, 0, 0, 0, 0x0);
  root->setConstraints(1262, 1262, 684, 684); // Viewport size
  root.get()->isroot =
      true; // Set root to true. This is used to expand root to occupy viewport.
  root->setTaskID(0);
  auto beg = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; i++) {
    root->preLayout(1); // Perform layout algorithm
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg)/1000;
  std::cout << "Completed DOM processing in " << time.count()
            << "nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates. Currently sets global coordinates
  std::cout << std::setw(4) << std::hex << root->toJson();


  //  Test Parallel Version
  std::unordered_map<std::string,tf::Task> taskmap;
  tf::Taskflow taskflows;
  taskmap[root->ltask] = taskflows.emplace([&]() { root->preLayout(0); }).name(root->ltask);

  taskmap[root->ptask] =
      taskflows.emplace([&]() { root->postLayout(); }).name(root->ptask);

  // row154a4->getTasks(taskmap,taskflows);

  taskmap[row154a4->ltask] = taskflows.emplace([&]() { row154a4->preLayout(0); }).name(row154a4->ltask);
  taskmap[row154a4->ptask] =
      taskflows.emplace([&]() { row154a4->postLayout(); }).name(row154a4->ptask);

  for(auto &child: row154a4->children) {
    taskmap[child->ltask] = taskflows.emplace([&]() { child->preLayout(0); }).name(child->ltask);
    taskmap[child->ptask] =
        taskflows.emplace([&]() { child->postLayout(); }).name(child->ptask);
  
    taskmap[row154a4->ltask].precede(taskmap[child->ltask]); // [0
    taskmap[child->ltask].precede(taskmap[child->ptask]); // [1]
    taskmap[child->ptask].precede(taskmap[row154a4->ptask]); // [2]
  }

  taskmap[root->ltask].precede(taskmap[row154a4->ltask]); // [0]
  taskmap[row154a4->ptask].precede(taskmap[root->ptask]); // [3]
  
  std::ofstream fout;
  fout.open("test_textbox.dot");
  taskflows.dump(fout);
  fout.close();

  for (int num_threads = 1; num_threads <= 8; num_threads = num_threads << 1) {
    tf::Executor executor_1(num_threads);
    beg = std::chrono::high_resolution_clock::now();
    executor_1.run_n(taskflows, 1000).wait();
    end = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
    std::cout << std::dec << num_threads
              << " thread DOM processing 1x work: " << time.count() / 1000
              << " nanoseconds.\n";

    root->setPosition(0, 0); // Set coordinates.
    std::cout << std::setw(4) << std::hex << root->toJson();
  }

  // FT_Done_Face(face);
  FT_Done_FreeType(library);

  // std::cout << root->ltask;
  return 0;
}

