#include "taskflow/taskflow.hpp"
#include <algorithm>
#include <ext/json.hpp>
#include <ft2build.h>
#include <sstream>
#include <vector>
#include FT_FREETYPE_H
// Order matters here. stb_image.h must be included after
// STB_IMAGE_IMPLEMENTATION is defined
#define STB_IMAGE_IMPLEMENTATION
#include <ext/stb_image.h>

using json = nlohmann::json;

#define HEXSTR(x) std::hex << std::setfill('0') << std::setw(6) << x

// Convert integer to hex string with 6 digits and return in string
std::string hexstr(uint32_t x) {
  std::stringstream ss;
  ss << "#" << std::hex << std::setfill('0') << std::setw(6) << x;
  return ss.str();
}

/**
Split the text into words.
For each word:
  Measure its width.
  If adding this word to the current line doesn't exceed the container width,
add the word's width to the line width. else, consider this the start of a new
line, reset the line width to the current word's width, and increase the total
  height. After all words are processed, the maximum line width will be the
widest line, and the total height will account for all lines.
 */
typedef struct TextMetrics {
  float width;
  float height;
} TextMetrics;

TextMetrics measureText(const std::string &text, float containerWidth,
                        FT_Face face, float size, float spacing) {
  float currentLineWidth = 0;
  float maxWidth = 0;
  float height = size; // Starting with one line of text.

  std::stringstream ss(text);
  std::string word;

  while (ss >> word) {
    float wordWidth = 0;
    std::string partWord = "";
    FT_UInt previous = 0;

    for (char c : word) {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        continue;
      }
      if (FT_HAS_KERNING(face) && previous) {
        FT_Vector delta;
        FT_Get_Kerning(face, previous, FT_Get_Char_Index(face, c),
                       FT_KERNING_DEFAULT, &delta);
        wordWidth += delta.x >> 6;
      }
      wordWidth += face->glyph->advance.x >> 6;
      partWord += c;
      previous = FT_Get_Char_Index(face, c);

      if (currentLineWidth + wordWidth > containerWidth) {
        maxWidth = std::max(maxWidth, currentLineWidth); // Save the max width.
        height += size * spacing; // Add another line's height.

        // Start a new line with the part of the word that fits.
        currentLineWidth = 0;
        wordWidth = 0;
        partWord = "-";
      }
    }

    if (currentLineWidth + wordWidth <= containerWidth) {
      currentLineWidth += wordWidth;
    } else {
      maxWidth = std::max(maxWidth,
                          currentLineWidth); // Save the max width if this line
                                             // is wider than previous ones.
      height += size * spacing;              // Add another line's height.

      // Start a new line with the current word.
      currentLineWidth = wordWidth;
    }
  }

  maxWidth = std::max(maxWidth, currentLineWidth); // Handle the last line.

  return {maxWidth, height};
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
  bool isroot = false;
  bool flatten = false;
  bool debug = false; // Debug mode
  float x = 0, y = 0;
  float flutterWidth = 0, flutterHeight = 0, flutterX = 0, flutterY = 0;
  double flex = 0, fixed = 0;
  uint32_t id;
  std::string ltask, ptask;
  std::chrono::time_point<std::chrono::steady_clock> start, end;
  std::chrono::nanoseconds duration;

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

  virtual inline void gold() {
    width = flutterWidth;
    height = flutterHeight;
    x = flutterX;
    y = flutterY;
  }
  virtual inline uint8_t error() {
    uint8_t error_code;
    error_code = (width != flutterWidth) << 3 | (height != flutterHeight) << 2 |
                 (x != flutterX) << 1 | (y != flutterY);
  }
  inline void startTimer() {
    start = std::chrono::high_resolution_clock::now();
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::nanoseconds::zero();
  }
  inline void addTime() {
    duration = duration + std::chrono::duration_cast<std::chrono::nanoseconds>(
                              std::chrono::high_resolution_clock::now() - end);
    end = std::chrono::high_resolution_clock::now();
  }
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
    width = std::clamp(width, minWidth, maxWidth);
    height = std::clamp(height, minHeight, maxHeight);
  }

  void postLayout() override { /*  assert(0 && "Unimplemented function"); */
    if (debug) {
      gold();
      return;
    }
  }

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
    j["error"] = error();
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
 * @brief ImageBox represents a box with an image.
 */
class ImageBox : public Box {
public:
  ImageBox(const std::string &imagePath, uint32_t id) {
    char *fontFile = getenv("IMG_FOLDER");
    if (fontFile == NULL) {
      std::cerr << "Error: IMG_FOLDER environment variable not set"
                << std::endl;
      exit(1);
    }
    this->imagePath = std::string(fontFile) + "/" + imagePath;
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

    // Using stb_image to get image dimensions
    int x, y, n; // n is the number of channels, which we won't use here
    if (stbi_info(imagePath.c_str(), &x, &y, &n)) {
      width = x;
      height = y;
    } else {
      // Handle error, image couldn't be read
      // For simplicity, setting width and height to 0
      width = 0;
      height = 0;
    }
    // Clamp the width and height to the specified dimensions
    width = std::clamp(width, minWidth, maxWidth);
    height = std::clamp(height, minHeight, maxHeight);
  }

