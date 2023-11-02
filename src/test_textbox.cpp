#include "RenderObjects.h"
#include "taskflow/taskflow.hpp"
#include <iomanip>
#include <iostream>

int main() {
  FT_Library library;
  FT_Init_FreeType(&library);
  int num_children = 8;
  std::unique_ptr<ColumnBox> row154a4 = std::make_unique<ColumnBox>();
  row154a4->isroot = false;
  row154a4->setTaskID(0x154a4);
  row154a4->children.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    std::string text = "* Maintained";
    
    std::unique_ptr<TextBox> textbox(new TextBox(text, library, "Helvetica-Bold.ttf", 16, 1.2, 0xac347 + i));
    
    row154a4->children.push_back(std::move(textbox));
    row154a4->children[i]->flex = 1;
    row154a4->children[i]->isroot = false;
  }

  std::unique_ptr<ContainerBox> root = std::make_unique<ContainerBox>(
      std::move(row154a4), 0, 0, 0, 0, 0, 0, 0, 0, 0x0);
  root->setConstraints(1262, 1262, 684, 684); // Viewport size
  root.get()->isroot =
      true; // Set root to true. This is used to expand root to occupy viewport.
  root->setTaskID(0);
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

  std::ofstream jsonfile;
  jsonfile.open("serial.json");
  jsonfile << std::setw(4) << std::hex << root->toJson();
  jsonfile.close();

  // //  Test Parallel Version
  std::unordered_map<std::string, tf::Task> taskmap;
  tf::Taskflow taskflows;
  // taskmap[root->ltask] =
  //     taskflows.emplace([&]() { root->preLayout(0); }).name(root->ltask);

  // taskmap[root->ptask] =
  //     taskflows.emplace([&]() { root->postLayout(); }).name(root->ptask);

  // // row154a4->getTasks(taskmap,taskflows);

  // taskmap[row154a4.get()->ltask] =
  //     taskflows.emplace([&]() { row154a4.get()->preLayout(0); })
  //         .name(row154a4.get()->ltask); 
  // taskmap[row154a4.get()->ptask] =
  //     taskflows.emplace([&]() { row154a4.get()->postLayout(); })
  //         .name(row154a4.get()->ptask);

  // for (auto &child : row154a4->children) {
  //   taskmap[child->ltask] =
  //       taskflows.emplace([&]() { child->preLayout(0); }).name(child->ltask);
  //   taskmap[child->ptask] =
  //       taskflows.emplace([&]() { child->postLayout(); }).name(child->ptask);

  //   taskmap[row154a4->ltask].precede(taskmap[child->ltask]); // [0
  //   taskmap[child->ltask].precede(taskmap[child->ptask]);    // [1]
  //   taskmap[child->ptask].precede(taskmap[row154a4->ptask]); // [2]
  // }

  // taskmap[root->ltask].precede(taskmap[row154a4->ltask]); // [0]
  // taskmap[row154a4->ptask].precede(taskmap[root->ptask]); // [3]

  root->getTasks(taskmap, taskflows);
  std::ofstream fout;
  fout.open("test_cols.dot");
  taskflows.dump(fout);
  fout.close();

  tf::Executor executor_1(8);
  beg = std::chrono::high_resolution_clock::now();
  executor_1.run_n(taskflows, 1000).wait();
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  std::cout << std::dec
            << "8 thread DOM processing 1x work: " << time.count() / 1000
            << " nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates. Currently sets global coordinates
  std::cout << std::setw(4) << std::hex << root->toJson();
  // std::ofstream jsonfile;
  jsonfile.open("parallel.json");
  jsonfile << std::setw(4) << std::hex << root->toJson();
  jsonfile.close();
  // FT_Done_Face(face);
  FT_Done_FreeType(library);

  // std::cout << root->ltask;
  return 0;
}
