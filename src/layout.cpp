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

  printBox(sizedBoxac347.get());
  printBox(container7060f.get());
  printBox(padding9c215.get());
  printBox(padding3b44a.get());
  printBox(padding154a4.get());
  printBox(root.get());

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