  void postLayout() override { /*  assert(0 && "Unimplemented function"); */
    if (debug) {
      gold();
      return;
    }
  }

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
    j["error"] = error();
    return j;
  }

  void getTasks(std::unordered_map<std::string, tf::Task> &taskmap,
                tf::Taskflow &tf) override {
    taskmap[ltask] = tf.emplace([&]() { preLayout(0); }).name(ltask);
    taskmap[ptask] = tf.emplace([&]() { postLayout(); }).name(ptask);
    taskmap[ltask].precede((taskmap[ptask]));
  }

private:
  std::string imagePath;
};

/**
 * @class TextBox
 * @brief Represents a box with text content.
 *
 * The TextBox class is a derived class of Box that represents a box with text.
 * It can be used to display text content within specific dimensions or
 * dynamically adjust based on the content.
 */
class TextBox : public Box {
public:
  TextBox(const std::string content, FT_Library &library, std::string ttf,
          float size, float spacing, uint32_t id) {
    this->content = content;
    char *fontFile = "/Users/ashriram/packages/src/taskflow/DOM-Parser/fonts";

    // if (fontFile == NULL) {
    //   std::cerr << "Error: FONT_FOLDER environment variable not set"
    //             << std::endl;
    //   exit(1);
    // }
    std::string fontPath = std::string(fontFile) + "/" + ttf;
    // Load a font face from a system font file on macOS
    if (FT_New_Face(library, fontPath.c_str(), 0, &(this->face))) {
      std::cerr << "Error loading font" << std::endl;
      exit(1);
    }
    this->size = size;
    this->spacing = spacing;
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
    startTimer();
    // Ensure that this box's constraints are within the constraints provided by
    // the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    FT_Set_Pixel_Sizes(face, 0, size);

    TextMetrics metrics = measureText(content, maxWidth, face, size, spacing);
    width = metrics.width;
    height = metrics.height;

    if (content == "") {
      // Clamp the width and height to the specified dimensions
      width = std::clamp(width, minWidth, maxWidth);
      height = std::clamp(height, minHeight, maxHeight);
    }
  addTime();
  }

  void postLayout() override { /*  assert(0 && "Unimplemented function"); */
    if (debug) {
      gold();
      return;
    }
  addTime();
  }

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
  }

  std::string getContent() const { return content; }

  json toJson() override {
    json j;
    std::stringstream str;
    str << "#" << std::hex << std::setfill('0') << std::setw(6) << id;
    j["id"] = str.str();
    j["type"] = "text";
    j["content"] = content;
    j["width"] = width;
    j["height"] = height;
    j["x"] = x;
    j["y"] = y;
    j["error"] = error();
    j["time"]  = duration.count();
    return j;
  }

  // ~TextBox() { FT_Done_Face(face); }

