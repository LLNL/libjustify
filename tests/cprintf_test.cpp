#include <cprintf.h>
#include <cstdio>
#include <gtest/gtest.h>

static char *Writefilename = "output.txt";

class OutputTest : public ::testing::Test {
protected:
  char buffer[1024];
  FILE* original_stdout;

  virtual void SetUp() {
    original_stdout = stdout;  // Save the original stdout
    stdout = freopen(Writefilename, "w", stdout);  // Redirect stdout
  }

  virtual void TearDown() {
    freopen(Writefilename, "r", stdout);  // Prepare to read the redirected output
    fgets(buffer, 1024, stdout);  // Read the output
    stdout = original_stdout;  // Reset stdout to its original state
  }

  // Helper function for retrieving output
  const char* GetOutput() {
    return buffer;
  }
};

class Single_Line_Single_String_NoTab : public OutputTest {};


TEST_F(Single_Line_Single_String_NoTab, Test1) {

  SetUp();
  cprintf("Hello, %s!\n", "world");
  cflush();
  const char* cprintf_output = GetOutput();

  printf("Hello, %s!\n", "world");
  const char* printf_output = GetOutput();
  TearDown();
  ASSERT_STREQ(cprintf_output, printf_output);
}

int main(int argc, char **argv) {
    cprintf("Writing output to %s", Writefilename);
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
}