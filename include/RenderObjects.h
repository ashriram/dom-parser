#include "taskflow/taskflow.hpp"
#include <algorithm>
#include <ext/json.hpp>
#include <sstream>
#include <vector>

using json = nlohmann::json;

#define HEXSTR(x) std::hex << std::setfill('0') << std::setw(6) << x

// Convert integer to hex string with 6 digits and return in string
std::string hexstr(uint32_t x) {
  std::stringstream ss;
  ss << "#" << std::hex << std::setfill('0') << std::setw(6) << x;
  return ss.str();
}

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
  float minWidth = 0, minHeight = 0, maxWidth = INT32_MAX,
        maxHeight = INT32_MAX;
  // Parent Constraints
  float parentMinWidth = 0, parentMaxWidth = 0, parentMinHeight = 0,
        parentMaxHeight = 0;
  float height, width;
  bool isroot;
  bool flatten;
  float x = 0, y = 0;
  double flex = 0, fixed = 0;
  uint32_t id;
  std::string ltask, ptask;
  virtual void setConstraints(float parentMinWidth, float parentMaxWidth,
                              float parentMinHeight, float parentMaxHeight) = 0;
  virtual void preLayout(int serial) = 0; // Pure virtual function

  virtual void setPosition(float x, float y) = 0;

  virtual void postLayout() = 0;

  // virtual void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
  //                       tf::Taskflow &) {

  //                       };

  virtual void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                        tf::Taskflow &tf) {
    taskmap[ltask] = (tf.emplace([this]() { preLayout(0); }).name(ltask));
    taskmap[ptask] = (tf.emplace([this]() { postLayout(); }).name(ptask));
    taskmap[ltask].precede(taskmap[ptask]);
  }

  virtual json toJson() = 0;

  virtual std::string setTaskID(uint32_t id) {
    this->id = id;
    ltask = hexstr(id);
    ptask = hexstr(id) + "_p";
    return hexstr(id);
  };
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
    ltask = hexstr(id);
    ptask = hexstr(id) + "_p";
  }

  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
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

  void postLayout() override { assert(0 && "Unimplemented function"); }

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
  }

  json toJson() override {
    json j;
    std::stringstream str;
    str << "#" << std::hex << std::setfill('0') << std::setw(6) << id;
    j["id"] = str.str();
    j["type"] = "sized";
    j["width"] = width;
    j["height"] = height;
    j["x"] = x;
    j["y"] = y;
    return j;
  }

  void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                tf::Taskflow &tf) override {
    taskmap[ltask] = tf.emplace([&]() { preLayout(0); }).name(ltask);
    taskmap[ptask] = tf.emplace([&]() { postLayout(); }).name(ptask);
    taskmap[ltask].precede((taskmap[ptask]));
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
  }
  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided
    // by the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Calculate the constraints for the child, taking into account the
    // padding
    int childMinWidth = minWidth - paddingLeft - paddingRight;
    int childMaxWidth = maxWidth - paddingLeft - paddingRight;
    int childMinHeight = minHeight - paddingTop - paddingBottom;
    int childMaxHeight = maxHeight - paddingTop - paddingBottom;

    // If there is a child, lay it out within the adjusted constraints
    if (child) {
      child->preLayout(serial);
    }
    postLayout();
  };

  void postLayout() override {
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

  void setPosition(float x, float y) override {
    this->x = paddingLeft;
    this->y = paddingTop;
    if (child) {
      child->setPosition(0 + paddingLeft, 0 + paddingTop);
    }
  }

  json toJson() override {
    json j;
    std::stringstream str;
    str << "#" << std::hex << std::setfill('0') << std::setw(6) << id;
    j["id"] = str.str();
    j["type"] = "padding";
    j["width"] = width;
    j["height"] = height;
    j["x"] = x;
    j["y"] = y;
    if (child) {
      j["child"] = child->toJson();
    }
    return j;
  }

  void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                tf::Taskflow &tf) override {
    taskmap[ltask] = tf.emplace([&]() { preLayout(0); }).name(ltask);
    taskmap[ptask] = tf.emplace([&]() { postLayout(); }).name(ptask);
    if (child) {
      child->getTasks(taskmap, tf);
      taskmap[ltask].precede((taskmap[child->ltask]));
      taskmap[child->ptask].precede((taskmap[ptask]));
    } else {
      taskmap[ltask].precede((taskmap[ptask]));
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

  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
    // Ensure that this box's constraints are within the constraints provided
    // by the parent
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
      child->preLayout(serial);
      width = std::max(width, child->width +
                                  (int)std::abs(stackChild.horizontalAlignment *
                                                child->width));
      height = std::max(height, child->height +
                                    (int)std::abs(stackChild.verticalAlignment *
                                                  child->height));
    }
  }

  void postLayout() override {
    // assert(0 && "Unimplemented function");
    // To be filled for enabling parallelism
  }

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
    for (StackChild &stackChild : children) {
      stackChild.child->setPosition(0, 0);
    }
  }

  json toJson() override {
    json j;
    std::stringstream str;
    str << "#" << std::hex << std::setfill('0') << std::setw(6) << id;
    j["id"] = str.str();
    j["type"] = "stack";
    j["width"] = width;
    j["height"] = height;
    j["x"] = x;
    j["y"] = y;
    json children;
    for (StackChild &stackChild : this->children) {
      children.push_back(stackChild.child->toJson());
    }
    j["children"] = children;
    return j;
  }
  // In stack; children have to be processed serially and post layout is dummy.
  void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                tf::Taskflow &tf) override {
    taskmap[ltask] = tf.emplace([&]() { preLayout(1); }).name(ltask);
    taskmap[ptask] = tf.emplace([&]() { postLayout(); }).name(ptask);
    taskmap[ltask].precede((taskmap[ptask]));
  }
};