public:
  std::string content;
  FT_Face face;  // The font face for rendering
  float size;    // font size
  float spacing; // line spacing
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
  std::unique_ptr<Box> child = nullptr;
  int paddingLeft = 0, paddingRight = 0, paddingTop = 0, paddingBottom = 0;

  PaddingBox(std::unique_ptr<Box> child, int paddingLeft, int paddingRight, int paddingTop,
             int paddingBottom, uint32_t id) {
    this->child = std::move(child);
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
    float childMinWidth = minWidth - paddingLeft - paddingRight;
    float childMaxWidth = maxWidth - paddingLeft - paddingRight;
    float childMinHeight = minHeight - paddingTop - paddingBottom;
    float childMaxHeight = maxHeight - paddingTop - paddingBottom;

    // If there is a child, lay it out within the adjusted constraints
    if (child) {
      if (!debug)
        child->preLayout(serial);
      else
        child->gold();
    }
    postLayout();
  };

  void postLayout() override {
    if (debug) {
      gold();
      return;
    }
    if (child) {
      // Add the padding back to compute the size of this box
      width = child->width + paddingLeft + paddingRight;
      height = child->height + paddingTop + paddingBottom;
    } else {
      // If there is no child, the box's size is just the padding
      width = paddingLeft + paddingRight;
      height = paddingTop + paddingBottom;
      width = std::clamp(width, minWidth, maxWidth);
      height = std::clamp(height, minHeight, maxHeight);
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
    j["error"] = error();
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
 * @class StackChild
 * @brief Represents a child box within a StackBox with alignment properties.
 *
 * The StackChild class wraps a Box and provides additional properties for
 * alignment within a StackBox. It allows specifying horizontal and vertical
 * alignment values to position the child within the stack.
 */
class StackChild {
public:
  std::unique_ptr<Box> child;
  float horizontalAlignment;
  float verticalAlignment;

  StackChild(std::unique_ptr<Box> *child, float horizontalAlignment, float verticalAlignment)
      : horizontalAlignment(horizontalAlignment),
        verticalAlignment(verticalAlignment) { child = std::move(child);}
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
    for (StackChild &stackChild : children){
      if (debug) {
        stackChild.child->gold();
        continue;
      }
      stackChild.child->setConstraints(minWidth, maxWidth, minHeight, maxHeight);
      stackChild.child->preLayout(serial);
      width = std::max(width, stackChild.child->width +
                                  (int)std::abs(stackChild.horizontalAlignment *
                                                stackChild.child->width));
      height = std::max(height, stackChild.child->height +
                                    (int)std::abs(stackChild.verticalAlignment *
                                                  stackChild.child->height));
    }
  }

  void postLayout() override {
    if (debug) {
      gold();
      return;
    }
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
    j["error"] = error();
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
  std::unique_ptr<Box> child;
  int paddingLeft = 0, paddingRight = 0, paddingTop = 0, paddingBottom = 0;
  int marginLeft = 0, marginRight = 0, marginTop = 0, marginBottom = 0;

  ContainerBox(std::unique_ptr<Box> child, int paddingLeft, int paddingRight, int paddingTop,
               int paddingBottom, int marginLeft, int marginRight,
               int marginTop, int marginBottom, uint32_t id)
      : paddingLeft(paddingLeft), paddingRight(paddingRight),
        paddingTop(paddingTop), paddingBottom(paddingBottom),
        marginLeft(marginLeft), marginRight(marginRight), marginTop(marginTop),
        marginBottom(marginBottom) {
    this->id = id;
    this->child = std::move(child);
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
    startTimer();
    // Ensure that this box's constraints are within the constraints
    // provided by the parent
    minWidth = std::max(minWidth, parentMinWidth);
    maxWidth = std::min(maxWidth, parentMaxWidth);
    minHeight = std::max(minHeight, parentMinHeight);
    maxHeight = std::min(maxHeight, parentMaxHeight);

    // Calculate the constraints for the child, taking into account the
    // padding and margin
    float childMinWidth =
        minWidth - paddingLeft - paddingRight - marginLeft - marginRight;
    float childMaxWidth =
        maxWidth - paddingLeft - paddingRight - marginLeft - marginRight;
    float childMinHeight =
        minHeight - paddingTop - paddingBottom - marginTop - marginBottom;
    float childMaxHeight =
        maxHeight - paddingTop - paddingBottom - marginTop - marginBottom;

    // If there is a child, lay it out within the adjusted constraints
    if (child) {
      child->setConstraints(childMinWidth, childMaxWidth, childMinHeight,
                            childMaxHeight);
      // Add the padding and margin back to compute the size of this box
    }
    addTime();
    if (serial) {
      // Serial version
      if (child) {
        if (!debug)
          child->preLayout(serial);
        else
          child->gold();
      }
      postLayout();
    }
  }

  void postLayout() override {

    if (debug) {
      gold();
      return;
    }

    if (child) {
      width =
          child->width + paddingLeft + paddingRight + marginLeft + marginRight;
      height =
          child->height + paddingTop + paddingBottom + marginTop + marginBottom;
    } else {
      // If there is no child, the box's size is just the padding plus
      // margin
      width = paddingLeft + paddingRight + marginLeft + marginRight + minWidth;
      height =
          paddingTop + paddingBottom + marginTop + marginBottom + minHeight;
    }
    if (isroot) {
      width = parentMinWidth;
      height = parentMinHeight;
    }
    addTime();
  }

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
    if (child) {
      child->setPosition(0 + marginLeft + paddingLeft,
                         0 + marginTop + paddingTop);
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
    j["time"] = duration.count();
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
  std::vector<std::unique_ptr<Box>> children;
  float availableWidth = 0;

  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
    startTimer();
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

    addTime();

    if (serial) {
      // Serial version
      // Invoke prelayout on fixed children
      for (const auto &child : children) {
        if (child->flex == 0.0) {
          if (!debug)
            child->preLayout(serial);
          else
            child->gold();
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
        if (child->flex > 0.0) {
          if (!debug)
            child->preLayout(serial);
          else
            child->gold();
        }
      }
    }
  }

  void postLayout() override {
    if (debug) {
      gold();
      return;
    }

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
    addTime();
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
    j["error"] = error();
    j["time"] = duration.count();
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
  std::vector<std::unique_ptr<Box>> children;
  float availableHeight;

  void setConstraints(float parentMinWidth, float parentMaxWidth,
                      float parentMinHeight, float parentMaxHeight) override {
    this->parentMinWidth = parentMinWidth;
    this->parentMaxWidth = parentMaxWidth;
    this->parentMinHeight = parentMinHeight;
    this->parentMaxHeight = parentMaxHeight;
  }

  void preLayout(int serial) override {
    startTimer();
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
          if (!debug)
            child->preLayout(serial);
          else
            child->gold();
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
    // addTime();
  }

  void postLayout() override {
    if (debug) {
      gold();
      return;
    }
    // addTime();
    // startTimer();
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
    // addTime();
  }

  void setPosition(float x, float y) override {
    this->x = x;
    this->y = y;
    float childY = 0;
    for (const auto& child : children) {
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
    j["error"] = error();
    j["time"] = duration.count();
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
