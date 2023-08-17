#include "RenderObjects.h"
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

  // push_back(std::move(children));
      // padding3b44a.get(), 16.0, 16.0, 16.0, 16.0, 0x154a4);

  std::unique_ptr<ContainerBox> root = std::make_unique<ContainerBox>(
      row154a4.get(), 0, 0, 0, 0, 0, 0, 0, 0, 0x154a4);
  root->setConstraints(1262, 1262, 684, 684); // Viewport size
  root.get()->isroot =
      true; // Set root to true. This is used to expand root to occupy viewport.

  // auto beg = std::chrono::high_resolution_clock::now();
  // for (int i = 0; i < 1000; i++) {
    root->prelayout(1); // Perform layout algorithm
  // }
  // auto end = std::chrono::high_resolution_clock::now();
  // auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg)/1000;
  // std::cout << "Completed DOM processing in " << time.count()
  //           << "nanoseconds.\n";

  root->setPosition(0, 0); // Set coordinates. Currently sets global coordinates

  for (auto &child : row154a4->children) {
    printBox(child);
  }
    // printBox(sizedBoxac347.get());
    // printBox(container7060f.get());
    // printBox(padding9c215.get());
    // printBox(padding3b44a.get());
    // printBox(padding154a4.get());
    // printBox(root.get());

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

//   containerBox->prelayout();
//   containerBox->setPosition(0,0);
//   // Perform layout algorithm
//   printBox(containerBox.get());
//   printBox(paddingBox.get());
//   // Print the layout
//   //   containerBox->printBox();

//   return 0;
// }