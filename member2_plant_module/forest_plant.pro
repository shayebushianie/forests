QT += core gui widgets
CONFIG += c++17 console

INCLUDEPATH += include

SOURCES = \
    main.cpp \
    src/plant/AbstractPlant.cpp \
    src/plant/Tree.cpp \
    src/plant/Flower.cpp \
    src/plant/OakTree.cpp \
    src/plant/PineTree.cpp \
    src/plant/Rose.cpp \
    src/plant/Sunflower.cpp \
    src/plant/PlantFactory.cpp \
    src/DatabaseManager.cpp \
    src/controller/FocusController.cpp \
    src/logic/CoinManager.cpp \
    src/logic/AchievementEngine.cpp \
    src/logic/StatisticsCalculator.cpp

HEADERS = \
    include/plant/AbstractPlant.h \
    include/plant/Tree.h \
    include/plant/Flower.h \
    include/plant/OakTree.h \
    include/plant/PineTree.h \
    include/plant/Rose.h \
    include/plant/Sunflower.h \
    include/plant/PlantFactory.h \
    include/DatabaseCommon.h \
    include/DatabaseManager.h \
    include/controller/FocusController.h \
    include/logic/CoinManager.h \
    include/logic/AchievementEngine.h \
    include/logic/StatisticsCalculator.h
