#pragma once

#include "gtest/gtest.h"

namespace yacl {
namespace test {

class tests_yacl : public ::testing::Test {
 protected:
  virtual void SetUp();
};

}
}
