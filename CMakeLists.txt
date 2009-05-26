
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(tspikes)

find_package(Boost 1.37.0 COMPONENTS thread unit_test_framework filesystem program_options system REQUIRED)
find_package(PkgConfig)

pkg_check_modules(GUIDEPS REQUIRED 
  sigc++-2.0>=2.0 
  glibmm-2.4>=2.13.0
  gtkmm-2.4>=2.10
  gtkglextmm-1.2>=1.2
  libglademm-2.4>=2.6)

pkg_check_modules(SOMADEPS REQUIRED
  somanetwork-1.0>=1.0
  somadspio-1.0>=1.0
  log4cpp>=1.0
)

ENABLE_TESTING()

include_directories(src)
add_subdirectory(src)