/**
 * @class ContainerBox
 * @brief Represents a box that contains a single child box with optional
 * padding and margin.
 *
 * The class is a derived class of Box that represents a box
 * element with a single child. It allows setting padding and margin values
 * that affect the child's layout and size calculations.
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
    ltask = hexstr(id);
    ptask = hexstr(id) + "_p";
  }
  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
    // Ensure that this box's constraints are within the constraints
    // provided by the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Calculate the constraints for the child, taking into account the
    // padding and margin
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
        child->preLayout(serial);
      }
      postLayout();
    }
  }

  void postLayout() override {
    if (child) {
      width =
          child->width + paddingLeft + paddingRight + marginLeft + marginRight;
      height =
          child->height + paddingTop + paddingBottom + marginTop + marginBottom;
    } else {
      // If there is no child, the box's size is just the padding plus
      // margin
      width = paddingLeft + paddingRight + marginLeft + marginRight + minWidth;
      height = paddingTop + paddingBottom + marginTop + marginBottom + minHeight;
    }
    if (isroot) {
      width = parentMinWidth;
      height = parentMinHeight;
    }
  }

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
    if (child) {
      child->setPosition(0+marginLeft + paddingLeft,
                         0+marginTop + paddingTop);
    }
  }

  json toJson() override {
    json j;
    std::stringstream str;
    str << "#" << std::hex << std::setfill('0') << std::setw(6) << id;
    j["id"] = str.str();
    j["type"] = "container";
    j["width"] = width;
    j["height"] = height;
    j["x"] = this->x;
    j["y"] = this->y;
    if (child) {
      j["child"] = child->toJson();
    }
    return j;
  }

  void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                tf::Taskflow &tf) override {

    // Serialize execution to single task.
    if (flatten || !child) {
      taskmap[ltask] = tf.emplace([&]() { preLayout(1); }).name(ltask);
      taskmap[ptask] = tf.emplace([&]() {}).name(ptask);
      taskmap[ltask].precede((taskmap[ptask]));
      return;
    }

    // Multiple tasks if child exists.
    taskmap[ltask] = tf.emplace([&]() { preLayout(0); }).name(ltask);
    taskmap[ptask] = tf.emplace([&]() { postLayout(); }).name(ptask);
    child->getTasks(taskmap, tf);
    taskmap[ltask].precede((taskmap[child->ltask]));
    taskmap[child->ptask].precede((taskmap[ptask]));
  }
};

/**
 * @class RowBox
 * @brief Represents a box that lays out its children horizontally in a row.
 *
 * The RowBox class is a derived class of Box that arranges its child boxes
 * in a horizontal row. The children are evenly spaced within the row, and
 * their widths can be adjusted based on their constraints.
 */
