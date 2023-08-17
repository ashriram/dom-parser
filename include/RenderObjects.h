#include <algorithm>
#include <vector>
/**
 * @class Box
 * @brief Base class representing a box element in a layout hierarchy.
 *
 * The Box class defines the common properties and behavior for all box
 * elements. It provides the layout method that derived classes must implement
 * to calculate the size and position of the box based on its constraints.
 */
class Box {
public:
  virtual ~Box() = default;

  // Constraints
  int minWidth = 0, minHeight = 0, maxWidth = INT32_MAX, maxHeight = INT32_MAX;
  // Parent Constraints
  int parentMinWidth = 0, parentMaxWidth = 0, parentMinHeight = 0,
      parentMaxHeight = 0;
  int height, width;
  bool isroot;
  int x = 0, y = 0;
  double flex = 0, fixed = 0;
  uint32_t id;
  virtual void setConstraints(int parentMinWidth, int parentMaxWidth,
                              int parentMinHeight, int parentMaxHeight) = 0;
  virtual void prelayout(int serial) = 0; // Pure virtual function

  virtual void setPosition(int x, int y) = 0;
};

/**
 * @class SizedBox
 * @brief Represents a box with a fixed size.
 *
 * The SizedBox class is a derived class of Box that represents a box with a
 * fixed width and height. It can be used to create empty or spacer boxes with
 * specific dimensions.
 */
class SizedBox : public Box {
public:
  SizedBox(int width, int height, uint32_t id) {
    this->width = width;
    this->height = height;
    this->id = id;
  }

  void setConstraints(int parentMinWidth, int parentMaxWidth,
                      int parentMinHeight, int parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void prelayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided by
    // the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Clamp the width and height to the specified dimensions
    width = width;   // std::clamp(width, minWidth, maxWidth);
    height = height; // std::clamp(height, minHeight, maxHeight);
  }

  void postlayout() {
    // Nothing to do
  }

  void setPosition(int x, int y) override {
    this->x = 0;
    this->y = 0;
  }
};

/**
 * @class PaddingBox
 * @brief Represents a box that adds padding to a single child box.
 *
 * The PaddingBox class is a derived class of Box that adds padding to a single
 * child box. It allows specifying padding values for the left, right, top, and
 * bottom sides of the child box.
 */
class PaddingBox : public Box {
public:
  Box *child = nullptr;
  int paddingLeft = 0, paddingRight = 0, paddingTop = 0, paddingBottom = 0;

  PaddingBox(Box *child, int paddingLeft, int paddingRight, int paddingTop,
             int paddingBottom, uint32_t id) {
    this->child = child;
    this->paddingLeft = paddingLeft;
    this->paddingRight = paddingRight;
    this->paddingTop = paddingTop;
    this->paddingBottom = paddingBottom;
    this->id = id;
  }

  void setConstraints(int parentMinWidth, int parentMaxWidth,
                      int parentMinHeight, int parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void prelayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided by
    // the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Calculate the constraints for the child, taking into account the padding
    int childMinWidth = minWidth - paddingLeft - paddingRight;
    int childMaxWidth = maxWidth - paddingLeft - paddingRight;
    int childMinHeight = minHeight - paddingTop - paddingBottom;
    int childMaxHeight = maxHeight - paddingTop - paddingBottom;

    // If there is a child, lay it out within the adjusted constraints
    if (child) {
      child->prelayout(serial);
    }
    postlayout();
  };

  void postlayout() {
    if (child) {
      // Add the padding back to compute the size of this box
      width = child->width + paddingLeft + paddingRight;
      height = child->height + paddingTop + paddingBottom;
    } else {
      // If there is no child, the box's size is just the padding
      width = paddingLeft + paddingRight;
      height = paddingTop + paddingBottom;
    }
  };

  void setPosition(int x, int y) override {
    this->x = paddingLeft;
    this->y = paddingTop;
    if (child) {
      child->setPosition(x + paddingLeft, y + paddingTop);
    }
  }
};

/**
 * @class StackChild
 * @brief Represents a child box within a StackBox with alignment properties.
 *
 * The StackChild class wraps a Box and provides additional properties for
 * alignment within a StackBox. It allows specifying horizontal and vertical
 * alignment values to position the child within the stack.
 */
