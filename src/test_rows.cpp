#include "RenderObjects.h"
#include <iomanip>
#include <iostream>

int main() {
  std::unique_ptr<RowBox> row154a4 = std::make_unique<RowBox>();
  row154a4.get()->isroot = false;
  row154a4.get()->id = 0x154a4;
  row154a4.get()->children.reserve(3);
  for (int i = 0; i < 3; i++) {
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

    return 0;
}