class RowBox : public Box {
public:
  std::vector<Box *> children;
  int availableWidth = 0;

  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
    // Ensure that this box's constraints are within the constraints
    // provided by the parent
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
          child->preLayout(serial);
        }
      }
    }
    // postlayout after fixed children
    // Calculate width and set constraints for flexible children
    if (serial) {
      postLayout();
    }

    // Invoke prelayout on flex children
    if (serial) {
      for (const auto &child : children) {
        if (child->flex > 0.0)
          child->preLayout(serial);
      }
    }
  }

  void postLayout() override {
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
    double flexChunkWidth = availableWidth / totalFlex;

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

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
    float childX = 0;
    for (const auto &child : children) {
      child->setPosition(childX, 0);
      childX += child->width;
    }
  }

  json toJson() override {
    json j;
    std::stringstream str;
    str << "#" << std::hex << std::setfill('0') << std::setw(6) << id;
    j["id"] = str.str();
    j["type"] = "row";
    j["width"] = width;
    j["height"] = height;
    j["x"] = x;
    j["y"] = y;
    json children;
    for (const auto &child : this->children) {
      children.push_back(child->toJson());
    }
    j["children"] = children;
    return j;
  }

  void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                tf::Taskflow &tf) override {
    if (!flatten) {
      taskmap[ltask] = tf.emplace([&]() { preLayout(0); }).name(ltask);
      taskmap[ptask] = tf.emplace([&]() { postLayout(); }).name(ptask);
      for (auto &child : children) {
        child->getTasks(taskmap, tf);
        // std::cout<<"Preceding "<<child->setTaskID()<<" with
        // "<<setTaskID()<<std::endl;
        taskmap[ltask].precede((taskmap[child->ltask]));
        taskmap[child->ptask].precede((taskmap[ptask]));
      };
    } else {
      taskmap[ltask] = tf.emplace([&]() { preLayout(1); }).name(ltask);
      // Dummy just so that others can coordiante. Otherwise since prelayout is
      // set to 1. All processing will happen on single task.
      taskmap[ptask] = tf.emplace([&]() {}).name(ptask);
      taskmap[ltask].precede((taskmap[ptask]));
    }
  }
};

/**
 * @class ColumnBox
 * @brief Represents a box that lays out its children vertically in a
 * column.
 *
 * The ColumnBox class is a derived class of Box that arranges its child
 * boxes in a vertical column. The children are evenly spaced within the
 * column, and their heights can be adjusted based on their constraints.
 */
class ColumnBox : public Box {
public:
  std::vector<Box *> children;
  int availableHeight;

  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
    // Ensure that this box's constraints are within the constraints
    // provided by the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Decide on a width for this box within the constraints.
    width = maxWidth;

    availableHeight = maxHeight;

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
          child->preLayout(serial);
        }
      }
    }
    // postlayout after fixed children
    // Calculate width and set constraints for flexible children
    if (serial) {
      postLayout();
    }

    // Invoke prelayout on flex children
    if (serial) {
      for (const auto &child : children) {
        if (child->flex > 0.0)
          child->preLayout(serial);
      }
    }
  }

  void postLayout() override {
    // Calculate the width available for flexible child boxes
    for (const auto &child : children) {
      if (child->flex == 0.0) {
        availableHeight -= child->height;
      }
    }

    // Calculate the total flex value of the children
    double totalFlex = 0.0;
    for (const auto &child : children) {
      totalFlex += child->flex;
    }

    // Calculate the height available for flexible child boxes
    double flexChunkHeight = availableHeight / totalFlex;

    // flex children constraints
    for (const auto &child : children) {
      assert(availableHeight >= 0);
      if (child->flex > 0.0) {
        double flexHeight = flexChunkHeight * child->flex;
        availableHeight = availableHeight - flexHeight;
        child->setConstraints(width, width, flexHeight, flexHeight);
      }
    }
    // This box's height is the combined height of all children
    height = maxHeight - availableHeight;
  }

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
    float childY = 0;
    for (Box *child : children) {
      child->setPosition(0, childY);
      childY += child->height;
    }
  }

  json toJson() override {
    json j;
    std::stringstream str;
    str << "#" << std::hex << std::setfill('0') << std::setw(6) << id;
    j["id"] = str.str();
    j["type"] = "column";
    j["width"] = width;
    j["height"] = height;
    j["x"] = x;
    j["y"] = y;
    json children;
    for (const auto &child : this->children) {
      children.push_back(child->toJson());
    }
    j["children"] = children;
    return j;
  }

  void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                tf::Taskflow &tf) override {
    if (!flatten) {
      taskmap[ltask] = tf.emplace([&]() { preLayout(0); }).name(ltask);
      taskmap[ptask] = tf.emplace([&]() { postLayout(); }).name(ptask);
      for (auto &child : children) {
        child->getTasks(taskmap, tf);
        // std::cout<<"Preceding "<<child->setTaskID()<<" with
        // "<<setTaskID()<<std::endl;
        taskmap[ltask].precede((taskmap[child->ltask]));
        taskmap[child->ptask].precede((taskmap[ptask]));
      };
    } else {
      taskmap[ltask] = tf.emplace([&]() { preLayout(1); }).name(ltask);
      // Dummy just so that others can coordiante. Otherwise since prelayout is
      // set to 1. All processing will happen on single task.
      taskmap[ptask] = tf.emplace([&]() {}).name(ptask);
      taskmap[ltask].precede((taskmap[ptask]));
    }
  }
};