class StackChild {
public:
  Box *child;
  float horizontalAlignment;
  float verticalAlignment;

  StackChild(Box *child, float horizontalAlignment, float verticalAlignment)
      : child(child), horizontalAlignment(horizontalAlignment),
        verticalAlignment(verticalAlignment) {}
};

/**
 * @class StackBox
 * @brief Represents a box that stacks its children on top of each other.
 *
 * The StackBox class is a derived class of Box that stacks its child boxes on
 * top of each other. The children can overlap, and their positions and sizes
 * can be adjusted using alignment properties.
 */
class StackBox : public Box {
public:
  std::vector<StackChild> children;

  void setConstraints(int parentMinWidth, int parentMaxWidth,
                      int parentMinHeight, int parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void prelayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided by
    // the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Initialize the dimensions of this box
    width = 0;
    height = 0;

    // Lay out each child within this box's constraints, then adjust the
    // dimensions of this box to encompass all children
    for (StackChild &stackChild : children) {
      Box *child = stackChild.child;
      child->setConstraints(minWidth, maxWidth, minHeight, maxHeight);
      child->prelayout(serial);
      width = std::max(width, child->width +
                                  (int)std::abs(stackChild.horizontalAlignment *
                                                child->width));
      height = std::max(height, child->height +
                                    (int)std::abs(stackChild.verticalAlignment *
                                                  child->height));
    }
  }

  void setPosition(int x, int y) override {
    this->x = 0;
    this->y = 0;
    for (StackChild &stackChild : children) {
      stackChild.child->setPosition(x, y);
    }
  }
};

/**
 * @class ContainerBox
 * @brief Represents a box that contains a single child box with optional
 * padding and margin.
 *
 * The class is a derived class of Box that represents a box
 * element with a single child. It allows setting padding and margin values that
 * affect the child's layout and size calculations.
 */
class ContainerBox : public Box {
public:
  Box *child = nullptr;
  int paddingLeft = 0, paddingRight = 0, paddingTop = 0, paddingBottom = 0;
  int marginLeft = 0, marginRight = 0, marginTop = 0, marginBottom = 0;

  ContainerBox(Box *child, int paddingLeft, int paddingRight, int paddingTop,
               int paddingBottom, int marginLeft, int marginRight,
               int marginTop, int marginBottom, uint32_t id)
      : child(child), paddingLeft(paddingLeft), paddingRight(paddingRight),
        paddingTop(paddingTop), paddingBottom(paddingBottom),
        marginLeft(marginLeft), marginRight(marginRight), marginTop(marginTop),
        marginBottom(marginBottom) {
    this->id = id;
  }

  void setConstraints(int parentMinWidth, int parentMaxWidth,
                      int parentMinHeight, int parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void prelayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided by
    // the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Calculate the constraints for the child, taking into account the padding
    // and margin
    int childMinWidth =
        minWidth - paddingLeft - paddingRight - marginLeft - marginRight;
    int childMaxWidth =
        maxWidth - paddingLeft - paddingRight - marginLeft - marginRight;
    int childMinHeight =
        minHeight - paddingTop - paddingBottom - marginTop - marginBottom;
    int childMaxHeight =
        maxHeight - paddingTop - paddingBottom - marginTop - marginBottom;

    // If there is a child, lay it out within the adjusted constraints
    if (child) {
      child->setConstraints(childMinWidth, childMaxWidth, childMinHeight,
                            childMaxHeight);
      // Add the padding and margin back to compute the size of this box
    }

    if (serial) {
      // Serial version
      if (child) {
        child->prelayout(serial);
      }
      postlayout();
    }
  }

  void postlayout() {
    if (child) {
      width =
          child->width + paddingLeft + paddingRight + marginLeft + marginRight;
      height =
          child->height + paddingTop + paddingBottom + marginTop + marginBottom;
    } else {
      // If there is no child, the box's size is just the padding plus margin
      width = paddingLeft + paddingRight + marginLeft + marginRight;
      height = paddingTop + paddingBottom + marginTop + marginBottom;
    }
    if (isroot) {
      width = parentMinWidth;
      height = parentMinHeight;
    }
  }

