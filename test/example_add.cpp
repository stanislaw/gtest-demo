#include "gtest/gtest.h"

#include <memory>

class QSignalSpy {
  int _count;

public:
  QSignalSpy() : _count(0) {}

  int count() {
    return _count;
  }

  void increaseCount() {
    _count++;
  }
};

namespace Enums {
  enum ModuleId {
    ONE = 1,
    ANOTHER = 2,
    FORTHY_TWO = 42
  };
}

class Module {
  Enums::ModuleId moduleId;
public:
  Module(Enums::ModuleId moduleId) : moduleId(moduleId) {}

  Enums::ModuleId getId() {
    return moduleId;
  }
  void reset(std::unique_ptr<Module> anotherModule) {
    moduleId = anotherModule->getId();
  }
};

class Engine {
  std::unique_ptr<Module> m_currentModule;
  QSignalSpy &idChangedSpy;
  QSignalSpy &moduleChangedSpy;

public:
  Engine(QSignalSpy &idChangedSpy, QSignalSpy &moduleChangedSpy) :
    m_currentModule(new Module(Enums::ModuleId::ONE)), idChangedSpy(idChangedSpy), moduleChangedSpy(moduleChangedSpy) {
  }

  Module * currentModule() {
    return m_currentModule.get();
  }

  Enums::ModuleId currentModuleId() {
    return m_currentModule->getId();
  }

  void setCurrentModuleId(Enums::ModuleId moduleId) {
//    if (Enums::ModuleId::FORTHY_TWO == moduleId) {
    if (currentModuleId() == moduleId) {
      return;
    }

    std::unique_ptr<Module> newModule(new Module(moduleId));
    m_currentModule->reset(std::move(newModule));
    currentModuleChanged(*m_currentModule);
    currentModuleIdChanged(moduleId);
  }

  void currentModuleChanged(Module &module) {
    moduleChangedSpy.increaseCount();
  }

  void currentModuleIdChanged(Enums::ModuleId) {
    idChangedSpy.increaseCount();
  }
};

class EngineTestSuite : public ::testing::Test {
protected:
  void SetUp() {
  }
};

TEST_F(EngineTestSuite, ChangingCurrentModuleTakesEffectAndNotifies)
{
  QSignalSpy idChangedSpy;
  QSignalSpy moduleChangedSpy;

  Engine m_engine(idChangedSpy, moduleChangedSpy);
  m_engine.setCurrentModuleId(Enums::ANOTHER);

  EXPECT_EQ(m_engine.currentModuleId(), Enums::ANOTHER);
  EXPECT_NE(m_engine.currentModule(), nullptr);
  ASSERT_EQ(idChangedSpy.count(), 1);
  // EXPECT_EQ(idChangedSpy.value(0).value(0).value<Enums::ModuleId>(), Enums::DisplayModuleId);

  ASSERT_EQ(moduleChangedSpy.count(), 1);
  // EXPECT_NE(moduleChangedSpy.value(0).value(0).value<Module*>(), nullptr);
}

TEST_F(EngineTestSuite, SettingCurrentModuleTwiceInARawDoesNothing)
{
  QSignalSpy idChangedSpy;
  QSignalSpy moduleChangedSpy;

  Engine m_engine(idChangedSpy, moduleChangedSpy);
  m_engine.setCurrentModuleId(Enums::ModuleId::ANOTHER);

  const Module* module = m_engine.currentModule();

  m_engine.setCurrentModuleId(Enums::ModuleId::ANOTHER);

  EXPECT_EQ(m_engine.currentModuleId(), Enums::ModuleId::ANOTHER);
  EXPECT_EQ(idChangedSpy.count(), 1);
  EXPECT_EQ(m_engine.currentModule(), module);
  EXPECT_EQ(moduleChangedSpy.count(), 1);
}