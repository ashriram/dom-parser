#include "RenderObjects.h"
#include "taskflow/taskflow.hpp"
#include <iomanip>
#include <iostream>

void printBox(Box *box) {
  // Display id in hex with 0x in front atleast 6 digits
  // Display width and height
  // Display x and y

  std::cout << "Box id: #" << std::internal << std::setfill('0') << std::setw(5)
            << std::hex << box->id << " " << std::dec;
  std::cout << "Width: " << box->width << ", Height: " << box->height << " at ("
            << box->x << "," << box->y << ")" << std::endl;
}

int main() {

  std::unique_ptr<SizedBox> sizedBoxac347 =
      std::make_unique<SizedBox>(24, 24, 0xac347);
  //   sizedBoxac347->setConstraints(24.0, 24.0, 24.0, 24.0);

  std::unique_ptr<ContainerBox> container7060f = std::make_unique<ContainerBox>(
      sizedBoxac347.get(), 0, 0, 0, 0, 0, 0, 0, 0, 0x7060f);

  std::unique_ptr<PaddingBox> padding9c215 = std::make_unique<PaddingBox>(
      container7060f.get(), 4.0, 6.0, 5.0, 7.0, 0x9c215);

  std::unique_ptr<PaddingBox> padding3b44a = std::make_unique<PaddingBox>(
      padding9c215.get(), 8.0, 8.0, 8.0, 8.0, 0x3b44a);

  std::unique_ptr<PaddingBox> padding154a4 = std::make_unique<PaddingBox>(
      padding3b44a.get(), 16.0, 16.0, 16.0, 16.0, 0x154a4);

  std::unique_ptr<ContainerBox> root = std::make_unique<ContainerBox>(
      padding154a4.get(), 0, 0, 0, 0, 0, 0, 0, 0, 0x154a4);
  root->setConstraints(1262, 1262, 684, 684); // Viewport size
  root->isroot =
      true; // Set root to true. This is used to expand root to occupy viewport.
            //   root->preLayout(1);  // Perform layout algorithm
  //   root->setPosition(0, 0); // Set coordinates. Currently sets global
  //   coordinates

  std::vector<tf::Task> tasks;
  tf::Taskflow taskflows[10], serial_taskflow, multiple_taskflow;
  int idx = 0;

  auto t_serial = serial_taskflow.emplace([&]() { root->preLayout(1); }).name("root_serial");
  // Write a range iterator on taskflows
  for (auto &taskflow : taskflows) {

    // Prelayout Tasks
    auto t1 = taskflow.emplace([&]() { root->preLayout(1); }).name("root");
    // auto t2 = taskflow.emplace([&]() { padding154a4->preLayout(0); })
    //               .name("padding154a4");
    // auto t3 = taskflow.emplace([&]() { padding3b44a->preLayout(0); })
    //               .name("padding3b44a");
    // auto t4 = taskflow.emplace([&]() { padding9c215->preLayout(0); })
    //               .name("padding9c215");
    // auto t5 = taskflow.emplace([&]() { container7060f->preLayout(0); })
    //               .name("container7060f");
    // auto t6 = taskflow.emplace([&]() { sizedBoxac347->preLayout(0); })
    //               .name("sizedBoxac347");
    // t1.precede(t2);
    // t2.precede(t3);
    // t3.precede(t4);
    // t4.precede(t5);
    // t5.precede(t6);

    // auto t6_p = taskflow.emplace([&]() { sizedBoxac347->postLayout(); })
    //                 .name("sizedBoxac347_p");
    // auto t5_p = taskflow.emplace([&]() { container7060f->postLayout(); })
    //                 .name("container7060f_p");
    // auto t4_p = taskflow.emplace([&]() { padding9c215->postLayout(); })
    //                 .name("padding9c215_p");
    // auto t3_p = taskflow.emplace([&]() { padding3b44a->postLayout(); })
    //                 .name("padding3b44a_p");
    // auto t2_p = taskflow.emplace([&]() { padding154a4->postLayout(); })
    //                 .name("padding154a4_p");
    // auto t1_p = taskflow.emplace([&]() { root->postLayout(); }).name("root_p");

    // t6.precede(t6_p);
    // t6_p.precede(t5_p);
    // t5_p.precede(t4_p);
    // t4_p.precede(t3_p);
    // t3_p.precede(t2_p);
    // t2_p.precede(t1_p);

    multiple_taskflow.composed_of(taskflow);
  }

//   multiple_taskflow.composed_of(taskflow).name("m1");
//   multiple_taskflow.composed_of(taskflow2).name("m2");

//   multiple_taskflow.dump(std::cout);

  tf::Executor executor_1(1);
  auto beg = std::chrono::high_resolution_clock::now();
  executor_1.run_n(multiple_taskflow, 1000).wait();
  auto end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  std::cout << "1 thread DOM processing 20x work: " << time.count() / 1000
            << " nanoseconds.\n";

  tf::Executor executor_4(2);
  beg = std::chrono::high_resolution_clock::now();
  executor_4.run_n(multiple_taskflow, 1000).wait();
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  std::cout << "4 thread DOM processing in 20x work: " << time.count() / 1000
            << " nanoseconds.\n";

  //   taskflow.dump(std::cout);
  // Dump string to file
  std::ofstream fout;
  fout.open("multiple_taskflow.dot");
  multiple_taskflow.dump(fout);
  fout.close();

  beg = std::chrono::high_resolution_clock::now();
  executor_1.run_n(taskflows[0], 1000).wait();
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  std::cout << "1 thread DOM processing 1x work: " << time.count() / 1000
            << " nanoseconds.\n";

  beg = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < 1000; i++) {
    root->preLayout(1);
  }
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
//   std::cout << "Serial DOM processing 20x work: " << time.count() / 50
            // << " nanoseconds.\n";
  std::cout << "Serial DOM processing 1x work: " << time.count() / 1000
            << " nanoseconds.\n";

  beg = std::chrono::high_resolution_clock::now();
  executor_1.run_n(serial_taskflow, 1000).wait();
  end = std::chrono::high_resolution_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);
  //   std::cout << "Serial DOM processing 20x work: " << time.count() / 50
  // << " nanoseconds.\n";
  std::cout << "Task serial DOM processing 1x work: " << time.count() / 1000
            << " nanoseconds.\n";

  root->setPosition(0, 0);
  printBox(sizedBoxac347.get());
  printBox(container7060f.get());
  printBox(padding9c215.get());
  printBox(padding3b44a.get());
  printBox(padding154a4.get());
  printBox(root.get());

  // std::cout << "Counter " << counter << std::endl;
  // assert(counter + 1 == tasks.size());

  //   loadTest.set_file(model);
  //   //   int select_file = 2;

  //   //   loadTest.set_file("./test/" + files[select_file]);
  //   loadTest.set_verbose(true);
  //   long long ms = loadTest.run(output_file);
  // cout << "Completed Load Test on " << model << " in " << total_time
  //  << " milliseconds.\n";

  // cout << "Completed DOM processing in " << time.count() << "
  // microseconds.\n";

  return 0;
}

// int main() {
//   // Create the box hierarchy manually
//   std::unique_ptr<PaddingBox> paddingBox =
//       std::make_unique<PaddingBox>(nullptr, 5, 5, 5, 5);
//   std::unique_ptr<ContainerBox> containerBox =
//       std::make_unique<ContainerBox>(paddingBox.get(), 5, 5, 5, 5, 1, 1, 1,
//       1);

//   containerBox->setConstraints(0, 0, 70, 70);

//   containerBox->preLayout();
//   containerBox->setPosition(0,0);
//   // Perform layout algorithm
//   printBox(containerBox.get());
//   printBox(paddingBox.get());
//   // Print the layout
//   //   containerBox->printBox();

//   return 0;
// }