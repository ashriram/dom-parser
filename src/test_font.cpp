#include "RenderObjects.h"

int main() {
  FT_Library library;
  FT_Face face;

  if (FT_Init_FreeType(&library)) {
    std::cerr << "Error initializing FreeType library" << std::endl;
    return 0;
  }

  // Get terminal variable for font file
  char *fontFile = getenv("FONT_FOLDER");
  if (fontFile == NULL) {
    std::cerr << "Error: FONT_FOLDER environment variable not set" << std::endl;
    return 0;
  }

  std::string fontPath = std::string(fontFile) + "/Helvetica-Bold.ttf";
  
  // Load a font face from a system font file on macOS
  if (FT_New_Face(library, fontPath.c_str(), 0, &face)) {
    std::cerr << "Error loading font" << std::endl;
    return 0;
  }

  // Set font size. This is just for the test; in a real-world scenario, you
  // would probably have varying font sizes.
  FT_Set_Pixel_Sizes(face, 0, 16); // 16 pixel font height

  // Test case 1: Simple word, no wrapping expected
  std::string testString1 = "Test";
  TextMetrics metrics1 = measureText(testString1, 300, face, 16,
                                     1.2f); // Wide container, no wrapping
  assert(metrics1.width > 0 &&
         metrics1.width <
             300); // The width should be positive but less than 300
  assert(metrics1.height == 16); // Only one line

  // Test case 2: Word too long for container
  std::string testString2 = "ThisIsALongWordWithoutSpaces";
  TextMetrics metrics2 =
      measureText(testString2, 50, face, 16, 1.2f); // Narrow container
  printf("%f\n", metrics2.width);
  assert(metrics2.width <=
         50); // Word should wrap and each line should be capped to 50 width
  assert(metrics2.height > 16); // Multiple lines expected

  // Test case 3: Normal sentence
  std::string testString3 = "This is a simple sentence for testing.";
  TextMetrics metrics3 = measureText(testString3, 100, face, 16, 1.2f);
  assert(metrics3.width <= 100);
  assert(metrics3.height > 16); // Given the container width and font size,
                                // expect wrapping to multiple lines

  FT_Done_Face(face);
  FT_Done_FreeType(library);

  std::cout << "All tests passed!" << std::endl;
  return 0;
}