  void setPosition(int x, int y) override {
    this->x = 0;
    this->y = 0;
    if (child) {
      child->setPosition(x + marginLeft + paddingLeft,
                         y + marginTop + paddingTop);
    }
  }
};

/**
 * @class RowBox
 * @brief Represents a box that lays out its children horizontally in a row.
 *
 * The RowBox class is a derived class of Box that arranges its child boxes in a
 * horizontal row. The children are evenly spaced within the row, and their
 * widths can be adjusted based on their constraints.
 */
class RowBox : public Box {
public:
  std::vector<Box*> children;
  int availableWidth = 0;

  void setConstraints(int parentMinWidth, int parentMaxWidth,
                      int parentMinHeight, int parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void prelayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided by
    // the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Decide on a height for this box within the constraints
    height = maxHeight;

    // Calculate the width available for flexible child boxes
     availableWidth = maxWidth;

    
    // Fixed children constraints
    for (const auto &child : children) {
      if (child->flex == 0.0) {
        child->setConstraints(child->parentMinWidth, child->parentMaxWidth,
                              height, height);
      }
    }

    if (serial) {
      // Serial version
      // Invoke prelayout on fixed children
      for (const auto &child : children) {
        if (child->flex == 0.0) {
          child->prelayout(serial);
        }
      }
    }
    // postlayout after fixed children
    // Calculate width and set constraints for flexible children
    if (serial) {
      postlayout_fixed();
    }

    // Invoke prelayout on flex children
    if (serial) {
      for (const auto &child : children) {
        if (child->flex > 0.0)
        child->prelayout(serial);
      }
    }
  }

  void postlayout_fixed() {
    // Calculate the width available for flexible child boxes
    for (const auto &child : children) {
      if (child->flex == 0.0) {
        availableWidth -= child->width;
      }
    }

    // Calculate the total flex value of the children
    double totalFlex = 0.0;
    for (const auto &child : children) {
      totalFlex += child->flex;
    }

    // Calculate the width available for flexible child boxes
    double flexChunkWidth = availableWidth/totalFlex;

    // flex children constraints
    for (const auto &child : children) {
      assert(availableWidth >= 0);
      if (child->flex > 0.0) {
        double flexWidth = flexChunkWidth * child->flex;
        availableWidth = availableWidth - flexWidth;
        child->setConstraints(flexWidth, flexWidth, height, height);
      }
    }
    width = maxWidth - availableWidth;
  }


  void setPosition(int x, int y) override {
    this->x = 0;
    this->y = 0;
    int childX = x;
    for (const auto &child : children) {
      child->setPosition(childX, y);
      childX += child->width;
    }
  }
};

/**
 * @class ColumnBox
 * @brief Represents a box that lays out its children vertically in a column.
 *
 * The ColumnBox class is a derived class of Box that arranges its child boxes
 * in a vertical column. The children are evenly spaced within the column, and
 * their heights can be adjusted based on their constraints.
 */
class ColumnBox : public Box {
public:
  std::vector<Box *> children;

  void setConstraints(int parentMinWidth, int parentMaxWidth,
                      int parentMinHeight, int parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void prelayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided by
    // the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Decide on a width for this box within the constraints.
    width = maxWidth;

    // Calculate the total flex value of the children
    double totalFlex = 0.0;
    for (Box *child : children) {
      totalFlex += (*child).flex;
    }

    // Calculate the height available for flexible child boxes
    int availableHeight = maxHeight;
    for (Box *child : children) {
      if (child->flex > 0.0) {
        double flexHeight = availableHeight * (*child).flex / totalFlex;
        child->setConstraints(0, width, 0, flexHeight);
      } else {
        child->setConstraints(0, width, 0, child->minHeight);
      }
      child->prelayout(serial);
      availableHeight -= child->height;
    }
    // This box's height is the combined height of all children
    height = maxHeight - availableHeight;
  }

  void setPosition(int x, int y) override {
    this->x = 0;
    this->y = 0;
    int childY = y;
    for (Box *child : children) {
      child->setPosition(x, childY);
      childY += child->height;
    }
  }
};
