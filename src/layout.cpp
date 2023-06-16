#include "RenderObjects.h"
#include <iostream>
void printBox(Box *box) {
  std::cout << "Width: " << box->width << ", Height: " << box->height << " at (" << box->x << "," << box->y << ")" << std::endl;
            
}

int main() {
  // Create the box hierarchy manually
  std::unique_ptr<PaddingBox> paddingBox =
      std::make_unique<PaddingBox>(nullptr, 5, 5, 5, 5);
  std::unique_ptr<ContainerBox> containerBox =
      std::make_unique<ContainerBox>(paddingBox.get(), 5, 5, 5, 5, 1, 1, 1, 1);

  containerBox->setConstraints(0, 0, 70, 70);

  containerBox->prelayout();
  containerBox->setPosition(0,0);
  // Perform layout algorithm
  printBox(containerBox.get());
  printBox(paddingBox.get());
  // Print the layout
  //   containerBox->printBox();

  return 0;
}