#include "RenderObjects.h"
#include "taskflow/taskflow.hpp"
#include <iomanip>
#include <iostream>

int main() {
  std::unique_ptr<RowBox> row154a4 = std::make_unique<RowBox>();
  row154a4.get()->isroot = false;
  row154a4.get()->id = 0x154a4;
  row154a4.get()->children.reserve(20);
  for (int i = 0; i < 20; i++) {
    row154a4->children.push_back(new ContainerBox(nullptr, 0,0,0,0,0,0,0,0, 0xac347 + i));
    auto child = row154a4->children[i];
    child->flex = 1;
    child->isroot = true;
  }

  std::unique_ptr<ContainerBox> root = std::make_unique<ContainerBox>(
      row154a4.get(), 0, 0, 0, 0, 0, 0, 0, 0, 0x154a4);
  root->setConstraints(1262, 1262, 684, 684); // Viewport size
  root.get()->isroot =
      true; // Set root to true. This is used to expand root to occupy viewport.

  auto beg = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; i++) {
    root->prelayout(1); // Perform layout algorithm
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg)/1000;
  std::cout << "Completed DOM processing in " << time.count()
            << "nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates. Currently sets global coordinates
  std::cout << std::setw(4) << std::hex << root->tojson();


  //  Test Parallel Version
  std::unordered_map<std::string,tf::Task> taskmap;
  tf::Taskflow taskflows;
  taskmap["root"] = taskflows.emplace([&]() { root->prelayout(0); }).name("root");
  taskmap["root_p"] =
      taskflows.emplace([&]() { root->postlayout(); }).name("root_p");

  taskmap["row154a4"] = taskflows.emplace([&]() { row154a4->prelayout(0); }).name("row154a4");
  taskmap["row154a4_p"] =
      taskflows.emplace([&]() { row154a4->postlayout(); }).name("row154a4_p");

  for(auto &child: row154a4->children) {
    std::stringstream ss_pre,ss_post;
    ss_pre << "#" << std::hex << std::setfill('0') << std::setw(6) << child->id;
    taskmap[ss_pre.str()] = taskflows.emplace([&]() { child->prelayout(0); }).name(ss_pre.str());

    ss_post << "p#" << std::hex << std::setfill('0') << std::setw(6) << child->id;
    taskmap[ss_post.str()] =
        taskflows.emplace([&]() { child->postlayout(); }).name(ss_post.str());
  
    taskmap["row154a4"].precede(taskmap[ss_pre.str()]); // [0
    taskmap[ss_pre.str()].precede(taskmap[ss_post.str()]); // [1]
    taskmap[ss_post.str()].precede(taskmap["row154a4_p"]); // [2]
  }

  taskmap["root"].precede(taskmap["row154a4"]); // [0]
  taskmap["row154a4_p"].precede(taskmap["root_p"]); // [3]

  std::ofstream fout;
  fout.open("test_rows.dot");
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
  std::cout << std::setw(4) << std::hex << root->tojson();

  return 0;
}

