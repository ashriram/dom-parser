#include "RenderObjects.h"
#include "taskflow/taskflow.hpp"
#include <iomanip>
#include <iostream>

int main() {
  std::unique_ptr<ColumnBox> row154a4 = std::make_unique<ColumnBox>();
  row154a4.get()->isroot = false;
  row154a4.get()->id = 0x154a4;
  row154a4->getID();
  row154a4.get()->children.reserve(20);
  for (int i = 0; i < 20; i++) {
    row154a4->children.push_back(new ContainerBox(nullptr, 0,0,0,0,0,0,0,0, 0xac347 + i));
    auto child = row154a4->children[i];
    child->flex = 1;
    child->isroot = true;
    child->getID();
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
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg)/1000;
  std::cout << "Completed DOM processing in " << time.count()
            << "nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates. Currently sets global coordinates
  std::cout << std::setw(4) << std::hex << root->toJson();


  //  Test Parallel Version
  std::unordered_map<std::string,tf::Task> taskmap;
  tf::Taskflow taskflows;
  root->getID();
  root->getTasks(taskmap,taskflows);
  // taskmap[ltask] = (taskflows.emplace([&]() { root->preLayout(0); }).name(root->getID()));

  // taskmap[root->getID()+"_p"] =
  //     (taskflows.emplace([&]() { root->postLayout(); }).name(root->getID()+"_p"));

  row154a4->getTasks(taskmap,taskflows);

  // taskmap[row154a4->getID()] = taskflows.emplace([&]() { row154a4->preLayout(0); }).name(row154a4->getID());
  // taskmap[row154a4->getID()+"_p"] =
  //     taskflows.emplace([&]() { row154a4->postLayout(); }).name(row154a4->getID()+"_p");

  // for(auto &child: row154a4->children) {
  //   // std::stringstream ss_pre,ss_post;
  //   // ss_pre << "#" << std::hex << std::setfill('0') << std::setw(6) << child->id;
  //   taskmap[child->getID()] = taskflows.emplace([&]() { child->preLayout(0); }).name(child->getID());

  //   // ss_post << "p#" << std::hex << std::setfill('0') << std::setw(6) << child->id;
  //   taskmap[child->getID()+"_p"] =
  //       taskflows.emplace([&]() { child->postLayout(); }).name(child->getID()+"_p");
  
  //   taskmap[row154a4->getID()].precede(taskmap[child->getID()]); // [0
  //   taskmap[child->getID()].precede(taskmap[child->getID()+"_p"]); // [1]
  //   taskmap[child->getID()+"_p"].precede(taskmap[row154a4->getID()+"_p"]); // [2]
  // }

  taskmap[root->ltask].precede(taskmap[row154a4->ltask]); // [0]
  taskmap[row154a4->ptask].precede(
      taskmap[root->ptask]); // [3]

  std::ofstream fout;
  fout.open("test_tasks.dot");
  taskflows.dump(fout);
  fout.close();

  tf::Executor executor_1(4);
  beg = std::chrono::high_resolution_clock::now();
  executor_1.run_n(taskflows, 1000).wait();
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  std::cout << std::dec << "8 thread DOM processing 1x work: " << time.count() / 1000
            << " nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates. Currently sets global coordinates
  std::cout << std::setw(4) << std::hex << root->toJson();

  // std::cout << root->getID();
  return 0;
}

