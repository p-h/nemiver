


#pragma once


#define NEMIVER_SETUP_TIMEOUT(loop, sec) \
  bool timeout = false; \
Glib::signal_timeout ().connect_seconds_once ( \
  [&timeout] { \
    timeout = true; \
    loop->quit (); \
  }, sec)

#define NEMIVER_CHECK_NO_TIMEOUT \
  BOOST_REQUIRE (!timeout)
