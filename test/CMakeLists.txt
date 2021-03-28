#
# MIT License
# Copyright (c) Mariusz Łapiński <gmail:isameru>
#
#  ██████╗  ██████╗  ██████╗ ██╗  ██╗███╗   ███╗ ██████╗ ██╗     ███████╗
#  ██╔══██╗██╔═══██╗██╔═══██╗██║ ██╔╝████╗ ████║██╔═══██╗██║     ██╔════╝
#  ██████╔╝██║   ██║██║   ██║█████╔╝ ██╔████╔██║██║   ██║██║     █████╗
#  ██╔══██╗██║   ██║██║   ██║██╔═██╗ ██║╚██╔╝██║██║   ██║██║     ██╔══╝
#  ██║  ██║╚██████╔╝╚██████╔╝██║  ██╗██║ ╚═╝ ██║╚██████╔╝███████╗███████╗
#  ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚══════╝
#

# rookmole.test
add_executable(rookmole.test rookmole.test.cpp)
target_compile_features(rookmole.test PUBLIC cxx_std_17)
set_target_properties(rookmole.test PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(rookmole.test rookmole)
add_test(NAME rookmole.test COMMAND rookmole.test)

# play.rookmole
add_executable(play.rookmole play.rookmole.cpp)
target_compile_features(play.rookmole PUBLIC cxx_std_17)
set_target_properties(play.rookmole PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(play.rookmole rookmole)
